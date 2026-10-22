#include "ffp/immediate.hpp"
#include "ffp/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glBegin(GLenum mode);
void glEnd();

void FFP::registerImmediateFunctions() {
    FOGLTLOGLES::provide("glBegin", TO_FUNCPTR(FFPE::Rendering::ImmediateMode::begin));
    FOGLTLOGLES::provide("glEnd", TO_FUNCPTR(FFPE::Rendering::ImmediateMode::end));
}
