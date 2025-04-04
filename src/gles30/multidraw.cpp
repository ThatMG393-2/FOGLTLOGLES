#include "es/multidraw.h"
#include "es/binding_saver.h"
#include "gles30/main.h"
#include "main.h"
#include "utils/log.h"
#include "utils/pointers.h"

#include <GLES3/gl3.h>
#include <omp.h>

void glMultiDrawArrays3(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
void glMultiDrawElements3(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);

void GLES30::registerMultiDrawEmulation() {
    LOGI("Nevermind, enjoy a somewhat accelerated glMultiDrawArrays and glMultiDrawElements implementation!");

    REGISTERREDIR(glMultiDrawArrays, glMultiDrawArrays3);
    REGISTERREDIR(glMultiDrawElements, glMultiDrawElements3);

    batcher = MakeAggregateShared<MDElementsBatcher>();
}

void glMultiDrawArrays3(
    GLenum mode,
    const GLint* first,
    const GLsizei* count,
    GLsizei drawcount
) {
    if (drawcount <= 0) return;

    GLint mergedFirst = first[0];
    GLsizei mergedCount = count[0];

    for (GLint i = 1; i < drawcount; ++i) {
        if (first[i] == (mergedFirst + mergedCount)) {
            mergedCount += count[i];
        } else {
            glDrawArrays(mode, mergedFirst, mergedCount);

            mergedFirst = first[i];
            mergedCount = count[i];
        }
    }

    glDrawArrays(mode, mergedFirst, mergedCount);
}

void glMultiDrawElements3(
    GLenum mode,
    const GLsizei* count,
    GLenum type,
    const void* const* indices,
    GLsizei primcount
) {
    if (primcount <= 0) return;
    // if (primcount > 32) {
        for (GLsizei i = 0; i < primcount; ++i) {
            if (count[i] > 0) glDrawElements(mode, count[i], type, indices[i]);
        }
        return;
    /* }

    GLsizei threadNum = std::min(omp_get_max_threads(), std::max(1, primcount / 64));

    GLint typeSize = getTypeSize(type);
    if (typeSize == 0) return; // Unsupported type
    
    GLsizei totalCount = 0;
    #pragma omp parallel for \
        if(primcount > 512) \
        schedule(static, primcount / threadNum) \
        num_threads(threadNum) \
        reduction(+:totalCount)
    for (GLsizei i = 0; i < primcount; ++i) totalCount += count[i];
    if (totalCount == 0) return;

    SaveBoundedBuffer sbb(GL_ELEMENT_ARRAY_BUFFER);

    batcher->batch(totalCount, typeSize, primcount, count, indices, sbb);
    glDrawElements(mode, totalCount, type, (void*) 0); */
}
