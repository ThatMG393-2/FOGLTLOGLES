#include "draw/draw.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl3.h>

void glMultiDrawArrays3(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
void glMultiDrawElements3(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);

void Draw::registerBatchedMultiDraw() {
    LOGI("Nevermind, enjoy a somewhat accelerated glMultiDrawArrays and glMultiDrawElements implementation!");

    // PROVIDE(glMultiDrawArrays, glMultiDrawArrays3);
    // PROVIDE(glMultiDrawElements, glMultiDrawElements3);

    // batcher = MakeAggregateShared<MDElementsBatcher>();
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

// Determine the byte-size of one index, given its GLenum type.
static inline GLsizei getByteSize(GLenum type) {
    switch(type) {
        case GL_UNSIGNED_BYTE:  return sizeof(GLubyte);
        case GL_UNSIGNED_SHORT: return sizeof(GLushort);
        case GL_UNSIGNED_INT:   return sizeof(GLuint);
        default:
            return 0;
    }
}

void glMultiDrawElements3(
    GLenum mode,                 
    const GLsizei* counts,
    GLenum type,
    const void* const* indices,
    GLsizei primcount
) {
    const GLsizei idxSize = getByteSize(type);
    if (primcount <= 0 || idxSize == 0) return;

    const void* mergedBase = indices[0];
    GLsizei mergedCount = counts[0];
    const char* lastPtr = (const char*) indices[0] + counts[0] * idxSize;

    for(GLsizei i = 1; i < primcount; ++i) {
        const char* currBase = (const char*) indices[i];
        GLsizei currCount = counts[i];

        if (currBase == lastPtr) {
            // extend the merged range
            mergedCount += currCount;
            lastPtr = currBase + currCount*idxSize;
        }
        else {
            // flush
            glDrawElements(mode, mergedCount, type, mergedBase);
            
            // new merged range
            mergedBase = indices[i];
            mergedCount = currCount;
            lastPtr = currBase + currCount * idxSize;
        }
    }
    // final flush
    glDrawElements(mode, mergedCount, type, mergedBase);
}

/*
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
    }

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
    glDrawElements(mode, totalCount, type, (void*) 0);
}
*/
