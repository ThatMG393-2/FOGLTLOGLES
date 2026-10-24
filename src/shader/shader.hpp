#pragma once

#include "common/env.hpp"
#include "common/es_utils.hpp"
#include "common/log.hpp"
#include "common/types.hpp"
#include "shaderc/env.h"
#include "spirv_glsl.hpp"
#include "shaderc/shaderc.hpp"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include "shaderc/shaderc.h"
#include "shaderc/status.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include <shaderc/shaderc.hpp>
#include <stdexcept>
#include <GLES3/gl31.h>
#include <memory>

namespace ShaderConverter::Cache {
    // Bump whenever converter output changes for the same input.
    inline constexpr uint32_t CONVERTER_REVISION = 2;

    inline uint64_t hashBytes(const void* data, size_t size, uint64_t seed = 0) {
        constexpr uint64_t k1 = 0x9E3779B97F4A7C15ull;
        constexpr uint64_t k2 = 0xC2B2AE3D27D4EB4Full;

        const auto* p = static_cast<const uint8_t*>(data);
        uint64_t h = seed ^ (size * k1);

        while (size >= 8) {
            uint64_t v;
            std::memcpy(&v, p, 8);
            v *= k2;
            v ^= v >> 29;
            h = (h ^ v) * k1;
            h ^= h >> 32;
            p += 8;
            size -= 8;
        }

        uint64_t tail = 0;
        std::memcpy(&tail, p, size);
        h = (h ^ (tail * k2)) * k1;
        h ^= h >> 31;
        h *= k2;
        h ^= h >> 29;

        return h;
    }

    inline uint64_t hashCombine(uint64_t seed, uint64_t value) {
        return hashBytes(&value, sizeof(value), seed);
    }

    template<typename T>
    inline size_t getHash(const T& key) {
        if (!ENABLE_SHADER_CACHE) return 0;

        if constexpr (std::is_convertible_v<const T&, std::string_view>) {
            std::string_view view = key;
            return hashBytes(view.data(), view.size());
        } else {
            static_assert(std::is_trivially_copyable_v<T>);
            return hashBytes(&key, sizeof(T));
        }
    }

    class Store {
    public:
        explicit Store(const char* name) : name(name) {}

        void init(const std::string& root, time_t retentionSeconds) {
            std::lock_guard lock(mutex);

            directory = root + "/" + name;
            keys.clear();

            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec) {
                LOGW("Failed to create cache directory '%s': %s", directory.c_str(), ec.message().c_str());
                directory.clear();
                return;
            }

            const time_t now = time(nullptr);
            size_t removed = 0;

            for (std::filesystem::directory_iterator it(directory, ec), end; !ec && it != end; it.increment(ec)) {
                const std::string path = it->path().string();
                const std::string file = it->path().filename().string();

                struct stat st;
                bool stale = stat(path.c_str(), &st) != 0
                    || file.size() != 16
                    || now - std::max(st.st_atime, st.st_mtime) > retentionSeconds;

                char* parseEnd = nullptr;
                const uint64_t key = stale ? 0 : std::strtoull(file.c_str(), &parseEnd, 16);
                if (!stale && (parseEnd == nullptr || *parseEnd != '\0')) stale = true;

                if (stale) {
                    std::filesystem::remove(it->path(), ec);
                    removed++;
                    ec.clear();
                    continue;
                }

                keys.emplace(key, false);
            }

            LOGI("Cache '%s': %zu entries, %zu removed", name, keys.size(), removed);
        }

        bool contains(uint64_t key) const {
            std::lock_guard lock(mutex);
            return !directory.empty() && keys.find(key) != keys.end();
        }

        bool read(uint64_t key, std::string& out) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                auto it = keys.find(key);
                if (directory.empty() || it == keys.end()) return false;

