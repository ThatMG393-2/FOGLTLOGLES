#include "ffp/states.hpp"
#include "ffp/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>

void glClientActiveTexture(GLenum unit);

void FFP::registerTextureFunctions() {
    PROVIDE(glClientActiveTexture);
}

void glClientActiveTexture(GLenum unit) {
    // if (debugEnabled) LOGI("glClientActiveTexture : unit=%u", unit);
    
    FFPE::States::ClientState::currentTexCoordUnit = unit;
    // FFPE::States::ClientState::texCoordArrayTexUnits.push_back(unit);
}
