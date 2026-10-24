#include "ffp/list/list.hpp"
#include "ffp/manager/manager.hpp"
#include "state/state.hpp"
#include "core/core.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <utility>

static void HK_glBindFramebuffer(NEXT(glBindFramebuffer) next, GLenum target, GLuint framebuffer);
static void HK_glGenFramebuffers(NEXT(glGenFramebuffers) next, GLsizei n, GLuint* framebuffers);
static void HK_glDeleteFramebuffers(NEXT(glDeleteFramebuffers) next, GLsizei n, const GLuint* framebuffers);
static void HK_glBindTexture(NEXT(glBindTexture) next, GLenum target, GLuint texture);
static void HK_glDeleteTextures(NEXT(glDeleteTextures) next, GLsizei n, const GLuint *textures);
static void HK_glActiveTexture(NEXT(glActiveTexture) next, GLuint texture);
static void HK_glBindBuffer(NEXT(glBindBuffer) next, GLenum target, GLuint buffer);
static void HK_glBufferData(NEXT(glBufferData) next, GLenum target, GLsizeiptr size, const void* data, GLenum usage);
static void HK_glBindVertexArray(NEXT(glBindVertexArray) next, GLuint array);
static void HK_glDeleteVertexArrays(NEXT(glDeleteVertexArrays) next, GLsizei n, const GLuint* arrays);
static void HK_glBindRenderbuffer(NEXT(glBindRenderbuffer) next, GLenum target, GLuint renderbuffer);
static void HK_glDeleteRenderbuffers(NEXT(glDeleteRenderbuffers) next, GLsizei n, const GLuint *renderbuffers);
static void HK_glRenderbufferStorage(NEXT(glRenderbufferStorage) next, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
static void HK_glEnable(NEXT(glEnable) next, GLenum cap);
static void HK_glDisable(NEXT(glDisable) next, GLenum cap);
static void HK_glUseProgram(NEXT(glUseProgram) next, GLuint program);
static void HK_glDeleteProgram(NEXT(glDeleteProgram) next, GLuint program);

void State::init() {
    // registered first so these see every call, before any module swallows it
    HOOK(glEnable);
    HOOK(glDisable);
    HOOK(glUseProgram);
    HOOK(glDeleteProgram);
    HOOK(glRenderbufferStorage);

    PROVIDE(glBindFramebuffer);
    HOOK(glBindFramebuffer);
    PROVIDE(glGenFramebuffers);
    HOOK(glGenFramebuffers);
    PROVIDE(glDeleteFramebuffers); // deleting tracked framebuffers
    HOOK(glDeleteFramebuffers);

    PROVIDE(glBindTexture);
    HOOK(glBindTexture);
    PROVIDE(glDeleteTextures); // deleting tracked iformat
    HOOK(glDeleteTextures);
    PROVIDE(glActiveTexture);
    HOOK(glActiveTexture);

    PROVIDE(glBindBuffer);
    HOOK(glBindBuffer);
    PROVIDE(glBufferData);
    HOOK(glBufferData);

    PROVIDE(glBindVertexArray);
    HOOK(glBindVertexArray);
    PROVIDE(glDeleteVertexArrays);
    HOOK(glDeleteVertexArrays);

    PROVIDE(glBindRenderbuffer);
    HOOK(glBindRenderbuffer);
    PROVIDE(glDeleteRenderbuffers); // deleting tracked iformat
    HOOK(glDeleteRenderbuffers);

    trackedStates = std::make_shared<TrackedStates>();
}

static void HK_glBindFramebuffer(NEXT(glBindFramebuffer) next, GLenum target, GLuint framebuffer) {
    next(target, framebuffer);

    switch (target) {
        case GL_FRAMEBUFFER:
            trackedStates->framebufferState.boundDrawFramebuffer = trackedStates->framebufferState.boundReadFramebuffer = framebuffer;
        break;
        case GL_DRAW_FRAMEBUFFER: trackedStates->framebufferState.boundDrawFramebuffer = framebuffer; break;
        case GL_READ_FRAMEBUFFER: trackedStates->framebufferState.boundReadFramebuffer = framebuffer; break;
    }

    trackedStates->framebufferState.recentlyBoundFramebuffer = std::make_pair(target, framebuffer);
}


static void HK_glGenFramebuffers(NEXT(glGenFramebuffers) next, GLsizei n, GLuint* framebuffers) {
    next(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.insert({ framebuffers[i], std::make_shared<Framebuffer>() });
    }
}

static void HK_glDeleteFramebuffers(NEXT(glDeleteFramebuffers) next, GLsizei n, const GLuint* framebuffers) {
    next(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.erase(framebuffers[i]);
    }
}

static void HK_glBindTexture(NEXT(glBindTexture) next, GLenum target, GLuint texture) {
    if (FFPE::List::addCommand<CHAIN(glBindTexture)>(target, texture)) return;
    next(target, texture);

    trackedStates->activeTextureState->bindTextureToTarget(target, texture);
    if (debugEnabled) LOGI("glBindTexture : target=%u texture=%u", target, texture);
}

static void HK_glDeleteTextures(NEXT(glDeleteTextures) next, GLsizei n, const GLuint *textures) {
    next(n, textures);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->activeTextureState->textureInternalFormats.erase(textures[i]);
    }
}