                path = pathFor(key);
                if (!it->second) {
                    it->second = true;
                    utimes(path.c_str(), nullptr);
                }
            }

            FILE* file = std::fopen(path.c_str(), "rb");
            if (!file) {
                erase(key);
                return false;
            }

            Header header;
            bool ok = std::fread(&header, sizeof(header), 1, file) == 1
                && header.magic == MAGIC
                && header.size < (1ull << 30);

            if (ok) {
                out.resize(header.size);
                ok = std::fread(out.data(), 1, header.size, file) == header.size
                    && hashBytes(out.data(), out.size()) == header.checksum;
            }

            std::fclose(file);

            if (!ok) {
                erase(key);
                out.clear();
            }

            return ok;
        }

        bool write(uint64_t key, const void* data, size_t size) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                if (directory.empty()) return false;
                path = pathFor(key);
            }

            const std::string tmp = path + "." + std::to_string(getpid()) + ".tmp";
            FILE* file = std::fopen(tmp.c_str(), "wb");
            if (!file) return false;

            const Header header { MAGIC, 0, size, hashBytes(data, size) };
            bool ok = std::fwrite(&header, sizeof(header), 1, file) == 1
                && (size == 0 || std::fwrite(data, 1, size, file) == size);
            ok = (std::fclose(file) == 0) && ok;

            if (!ok || std::rename(tmp.c_str(), path.c_str()) != 0) {
                std::remove(tmp.c_str());
                return false;
            }

            std::lock_guard lock(mutex);
            keys[key] = true;
            return true;
        }

        bool erase(uint64_t key) {
            std::string path;
            {
                std::lock_guard lock(mutex);
                if (keys.erase(key) == 0) return false;
                path = pathFor(key);
            }

            return std::remove(path.c_str()) == 0;
        }

    private:
        static constexpr uint32_t MAGIC = 0x4C474F46; // "FOGL"

        struct Header {
            uint32_t magic;
            uint32_t reserved;
            uint64_t size;
            uint64_t checksum;
        };

        std::string pathFor(uint64_t key) const {
            char name[17];
            std::snprintf(name, sizeof(name), "%016llx", static_cast<unsigned long long>(key));
            return directory + "/" + name;
        }

        const char* name;
        std::string directory;
        std::unordered_map<uint64_t, bool> keys; // value: mtime refreshed this session
        mutable std::mutex mutex;
    };

    inline Store convertedStore("converted");
    inline Store binaryStore("binaries");

    using UBOBinding = std::pair<std::string, uint32_t>;

    inline bool isShaderInCache(size_t key) {
        return ENABLE_SHADER_CACHE && key && convertedStore.contains(key);
    }

    inline void putShaderInCache(size_t key, const std::string& source) {
        if (!ENABLE_SHADER_CACHE || !key) return;
        convertedStore.write(key, source.data(), source.size());
    }

    inline std::string getCachedShaderSource(size_t key) {
        std::string source;
        if (ENABLE_SHADER_CACHE && key) convertedStore.read(key, source);
        return source;
    }

    inline bool invalidateShaderCache(size_t key) {
        return ENABLE_SHADER_CACHE && key && convertedStore.erase(key);
    }

    // Converted GLSL entries also carry the UBO bindings the converter assigned,
    // so a cache hit can restore the name -> binding table.
    inline void putConverted(size_t key, const std::string& source, const std::vector<UBOBinding>& ubos) {
        if (!ENABLE_SHADER_CACHE || !key) return;

        std::string blob;
        auto append32 = [&](uint32_t v) { blob.append(reinterpret_cast<const char*>(&v), sizeof(v)); };

        append32(static_cast<uint32_t>(ubos.size()));
        for (const auto& [name, binding] : ubos) {
            append32(binding);
            append32(static_cast<uint32_t>(name.size()));
            blob += name;
        }
        blob += source;

        convertedStore.write(key, blob.data(), blob.size());
    }

    inline bool getConverted(size_t key, std::string& source, std::vector<UBOBinding>& ubos) {
        std::string blob;
        if (!ENABLE_SHADER_CACHE || !key || !convertedStore.read(key, blob)) return false;

        size_t pos = 0;
        auto read32 = [&](uint32_t& v) {
            if (blob.size() - pos < sizeof(v)) return false;
            std::memcpy(&v, blob.data() + pos, sizeof(v));
            pos += sizeof(v);
            return true;
        };

        uint32_t count = 0;
        bool ok = read32(count) && count < 4096;
        ubos.clear();

        for (uint32_t i = 0; ok && i < count; i++) {
            uint32_t binding = 0, len = 0;
            ok = read32(binding) && read32(len) && blob.size() - pos >= len;
            if (!ok) break;

            ubos.emplace_back(blob.substr(pos, len), binding);
            pos += len;
        }

        if (!ok) {
            convertedStore.erase(key);
            return false;
        }

        source.assign(blob, pos, std::string::npos);
        return !source.empty();
    }

    namespace Binary {
        inline bool get(size_t key, std::string& out) {
            return ENABLE_BINARY_CACHE && binaryStore.read(key, out);
        }

        inline void put(size_t key, const void* data, size_t size) {
            if (ENABLE_BINARY_CACHE) binaryStore.write(key, data, size);
        }

        inline void invalidate(size_t key) {
            if (ENABLE_BINARY_CACHE) binaryStore.erase(key);
        }
    }

    inline void init() {
        if (!ENABLE_SHADER_CACHE || SHADER_CACHE_DIR.empty()) {
            LOGI("Shader cache is disabled!");
            ENABLE_SHADER_CACHE = false;
            ENABLE_BINARY_CACHE = false;
            return;
        }

        const time_t retention = static_cast<time_t>(SHADER_RETENTION_DAYS) * 24 * 60 * 60;

        convertedStore.init(SHADER_CACHE_DIR, retention);
        if (ENABLE_BINARY_CACHE) binaryStore.init(SHADER_CACHE_DIR, retention);
    }
}

