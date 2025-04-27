#pragma once

#include "es/binding_saver.h"
#include "es/state_tracking.h"
#include "gles20/buffer_tracking.h"
#include "gles20/shader_overrides.h"

#include <GLES3/gl32.h>
#include <memory>
#include <string>
#include <vector>

// Some of the code comes from:
// https://github.com/MobileGL-Dev/MobileGlues/blob/8727ed43fde193ae595d73e84a8991ee771e43e7/src/main/cpp/gl/multidraw.cpp#L418

inline const std::string COMPUTE_BATCHER_GLSL_BASE = R"(#version 320 es
layout(local_size_x = 32) in;

struct DrawCommand {
    uint  firstIndex;
    int   baseVertex;
    uint  prefix;
    uint  padding;
};

layout(std430, binding = 0) readonly buffer Input {
    uint inputElementBuffer[];
};

layout(std430, binding = 1) readonly buffer DrawCommands {
    DrawCommand drawCommands[];
};

layout(std430, binding = 2) writeonly buffer Output {
    uint outputIndices[];
};

void main() {
    uint outputIndex = gl_GlobalInvocationID.x;
    if (outputIndex >= drawCommands[drawCommands.length() - 1].prefix) return;

    int low = 0, high = drawCommands.length() - 1;
    while (low < high) {
        int mid = (low + high) >> 1;
        if (drawCommands[mid].prefix > outputIndex) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    DrawCommand cmd = drawCommands[low];
    uint localIndex = outputIndex - ((low == 0) ? 0u : (drawCommmands[low - 1].prefix));
    uint inputIndex = localIndex + cmd.firstIndex;

    outputIndices[outputIndex] = uint(int(inputElementBuffer[inputIndex]) + cmd.baseVertex);
})";

struct DrawCommand {
    GLuint firstIndex;
    GLint  baseVertex;
    GLuint prefix;
    GLuint padding;
};

inline GLuint getTypeByteSize(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE: return 1;
        case GL_UNSIGNED_SHORT: return 2;
        case GL_UNSIGNED_INT: return 4;
        default: return 0;
    }
}

struct MDElementsBaseVertexBatcher {
    GLuint computeProgram;

    GLuint paramsSSBO;
    GLuint outputIndexSSBO;

    ~MDElementsBaseVertexBatcher() {
        glDeleteProgram(computeProgram);
        glDeleteBuffers(1, &paramsSSBO);
        glDeleteBuffers(1, &outputIndexSSBO);
    }

    void init() {
        computeProgram = glCreateProgram();
        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
        
        const GLchar* castedSource = COMPUTE_BATCHER_GLSL_BASE.c_str();
        OV_glShaderSource(computeShader, 1, &castedSource, nullptr);
        OV_glCompileShader(computeShader);

        glAttachShader(computeProgram, computeShader);
        OV_glLinkProgram(computeProgram);

        glDeleteShader(computeShader);

        glGenBuffers(1, &paramsSSBO);
        glGenBuffers(1, &outputIndexSSBO);
    }

    void batch(
        GLenum mode,
        const GLsizei* count,
        GLenum type,
        const void* const* indices,
        GLsizei drawcount,
        const GLint* basevertex
    ) {
        const GLuint elemSize = getTypeByteSize(type);
        if (drawcount <= 0 || elemSize != 4) return; // force GL_UINT support for now

        /* if (!computeReady || drawcount < 1024) {
            for (GLint i = 0; i < drawcount; ++i) {
                if (count[i] > 0) glDrawElementsBaseVertex(mode, count[i], type, indices[i], basevertex[i]);
            }
        } */
        
        SaveBoundedBuffer sbb(GL_DRAW_INDIRECT_BUFFER);
        OV_glBindBuffer(GL_DRAW_INDIRECT_BUFFER, paramsSSBO);

        int previousSSBOSize = trackedStates->boundBuffers[GL_DRAW_INDIRECT_BUFFER].size / sizeof(DrawCommand);
        if (previousSSBOSize < drawcount) {
            LOGI("Resizing DrawCommands SSBO from %i to %i", previousSSBOSize, drawcount);
            OV_glBufferData(
                GL_DRAW_INDIRECT_BUFFER,
                drawcount * sizeof(DrawCommand),
                nullptr, GL_DYNAMIC_DRAW
            );
        }

        DrawCommand* drawCommands = reinterpret_cast<DrawCommand*>(
            glMapBufferRange(
                GL_DRAW_INDIRECT_BUFFER, 0,
                drawcount * sizeof(DrawCommand),
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
            )
        );

        for (GLsizei i = 0; i < drawcount; ++i) {
            uintptr_t byteOffset = reinterpret_cast<uintptr_t>(indices[i]);
            drawCommands[i].firstIndex = static_cast<GLuint>(byteOffset / elemSize);
            drawCommands[i].baseVertex = basevertex ? basevertex[i] : 0;
            drawCommands[i].prefix = count[i] + ((i == 0) ? 0 : drawCommands[i - 1].prefix);
        }
        GLuint total = drawCommands[drawcount - 1].prefix;

        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

        OV_glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputIndexSSBO);
        OV_glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            total * sizeof(GLuint),
            nullptr, GL_DYNAMIC_DRAW
        );

        SaveBoundedBuffer sbb2(GL_ELEMENT_ARRAY_BUFFER);
        
        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER, 0, sbb2.boundedBuffer
        );
        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER, 1, paramsSSBO
        );
        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER, 2, outputIndexSSBO
        );

        SaveBoundedBuffer sbb3(GL_ARRAY_BUFFER);
        SaveUsedProgram sup;
        OV_glUseProgram(computeProgram);
        glDispatchCompute((total + 31) / 32, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        sup.restore();
        sbb3.restore();
        OV_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outputIndexSSBO);

        glDrawElements(mode, total, type, 0);

        LOGI("done dispatch and drawed!");
    }
};

inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher = std::make_shared<MDElementsBaseVertexBatcher>();