static void HK_glActiveTexture(NEXT(glActiveTexture) next, GLuint texture) {
    next(texture);

    trackedStates->setActiveTextureUnit(texture);
}

static void HK_glBindBuffer(NEXT(glBindBuffer) next, GLenum target, GLuint buffer) {
    next(target, buffer);

    // LOGI("BindBuffer: target %i, buffer %u", target, buffer);

    trackedStates->boundBuffers[target].target = target;
    trackedStates->boundBuffers[target].buffer = buffer;

    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        trackedStates->vertexArrayElementBuffers[trackedStates->boundVertexArray] = buffer;
    }
}

static void HK_glBindVertexArray(NEXT(glBindVertexArray) next, GLuint array) {
    next(array);

    trackedStates->boundVertexArray = array;
    trackedStates->boundBuffers[GL_ELEMENT_ARRAY_BUFFER].target = GL_ELEMENT_ARRAY_BUFFER;
    trackedStates->boundBuffers[GL_ELEMENT_ARRAY_BUFFER].buffer = trackedStates->vertexArrayElementBuffers[array];
}

static void HK_glDeleteVertexArrays(NEXT(glDeleteVertexArrays) next, GLsizei n, const GLuint* arrays) {
    next(n, arrays);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->vertexArrayElementBuffers.erase(arrays[i]);

        if (trackedStates->boundVertexArray == arrays[i]) {
            trackedStates->boundVertexArray = 0;
            trackedStates->boundBuffers[GL_ELEMENT_ARRAY_BUFFER].buffer = trackedStates->vertexArrayElementBuffers[0];
        }
    }
}

static void HK_glBufferData(NEXT(glBufferData) next, GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    next(target, size, data, usage);

    // LOGI("BufferData: target %i, size %ld, usage %i", target, size, usage);
    trackedStates->boundBuffers[target].size = size;
}

static void HK_glBindRenderbuffer(NEXT(glBindRenderbuffer) next, GLenum target, GLuint renderbuffer) {
    next(target, renderbuffer);

    trackedStates->boundRenderbuffer = renderbuffer;
}

static void HK_glDeleteRenderbuffers(NEXT(glDeleteRenderbuffers) next, GLsizei n, const GLuint *renderbuffers) {
    next(n, renderbuffers);

    for (GLint i = 0; i < n; ++i) {
        trackedStates->renderbufferInternalFormats.erase(renderbuffers[i]);
    }
}

static void HK_glRenderbufferStorage(NEXT(glRenderbufferStorage) next, GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    next(target, internalformat, width, height);

    trackedStates->renderbufferInternalFormats.insert({ trackedStates->boundRenderbuffer, internalformat });
}

static void HK_glEnable(NEXT(glEnable) next, GLenum cap) {
    if (FFPE::List::addCommand<CHAIN(glEnable)>(cap)) return;
    next(cap);

    trackedStates->capabilitiesState[cap] = true;
}

static void HK_glDisable(NEXT(glDisable) next, GLenum cap) {
    if (FFPE::List::addCommand<CHAIN(glDisable)>(cap)) return;
    next(cap);

    trackedStates->capabilitiesState[cap] = false;
}

static void HK_glUseProgram(NEXT(glUseProgram) next, GLuint program) {
    next(program);

    trackedStates->currentlyUsedProgram = program;
}

static void HK_glDeleteProgram(NEXT(glDeleteProgram) next, GLuint program) {
    next(program);

    if (trackedStates->currentlyUsedProgram == program) {
        trackedStates->currentlyUsedProgram = 0;
    }
}
