#pragma once

#include "common/enums.hpp"
#include "common/es_utils.hpp"
#include "common/fast_map.hpp"
#include "common/log.hpp"
#include "shader/shader.hpp"
#include "shaderc/shaderc.h"
#include "state/state.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "fmt/base.h"
#include "fmt/ostream.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "fmt/format.h"
#include "core/core.hpp"
#include "fogltlogles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "draw/draw.hpp"

#include <GLES3/gl32.h>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>
#include <cstddef>
#include <span>

void glEnableClientState(GLenum array);
void glDisableClientState(GLenum array);

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);;
void glNormalPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);

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

namespace FFPE::Rendering::ShaderGen::Common {

inline constexpr std::string_view VS_TEMPLATE = R"(#version 320 es
// FOGLTLOGLES ShaderGen : Vertex Shader

// feature inputs
{}

// default outputs
flat out lowp int vertexID;

// feature outputs
{}

void main() {{
    // feature operations
    {}

    // feature output operations
    {}

    vertexID = gl_VertexID;
}})";

inline constexpr std::string_view FS_TEMPLATE = R"(#version 320 es
precision mediump float;

// FOGLTLOGLES ShaderGen : Fragment Shader

// default inputs
flat in lowp int vertexID;

// feature inputs
{}

uniform sampler2D tex0;

// feature outputs
{}

void main() {{
    // feature operations
    {}

    color *= texture(tex0, vertexTexCoord.xy);

    // feature output operations
    {}
}})";

namespace Whitespaces {
inline constexpr std::string DOUBLE_NEWLINE = "\n\n";
inline constexpr std::string SINGLE_NEWLINE = "\n";

inline constexpr std::string DOUBLE_NEWLINE_TAB = "\n\n\t\t";
inline constexpr std::string SINGLE_NEWLINE_TAB = "\n\t\t";
}

}

namespace FFPE::States {

namespace VertexData {
    template<int PC, typename P, int CC, typename C, int NC, typename N, int TCC, typename TC>
    struct VertexRepresentation {
        VertexRepresentation() = default;
        VertexRepresentation(
            glm::vec<PC, P> p, glm::vec<CC, C> c, glm::vec<NC, N> n, glm::vec<TCC, TC> tc
        ) : position(p), color(c), normal(n), texCoord(tc) { }

        glm::vec<PC, P> position = glm::vec4(0, 0, 0, 1);
        glm::vec<CC, C> color = glm::vec4(255, 255, 255, 255);
        glm::vec<NC, N> normal = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec<TCC, TC> texCoord = glm::vec4(0, 0, 0, 0);
    };

    inline glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    inline glm::vec4 color = glm::vec4(1.0f);
    inline glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
    inline glm::vec4 texCoord = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // TODO: make a <texunit, texcoord> map

    template<typename SRC, typename DST>
    inline void set(SRC src, DST& to) {
        static_assert(
            SRC::length() <= DST::length(),
            "'src' components must be less than or equal to 'dst' components"
        );
        for (size_t i = 0; i < SRC::length(); ++i) to[i] = src[i];
    }
}
namespace AlphaTest {
    inline GLenum op = GL_ALWAYS;
    inline GLclampf threshold = 0.0f;
}
namespace ShadeModel {
    inline GLenum type = GL_SMOOTH;
}
namespace Fog {
    inline GLenum fogMode = GL_LINEAR;
    inline glm::vec4 fogColor = glm::vec4(1.0f);

    namespace Linear {
        inline float fogStart = 0.0f;
        inline float fogEnd = 0.0f;
    }

    namespace Exp {
        inline float fogDensity = 0.0f;
    }
}
namespace ClientState {
    namespace Arrays {
        struct ArrayParameters {
            bool planar; // 1> = TRUE, !1 = !TRUE

            GLint size;
            GLenum type;
            bool normalized;
            GLsizei stride; // if stride == 0 then stride = size * sizeof(type) else stride = stride
            const void* firstElement;
        };

        struct ArrayState {
            bool enabled;
            ArrayParameters parameters;
        };

        inline std::unordered_map<GLenum, ArrayState> arrayStates;

        // GL_TEXTUREi, ArrayState
        inline std::unordered_map<GLenum, ArrayState> texCoordArrayStates;

        inline bool isArrayEnabled(GLenum array) {
            return arrayStates[array].enabled;
        }

        inline bool isTexCoordArrayEnabled(GLenum unit) {
            return texCoordArrayStates[unit].enabled;
        }

        inline ArrayState* getArray(GLenum array) {
            return &arrayStates[array];
        }

