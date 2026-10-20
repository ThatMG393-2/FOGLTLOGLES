#include "ffp/immediate.hpp"
#include "ffp/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glBegin(GLenum mode);
void glEnd();

void FFP::registerImmediateFunctions() {
    REGISTERREDIR("glBegin", FFPE::Rendering::ImmediateMode::begin);
    REGISTERREDIR("glEnd", FFPE::Rendering::ImmediateMode::end);
}