inline shaderc::CompileOptions generateGLSL2SPVOptions(int glslVersion) {
    shaderc::CompileOptions options;
    options.SetGenerateDebugInfo();
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_opengl, glslVersion);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetVulkanRulesRelaxed(true);

    options.SetAutoMapLocations(true);
    options.SetAutoBindUniforms(true);

    return options;
}

inline spirv_cross::CompilerGLSL::Options generateSPV2ESSLOptions(int esslVersion) {
    spirv_cross::CompilerGLSL::Options options;
    options.version = esslVersion;
    options.es = true;
    options.vulkan_semantics = false;
    options.enable_420pack_extension = false;
    options.force_flattened_io_blocks = true;
    options.enable_storage_image_qualifier_deduction = false;

    return options;
}

class SPVCExposed_CompilerGLSL : public spirv_cross::CompilerGLSL {
    public:
        using spirv_cross::CompilerGLSL::CompilerGLSL;
        using spirv_cross::CompilerGLSL::get;
};

namespace ShaderConverter::SPVCPostprocessor {
    enum {
        rLocation = 1 << 1,
        rBinding = 1 << 2,
        rDescSet = 1 << 3,
        rDefault = rLocation | rBinding | rDescSet
    };

    inline void removeLocationBindingAndDescriptorSets(
        SPVCExposed_CompilerGLSL& compiler,
        const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
        int flags = rDefault
    ) {
        for (const auto& resource : resources) {
            if (flags & rLocation) compiler.unset_decoration(resource.id, spv::DecorationLocation);
            if (flags & rBinding) compiler.unset_decoration(resource.id, spv::DecorationBinding);
            if (flags & rDescSet) compiler.unset_decoration(resource.id, spv::DecorationDescriptorSet);
        }
    }

    inline std::unordered_map<std::string, uint32_t> uniformBuffersBindingIndex;
    inline uint32_t currentBindingIndex = 0;

    // Bindings assigned or reused by the shader currently being converted.
    inline std::vector<std::pair<std::string, uint32_t>>* bindingSink = nullptr;

    inline void assignUniformBufferBindings(
        SPVCExposed_CompilerGLSL& compiler,
        const spirv_cross::SmallVector<spirv_cross::Resource>& resources
    ) {
        for (const auto& resource : resources) {
            std::string name = compiler.get_name(compiler.get_type(resource.base_type_id).self);

            auto it = uniformBuffersBindingIndex.find(name);
            if (it == uniformBuffersBindingIndex.end()) {
                it = uniformBuffersBindingIndex.emplace(name, currentBindingIndex++).first;
            }

            compiler.set_decoration(resource.id, spv::DecorationBinding, it->second);
            if (bindingSink) bindingSink->emplace_back(std::move(name), it->second);
        }
    }

    // Restores bindings baked into a cached shader. Fails without side effects on conflict.
    inline bool registerBindings(const std::vector<std::pair<std::string, uint32_t>>& bindings) {
        for (const auto& [name, binding] : bindings) {
            auto it = uniformBuffersBindingIndex.find(name);
            if (it != uniformBuffersBindingIndex.end()) {
                if (it->second != binding) return false;
                continue;
            }

            for (const auto& [otherName, otherBinding] : uniformBuffersBindingIndex) {
                if (otherBinding == binding) return false;
            }
        }

        for (const auto& [name, binding] : bindings) {
            uniformBuffersBindingIndex.emplace(name, binding);
            if (binding >= currentBindingIndex) currentBindingIndex = binding + 1;
        }

        return true;
    }

