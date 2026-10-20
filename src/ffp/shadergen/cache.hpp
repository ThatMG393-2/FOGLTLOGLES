#pragma once

#include "ffp/enums.hpp"
#include "shader/cache.hpp"
#include "shaderc/shaderc.h"
#include "common/fast_map.hpp"

#include <GLES3/gl32.h>
#include <string>
#include <unordered_map>

namespace FFPE::Rendering::ShaderGen::Cache {

namespace Shaders {
// consider caching the shader binaries itself
// caveat: os 😭

inline FastMapBI<GLbitfield64, std::string> lazyCachedSources;

inline size_t tagHash(GLbitfield64 state, shaderc_shader_kind kind) {
#if INTPTR_MAX == INT64_MAX
#   include <cstdint>
    return (ShaderConverter::Cache::getHash(state) & 0x3FFFFFFFFFFFFFFF) | (uint64_t(kind) << 62);
#else
    return (ShaderConverter::Cache::getHash(state) & 0x3FFFFFFF) | (kind << 30);
#endif
}

inline void putToCache(GLbitfield64 state, shaderc_shader_kind kind, std::string& source) {
    lazyCachedSources[tagHash(state, kind)] = source;
    ShaderConverter::Cache::putShaderInCache(
        tagHash(state, kind), source
    );
}

inline std::string getFromCache(GLbitfield64 state, shaderc_shader_kind kind) {
    auto cachedSource = lazyCachedSources.find(tagHash(state, kind));
    if (cachedSource != lazyCachedSources.end()) return cachedSource->second;

    std::string source = ShaderConverter::Cache::getCachedShaderSource(tagHash(state, kind));
    if (!source.empty()) lazyCachedSources[tagHash(state, kind)] = source;

    return source;
}

}

namespace Programs {

inline FastMapBI<GLbitfield64, GLuint> cachedGeneratedPrograms;

inline void putToCache(GLbitfield64 state, GLuint program) {
    cachedGeneratedPrograms[state] = program;
}

inline GLuint getFromCache(GLbitfield64 state) {
    auto cachedProgram = cachedGeneratedPrograms.find(state);
    if (cachedProgram != cachedGeneratedPrograms.end()) return cachedProgram->second;

    return 0;
}

}

namespace Uniforms {

inline FastMapBI<GLuint, std::unordered_map<std::string, GLint>> cachedLocations;

inline GLint getCachedUniformLocation(GLuint program, std::string name) {
    auto& cachedUniformLocations = cachedLocations[program];
    
    GLint uniformLocationLoc = 0;
    auto uniformLocationIT = cachedUniformLocations.find(name);
    if (uniformLocationIT == cachedUniformLocations.end()) {
        uniformLocationLoc = glGetUniformLocation(program, name.c_str());
        if (uniformLocationLoc != -1) cachedUniformLocations[name] = uniformLocationLoc;
    } else uniformLocationLoc = uniformLocationIT->second;
        
    return uniformLocationLoc;
}

}

}