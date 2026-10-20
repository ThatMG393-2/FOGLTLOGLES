#pragma once

#include "ffp/shadergen/features/fog.hpp"
#include "ffp/shadergen/features/registry.hpp"
#include "ffp/states.hpp"
#include "ffp/shadergen/common.hpp"
#include "ffp/shadergen/features/base.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "ffp/enums.hpp"

#include <GLES3/gl32.h>
#include <sstream>
#include <stdexcept>
#include <string>

namespace FFPE::Rendering::ShaderGen::Feature::VertexAttrib {

struct VertexAttribFeature : public Feature::BaseFeature {

static constexpr std::string_view vertexPositionInputVS = "layout(location = 0) in mediump vec{} iVertexPosition;";
static constexpr std::string_view vertexColorInputVS = "layout(location = 1) in lowp vec{} iVertexColor;";
static constexpr std::string_view vertexNormalInputVS = "layout(location = 2) in lowp vec{} iVertexNormal;";
static constexpr std::string_view vertexTexCoordInputVS = "layout(location = 3) in mediump vec{} iVertexTexCoord;";
static constexpr std::string_view vertexColorOutputVS = "{} out lowp vec{} vertexColor;";
static constexpr std::string_view vertexNormalOutputVS = "out lowp vec{} vertexNormal;";
static constexpr std::string_view vertexTexCoordOutputVS = "out mediump vec{} vertexTexCoord;";

static constexpr std::string_view vertexColorInputFS = "{} in lowp vec{} vertexColor;";
static constexpr std::string_view vertexNormalInputFS = "in lowp vec{} vertexNormal;";
static constexpr std::string_view vertexTexCoordInputFS = "in mediump vec{} vertexTexCoord;";
static constexpr std::string_view fragColorOutputFS = "out vec4 oFragColor;";

static constexpr std::string_view colorVariableFS = "lowp vec{} color = vertexColor;";

bool isEnabled() const override { return true; }

void buildVS(
    std::stringstream& finalInputs,
    std::stringstream& finalOutputs,
    std::stringstream& finalOperations,
    std::stringstream& finalOutputOperations
) const override {
    auto* vertexArray = States::ClientState::Arrays::getArray(GL_VERTEX_ARRAY);
    auto* colorArray = States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    auto* normalArray = States::ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    auto* texCoordArray = States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);

    finalInputs << fmt::format(
        vertexPositionInputVS,
        vertexArray->parameters.size
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalInputs << fmt::format(
        vertexColorInputVS,
        colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalInputs << fmt::format(
        vertexNormalInputVS,
        normalArray->enabled ? normalArray->parameters.size : decltype(States::VertexData::normal)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalInputs << fmt::format(
        vertexTexCoordInputVS,
        texCoordArray->enabled ? texCoordArray->parameters.size : decltype(States::VertexData::texCoord)::length()
    ) << Common::Whitespaces::DOUBLE_NEWLINE;

    finalOutputs << fmt::format(
        vertexColorOutputVS,
        getColorInterpolation(),
        colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalOutputs << fmt::format(
        vertexNormalOutputVS,
        normalArray->enabled ? normalArray->parameters.size : decltype(States::VertexData::normal)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalOutputs << fmt::format(
        vertexTexCoordOutputVS,
        texCoordArray->enabled ? texCoordArray->parameters.size : decltype(States::VertexData::texCoord)::length()
    ) << Common::Whitespaces::DOUBLE_NEWLINE;

    finalOperations << getPositionExpression(vertexArray->parameters.size) << Common::Whitespaces::DOUBLE_NEWLINE_TAB;

    finalOutputOperations << "vertexColor = iVertexColor;" << Common::Whitespaces::SINGLE_NEWLINE_TAB;
    finalOutputOperations << "vertexNormal = iVertexNormal;" << Common::Whitespaces::SINGLE_NEWLINE_TAB;
    finalOutputOperations << "vertexTexCoord = iVertexTexCoord;" << Common::Whitespaces::DOUBLE_NEWLINE_TAB;
}

void buildFS(
    std::stringstream& finalInputs,
    std::stringstream& finalOutputs,
    std::stringstream& finalOperations,
    std::stringstream& finalOutputOperations
) const override {
    auto* colorArray = States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    auto* normalArray = States::ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    auto* texCoordArray = States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);

    finalInputs << fmt::format(
        vertexColorInputFS,
        getColorInterpolation(),
        colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalInputs << fmt::format(
        vertexNormalInputFS,
        normalArray->enabled ? normalArray->parameters.size : decltype(States::VertexData::normal)::length()
    ) << Common::Whitespaces::SINGLE_NEWLINE;

    finalInputs << fmt::format(
        vertexTexCoordInputFS,
        texCoordArray->enabled ? texCoordArray->parameters.size : decltype(States::VertexData::texCoord)::length()
    ) << Common::Whitespaces::DOUBLE_NEWLINE;

    finalOutputs << fragColorOutputFS << Common::Whitespaces::SINGLE_NEWLINE_TAB;

    finalOperations << fmt::format(
        colorVariableFS,
        colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
    ) << Common::Whitespaces::DOUBLE_NEWLINE_TAB;


    if (Registry::getFeatureInstance<Fog::FogFeature>()->isEnabled()) {
        finalOutputOperations << "// getOutputColorExpression() says 'its up to you Feature::Fog!'";
    } else {
        finalOutputOperations << getOutputColorExpression(
            colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
        );
    }
    finalOutputOperations << Common::Whitespaces::DOUBLE_NEWLINE_TAB;
}

std::string getPositionExpression(GLint componentSize) const {
    switch (componentSize) {
        case 1: return "gl_Position = uModelViewProjection * vec4(iVertexPosition, 0, 0, 1);";
        case 2: return "gl_Position = uModelViewProjection * vec4(iVertexPosition, 0, 1);";
        case 3: return "gl_Position = uModelViewProjection * vec4(iVertexPosition, 1);";
        case 4: return "gl_Position = uModelViewProjection * iVertexPosition;";
        default: throw std::runtime_error("Failed to determine position expression! Is componentSize bigger than 4?");
    }
}

std::string getOutputColorExpression(GLint componentSize) const {
    switch (componentSize) {
        case 1: return "oFragColor = vec4(color, 0, 0, 1);";
        case 2: return "oFragColor = vec4(color, 0, 1);";
        case 3: return "oFragColor = vec4(color, 1);";
        case 4: return "oFragColor = color;";
        default: throw std::runtime_error("Failed to determine output color expression! Is componentSize bigger than 4?");
    }
}

std::string getColorInterpolation() const {
    switch (FFPE::States::ShadeModel::type) {
        case GL_SMOOTH: return "smooth";
        case GL_FLAT: return "flat";
        default: throw std::runtime_error("Failed to determine color interpolation! Is ShadeModel::type not GL_FLAT nor GL_SMOOTH?");
    }
}

};

}
