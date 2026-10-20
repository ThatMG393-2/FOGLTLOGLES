#pragma once

#include "ffp/states.hpp"
#include "ffp/shadergen/cache.hpp"
#include "ffp/shadergen/common.hpp"
#include "ffp/shadergen/features/base.hpp"
#include "state/state_tracking.hpp"
#include "fmt/base.h"
#include "fmt/ostream.h"
#include "ffp/enums.hpp"

#include <GLES3/gl32.h>
#include <sstream>
#include <string>

namespace FFPE::Rendering::ShaderGen::Feature::AlphaTest {

struct AlphaTestFeature : public Feature::BaseFeature {

static constexpr std::string_view uniforms = "uniform float alphaTestThreshold;";
static constexpr std::string_view baseOperation = "if (color.a {} alphaTestThreshold) discard;";

bool isEnabled() const override { return trackedStates->isCapabilityEnabled(GL_ALPHA_TEST); }

void buildFS(
    std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const override {
    finalInputs << uniforms << Common::Whitespaces::DOUBLE_NEWLINE;

    std::string operation;
    switch (States::AlphaTest::op) {
        case GL_LESS:
            operation = "<";
        break;

        case GL_EQUAL:
            operation = "==";
        break;

        case GL_LEQUAL:
            operation = "<=";
        break;

        case GL_GREATER:
            operation = ">";
        break;

        case GL_NOTEQUAL:
            operation = "!=";
        break;

        case GL_GEQUAL:
            operation = ">=";
        break;

        case GL_NEVER: finalOperations << "discard;"; return;
        case GL_ALWAYS: return;
    }

    fmt::print(
        finalOperations,
        baseOperation,
        operation
    );

    finalOperations << Common::Whitespaces::DOUBLE_NEWLINE_TAB;
}

void sendData(GLuint program) const override {
    GLint alphaTestThresholdUniLoc = Cache::Uniforms::getCachedUniformLocation(program, "alphaTestThreshold");
    if (alphaTestThresholdUniLoc == -1) return;
        
    glUniform1f(alphaTestThresholdUniLoc, States::AlphaTest::threshold);
}

};

}