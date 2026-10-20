#include "ffp/lists.hpp"
#include "state/state_tracking.hpp"
#include "core/main.hpp"
#include "state/buffer_tracking.hpp"
#include "state/framebuffer_tracking.hpp"
#include "state/main.hpp"
#include "state/texture_tracking.hpp"
#include "state/renderbuffer_tracking.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES2/gl2.h>
#include <utility>

static void glBindFramebufferHook(NEXT(glBindFramebuffer) next, GLenum target, GLuint framebuffer);
static void glGenFramebuffersHook(NEXT(glGenFramebuffers) next, GLsizei n, GLuint* framebuffers);
static void glDeleteFramebuffersHook(NEXT(glDeleteFramebuffers) next, GLsizei n, const GLuint* framebuffers);
static void glBindTextureHook(NEXT(glBindTexture) next, GLenum target, GLuint texture);
static void glDeleteTexturesHook(NEXT(glDeleteTextures) next, GLsizei n, const GLuint *textures);
static void glActiveTextureHook(NEXT(glActiveTexture) next, GLuint texture);
static void glBindBufferHook(NEXT(glBindBuffer) next, GLenum target, GLuint buffer);
static void glBufferDataHook(NEXT(glBufferData) next, GLenum target, GLsizeiptr size, const void* data, GLenum usage);
static void glBindRenderbufferHook(NEXT(glBindRenderbuffer) next, GLenum target, GLuint renderbuffer);
static void glDeleteRenderbuffersHook(NEXT(glDeleteRenderbuffers) next, GLsizei n, const GLuint *renderbuffers);

void State::init() {
    PROVIDE(glBindFramebuffer);
    HOOK(glBindFramebuffer, glBindFramebufferHook);
    PROVIDE(glGenFramebuffers);
    HOOK(glGenFramebuffers, glGenFramebuffersHook);
    PROVIDE(glDeleteFramebuffers); // deleting tracked framebuffers
    HOOK(glDeleteFramebuffers, glDeleteFramebuffersHook);

    PROVIDE(glBindTexture);
    HOOK(glBindTexture, glBindTextureHook);
    PROVIDE(glDeleteTextures); // deleting tracked iformat
    HOOK(glDeleteTextures, glDeleteTexturesHook);
    PROVIDE(glActiveTexture);
    HOOK(glActiveTexture, glActiveTextureHook);

    PROVIDE(glBindBuffer);
    HOOK(glBindBuffer, glBindBufferHook);
    PROVIDE(glBufferData);
    HOOK(glBufferData, glBufferDataHook);

    PROVIDE(glBindRenderbuffer);
    HOOK(glBindRenderbuffer, glBindRenderbufferHook);
    PROVIDE(glDeleteRenderbuffers); // deleting tracked iformat
    HOOK(glDeleteRenderbuffers, glDeleteRenderbuffersHook);

    trackedStates = std::make_shared<TrackedStates>();
}

static void glBindFramebufferHook(NEXT(glBindFramebuffer) next, GLenum target, GLuint framebuffer) {
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


static void glGenFramebuffersHook(NEXT(glGenFramebuffers) next, GLsizei n, GLuint* framebuffers) {
    next(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.insert({ framebuffers[i], std::make_shared<Framebuffer>() });
    }
}

static void glDeleteFramebuffersHook(NEXT(glDeleteFramebuffers) next, GLsizei n, const GLuint* framebuffers) {
    next(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.erase(framebuffers[i]);
    }
}

static void glBindTextureHook(NEXT(glBindTexture) next, GLenum target, GLuint texture) {
    if (FFPE::List::addCommand<CHAIN(glBindTexture)>(target, texture)) return;
    next(target, texture);

    trackedStates->activeTextureState->bindTextureToTarget(target, texture);
    if (debugEnabled) LOGI("glBindTexture : target=%u texture=%u", target, texture);
}

static void glDeleteTexturesHook(NEXT(glDeleteTextures) next, GLsizei n, const GLuint *textures) {
    next(n, textures);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->activeTextureState->textureInternalFormats.erase(textures[i]);
    }
}

static void glActiveTextureHook(NEXT(glActiveTexture) next, GLuint texture) {
    next(texture);

    trackedStates->setActiveTextureUnit(texture);
}

static void glBindBufferHook(NEXT(glBindBuffer) next, GLenum target, GLuint buffer) {
    next(target, buffer);

    // LOGI("BindBuffer: target %i, buffer %u", target, buffer);

    trackedStates->boundBuffers[target].target = target;
    trackedStates->boundBuffers[target].buffer = buffer;
}

static void glBufferDataHook(NEXT(glBufferData) next, GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    next(target, size, data, usage);

    // LOGI("BufferData: target %i, size %ld, usage %i", target, size, usage);
    trackedStates->boundBuffers[target].size = size;
}

static void glBindRenderbufferHook(NEXT(glBindRenderbuffer) next, GLenum target, GLuint renderbuffer) {
    next(target, renderbuffer);

    trackedStates->boundRenderbuffer = renderbuffer;
}

static void glDeleteRenderbuffersHook(NEXT(glDeleteRenderbuffers) next, GLsizei n, const GLuint *renderbuffers) {
    next(n, renderbuffers);

    for (GLint i = 0; i < n; ++i) {
        trackedStates->renderbufferInternalFormats.erase(renderbuffers[i]);
    }
}