        inline ArrayState* getTexCoordArray(GLenum unit) {
            return &texCoordArrayStates[unit];
        }
    }

    inline GLenum currentTexCoordUnit;
    inline std::vector<GLenum> texCoordArrayTexUnits;
}

namespace Manager {

namespace States {
    inline union StateRepresentation {
        struct Fields {
            GLuint64 alphaOp : 4; // 0-7 (8 possible values)
            GLuint64 shadeType : 1; // 0-1 (2 possible values)

            GLuint64 vertexArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 vertexArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 colorArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 colorArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 normalArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 normalArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 texCoordArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 texCoordArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint fogEnabled : 1; // 0-1 (2 possible values)
            GLuint64 fogMode : 2; // 0-3 (4 possible values)
        } fields;
        GLbitfield64 value;
    } currentState;

    inline bool shouldRecalculate = true;
}

inline void invalidateCurrentState() {
    States::shouldRecalculate = true;
}

inline GLbitfield64 getOrBuildState() {
    if (!States::shouldRecalculate) return States::currentState.value;

    States::StateRepresentation result { };

    result.fields.alphaOp = AlphaTest::op - GL_NEVER;
    result.fields.shadeType = ShadeModel::type - GL_FLAT;

    auto* vertex = ClientState::Arrays::getArray(GL_VERTEX_ARRAY);
    result.fields.vertexArrayEnabled = vertex->enabled;
    result.fields.vertexArrayComponentSize = vertex->parameters.size - 1;

    auto* color = ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    result.fields.colorArrayEnabled = color->enabled;
    result.fields.colorArrayComponentSize = color->parameters.size - 1;

    auto* normal = ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    result.fields.normalArrayEnabled = normal->enabled;
    result.fields.normalArrayComponentSize = normal->parameters.size - 1;

    auto* texCoord = ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);
    result.fields.texCoordArrayEnabled = texCoord->enabled;
    result.fields.texCoordArrayComponentSize = texCoord->parameters.size - 1;

    result.fields.fogEnabled = trackedStates->isCapabilityEnabled(GL_FOG);
    result.fields.fogMode = ([]() {
        switch (Fog::fogMode) {
            case GL_LINEAR: return 0;
            case GL_EXP: return 1;
            case GL_EXP2: return 2;
            default: throw std::runtime_error("Invalid Fog::fogMode!");
        }
    })();

    States::shouldRecalculate = false;
    States::currentState = result;

    return result.value;
}

}

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

namespace FFPE::Rendering::ShaderGen::Feature::Fog {

struct FogFeature : public Feature::BaseFeature {

static constexpr std::string_view depthValueOutputVS = "out float depthValue;";

static constexpr std::string_view depthValueInputFS = "in float depthValue;";
static constexpr std::string_view fogDefaultUniformsFS = "uniform vec4 uFogColor;";
static constexpr std::string_view fogLinearUniformsFS = R"(uniform float uFogStart;
uniform float uFogEnd;)";
static constexpr std::string_view fogExpUniformFS = "uniform float uFogDensity;";

bool isEnabled() const override { return trackedStates->isCapabilityEnabled(GL_FOG); }

void buildVS(
    [[maybe_unused]] std::stringstream& finalInputs,
    std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    std::stringstream& finalOutputOperations
) const override {
    auto* vertexArray = States::ClientState::Arrays::getArray(GL_VERTEX_ARRAY);

    finalOutputs << depthValueOutputVS << Common::Whitespaces::DOUBLE_NEWLINE;

    finalOutputOperations << getDepthValueExpression(
        vertexArray->parameters.size
    ) << Common::Whitespaces::DOUBLE_NEWLINE_TAB;
}

void buildFS(
    std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    std::stringstream& finalOperations,
    std::stringstream& finalOutputOperations
) const override {
    auto* colorArray = States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);

    finalInputs << depthValueInputFS << Common::Whitespaces::SINGLE_NEWLINE;
    finalInputs << fogDefaultUniformsFS << Common::Whitespaces::SINGLE_NEWLINE;
    finalInputs << getFogUniforms() << Common::Whitespaces::DOUBLE_NEWLINE;

    finalOperations << getFogFactorExpression() << Common::Whitespaces::DOUBLE_NEWLINE_TAB;

    finalOutputOperations << getOutputColorExpression(
        colorArray->enabled ? colorArray->parameters.size : decltype(States::VertexData::color)::length()
    );
}

