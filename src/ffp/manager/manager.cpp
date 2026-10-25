#include "ffp/manager/manager.hpp"
#include "ffp/alphatest/alphatest.hpp"
#include "ffp/arrays/arrays.hpp"
#include "ffp/fog/fog.hpp"
#include "ffp/shading/shading.hpp"
#include "state/state.hpp"

#include <GLES3/gl32.h>
#include <stdexcept>

namespace FFPE::States::Manager {

namespace {
    union StateRepresentation {
        struct Fields {
            GLuint64 alphaOp : 4; // 0-7 (8 possible values)
            GLuint64 alphaTestEnabled : 1; // 0-1 (2 possible values)
            GLuint64 texture2DEnabled : 1; // 0-1 (2 possible values)
            GLuint64 shadeType : 1; // 0-1 (2 possible values)

            GLuint64 vertexArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 vertexArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 colorArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 colorArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 normalArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 normalArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 texCoordArrayEnabled : 1; // 0-1 (2 possible values)
            GLuint64 texCoordArrayComponentSize : 2; // 0-3 (4 possible values)

            GLuint64 fogEnabled : 1; // 0-1 (2 possible values)
            GLuint64 fogMode : 2; // 0-3 (4 possible values)
        } fields;
        GLbitfield64 value;
    } currentState;
}

GLbitfield64 getOrBuildState() {
    if (!States::shouldRecalculate) return currentState.value;

    StateRepresentation result { };

    result.fields.alphaOp = AlphaTest::op - GL_NEVER;
    result.fields.alphaTestEnabled = trackedStates->isCapabilityEnabled(GL_ALPHA_TEST);
    result.fields.texture2DEnabled = trackedStates->textureUnits[GL_TEXTURE0].texture2DEnabled;
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
    currentState = result;

    return result.value;
}

}
