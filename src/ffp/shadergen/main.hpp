#pragma once

#include "ffp/shadergen/cache.hpp"
#include "ffp/shadergen/common.hpp"
#include "ffp/shadergen/features/alphatest.hpp"
#include "ffp/shadergen/features/fog.hpp"
#include "ffp/shadergen/features/mvp.hpp"
#include "ffp/shadergen/features/registry.hpp"
#include "ffp/shadergen/features/vertexbuffer.hpp"
#include "common/raii_helpers.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "ffp/enums.hpp"
#include "core/main.hpp"
#include "shader/overrides.hpp"

#include <GLES3/gl32.h>
#include <string>

namespace FFPE::Rendering::ShaderGen {

inline std::string buildVertexShader() {
    std::stringstream inputs;
    std::stringstream outputs;
    std::stringstream operations;
    std::stringstream outputOperations;

    for (const auto& feature : Feature::Registry::Data::registeredFeatures) {
        if (feature->isEnabled()) feature->buildVS(inputs, outputs, operations, outputOperations);
    }

    return fmt::format(
        Common::VS_TEMPLATE,
        inputs.str(), outputs.str(), operations.str(), outputOperations.str()
    );
}

inline std::string buildFragmentShader() {
    std::stringstream inputs;
    std::stringstream outputs;
    std::stringstream operations;
    std::stringstream outputOperations;

    for (const auto& feature : Feature::Registry::Data::registeredFeatures) {
        if (feature->isEnabled()) feature->buildFS(inputs, outputs, operations, outputOperations);
    }

    return fmt::format(
        Common::FS_TEMPLATE,
        inputs.str(), outputs.str(), operations.str(), outputOperations.str()
    );
}

inline GLuint getCachedOrNewProgram(GLbitfield64 state) {
    GLuint cachedProgram = Cache::Programs::getFromCache(state);
    if (cachedProgram) return cachedProgram;

    if (debugEnabled) LOGI("Generating shader for state '%llu'", state);
    GLDebugGroup gldg("ShaderGen : state=" + std::to_string(state));

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexSource = Cache::Shaders::getFromCache(state, shaderc_vertex_shader);
    if (vertexSource.empty()) {
        vertexSource = buildVertexShader();
        Cache::Shaders::putToCache(state, shaderc_vertex_shader, vertexSource);
    }

    const GLchar* convertedVS = vertexSource.c_str();
    glShaderSource(
        vertexShader, 1,
        &convertedVS, nullptr
    );

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentSource = Cache::Shaders::getFromCache(state, shaderc_fragment_shader);
    if (fragmentSource.empty()) {
        fragmentSource = buildFragmentShader();
        Cache::Shaders::putToCache(state, shaderc_fragment_shader, fragmentSource);
    }

    const GLchar* convertedFS = fragmentSource.c_str();
    glShaderSource(
        fragmentShader, 1,
        &convertedFS, nullptr
    );

    CALL(glCompileShader, vertexShader);
    CALL(glCompileShader, fragmentShader);

    if (debugEnabled) {
        LOGI("ShaderGen | VS :");
        LOGI("%s", vertexSource.c_str());

        LOGI("ShaderGen | FS :");
        LOGI("%s", fragmentSource.c_str());
    }

    GLuint renderingProgram = glCreateProgram();
    glAttachShader(renderingProgram, vertexShader);
    glAttachShader(renderingProgram, fragmentShader);

    CALL(glLinkProgram, renderingProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    Cache::Programs::putToCache(state, renderingProgram);

    return renderingProgram;
}

inline void initFeatures() {
    // NOTE: order matters!
    Feature::Registry::registerFeature<Feature::VertexAttrib::VertexAttribFeature>();
    Feature::Registry::registerFeature<Feature::MVP::MVPFeature>();
    // Feature::Registry::registerFeature<Feature::Texture::TextureFeature>();
    Feature::Registry::registerFeature<Feature::Fog::FogFeature>();

    Feature::Registry::registerFeature<Feature::AlphaTest::AlphaTestFeature>();
}

}