void sendData(GLuint program) const override {
    glUniform4fv(
        Cache::Uniforms::getCachedUniformLocation(
            program, "uFogColor"
        ), 1, glm::value_ptr(
            States::Fog::fogColor
        )
    );

    switch (States::Fog::fogMode) {
        case GL_LINEAR:
            glUniform1f(
                Cache::Uniforms::getCachedUniformLocation(
                    program, "uFogStart"
                ), States::Fog::Linear::fogStart
            );

            glUniform1f(
                Cache::Uniforms::getCachedUniformLocation(
                    program, "uFogEnd"
                ), States::Fog::Linear::fogEnd
            );
        break;

        case GL_EXP:
        case GL_EXP2:
            glUniform1f(
                Cache::Uniforms::getCachedUniformLocation(
                    program, "uFogDensity"
                ), States::Fog::Exp::fogDensity
            );
        break;

        default: throw std::runtime_error("Failed to bind fog uniforms! Is Fog::fogMode not GL_LINEAR nor GL_EXP nor GL_EXP2?");
    }
}

std::string getFogUniforms() const {
    switch (States::Fog::fogMode) {
        case GL_LINEAR: return std::string(fogLinearUniformsFS);
        case GL_EXP:
        case GL_EXP2:
            return std::string(fogExpUniformFS);

        default: throw std::runtime_error("Failed to determine fog uniforms! Is Fog::fogMode not GL_LINEAR nor GL_EXP nor GL_EXP2?");
    }
}

std::string getDepthValueExpression(GLint componentSize) const {
    switch (componentSize) {
        case 1: return "depthValue = (uModelView * vec4(iVertexPosition, 0, 0, 1)).z;";
        case 2: return "depthValue = (uModelView * vec4(iVertexPosition, 0, 1)).z;";
        case 3: return "depthValue = (uModelView * vec4(iVertexPosition, 1)).z;";
        case 4: return "depthValue = (uModelView * iVertexPosition).z;";
        default: throw std::runtime_error("Failed to determine position expression! Is componentSize bigger than 4?");
    }
}

std::string getFogFactorExpression() const {
    switch (States::Fog::fogMode) {
        case GL_LINEAR:
            return "float fogFactor = (uFogEnd - depthValue) / (uFogEnd - uFogStart);";
        case GL_EXP:
            return "float fogFactor = exp(-uFogDensity * depthValue);";
        case GL_EXP2:
            return "float fogFactor = exp(-pow(uFogDensity * depthValue, 2.0));";
        default:
            throw std::runtime_error("Failed to determine fog factor expression! Is Fog::fogMode not GL_LINEAR nor GL_EXP nor GL_EXP2?");
    }
}

std::string getOutputColorExpression(GLint componentSize) const {
    switch (componentSize) {
        case 1: return "oFragColor = mix(uFogColor, vec4(color, 0, 0, 1), fogFactor);";
        case 2: return "oFragColor = mix(uFogColor, vec4(color, 0, 1), fogFactor);";
        case 3: return "oFragColor = mix(uFogColor, vec4(color, 1), fogFactor);";
        case 4: return "oFragColor = mix(uFogColor, color, fogFactor);";
        default: throw std::runtime_error("Failed to determine output color expression! Is componentSize bigger than 4?");
    }
}

};

}

namespace FFPE::Rendering::Matrices {

namespace MVP {

namespace States {
    inline bool shouldRecalculate;

