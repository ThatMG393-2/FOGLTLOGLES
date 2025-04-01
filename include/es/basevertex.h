#pragma once

#include "es/binding_saver.h"
#include "es/state_tracking.h"
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
    GLuint indirectBuffer;
    
    MDElementsBaseVertexBatcher() {
        glGenBuffers(1, &indirectBuffer);

        SaveBoundedBuffer sbb(GL_DRAW_INDIRECT_BUFFER);
        OV_glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffers);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, 512 * sizeof(indirect_pass_t), nullptr, GL_STREAM_DRAW);
    }

    ~MDElementsBaseVertexBatcher() {
        glDeleteBuffers(1, &indirectBuffer);
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

        if (trackedStates->boundBuffers[GL_ELEMENT_ARRAY_BUFFER_BINDING] == 0) return;
        /* void* mappedBuffer = glMapBufferRange(
            GL_DRAW_INDIRECT_BUFFER, 0, 
            drawcount * sizeof(indirect_pass_t),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
        );
        
        if (!mappedBuffer) return; */
        indirect_pass_t* commands[drawcount]; // = static_cast<indirect_pass_t*>(mappedBuffer)
        
        #pragma omp parallel for schedule(static, drawcount / omp_get_max_threads()) num_threads(omp_get_max_threads())
        for (GLsizei i = 0; i < drawcount; ++i) {
            commands[i] = {
                static_cast<GLuint>(counts[i]),
                1, // instanceCount is always 1
                static_cast<GLuint>(reinterpret_cast<uintptr_t>(indices[i]) / typeSize),   
                basevertex[i], 
                0 // reservedMustBeZero
            };
        }
        
        // glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        SaveBoundedBuffer sbb(GL_DRAW_INDIRECT_BUFFER);
        OV_glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, (long) sizeof(indirect_pass_t) * drawcount, commands, GL_STREAM_DRAW);

        for (GLsizei i = 0; i < drawcount; ++i) {
            glDrawElementsIndirect(mode, type, reinterpret_cast<const void*>(i * sizeof(indirect_pass_t)));
        }
    }
};

inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher;