    inline void processSPVBytecode(SPVCExposed_CompilerGLSL& compiler, shaderc_shader_kind kind) {
        if (kind == shaderc_glsl_compute_shader) {
            LOGI("Compute shader processing is unimplemented right now...");
            return;
        }

        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        removeLocationBindingAndDescriptorSets(compiler, resources.separate_images);
        removeLocationBindingAndDescriptorSets(compiler, resources.sampled_images);
        removeLocationBindingAndDescriptorSets(compiler, resources.separate_samplers);

        removeLocationBindingAndDescriptorSets(compiler, resources.uniform_buffers);
        assignUniformBufferBindings(compiler, resources.uniform_buffers);

        removeLocationBindingAndDescriptorSets(compiler, resources.gl_plain_uniforms);

        removeLocationBindingAndDescriptorSets(compiler, resources.stage_inputs);

        int flags = rDefault;
        if (kind == shaderc_fragment_shader
         && resources.stage_outputs.size() > 1) flags = rDescSet | rBinding;
        removeLocationBindingAndDescriptorSets(compiler, resources.stage_outputs, flags);
    }
}; // namespace ShaderConverter::SPVCPostprocessor

namespace ShaderConverter::GLSLPreprocessor {
    namespace detail {
        inline bool isWord(char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }

        inline bool isBlank(char c) {
            return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
        }

        inline size_t skipBlank(std::string_view s, size_t i) {
            while (i < s.size() && isBlank(s[i])) i++;
            return i;
        }

        inline size_t wordEnd(std::string_view s, size_t i) {
            while (i < s.size() && isWord(s[i])) i++;
            return i;
        }

        template<typename Fn>
        inline void forEachLine(std::string_view s, Fn&& fn) {
            size_t pos = 0;
            while (pos < s.size()) {
                size_t nl = s.find('\n', pos);
                size_t end = nl == std::string_view::npos ? s.size() : nl;
                fn(s.substr(pos, end - pos), nl != std::string_view::npos);
                pos = end + 1;
            }
        }

        struct UniformInit {
            size_t begin, nameEnd, end;
            std::string_view name, init;
        };

        // uniform mat4 <name> = <init>;
        inline bool matchUniformInit(std::string_view line, UniformInit& m) {
            for (size_t p = line.find("uniform"); p != std::string_view::npos; p = line.find("uniform", p + 1)) {
                if (p > 0 && isWord(line[p - 1])) continue;

                size_t i = skipBlank(line, p + 7);
                if (i == p + 7 || line.compare(i, 4, "mat4") != 0) continue;

                size_t j = skipBlank(line, i + 4);
                if (j == i + 4) continue;

                size_t nameEnd = wordEnd(line, j);
                if (nameEnd == j) continue;

                size_t k = skipBlank(line, nameEnd);
                if (k >= line.size() || line[k] != '=') continue;

                size_t initBegin = skipBlank(line, k + 1);
                size_t semi = line.find(';', initBegin);
                if (semi == std::string_view::npos || semi == initBegin) continue;

                m = { p, nameEnd, semi + 1, line.substr(j, nameEnd - j), line.substr(initBegin, semi - initBegin) };
                return true;
            }

            return false;
        }

        template<typename Map>
        inline void replaceWords(std::string_view line, const Map& map, std::string& out) {
            size_t i = 0;
            while (i < line.size()) {
                if (!isWord(line[i])) {
                    out += line[i++];
                    continue;
                }

                size_t end = wordEnd(line, i);
                std::string_view word = line.substr(i, end - i);
                auto it = map.find(word);

                if (it != map.end()) out += it->second;
                else out += word;

                i = end;
            }
        }
    }

    // Initializers on uniforms are illegal in the target; hoist them into locals in main().
    inline void moveVariableInitialization(std::string& source) {
        using namespace detail;

        if (source.find("mat4") == std::string::npos) return;

        std::vector<std::pair<std::string, std::string>> uniforms;
        std::string stripped;
        stripped.reserve(source.size());

        forEachLine(source, [&](std::string_view line, bool hasNewline) {
            UniformInit m;
            if (matchUniformInit(line, m)) {
                uniforms.emplace_back(m.name, m.init);
                stripped.append(line.substr(0, m.nameEnd));
                stripped += ';';
                stripped.append(line.substr(m.end));
            } else {
                stripped.append(line);
            }

            if (hasNewline) stripped += '\n';
        });

        if (uniforms.empty()) return;

        std::unordered_map<std::string_view, std::string> replacements;
        for (size_t i = 0; i < uniforms.size(); i++) {
            replacements[uniforms[i].first] = "customMat4Var" + std::to_string(i + 1) + "Fix";
        }

        std::string output;
        output.reserve(stripped.size() + uniforms.size() * 64);

        bool inMain = false, inserted = false;
        forEachLine(stripped, [&](std::string_view line, bool hasNewline) {
            if (!inMain && line.find("void main()") != std::string_view::npos) inMain = true;

            if (inMain && !inserted && line.find('{') != std::string_view::npos) {
                output.append(line);
                output += '\n';

                for (const auto& [name, init] : uniforms) {
                    output += "mat4 ";
                    output += replacements[name];
                    output += " = ";
                    output += init;
                    output += ";\n";
                }

                inserted = true;
                return;
            }

            if (inserted) replaceWords(line, replacements, output);
            else output.append(line);

            if (hasNewline) output += '\n';
        });

        source = std::move(output);
    }

