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

void State::init() {
    REGISTEROV(glBindFramebuffer);
    REGISTEROV(glGenFramebuffers);
    REGISTEROV(glDeleteFramebuffers); // deleting tracked framebuffers

    REGISTEROV(glBindTexture);
    REGISTEROV(glDeleteTextures); // deleting tracked iformat
    REGISTEROV(glActiveTexture);

    REGISTEROV(glBindBuffer);
    REGISTEROV(glBufferData);

    REGISTEROV(glBindRenderbuffer);
    REGISTEROV(glDeleteRenderbuffers); // deleting tracked iformat

    trackedStates = std::make_shared<TrackedStates>();
}

void OV_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    glBindFramebuffer(target, framebuffer);

    switch (target) {
        case GL_FRAMEBUFFER:
            trackedStates->framebufferState.boundDrawFramebuffer = trackedStates->framebufferState.boundReadFramebuffer = framebuffer;
        break;
        case GL_DRAW_FRAMEBUFFER: trackedStates->framebufferState.boundDrawFramebuffer = framebuffer; break;
        case GL_READ_FRAMEBUFFER: trackedStates->framebufferState.boundReadFramebuffer = framebuffer; break;
    }

    trackedStates->framebufferState.recentlyBoundFramebuffer = std::make_pair(target, framebuffer);
}


void OV_glGenFramebuffers(GLsizei n, GLuint* framebuffers) {
    glGenFramebuffers(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.insert({ framebuffers[i], std::make_shared<Framebuffer>() });
    }
}

void OV_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
    glDeleteFramebuffers(n, framebuffers);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->framebufferState.trackedFramebuffers.erase(framebuffers[i]);
    }
}

void OV_glBindTexture(GLenum target, GLuint texture) {
    if (FFPE::List::addCommand<OV_glBindTexture>(target, texture)) return;
    glBindTexture(target, texture);

    trackedStates->activeTextureState->bindTextureToTarget(target, texture);
    if (debugEnabled) LOGI("glBindTexture : target=%u texture=%u", target, texture);
}

void OV_glDeleteTextures(GLsizei n, const GLuint *textures) {
    glDeleteTextures(n, textures);

    for (GLsizei i = 0; i < n; ++i) {
        trackedStates->activeTextureState->textureInternalFormats.erase(textures[i]);
    }
}

void OV_glActiveTexture(GLuint texture) {
    glActiveTexture(texture);

    trackedStates->setActiveTextureUnit(texture);
}

void OV_glBindBuffer(GLenum target, GLuint buffer) {
    glBindBuffer(target, buffer);

    // LOGI("BindBuffer: target %i, buffer %u", target, buffer);

    trackedStates->boundBuffers[target].target = target;
    trackedStates->boundBuffers[target].buffer = buffer;
}

void OV_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    glBufferData(target, size, data, usage);

    // LOGI("BufferData: target %i, size %ld, usage %i", target, size, usage);
    trackedStates->boundBuffers[target].size = size;
}

void OV_glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    glBindRenderbuffer(target, renderbuffer);

    trackedStates->boundRenderbuffer = renderbuffer;
}

void OV_glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
    glDeleteRenderbuffers(n, renderbuffers);

    for (GLint i = 0; i < n; ++i) {
        trackedStates->renderbufferInternalFormats.erase(renderbuffers[i]);
    }
}
