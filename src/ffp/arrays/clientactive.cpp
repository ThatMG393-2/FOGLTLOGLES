#include "ffp/arrays/arrays.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glClientActiveTexture(GLenum unit);

void FFP::registerTextureFunctions() {
    PROVIDE(glClientActiveTexture);
}

void glClientActiveTexture(GLenum unit) {
    FFPE::States::ClientState::currentTexCoordUnit = unit;
}