    inline void removeLineDirectives(std::string& source) {
        using namespace detail;

        if (source.find("#line") == std::string::npos) return;

        std::string output;
        output.reserve(source.size());

        forEachLine(source, [&](std::string_view line, bool hasNewline) {
            size_t i = skipBlank(line, 0);
            bool directive = line.compare(i, 5, "#line") == 0 && (i + 5 >= line.size() || !isWord(line[i + 5]));

            if (!directive) output.append(line);
            if (hasNewline) output += '\n';
        });

        source = std::move(output);
    }

    inline void fixDeprecatedTextureFunction(std::string& source) {
        using namespace detail;

        if (source.find("texture2D") == std::string::npos) return;

        std::string output;
        output.reserve(source.size());

        size_t i = 0;
        while (i < source.size()) {
            if (!isWord(source[i])) {
                output += source[i++];
                continue;
            }

            size_t end = wordEnd(source, i);
            std::string_view word(source.data() + i, end - i);

            size_t paren = skipBlank(source, end);
            if (word == "texture2D" && paren < source.size() && source[paren] == '(') {
                output += "texture";
                i = paren;
            } else {
                output.append(word);
                i = end;
            }
        }

        source = std::move(output);
    }

    inline void run(std::string& source, int version) {
        moveVariableInitialization(source);
        removeLineDirectives(source);

        if (version <= 120) fixDeprecatedTextureFunction(source);
    }
}

inline shaderc::Compiler& getShaderCompiler() {
    static thread_local shaderc::Compiler compiler;
    return compiler;
}

inline void upgradeTo330(shaderc_shader_kind kind, std::string& src) {
    static const shaderc::CompileOptions options = [] {
        shaderc::CompileOptions o;
        o.SetGenerateDebugInfo();
        o.SetSourceLanguage(shaderc_source_language_glsl);
        o.SetTargetEnvironment(shaderc_target_env_opengl, 330);
        o.SetForcedVersionProfile(330, shaderc_profile_core);
        o.SetOptimizationLevel(shaderc_optimization_level_zero);
        o.SetVulkanRulesRelaxed(true);
        o.SetAutoMapLocations(true);
        o.SetAutoBindUniforms(true);
        return o;
    }();

    shaderc::SpvCompilationResult module = getShaderCompiler().CompileGlslToSpv(src, kind, "shader_upgto330", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error("Shader upgrade error: " + module.GetErrorMessage());
    }

    spirv_cross::CompilerGLSL glslCompiler(module.cbegin(), static_cast<size_t>(module.cend() - module.cbegin()));
    spirv_cross::CompilerGLSL::Options glslOptions;
    glslOptions.version = 330;
    glslOptions.es = false;
    glslOptions.vulkan_semantics = false;
    glslOptions.enable_420pack_extension = false;
    glslOptions.force_flattened_io_blocks = true;
    glslOptions.enable_storage_image_qualifier_deduction = false;
    glslCompiler.set_common_options(glslOptions);

    src = glslCompiler.compile();

    if (SHADER_DUMP) {
        LOGI("Upgraded shader source:");
        LOGI("%s", src.c_str());
    }
}

inline void combineSources(GLsizei count, const GLchar *const* sources, const GLint* lengths, std::string& destination) {
    for (GLsizei i = 0; i < count; i++) {
        if (!sources[i]) continue;

        if (lengths && lengths[i] >= 0) destination.append(sources[i], lengths[i]);
        else destination.append(sources[i]);
    }
}

