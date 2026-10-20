#include "ffp/draw.hpp"
#include "ffp/lists.hpp"
#include "draw/draw_overrides.hpp"
#include "core/main.hpp"
#include "draw/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>

void Draw::registerDrawOverride() {
    REGISTEROV(glDrawArrays);
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
