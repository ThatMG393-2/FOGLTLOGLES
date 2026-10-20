#include "draw/basevertex.hpp"
#include "draw/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex);

void Draw::registerBaseVertexFunction() {
    PROVIDE(glMultiDrawElementsBaseVertex);

    batcher->init();
}

void glMultiDrawElementsBaseVertex(
    GLenum mode,
    const GLsizei* count,
    GLenum type,
    const void* const* indices,
    GLsizei drawcount,
    const GLint* basevertex
) {
    batcher->batch(
        mode,
        count,
        type,
        indices,
        drawcount,
        basevertex
    ); 
}