    inline glm::mat4 modelViewProjection;
}

inline void invalidateMVP() {
    States::shouldRecalculate = true;
}

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

inline void setCurrentMatrix(GLenum mode) {
    States::currentMatrixType = mode;
    States::currentMatrix = &States::matrices[mode];
}

inline void modifyCurrentMatrix(const std::function<glm::mat4(glm::mat4)>& newMatrix) {
    States::currentMatrix->matrix = newMatrix(States::currentMatrix->matrix);

    if (States::currentMatrixType == GL_PROJECTION
     || States::currentMatrixType == GL_MODELVIEW) MVP::invalidateMVP();
}

inline void pushCurrentMatrix() {
    States::currentMatrix->stack.push(States::currentMatrix->matrix);
}

inline void popTopMatrix() {
    if (States::currentMatrix->stack.empty()) return;

    States::currentMatrix->matrix = States::currentMatrix->stack.top();
    States::currentMatrix->stack.pop();

    if (States::currentMatrixType == GL_PROJECTION
     || States::currentMatrixType == GL_MODELVIEW) MVP::invalidateMVP();
}

namespace MVP {

inline const glm::mat4 getModelViewProjection() {
    if (!States::shouldRecalculate) return States::modelViewProjection;

    States::shouldRecalculate = false;
    return States::modelViewProjection = getMatrix(GL_PROJECTION).matrix * getMatrix(GL_MODELVIEW).matrix;
}

}

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

#define GLM_ENABLE_EXPERIMENTAL



template<int PC, typename P, int CC, typename C, int NC, typename N, int TCC, typename TC>
using TypedVertexData = FFPE::States::VertexData::VertexRepresentation<PC, P, CC, C, NC, N, TCC, TC>;

namespace FFPE::Rendering::VAO {

namespace AttributeLocations {
    inline const GLuint POSITION_LOCATION = 0;
    inline const GLuint COLOR_LOCATION = 1;
    inline const GLuint NORMAL_LOCATION = 2;
    inline const GLuint TEX_COORD_LOCATION = 3;
}

inline GLuint vao;
inline GLuint vab;

inline GLsizei currentVABSize;

inline void init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vab);
}

template<typename F>
inline void mapVertexData(
    GLsizei count,
    States::ClientState::Arrays::ArrayState* vertex,
    States::ClientState::Arrays::ArrayState* color,
    States::ClientState::Arrays::ArrayState* normal,
    States::ClientState::Arrays::ArrayState* texCoord,
    const F&& callback
) {
    ESUtils::TypeTraits::typeToPrimitive(vertex->parameters.type, [&]<typename POS>() {
        ESUtils::TypeTraits::typeToPrimitive(color->parameters.type, [&]<typename COL>() {
            ESUtils::TypeTraits::typeToPrimitive(normal->parameters.type, [&]<typename NOR>() {
                ESUtils::TypeTraits::typeToPrimitive(texCoord->parameters.type, [&]<typename TEX>() {
                    using VertexData = TypedVertexData<4, POS, 4, COL, 3, NOR, 4, TEX>;
                    GLsizei newVABSize = count * sizeof(VertexData);

                    CALL(glBindBuffer, GL_ARRAY_BUFFER, vab);
                    CALL(glBufferData,
                        GL_ARRAY_BUFFER,
                        newVABSize,
                        nullptr,
                        GL_DYNAMIC_DRAW
                    );

                    auto* v = static_cast<VertexData*>(glMapBufferRange(
                        GL_ARRAY_BUFFER, 0,
                        newVABSize, GL_MAP_WRITE_BIT
                    ));

                    callback(v);

                    glUnmapBuffer(GL_ARRAY_BUFFER);
                });
            });
        });
    });
}

template<typename T1, typename T2>
inline void fillDataComponents(GLuint& offsetTracker, std::span<const T1> src, T2* dst) {
    for (size_t i = 0; i < src.size(); i++) (*dst)[i] = src[i];
    offsetTracker += src.size();
}

template<typename T, typename VD>
inline void putVertexDataInternal(GLenum arrayType, GLsizei componentSize, GLuint verticesCount, const T* src, VD* dst) {
    GLuint srcOffset = 0;

    switch (arrayType) {
        case GL_VERTEX_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].position
                );
            }
        break;

        case GL_COLOR_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].color
                );
            }
        break;

        case GL_TEXTURE_COORD_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].texCoord
                );
            }
        break;
    }
}

template<typename VD>
inline void putVertexData(GLenum arrayType, FFPE::States::ClientState::Arrays::ArrayState* array, VD* vertices, GLuint verticesCount) {
    ESUtils::TypeTraits::typeToPrimitive(array->parameters.type, [&]<typename T>() {
        putVertexDataInternal(
            arrayType, array->parameters.size, verticesCount,
            ESUtils::TypeTraits::asTypedArray<T>(
                array->parameters.firstElement
            ),
            vertices
        );
    });
}

// TODO: bind gl*Pointer here as VAO's (doing this with no physical keyboard is making me mentally insane, please send help)

