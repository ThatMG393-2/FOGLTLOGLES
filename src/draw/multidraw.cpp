#include "draw/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES2/gl2.h>

void glMultiDrawArrays2(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
void glMultiDrawElements2(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);

void Draw::registerMultiDrawEmulation() {
    LOGI("Using generic glMultiDrawArrays and glMultiDrawElements implementation.");
    REGISTERREDIR(glMultiDrawArrays, glMultiDrawArrays2);
    REGISTERREDIR(glMultiDrawElements, glMultiDrawElements2);
}

void glMultiDrawArrays2(
    GLenum mode,
    const GLint* first,
    const GLsizei* count,
    GLsizei drawcount
) {
    if (drawcount == 0) return;

    for (GLsizei i = 0; i < drawcount; ++i) {
        if (count[i] > 0) glDrawArrays(mode, first[i], count[i]);
    }
}

void glMultiDrawElements2(
    GLenum mode,
    const GLsizei* count,
    GLenum type,
    const void* const* indices,
    GLsizei drawcount
) {
    if (drawcount == 0) return;
    
    for (GLsizei i = 0; i < drawcount; ++i) {
        if (count[i] > 0) glDrawElements(mode, count[i], type, indices[i]);
    }
}