inline shaderc_shader_kind getKindFromShader(GLuint shader) {
    GLint shaderType;
    glGetShaderiv(shader, GL_SHADER_TYPE, &shaderType);

    switch (shaderType) {
        case GL_FRAGMENT_SHADER:
            return shaderc_fragment_shader;
        case GL_VERTEX_SHADER:
            return shaderc_vertex_shader;
        case GL_COMPUTE_SHADER:
            if (ESUtils::version.first == 3 && ESUtils::version.second >= 1) return shaderc_compute_shader;
            throw std::runtime_error("You need OpenGL ES 3.1 or newer for compute shaders!");

        default:
            LOGI("%u", shader);
            throw std::runtime_error("Received an unsupported shader type!");
    }
}

inline str getKindStringFromKind(shaderc_shader_kind kind) {
    switch (kind) {
        case shaderc_vertex_shader:
            return "VertexShader";
        case shaderc_fragment_shader:
            return "FragmentShader";
        case shaderc_compute_shader:
            return "ComputeShader";
        default: return "";
    }
}

inline bool getShaderVersion(std::string_view source, int& version, std::string_view& profile) {
    constexpr std::string_view token = "#version";

    size_t pos = source.find(token);
    if (pos == std::string_view::npos) return false;

    size_t i = pos + token.size();
    while (i < source.size() && (source[i] == ' ' || source[i] == '\t')) i++;

    size_t digits = i;
    int value = 0;
    while (i < source.size() && source[i] >= '0' && source[i] <= '9') {
        value = value * 10 + (source[i] - '0');
        i++;
    }
    if (i == digits) return false;

    version = value;
    profile = {};

    while (i < source.size() && (source[i] == ' ' || source[i] == '\t')) i++;
    for (std::string_view candidate : { std::string_view("compatibility"), std::string_view("core"), std::string_view("es") }) {
        if (source.compare(i, candidate.size(), candidate) == 0) {
            size_t after = i + candidate.size();
            if (after >= source.size() || !(source[after] == '_' || (source[after] >= '0' && source[after] <= '9')
                || (source[after] >= 'a' && source[after] <= 'z') || (source[after] >= 'A' && source[after] <= 'Z'))) {
                profile = candidate;
                break;
            }
        }
    }

    return true;
}

namespace ShaderConverter {
    inline shaderc::SpvCompilationResult compileToSPV(shaderc_shader_kind kind, std::string& source) {
        int shaderVersion = 0;
        std::string_view profile;
        getShaderVersion(source, shaderVersion, profile);

        GLSLPreprocessor::run(source, shaderVersion);

        if (SHADER_DUMP) {
            LOGE("Preprocessed source:");
            LOGE("%s", source.c_str());
        }

        if (shaderVersion < 330) {
            upgradeTo330(kind, source);
            shaderVersion = 330;
        }

        shaderc::CompileOptions options = generateGLSL2SPVOptions(shaderVersion);
        shaderc::SpvCompilationResult result = getShaderCompiler().CompileGlslToSpv(source, kind, "glslShader", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOGE("Failed to compile shader: %s", result.GetErrorMessage().c_str());
            throw std::runtime_error("Shader compilation failed!");
        }

        return result;
    }

    inline void transpileSPV2ESSL(shaderc_shader_kind kind, const shaderc::SpvCompilationResult& module, std::string& target) {
        SPVCExposed_CompilerGLSL compiler(module.cbegin(), static_cast<size_t>(module.cend() - module.cbegin()));
        compiler.set_common_options(generateSPV2ESSLOptions(ESUtils::shadingVersion));

        SPVCPostprocessor::processSPVBytecode(compiler, kind);

        target = compiler.compile();

        if (SHADER_DUMP) {
            LOGI("Transpiled GLSL -> ESSL source:");
            LOGI("%s", target.c_str());
        }
    }

    // `ubos` receives the UBO bindings used by the converted shader.
    inline void convertAndFix(
        shaderc_shader_kind kind,
        std::string& source,
        std::vector<std::pair<std::string, uint32_t>>* ubos = nullptr
    ) {
        if (SHADER_DUMP) {
            LOGI("Converting and fixing %s!", getKindStringFromKind(kind));
            LOGI("Received GLSL source:");
            LOGI("%s", source.c_str());
        }

        shaderc::SpvCompilationResult spirv = compileToSPV(kind, source);

        SPVCPostprocessor::bindingSink = ubos;
        try {
            transpileSPV2ESSL(kind, spirv, source);
        } catch (...) {
            SPVCPostprocessor::bindingSink = nullptr;
            throw;
        }
        SPVCPostprocessor::bindingSink = nullptr;
    }
}; // namespace ShaderConverter

namespace Shader {
    void init();
}
