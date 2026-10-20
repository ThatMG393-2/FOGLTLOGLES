#pragma once

#include "ffp/matrices.hpp"
#include "ffp/shadergen/cache.hpp"
#include "ffp/shadergen/common.hpp"
#include "ffp/shadergen/features/base.hpp"
#include "ffp/shadergen/features/fog.hpp"
#include "ffp/shadergen/features/registry.hpp"
#include "glm/gtc/type_ptr.hpp"


#include <GLES3/gl32.h>
#include <sstream>

namespace FFPE::Rendering::ShaderGen::Feature::MVP {

struct MVPFeature : public Feature::BaseFeature {

static constexpr std::string_view modelViewProjectionUniformVS = "uniform mat4 uModelViewProjection;";
static constexpr std::string_view modelViewUniformVS = "uniform mat4 uModelView;";

bool isEnabled() const override { return true; }

void buildVS(
    std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const override {
    finalInputs << modelViewProjectionUniformVS << Common::Whitespaces::DOUBLE_NEWLINE;

    if (Registry::getFeatureInstance<Fog::FogFeature>()->isEnabled()) {
        finalInputs << modelViewUniformVS << Common::Whitespaces::DOUBLE_NEWLINE;
    }
}

void sendData(GLuint program) const override {
    glUniformMatrix4fv(
        Cache::Uniforms::getCachedUniformLocation(
            program, "uModelViewProjection"
        ), 1, GL_FALSE,
        glm::value_ptr(
            Matrices::MVP::getModelViewProjection()
        )
    );

    if (Registry::getFeatureInstance<Fog::FogFeature>()->isEnabled()) {
        glUniformMatrix4fv(
            Cache::Uniforms::getCachedUniformLocation(
                program, "uModelView"
            ), 1, GL_FALSE,
            glm::value_ptr(
                Matrices::getMatrix(GL_MODELVIEW).matrix
            )
        );
    }
}

};

}
