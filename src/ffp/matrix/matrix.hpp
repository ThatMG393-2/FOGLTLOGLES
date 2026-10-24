#pragma once

#include "common/enums.hpp"
#include "ffp/fog/fog.hpp"
#include "ffp/shadergen/shadergen.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <GLES3/gl32.h>
#include <functional>
#include <sstream>
#include <stack>
#include <string_view>
#include <unordered_map>

namespace FFPE::Rendering::Matrices {

namespace MVP {

namespace States {
    inline bool shouldRecalculate;

    inline glm::mat4 modelViewProjection;
}

inline void invalidateMVP() {
    States::shouldRecalculate = true;
}

const glm::mat4 getModelViewProjection();

}

struct Matrix {
    GLenum type;
    glm::mat4 matrix = glm::mat4(1.0f);

    std::stack<glm::mat4> stack;
};

namespace States {
    inline GLenum currentMatrixType = GL_MODELVIEW;

    inline std::unordered_map<GLenum, Matrix> matrices;
    inline Matrix* currentMatrix;
}

inline const Matrix getMatrix(GLenum mode) {
    return States::matrices[mode];
}

inline const Matrix getCurrentMatrix() {
    return *States::currentMatrix;
}

void setCurrentMatrix(GLenum mode);
void modifyCurrentMatrix(const std::function<glm::mat4(glm::mat4)>& newMatrix);
void pushCurrentMatrix();
void popTopMatrix();

}

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

namespace FFP {
    void registerMatrixFunctions();
}
