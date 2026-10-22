#pragma once

#include "common/raii_helpers.hpp"
#include "shader/overrides.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Some of the code comes from:
// https://github.com/MobileGL-Dev/MobileGlues/blob/8727ed43fde193ae595d73e84a8991ee771e43e7/src/main/cpp/gl/multidraw.cpp#L418
inline const std::string COMPUTE_BATCHER_GLSL_BASE = R"(#version 310 es
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer Input {
    uint inputElementBuffer[];
};

layout(std430, binding = 1) readonly buffer Offsets {
    uint offsets[];
};

layout(std430, binding = 2) readonly buffer BaseVertices {
    int baseVertices[];
};

layout(std430, binding = 3) readonly buffer PrefixSums {
    uint prefixSums[];
};

layout(std430, binding = 4) writeonly buffer Output {
    uint outputIndices[];
};

void main() {
    uint outputIndex = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * 64u) + gl_GlobalInvocationID.x;
    if (outputIndex >= prefixSums[prefixSums.length() - 1]) {
        return;
    }

    int low = 0, high = prefixSums.length() - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (prefixSums[mid] > outputIndex) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    uint localIndex = outputIndex - ((low == 0) ? 0u : (prefixSums[low - 1]));
    uint inputIndex = localIndex + (offsets[low] / 4u);

    outputIndices[outputIndex] = uint(int(inputElementBuffer[inputIndex]) + baseVertices[low]);
})";

struct MDElementsBaseVertexBatcher {
    GLuint computeProgram = 0;

    GLuint offsetsSSBO = 0;
    GLuint baseVerticesSSBO = 0;
    GLuint prefixSSBO = 0;
    GLuint outputIndexSSBO = 0;

    GLint maxGroupsX = 65535;
    bool computeReady = false;

    ~MDElementsBaseVertexBatcher() {
        glDeleteProgram(computeProgram);
        glDeleteBuffers(1, &offsetsSSBO);
        glDeleteBuffers(1, &baseVerticesSSBO);
        glDeleteBuffers(1, &prefixSSBO);
        glDeleteBuffers(1, &outputIndexSSBO);
    }

    void init() {
        LOGI("MDElementsBaseVertexBatcher init!");

        computeProgram = glCreateProgram();
        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

        const GLchar* castedSource = COMPUTE_BATCHER_GLSL_BASE.c_str();
        CALL(glShaderSource, computeShader, 1, &castedSource, nullptr);
        CALL(glCompileShader, computeShader);

        glAttachShader(computeProgram, computeShader);
        CALL(glLinkProgram, computeProgram);

        glDeleteShader(computeShader);

        GLint linked = GL_FALSE;
        glGetProgramiv(computeProgram, GL_LINK_STATUS, &linked);
        computeReady = (linked == GL_TRUE);
        if (!computeReady) {
            LOGI("MDElementsBaseVertexBatcher compute program failed to link, using fallback");
            return;
        }

        GLint limit = 0;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &limit);
        if (limit > 0) maxGroupsX = limit;

        glGenBuffers(1, &offsetsSSBO);
        glGenBuffers(1, &baseVerticesSSBO);
        glGenBuffers(1, &prefixSSBO);
        glGenBuffers(1, &outputIndexSSBO);
    }

    void drawFallback(
        GLenum mode,
        const GLsizei* counts,
        GLenum type,
        const void* const* indices,
        GLsizei drawcount,
        const GLint* basevertex
    ) {
        for (GLsizei i = 0; i < drawcount; ++i) {
            if (counts[i] > 0) glDrawElementsBaseVertex(mode, counts[i], type, indices[i], basevertex[i]);
        }
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

        if (
            !computeReady ||
            type != GL_UNSIGNED_INT ||
            trackedStates->boundBuffers[GL_ELEMENT_ARRAY_BUFFER].buffer == 0
        ) {
            drawFallback(mode, counts, type, indices, drawcount, basevertex);
            return;
        }

        std::vector<GLuint> offsets(drawcount);
        std::vector<GLuint> prefix(drawcount);
        GLuint total = 0;
        for (GLsizei i = 0; i < drawcount; ++i) {
            offsets[i] = static_cast<GLuint>(reinterpret_cast<uintptr_t>(indices[i]));
            total += static_cast<GLuint>(std::max(counts[i], 0));
            prefix[i] = total;
        }
        if (total == 0) return;

        SaveBoundedBuffer elementArraySaver(GL_ELEMENT_ARRAY_BUFFER);
        SaveBoundedBuffer shaderStorageSaver(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetsSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            drawcount * sizeof(GLuint),
            offsets.data(), GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, baseVerticesSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            drawcount * sizeof(GLint),
            basevertex, GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            drawcount * sizeof(GLuint),
            prefix.data(), GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputIndexSSBO);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(total) * sizeof(GLuint),
            nullptr, GL_DYNAMIC_DRAW
        );

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, elementArraySaver.saved());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, offsetsSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, baseVerticesSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, prefixSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, outputIndexSSBO);

        {
            SaveUsedProgram currentProgramSaver;

            const GLuint groups = (total + 63) / 64;
            const GLuint groupsX = std::min<GLuint>(groups, static_cast<GLuint>(maxGroupsX));
            const GLuint groupsY = (groups + groupsX - 1) / groupsX;

            glUseProgram(computeProgram);
            glDispatchCompute(groupsX, groupsY, 1);
            glMemoryBarrier(GL_ELEMENT_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outputIndexSSBO);
        glDrawElements(mode, static_cast<GLsizei>(total), type, nullptr);
    }
};

inline std::shared_ptr<MDElementsBaseVertexBatcher> batcher = std::make_shared<MDElementsBaseVertexBatcher>();
