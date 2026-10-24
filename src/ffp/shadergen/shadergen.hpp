#pragma once

#include "common/enums.hpp"
#include "common/fast_map.hpp"
#include "common/log.hpp"
#include "shader/shader.hpp"
#include "shaderc/shaderc.h"

#include <GLES3/gl32.h>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace FFPE::Rendering::ShaderGen::Cache {

namespace Shaders {
// consider caching the shader binaries itself
// caveat: os 😭

inline FastMapBI<GLbitfield64, std::string> lazyCachedSources;

inline size_t tagHash(GLbitfield64 state, shaderc_shader_kind kind) {
#if INTPTR_MAX == INT64_MAX
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

namespace FFPE::Rendering::ShaderGen::Common::Whitespaces {
    inline constexpr std::string DOUBLE_NEWLINE = "\n\n";
    inline constexpr std::string SINGLE_NEWLINE = "\n";

    inline constexpr std::string DOUBLE_NEWLINE_TAB = "\n\n\t\t";
    inline constexpr std::string SINGLE_NEWLINE_TAB = "\n\t\t";
}

namespace FFPE::Rendering::ShaderGen::Feature {

struct BaseFeature {

virtual ~BaseFeature() = default;

virtual void buildVS(
    [[maybe_unused]] std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const { }

virtual void buildFS(
    [[maybe_unused]] std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const { }

virtual void sendData([[maybe_unused]] GLuint program) const { }

virtual bool isEnabled() const { return false; }

};

}

namespace FFPE::Rendering::ShaderGen::Feature::Registry {

namespace Data {
    inline std::vector<std::unique_ptr<BaseFeature>> registeredFeatures;
    inline std::unordered_map<std::type_index, std::reference_wrapper<BaseFeature>> registeredFeaturesMap;
}

template <typename O, typename D>
concept DerivedFrom = std::is_base_of<D, O>::value;

template<DerivedFrom<BaseFeature> F>
inline void registerFeature() {
    LOGI("Registering feature : %s", typeid(F).name());

    Data::registeredFeaturesMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(typeid(F)),
        std::forward_as_tuple(std::ref(
            *Data::registeredFeatures.emplace_back(new F())
        ))
    );
}

template<DerivedFrom<BaseFeature> F>
inline F* getFeatureInstance() {
    auto it = Data::registeredFeaturesMap.find(typeid(F));
    if (it != Data::registeredFeaturesMap.end()) {
        return static_cast<F*>(&it->second.get());
    }
    return nullptr;
}

}

namespace FFPE::Rendering::ShaderGen {

std::string buildVertexShader();
std::string buildFragmentShader();
GLuint getCachedOrNewProgram(GLbitfield64 state);
void initFeatures();

}

namespace FFPE::Rendering::ShaderGen::Uniforms {

void setupInputsForRendering(GLuint program);

}
