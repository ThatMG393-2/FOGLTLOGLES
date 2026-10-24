#include "ffp/ffp.hpp"
#include "state/state.hpp"
#include "common/enums.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glAlphaFunc(GLenum op, GLclampf threshold);

void FFP::registerAlphaTestFunctions() {
    PROVIDE(glAlphaFunc);
}

void glAlphaFunc(GLenum op, GLclampf threshold) {
    if (!trackedStates->isCapabilityEnabled(GL_ALPHA_TEST)) return;
    if (FFPE::List::addCommand<glAlphaFunc>(op, threshold)) return;

    FFPE::States::AlphaTest::op = op;
    FFPE::States::AlphaTest::threshold = threshold;

    FFPE::States::Manager::invalidateCurrentState();
}
