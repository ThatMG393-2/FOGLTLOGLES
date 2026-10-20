#include "es/state_tracking.hpp"
#include "shader/cache.hpp"
#include "shader/converter.hpp"
#include "shader/utils.hpp"
#include "gles20/main.hpp"
#include "gles20/shader_overrides.hpp"
#include "main.hpp"
#include "utils/env.hpp"
#include "utils/fast_map.hpp"

#include <GLES3/gl32.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace Cache = ShaderConverter::Cache;

namespace {
    struct ShaderState {
        size_t sourceKey = 0;    // original source + shader type + converter config
        size_t convertedKey = 0; // non-zero when the driver source came from the converter
    };

    struct ProgramState {
        std::vector<std::pair<GLuint, size_t>> shaders; // id, source key at attach time
        std::vector<std::pair<GLuint, std::string>> attribs;
        std::vector<std::string> feedbackVaryings;
        GLenum feedbackMode = 0;
    };

    FastMapBI<GLuint, ShaderState> shaderStates;
    FastMapBI<GLuint, ProgramState> programStates;

    size_t makeSourceKey(GLenum type, std::string_view source) {
        uint64_t h = Cache::hashBytes(source.data(), source.size());
        h = Cache::hashCombine(h, type);
        h = Cache::hashCombine(h, ESUtils::shadingVersion);
        h = Cache::hashCombine(h, Cache::CONVERTER_REVISION);
        return h ? h : 1;
    }

    uint64_t driverHash() {
        static const uint64_t hash = [] {
            uint64_t h = Cache::CONVERTER_REVISION;
            for (GLenum name : { GL_VENDOR, GL_RENDERER, GL_VERSION }) {
                const auto* value = reinterpret_cast<const char*>(glGetString(name));
                if (value) h = Cache::hashBytes(value, std::strlen(value), h);
            }
            return h;
        }();
        return hash;
    }

    bool binaryCacheUsable() {
        static const bool usable = [] {
            GLint formats = 0;
            glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
            return ENABLE_BINARY_CACHE && formats > 0;
        }();
        return usable;
    }

    bool computeProgramKey(const ProgramState& state, size_t& out) {
        if (state.shaders.empty()) return false;

        std::vector<size_t> sources;
        for (const auto& [id, key] : state.shaders) {
            if (!key) return false;
            sources.push_back(key);
        }
        std::sort(sources.begin(), sources.end());

        auto attribs = state.attribs;
        std::sort(attribs.begin(), attribs.end());

        uint64_t h = driverHash();
        for (size_t key : sources) h = Cache::hashCombine(h, key);
        for (const auto& [index, name] : attribs) {
            h = Cache::hashCombine(h, index);
            h = Cache::hashBytes(name.data(), name.size(), h);
        }
        for (const auto& name : state.feedbackVaryings) h = Cache::hashBytes(name.data(), name.size(), h);
        h = Cache::hashCombine(h, state.feedbackMode);

        out = h ? h : 1;
        return true;
    }

    bool loadProgramBinary(GLuint program, size_t key) {
        std::string blob;
        if (!Cache::Binary::get(key, blob) || blob.size() <= sizeof(GLenum)) return false;

        GLenum format;
        std::memcpy(&format, blob.data(), sizeof(format));

        glProgramBinary(program, format, blob.data() + sizeof(format), static_cast<GLsizei>(blob.size() - sizeof(format)));

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        for (int i = 0; i < 8 && glGetError() != GL_NO_ERROR; i++) {}

        if (linked == GL_TRUE) return true;

        Cache::Binary::invalidate(key);
        return false;
    }

    void saveProgramBinary(GLuint program, size_t key) {
        GLint length = 0;
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
        if (length <= 0) return;

        std::string blob(sizeof(GLenum) + length, '\0');
        GLsizei written = 0;
        GLenum format = 0;
        glGetProgramBinary(program, length, &written, &format, blob.data() + sizeof(format));
        if (written <= 0) return;

        std::memcpy(blob.data(), &format, sizeof(format));
        blob.resize(sizeof(format) + written);
        Cache::Binary::put(key, blob.data(), blob.size());
    }

    std::string getInfoLog(GLuint object, bool program) {
        GLint length = 0;
        if (program) glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
        else glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
        if (length <= 1) return {};

        std::string log(length, '\0');
        if (program) glGetProgramInfoLog(object, length, nullptr, log.data());
        else glGetShaderInfoLog(object, length, nullptr, log.data());
        return log;
    }
}

void GLES20::registerShaderOverrides() {
    REGISTEROV(glShaderSource);
    REGISTEROV(glCompileShader);
    REGISTEROV(glDeleteShader);
    REGISTEROV(glAttachShader);
    REGISTEROV(glDetachShader);
    REGISTEROV(glBindAttribLocation);
    REGISTEROV(glTransformFeedbackVaryings);
    REGISTEROV(glLinkProgram);
    REGISTEROV(glUseProgram);
    REGISTEROV(glDeleteProgram);
}

