#include "draw/draw.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex);

void Draw::registerBaseVertexFunction() {
    PROVIDE(glMultiDrawElementsBaseVertex);

    baseVertexBatcher->init();
}

void glMultiDrawElementsBaseVertex(
    GLenum mode,
    const GLsizei* count,
    GLenum type,
    const void* const* indices,
    GLsizei drawcount,
    const GLint* basevertex
) {
    baseVertexBatcher->batch(
        mode,
        count,
        type,
        indices,
        drawcount,
        basevertex
    ); 
}
