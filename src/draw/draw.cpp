#include "ffp/ffp.hpp"
#include "draw/draw.hpp"
#include "core/core.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>

void Draw::registerDrawOverride() {
    OVERRIDE(glDrawArrays);
}

void OV_glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    if (count <= 0) return;
    if (FFPE::List::addCommand<OV_glDrawArrays>(mode, first, count)) return;
    if (debugEnabled) LOGI("OV_glDrawArrays : mode=%u count=%i", mode, count);

    GLDebugGroup gldg("Draw Arrays");
    switch (mode) {
        case GL_QUADS:
            FFPE::Rendering::Arrays::Quads::drawQuads(count);
            break;

        default:
            FFPE::Rendering::Arrays::drawArrays(mode, first, count);
    }
}

void Draw::init() {
    LOGI("Draw entrypoint!");

    Draw::registerDrawOverride();
    Draw::registerMultiDrawEmulation();
    Draw::registerBatchedMultiDraw();
    Draw::registerBaseVertexFunction();
}