void OV_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
    std::string source;
    combineSources(count, string, length, source);

    if (source.empty()) {
        LOGW("glShaderSource was called without a shader source? Skipping...");
        return;
    }

    int version = 0;
    std::string_view profile;
    if (!getShaderVersion(source, version, profile)) {
        throw std::runtime_error("Shader with no version preprocessor!");
    }

    GLint type = 0;
    glGetShaderiv(shader, GL_SHADER_TYPE, &type);

    ShaderState& state = shaderStates[shader];
    state = {};
    state.sourceKey = makeSourceKey(type, source);

    if (profile == "es") {
        const GLchar* passthrough = source.c_str();
        glShaderSource(shader, 1, &passthrough, nullptr);
        return;
    }

    std::string converted;
    std::vector<Cache::UBOBinding> ubos;

    bool hit = Cache::getConverted(state.sourceKey, converted, ubos)
        && ShaderConverter::SPVCPostprocessor::registerBindings(ubos);

    if (hit) {
        if (SHADER_DUMP) LOGI("Cache hit! Shader %u (%zu)", shader, state.sourceKey);
    } else {
        converted = std::move(source);
        ubos.clear();

        ShaderConverter::convertAndFix(getKindFromShader(shader), converted, &ubos);
        Cache::putConverted(state.sourceKey, converted, ubos);
    }

    state.convertedKey = state.sourceKey;

    const GLchar* newSource = converted.c_str();
    glShaderSource(shader, 1, &newSource, nullptr);
}

void OV_glCompileShader(GLuint shader) {
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) return;

    LOGE("Compile error for shader %u:", shader);
    LOGE("%s", getInfoLog(shader, false).c_str());

    GLint sourceLength = 0;
    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &sourceLength);
    if (sourceLength > 0) {
        std::string source(sourceLength, '\0');
        glGetShaderSource(shader, sourceLength, nullptr, source.data());

        LOGE("Shader %u source:", shader);
        LOGE("%s", source.c_str());
    }

    if (auto it = shaderStates.find(shader); it != shaderStates.end() && it->second.convertedKey) {
        if (Cache::invalidateShaderCache(it->second.convertedKey)) LOGI("Shader invalidated as it failed to compile.");
    }

    throw std::runtime_error("Failed to compile shader!");
}

void OV_glDeleteShader(GLuint shader) {
    glDeleteShader(shader);
    shaderStates.erase(shader);
}

void OV_glAttachShader(GLuint program, GLuint shader) {
    glAttachShader(program, shader);

    auto it = shaderStates.find(shader);
    programStates[program].shaders.emplace_back(shader, it != shaderStates.end() ? it->second.sourceKey : 0);
}

void OV_glDetachShader(GLuint program, GLuint shader) {
    glDetachShader(program, shader);

    if (auto it = programStates.find(program); it != programStates.end()) {
        auto& shaders = it->second.shaders;
        shaders.erase(std::remove_if(shaders.begin(), shaders.end(), [&](const auto& s) { return s.first == shader; }), shaders.end());
    }
}

void OV_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name) {
    glBindAttribLocation(program, index, name);

    auto& attribs = programStates[program].attribs;
    auto it = std::find_if(attribs.begin(), attribs.end(), [&](const auto& a) { return a.second == name; });
    if (it != attribs.end()) it->first = index;
    else attribs.emplace_back(index, name);
}

void OV_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) {
    glTransformFeedbackVaryings(program, count, varyings, bufferMode);

    ProgramState& state = programStates[program];
    state.feedbackMode = bufferMode;
    state.feedbackVaryings.assign(varyings, varyings + count);
}

void OV_glLinkProgram(GLuint program) {
    size_t key = 0;

    if (binaryCacheUsable()) {
        auto it = programStates.find(program);
        if (it != programStates.end() && computeProgramKey(it->second, key)) {
            if (loadProgramBinary(program, key)) return;
            glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
        } else {
            key = 0;
        }
    }

    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        LOGE("Link error for program %u:", program);
        LOGE("%s", getInfoLog(program, true).c_str());

        throw std::runtime_error("Failed to link program!");
    }

    if (key) saveProgramBinary(program, key);
}

void OV_glUseProgram(GLuint program) {
    glUseProgram(program);

    trackedStates->currentlyUsedProgram = program;
}

void OV_glDeleteProgram(GLuint program) {
    glDeleteProgram(program);
    programStates.erase(program);

    if (trackedStates->currentlyUsedProgram == program) {
        trackedStates->currentlyUsedProgram = 0;
    }
}
