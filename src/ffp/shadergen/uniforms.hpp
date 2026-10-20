#pragma once

#include "ffp/shadergen/cache.hpp"
#include "ffp/shadergen/features/registry.hpp"
#include "common/raii_helpers.hpp"

#include <GLES3/gl32.h>

namespace FFPE::Rendering::ShaderGen::Uniforms {

inline void setupInputsForRendering(GLuint program) {
    GLDebugGroup gldg("Bind uniforms");

    for (const auto& feature : Feature::Registry::Data::registeredFeatures) {
        if (feature->isEnabled()) feature->sendData(program);
    }

    glUniform1i(
        Cache::Uniforms::getCachedUniformLocation(
            program, "tex0"
        ), 0
    );
}

}