[[nodiscard]]
inline std::unique_ptr<SaveBoundedBuffer> prepareVAOForRendering(GLsizei count) {
    GLDebugGroup gldg("VAO preparations");

    auto* vertexArray = States::ClientState::Arrays::getArray(GL_VERTEX_ARRAY);
    auto* colorArray = States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    auto* normalArray = States::ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    auto* texCoordArray = States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);

    glBindVertexArray(vao);

    if (vertexArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::POSITION_LOCATION);
    } else {
        throw std::runtime_error("Vertex array not enabled! Don't know what to do!!");
    }

    if (colorArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::COLOR_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::COLOR_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::COLOR_LOCATION,
            glm::value_ptr(
                States::VertexData::color
            )
        );
    }

    if (normalArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::NORMAL_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::NORMAL_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::NORMAL_LOCATION,
            glm::value_ptr(
                States::VertexData::normal
            )
        );
    }

    if (texCoordArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::TEX_COORD_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::TEX_COORD_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::TEX_COORD_LOCATION,
            glm::value_ptr(
                States::VertexData::texCoord
            )
        );
    }

    std::unique_ptr<SaveBoundedBuffer> sbb;
    if (trackedStates->boundBuffers[GL_ARRAY_BUFFER].buffer == 0) {
        sbb = std::make_unique<SaveBoundedBuffer>(GL_ARRAY_BUFFER);

        if (!vertexArray->parameters.planar) {
            GLsizei newVABSize = count * vertexArray->parameters.stride;

            CALL(glBindBuffer, GL_ARRAY_BUFFER, vab);
            CALL(glBufferData,
                GL_ARRAY_BUFFER,
                newVABSize,
                vertexArray->parameters.firstElement,
                GL_DYNAMIC_DRAW
            );

            uintptr_t base = reinterpret_cast<uintptr_t>(vertexArray->parameters.firstElement);
            vertexArray->parameters = {
                vertexArray->parameters.planar,
                vertexArray->parameters.size, vertexArray->parameters.type,
                GL_FALSE, vertexArray->parameters.stride,
                nullptr
            };

            if (colorArray->enabled) {
                colorArray->parameters = {
                    colorArray->parameters.planar,
                    colorArray->parameters.size, colorArray->parameters.type,
                    GL_TRUE, colorArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(colorArray->parameters.firstElement) - base)
                };
            }

            if (normalArray->enabled) {
                normalArray->parameters = {
                    normalArray->parameters.planar,
                    normalArray->parameters.size, normalArray->parameters.type,
                    GL_FALSE, normalArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(normalArray->parameters.firstElement) - base)
                };
            }

            if (texCoordArray->enabled) {
                texCoordArray->parameters = {
                    texCoordArray->parameters.planar,
                    texCoordArray->parameters.size, texCoordArray->parameters.type,
                    GL_FALSE, texCoordArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(texCoordArray->parameters.firstElement) - base)
                };
            }

            goto buffered;
        }

        mapVertexData(
            count,
            vertexArray, colorArray, normalArray, texCoordArray,
            [&](auto* vertices) {
                using VertexData = std::remove_pointer_t<decltype(vertices)>;

                putVertexData(GL_VERTEX_ARRAY, vertexArray, vertices, count);
                vertexArray->parameters = {
                    vertexArray->parameters.planar,
                    decltype(VertexData::position)::length(),
                    vertexArray->parameters.type, GL_FALSE,
                    sizeof(VertexData),
                    (void*) offsetof(VertexData, position)
                };

                if (colorArray->enabled) {
                    putVertexData(GL_COLOR_ARRAY, colorArray, vertices, count);
                    colorArray->parameters = {
                        colorArray->parameters.planar,
                        decltype(VertexData::color)::length(),
                        colorArray->parameters.type, GL_TRUE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, color)
                    };
                }

                if (normalArray->enabled) {
                    putVertexData(GL_NORMAL_ARRAY, normalArray, vertices, count);
                    normalArray->parameters = {
                        normalArray->parameters.planar,
                        decltype(VertexData::normal)::length(),
                        normalArray->parameters.type, GL_FALSE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, normal)
                    };
                }

                if (texCoordArray->enabled) {
                    putVertexData(GL_TEXTURE_COORD_ARRAY, texCoordArray, vertices, count);
                    texCoordArray->parameters = {
                        texCoordArray->parameters.planar,
                        decltype(VertexData::texCoord)::length(),
                        texCoordArray->parameters.type, GL_FALSE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, texCoord)
                    };
                }
            }
        );
    }

buffered:
    glVertexAttribPointer(
        AttributeLocations::POSITION_LOCATION,
        vertexArray->parameters.size, vertexArray->parameters.type,
        vertexArray->parameters.normalized,
        vertexArray->parameters.stride,
        vertexArray->parameters.firstElement
    );

    if (colorArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::COLOR_LOCATION,
            colorArray->parameters.size, colorArray->parameters.type,
            colorArray->parameters.normalized,
            colorArray->parameters.stride,
            colorArray->parameters.firstElement
        );
    }

    if (normalArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::NORMAL_LOCATION,
            normalArray->parameters.size, normalArray->parameters.type,
            normalArray->parameters.normalized,
            normalArray->parameters.stride,
            normalArray->parameters.firstElement
        );
    }

    if (texCoordArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::TEX_COORD_LOCATION,
            texCoordArray->parameters.size, texCoordArray->parameters.type,
            texCoordArray->parameters.normalized,
            texCoordArray->parameters.stride,
            texCoordArray->parameters.firstElement
        );
    }

    return sbb;
}

}

