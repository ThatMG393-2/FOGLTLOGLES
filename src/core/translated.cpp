#include "core/core.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glClearDepth(double d);
void glDepthRange(double near, double far);

void Core::registerTranslatedFunctions() {
    PROVIDE(glClearDepth);
    PROVIDE(glDepthRange);
}

void glClearDepth(double d) {
    glClearDepthf(static_cast<float>(d));
}

void glDepthRange(double near, double far) {
    glDepthRangef(static_cast<float>(near), static_cast<float>(far));
}