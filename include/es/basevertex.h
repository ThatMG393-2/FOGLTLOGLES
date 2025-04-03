#pragma once

#include "es/binding_saver.h"
#include "es/state_tracking.h"
#include "gles20/buffer_tracking.h"

#include <vector>
#include <cstdint>
#include <GLES3/gl32.h>
#include <omp.h>

inline GLint getTypeSize(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE:  return 1;
        case GL_UNSIGNED_SHORT: return 2;
        case GL_UNSIGNED_INT:   return 4;
        default:                return 0;
    }
}

template<typename T>
inline void draw(
    GLenum mode,
    GLsizei totalCount,
    const GLsizei* counts,
    GLenum type,
    const void* const* indices,
    GLsizei drawcount,
    const GLint* basevertex
) {
    std::vector<T> combinedIndices(totalCount);

    GLsizei offset = 0;
    #pragma omp parallel for reduction(inscan, +:offset)
    for (GLsizei i = 0; i < drawcount; ++i) {
        // Save the current offset; this is where our data should be written.
        GLsizei currentOffset = offset;
        // Increment offset by the number of indices in this draw.
        offset += counts[i];
        // The scan directive ensures that each iteration sees the correct prefix sum.
        #pragma omp scan inclusive(offset)
        {
            // Cast the pointer to the correct type.
            const T* idxData = reinterpret_cast<const T*>(indices[i]);
            for (GLsizei j = 0; j < counts[i]; ++j) {
                // Adjust each index by the corresponding basevertex.
                combinedIndices[currentOffset + j] = idxData[j] + basevertex[i];
            }
        }
    }

    OV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 combinedIndices.size() * sizeof(T),
                 combinedIndices.data(),
                 GL_STATIC_DRAW);

    // Since the indices are pre-adjusted, we pass 0 as the base vertex.
    glDrawElementsBaseVertex(mode, totalCount, type, 0, 0);
}

struct MDElementsBaseVertexBatcher {
    GLuint ebo;

    MDElementsBaseVertexBatcher() {
        glGenBuffers(1, &ebo);
    }

    ~MDElementsBaseVertexBatcher() {
        glDeleteBuffers(1, &ebo);
    }

    // 'indices' is an array of pointers to index data.
    // 'counts' holds the number of indices per draw call.
    // 'drawcount' is the number of draws.
    // 'basevertex' holds the base vertex offset for each draw.
    void batch(GLenum mode,
               const GLsizei* counts,
               GLenum type,
               const void* const* indices,
               GLsizei drawcount,
               const GLint* basevertex) {
        if (drawcount <= 0) return;

        GLint typeSize = getTypeSize(type);
        if (typeSize == 0) return;

        GLsizei totalCount = 0;
        #pragma omp parallel for reduction(+:totalCount)
        for (GLsizei i = 0; i < drawcount; ++i) {
            totalCount += counts[i];
        }

        SaveBoundedBuffer sbb(GL_ELEMENT_ARRAY_BUFFER);
        switch (type) {
            case GL_UNSIGNED_BYTE:
                draw<GLubyte>(mode, totalCount, counts, type, indices, drawcount, basevertex);
            break;

            case GL_UNSIGNED_SHORT:
                draw<GLushort>(mode, totalCount, counts, type, indices, drawcount, basevertex);
            break;

            case GL_UNSIGNED_INT:
                draw<GLuint>(mode, totalCount, counts, type, indices, drawcount, basevertex);
            break;
        }
    }
};


inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher;