namespace FFPE::Rendering::Arrays {

namespace Quads {

// based on:
// https://github.com/MobileGL-Dev/MobileGlues/blob/4902f3a629629ad17ad34165c7eec17a3a2d46b6/src/main/cpp/gl/fpe/fpe.cpp#L31
inline const std::string quadEABGeneratorCS = R"(#version 310 es
layout(local_size_x = 64) in;

layout(std430, binding = 0) writeonly buffer IndexBuffer {
    uint indices[];
};

uniform uint numQuads;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= uint(numQuads)) return;

    uint baseIndex = i * 4u;
    uint outIndex = i * 6u;

    // first tri (0 1 2)
    indices[outIndex + 0u] = baseIndex + 0u;
    indices[outIndex + 1u] = baseIndex + 1u;
    indices[outIndex + 2u] = baseIndex + 2u;

    // second tri (2 3 0)
    indices[outIndex + 3u] = baseIndex + 2u;
    indices[outIndex + 4u] = baseIndex + 3u;
    indices[outIndex + 5u] = baseIndex + 0u;
})";

inline GLuint eabGeneratorProgram;
inline GLuint indicesOutputBuffer;
inline GLuint numQuadsUniLoc;

// quad amount, indices
inline FastMapBI<GLuint, std::vector<GLuint>> cachedIndices;

inline void init() {
    glGenBuffers(1, &indicesOutputBuffer);

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const GLchar* computeShaderSource = quadEABGeneratorCS.c_str();
    CALL(glShaderSource, computeShader, 1, &computeShaderSource, nullptr);
    CALL(glCompileShader, computeShader);

    eabGeneratorProgram = glCreateProgram();
    glAttachShader(eabGeneratorProgram, computeShader);
    CALL(glLinkProgram, eabGeneratorProgram);

    numQuadsUniLoc = glGetUniformLocation(eabGeneratorProgram, "numQuads");

    glDeleteShader(computeShader);
}

inline GLuint generateEAB(GLuint count) {
    GLuint quadCount = count / 4;
    GLuint eabCount = quadCount * 6;

    CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, indicesOutputBuffer);

    auto cached = cachedIndices.find(quadCount);
    if (cached != cachedIndices.end()) {
        CALL(glBufferData,
            GL_ELEMENT_ARRAY_BUFFER,
            cached->second.size() * sizeof(GLuint),
            cached->second.data(),
            GL_DYNAMIC_DRAW
        );

        return eabCount;
    }

    CALL(glBufferData,
        GL_ELEMENT_ARRAY_BUFFER,
        eabCount * sizeof(GLuint),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, indicesOutputBuffer);

    SaveUsedProgram sup;
    glUseProgram(eabGeneratorProgram);
    glUniform1ui(numQuadsUniLoc, quadCount);

    glDispatchCompute((quadCount + 63) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);

    GLuint* indices = (GLuint*) glMapBufferRange(
        GL_ELEMENT_ARRAY_BUFFER,
        0,
        eabCount * sizeof(GLuint),
        GL_MAP_READ_BIT
    );

    cachedIndices.emplace(
        quadCount,
        std::vector<GLuint>(indices, indices + eabCount)
    );

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    return eabCount;
}

inline void drawQuads(GLuint count) {
    SaveUsedProgram sup;
    SaveBoundedBuffer sbb(GL_ELEMENT_ARRAY_BUFFER);

    auto buffer = Rendering::VAO::prepareVAOForRendering(count);
    auto renderingProgram = Rendering::ShaderGen::getCachedOrNewProgram(States::Manager::getOrBuildState());
    CALL(glUseProgram, renderingProgram);
    Rendering::ShaderGen::Uniforms::setupInputsForRendering(renderingProgram);

    GLuint realCount = generateEAB(count);

    glDrawElements(GL_TRIANGLES, realCount, GL_UNSIGNED_INT, nullptr);
}

}

inline void init() {
    Quads::init();
    Rendering::VAO::init();
    Rendering::ShaderGen::initFeatures();
}

