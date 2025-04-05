#include "es/ffp.h"
#include "ffp/main.h"
#include "main.h"

#include <GLES/gl.h>

void glBegin(GLenum mode);
void glEnd();

void FFP::registerImmediateFunctions() {
    REGISTER(glBegin);
    REGISTER(glEnd);
}

void glBegin(GLenum mode) {
    intVertexBuffer.clear();
    floatVertexBuffer.clear();

    switch (mode) {
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

    currentPrimitive = mode;
}

void glEnd() {
    if (intVertexBuffer.size() > 0) {
        glDrawArrays(currentPrimitive, 0, intVertexBuffer.size() / 3);
        intVertexBuffer.clear();
    }

    if (floatVertexBuffer.size() > 0) {
        glDrawArrays(currentPrimitive, 0, floatVertexBuffer.size() / 3);
        floatVertexBuffer.clear();
    }

    currentPrimitive = GL_NONE;
}