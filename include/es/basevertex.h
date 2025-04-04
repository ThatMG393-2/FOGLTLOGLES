#pragma once

#include "utils/log.h"
#include <GLES3/gl32.h>
#include <omp.h>
#include <vector>

inline GLint getEnumByTypeSize(size_t size) {
    switch (size) {
        case 1: return GL_UNSIGNED_BYTE;
        case 2: return GL_UNSIGNED_SHORT;
        case 4: // passthrough
        default: return GL_UNSIGNED_INT;
    }
}

template<typename T>
inline std::vector<T> mergeIndices(
    const GLsizei* count,
    const void* const* indices,
    GLsizei drawcount,
    const GLint* basevertex
) {
    GLsizei totalCount = 0;
    #pragma omp parallel for reduction(+:totalCount)
    for (GLsizei i = 0; i < drawcount; ++i) {
        totalCount += count[i];
    }
    LOGI("reserve %d for mergedIndices", totalCount);

    std::vector<T> mergedIndices;
    mergedIndices.reserve(totalCount);

    for (GLsizei i = 0; i < drawcount; ++i) {
        const T* indexData = static_cast<const T*>(indices[i]);
        for (GLsizei j = 0; j < count[i]; ++j) {
            mergedIndices.push_back(indexData[j] + basevertex[i]);
        }
    }

    return mergedIndices;
}

struct MDElementsBaseVertexBatcher {
    MDElementsBaseVertexBatcher() {
    }

    ~MDElementsBaseVertexBatcher() {
    }

    void batch(
        GLenum mode,
        const GLsizei* count,
        GLenum type,
        const void* const* indices, // array of pointers to indices array
        GLsizei drawcount,
        const GLint* basevertex
    ) {
        if (!drawcount) return;
        
        // plan:                           the second arr in the indices
        // merge indices                            |  |  |
        // [0, 1, 2] | [0, 1, 2] turns to [0, 1, 2, 3, 4, 5]
        //              |  |  |
        //           the second arr
        //
        // indices (const void* const*):
        // [ptr1, ptr2]
        // ptr1: [0, 1, 2]
        // ptr2: [0, 1, 2]
        //
        // glDrawElementsBaseVertex(.., .., .., indices[i], ..);
        //                                              |
        //                                        array pointer
        //                                         const void*

        switch (type) {
            case GL_UNSIGNED_BYTE: {
                auto mergedIndices = mergeIndices<GLubyte>(count, indices, drawcount, basevertex);
                glDrawElementsBaseVertex(
                    mode,
                    mergedIndices.size(),
                    type,
                    reinterpret_cast<const void*>(
                        const_cast<const GLubyte*>(mergedIndices.data())
                    ),
                    0
                );
                break;
            }
            case GL_UNSIGNED_SHORT: {
                auto mergedIndices = mergeIndices<GLushort>(count, indices, drawcount, basevertex);
                glDrawElementsBaseVertex(
                    mode,
                    mergedIndices.size(),
                    type,
                    reinterpret_cast<const void*>(
                        const_cast<const GLushort*>(mergedIndices.data())
                    ),
                    0
                );
                break;
            }
            case GL_UNSIGNED_INT: {
                auto mergedIndices = mergeIndices<GLuint>(count, indices, drawcount, basevertex);
                glDrawElementsBaseVertex(
                    mode,
                    mergedIndices.size(),
                    type,
                    reinterpret_cast<const void*>(
                        const_cast<const GLuint*>(mergedIndices.data())
                    ),
                    0
                );
                break;
            }
            default:
                return;
        }
    }
};


inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher;