inline void drawArrays(GLenum mode, GLint first, GLuint count) {
    if (trackedStates->currentlyUsedProgram == 0) {
        // immediately assume FFP
        SaveUsedProgram sup;

        auto buffer = Rendering::VAO::prepareVAOForRendering(count);
        auto renderingProgram = Rendering::ShaderGen::getCachedOrNewProgram(FFPE::States::Manager::getOrBuildState());
        CALL(glUseProgram, renderingProgram);
        Rendering::ShaderGen::Uniforms::setupInputsForRendering(renderingProgram);

        glDrawArrays(mode, first, count);

        return;
    }

    glDrawArrays(mode, first, count);
}

}

template<auto Func>
constexpr std::string getTypeNameFromTemplate() {
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view fn = __PRETTY_FUNCTION__;
    constexpr std::string_view key = "Func = ";
    auto start = fn.find(key) + key.size();
    auto end = fn.find_first_of("];", start);
    return std::string(fn.substr(start, end - start));
#elif defined(_MSC_VER)
    constexpr std::string_view fn = __FUNCSIG__;
    constexpr std::string_view key = "Func=";
    auto start = fn.find(key) + key.size();
    auto end = fn.find_first_of(">,", start);
    return std::string(fn.substr(start, end - start));
#else
    return "unknown";
#endif
}

namespace FFPE::List {

struct Command {
    const std::string name;
    const std::function<void()> func;

    void operator() () const {
        this->func();
    }
};

// TODO: an optimize function
class DisplayList {
private:
    std::vector<Command> commands;

    GLenum mode = GL_NONE;

public:
    DisplayList() { }
    DisplayList(GLenum mode) : mode(mode) { }

    template<typename... Args>
    void addCommand(Args&&... args) {
        commands.emplace_back(std::forward<Args>(args)...);
    }

    void setMode(GLenum mode) {
        this->mode = mode;
    }

    void execute() const {
        for (const auto& command : commands) {
            command();
        }
    }

    GLenum getMode() const {
        return mode;
    }
};

namespace States {
    inline GLuint nextListIndex = 1;
    inline bool executingDisplayList;

    inline bool ignoreNextCallFlag;

    inline FastMapBI<GLuint, DisplayList> displayLists;

    inline GLuint activeDisplayListIndex;
    inline DisplayList activeDisplayList;
}

inline bool isList(GLuint list) {
    return States::displayLists.find(list) != States::displayLists.end();
}

inline bool isRecording() {
    return States::activeDisplayListIndex != 0;
}

inline bool isExecuting() {
    return States::executingDisplayList;
}

// i dont know if this is even correct
inline GLuint genDisplayLists(GLsizei range) {
    if (range <= 0) return 0;

    for (GLsizei i = 0; i < range; ++i) {
        States::displayLists.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(i + States::nextListIndex),
            std::forward_as_tuple(DisplayList())
        );
    }

    return std::exchange(States::nextListIndex, States::nextListIndex += range);
}

inline void startDisplayList(GLuint list, GLenum mode) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    States::activeDisplayListIndex = list;
    States::activeDisplayList = DisplayList(mode);

    std::string dbgMes = "Recording list : " + std::to_string(list);
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, dbgMes.c_str());
}

inline void ignoreNextCall() {
    if (!isRecording() || isExecuting()) return;
    States::ignoreNextCallFlag = true;
}

template<auto F, typename... Args>
requires std::invocable<decltype(F), Args...>
inline bool addCommand(Args&&... args) {
    if (!isRecording() || isExecuting()) return false;
    if (States::ignoreNextCallFlag) {
        States::ignoreNextCallFlag = false;
        return false;
    }

    using DecayedF = std::decay_t<decltype(F)>;

    States::activeDisplayList.addCommand(
        getTypeNameFromTemplate<F>(),
        [a = std::make_tuple(args...)]() mutable {
            std::apply(DecayedF { }, std::move(a));
        }
    );

    return !(States::activeDisplayList.getMode() == GL_COMPILE_AND_EXECUTE);
}

inline void endDisplayList() {
    if (!isRecording() || isExecuting()) return;

    States::displayLists.insert({
        States::activeDisplayListIndex,
        States::activeDisplayList
    });

    States::activeDisplayListIndex = 0;
    States::activeDisplayList = DisplayList();

    glPopDebugGroup();
}

inline void deleteDisplayLists(GLuint list, GLsizei range) {
    if (range <= 0) return;

    for (GLsizei i = 0; i < range; ++i) {
        if (!isList(list)) continue;
        States::displayLists.erase(list + i);
    }
}

inline void callDisplayList(GLuint list) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    GLDebugGroup gldg("List replay : " + std::to_string(list));

    States::executingDisplayList = true;
    States::displayLists[list].execute();
    States::executingDisplayList = false;
}

