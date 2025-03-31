#pragma once

#include "es/binding_saver.h"
#include "gles20/buffer_tracking.h"

#include <GLES3/gl32.h>
#include <vector>
#include <cstring>
#include <memory>
#include <algorithm>
#include <omp.h>

inline GLint getTypeSize(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE:  return 1;
        case GL_UNSIGNED_SHORT: return 2;
        case GL_UNSIGNED_INT:   return 4;
        default:                return 0;
    }
}

struct indirect_pass_t {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLint baseVertex;
    GLuint reservedMustBeZero;
};

struct MDElementsBaseVertexBatcher {
    GLuint indirectBuffers[2]; // Double buffering
    int currentBuffer = 0;     // Buffer swap index

    MDElementsBaseVertexBatcher() {
        glGenBuffers(2, indirectBuffers);

        SaveBoundedBuffer sbb(GL_DRAW_INDIRECT_BUFFER);
        for (int i = 0; i < 2; i++) {
            OV_glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffers[i]);
            glBufferData(GL_DRAW_INDIRECT_BUFFER, 512 * sizeof(indirect_pass_t), nullptr, GL_STREAM_DRAW);
        }
    }

    ~MDElementsBaseVertexBatcher() {
        glDeleteBuffers(2, indirectBuffers);
    }

    void batch(
        GLenum mode,
        const GLsizei* counts,
        GLenum type,
        const void* const* indices,
        GLsizei drawcount,
        const GLint* basevertex
    ) {
        if (drawcount <= 0) return;
        
        GLint typeSize = getTypeSize(type);
        if (typeSize == 0) return;

        // Swap buffers
        currentBuffer = (currentBuffer + 1) % 2;
        GLuint buffer = indirectBuffers[currentBuffer];

        SaveBoundedBuffer sbb(GL_DRAW_INDIRECT_BUFFER);
        OV_glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer);

        // Map with invalidation to prevent stalls
        void* mappedBuffer = glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 
                                              drawcount * sizeof(indirect_pass_t),
                                              GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        if (!mappedBuffer) return;

        // Populate commands in parallel
        indirect_pass_t* commands = static_cast<indirect_pass_t*>(mappedBuffer);
        
        #pragma omp parallel for schedule(static, 4)
        for (GLsizei i = 0; i < drawcount; ++i) {
            uintptr_t indicesPtr = reinterpret_cast<uintptr_t>(indices[i]);
            commands[i].count = counts[i];
            commands[i].instanceCount = 1;
            commands[i].firstIndex = static_cast<GLuint>(indicesPtr / typeSize);
            commands[i].baseVertex = basevertex[i];
            commands[i].reservedMustBeZero = 0;
        }

        // Unmap
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

        // Execute batched indirect draw
        #pragma omp parallel for schedule(static, 4) ordered
        for (GLsizei i = 0; i < drawcount; ++i) {
            #pragma omp ordered
            {
                glDrawElementsIndirect(mode, type, reinterpret_cast<const void*>(i * sizeof(indirect_pass_t)));
            }
        }
    }
};

// Inline global shared pointer
inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher;