template<typename T>
inline void callDisplayLists(GLsizei n, const T* lists) {
    States::executingDisplayList = true;
    for (GLsizei i = 0; i < n; ++i) {
        if (!isList(i)) continue;
        GLDebugGroup gldg("List replay : " + std::to_string(lists[i]));

        States::displayLists[static_cast<GLuint>(lists[i])].execute();
    }
    States::executingDisplayList = false;
}

}

using VertexData = FFPE::States::VertexData::VertexRepresentation<4, GLfloat, 4, GLfloat, 3, GLfloat, 4, GLfloat>;

namespace FFPE::Rendering::ImmediateMode {

namespace States {
    inline GLenum primitive;

    inline std::vector<VertexData> vertices;
}

inline bool isActive() {
    return States::primitive != GL_NONE;
}

inline void begin(GLenum primitive) {
    if (isActive()) {
        LOGE("glBegin has not been ended yet!");
        return;
    }

    switch (primitive) {
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case 0x7: // GL_QUADS
        case 0x8: // GL_QUAD_STRIP
        case 0x9: // GL_POLYGON
            break;
        default:
            return;
    }

    if (FFPE::List::addCommand<begin>(primitive)) return;

    // LOGI("glBegin()!");

    States::primitive = primitive;
}

inline void advance() {
    if (!isActive()) return;
    States::vertices.emplace_back(
        FFPE::States::VertexData::position,
        FFPE::States::VertexData::color,
        FFPE::States::VertexData::normal,
        FFPE::States::VertexData::texCoord
    );
}

// TODO: merge with other immediates on display list end record
inline void endInternal(
    const GLenum primitive,
    const std::vector<VertexData>& vertices
) {
    if (!isActive()) {
        LOGE("glEnd has not been called yet!");
        return;
    }

    // LOGI("glEnd()!");

    if (vertices.size() == 0) {
        LOGW("glEnd called with no vertices??");
        LOGW("Ignoring!!");
        return;
    }

    if (FFPE::List::addCommand<endInternal>(
        primitive,
        vertices
    )) return;

    GLDebugGroup gldg("Immediate mode rendering");

    /* SaveClientArray sca1(GL_VERTEX_ARRAY);
    SaveClientArray sca2(GL_COLOR_ARRAY);
    SaveTexCoordClientArray stcca(GL_TEXTURE0); */

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(
        decltype(VertexData::position)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::position
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data()
    );

    glColorPointer(
        decltype(VertexData::color)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::color
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data() + offsetof(VertexData, color)
    );

    glNormalPointer(
        decltype(VertexData::normal)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::normal
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data() + offsetof(VertexData, normal)
    );

    glTexCoordPointer(
        decltype(VertexData::texCoord)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::texCoord
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data() + offsetof(VertexData, texCoord)
    );

    FFPE::List::ignoreNextCall();
    OV_glDrawArrays(primitive, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

inline void end() {
    endInternal(States::primitive, States::vertices);

    States::primitive = GL_NONE;
    States::vertices.clear();
}

}

namespace FFP {
    void init();

    void registerStubFunctions();
    void registerImmediateFunctions();
    void registerColorFunctions();
    void registerTexCoordFunctions();
    void registerVertexFunctions();
    void registerMatrixFunctions();
    void registerListsFunctions();
    void registerAlphaTestFunctions();
    void registerTextureFunctions();
    void registerArrayFunctions();
    void registerShadingFunction();
    void registerFogFunctions();
    void registerNormalFunctions();
    void registerLightFunctions();
}

class SaveClientArray : public Restorable<SaveClientArray> {
    friend class Restorable<SaveClientArray>;
    GLenum targetArray;
    bool isEnabled;

    void _internal_restore() {
        if (isEnabled) glEnableClientState(targetArray);
        else glDisableClientState(targetArray);
    }

public:
    explicit SaveClientArray(GLenum array)
        : targetArray(array),
          isEnabled(FFPE::States::ClientState::Arrays::getArray(array)->enabled) { }
    ~SaveClientArray() { restore(); }
};

class SaveTexCoordClientArray : public Restorable<SaveTexCoordClientArray> {
    friend class Restorable<SaveTexCoordClientArray>;
    GLenum targetUnit;
    bool isEnabled;

    void _internal_restore() {
        FFPE::States::ClientState::Arrays::getTexCoordArray(targetUnit)->enabled = isEnabled;
    }

public:
    explicit SaveTexCoordClientArray(GLenum unit)
        : targetUnit(unit),
          isEnabled(FFPE::States::ClientState::Arrays::getTexCoordArray(unit)->enabled) { }
    ~SaveTexCoordClientArray() { restore(); }
};
