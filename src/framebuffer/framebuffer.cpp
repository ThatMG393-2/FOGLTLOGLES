// Based on:
// https://github.com/artdeell/LTW/blob/master/ltw/src/main/tinywrapper/framebuffer.c

#include "framebuffer/framebuffer.hpp"
#include "state/state_tracking.hpp"
#include "common/es_utils.hpp"
#include "framebuffer/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl3.h>

void glDrawBuffer(GLenum buffer);
static void glDrawBuffersHook(NEXT(glDrawBuffers) next, GLsizei n, const GLenum* buffers);

static void glClearBufferfvHook(NEXT(glClearBufferfv) next, GLenum buffer, GLint drawbuffer, const GLfloat* value);
static void glClearBufferivHook(NEXT(glClearBufferiv) next, GLenum buffer, GLint drawbuffer, const GLint* value);
static void glClearBufferuivHook(NEXT(glClearBufferuiv) next, GLenum buffer, GLint drawbuffer, const GLuint* value);

static void glFramebufferTexture2DHook(NEXT(glFramebufferTexture2D) next, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
static void glFramebufferTextureLayerHook(NEXT(glFramebufferTextureLayer) next, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
static void glFramebufferRenderbufferHook(NEXT(glFramebufferRenderbuffer) next, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

static void glGetFramebufferAttachmentParameterivHook(NEXT(glGetFramebufferAttachmentParameteriv) next, GLenum target, GLenum attachment, GLenum pname, GLint* params);
GLenum OV_glCheckFramebufferStatus(GLenum target);

void FBO::registerFramebufferOverrides() {
    PROVIDE(glDrawBuffer);
    PROVIDE(glDrawBuffers);
    HOOK(glDrawBuffers, glDrawBuffersHook);

    PROVIDE(glClearBufferfv);
    HOOK(glClearBufferfv, glClearBufferfvHook);
    PROVIDE(glClearBufferiv);
    HOOK(glClearBufferiv, glClearBufferivHook);
    PROVIDE(glClearBufferuiv);
    HOOK(glClearBufferuiv, glClearBufferuivHook);

    PROVIDE(glFramebufferTexture2D);
    HOOK(glFramebufferTexture2D, glFramebufferTexture2DHook);
    PROVIDE(glFramebufferTextureLayer);
    HOOK(glFramebufferTextureLayer, glFramebufferTextureLayerHook);
    PROVIDE(glFramebufferRenderbuffer);
    HOOK(glFramebufferRenderbuffer, glFramebufferRenderbufferHook);

    PROVIDE(glGetFramebufferAttachmentParameteriv);
    HOOK(glGetFramebufferAttachmentParameteriv, glGetFramebufferAttachmentParameterivHook);
    PROVIDE(glCheckFramebufferStatus, OV_glCheckFramebufferStatus);

    fakeDepthbuffer = std::make_shared<FakeDepthFramebuffer>();
}

void glDrawBuffer(GLenum buffer) {
    CALL(glDrawBuffers, 1, &buffer);
}

static void glDrawBuffersHook(NEXT(glDrawBuffers) next, GLsizei n, const GLenum* buffers) {
    auto framebuffer = getFramebufferObject(GL_DRAW_FRAMEBUFFER);
    if (!framebuffer) {
        next(n, buffers);
        return;
    }

    framebuffer->bufferAmount = n;
    std::memcpy(framebuffer->virtualDrawbuffers, buffers, n * sizeof(GLenum));

    // no bounds checking as theres a memcpy
    // above that clearly ignores MAX_DRAWBUFFERS
    for (int i = 0; i < n; ++i) {
        GLenum buffer = buffers[i];
        rebindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer, buffer);

        if (buffer != GL_NONE) framebuffer->physicalDrawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        else framebuffer->physicalDrawbuffers[i] = GL_NONE;
    }

    next(n, framebuffer->physicalDrawbuffers);
}

static void glClearBufferfvHook(NEXT(glClearBufferfv) next, GLenum buffer, GLint drawbuffer, const GLfloat* value) {
    clearFramebuffer(buffer, drawbuffer);
    next(buffer, drawbuffer, value);
}

static void glClearBufferivHook(NEXT(glClearBufferiv) next, GLenum buffer, GLint drawbuffer, const GLint* value) {
    clearFramebuffer(buffer, drawbuffer);
    next(buffer, drawbuffer, value);
}

static void glClearBufferuivHook(NEXT(glClearBufferuiv) next, GLenum buffer, GLint drawbuffer, const GLuint* value) {
    clearFramebuffer(buffer, drawbuffer);
    next(buffer, drawbuffer, value);
}


static void glFramebufferTexture2DHook(NEXT(glFramebufferTexture2D) next, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    auto framebuffer = getFramebufferObject(target);
    GLint attachmentIndex = getAttachmentIndex(attachment);

    if (!framebuffer || attachmentIndex == -1) {
        next(target, attachment, textarget, texture, level);
        return;
    }

    if (texture == 0) {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = GL_NONE;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = 0;
        framebuffer->colorInfo.colorLevels[attachmentIndex] = 0;
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = 0;
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = 0;
    } else {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = textarget;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = texture;
        framebuffer->colorInfo.colorLevels[attachmentIndex] = level;

        GLenum textureFormat = trackedStates->activeTextureState->textureInternalFormats[texture];
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = ESUtils::getComponentTypeFromFormat(textureFormat);
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = ESUtils::isSRGBFormat(textureFormat) ? GL_SRGB : GL_LINEAR;
    }

    rebindFramebuffer(target, framebuffer, attachment);
}

static void glFramebufferTextureLayerHook(NEXT(glFramebufferTextureLayer) next, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    auto framebuffer = getFramebufferObject(target);
    GLint attachmentIndex = getAttachmentIndex(attachment);

    if (!framebuffer || attachmentIndex == -1) {
        next(target, attachment, texture, level, layer);
        return;
    }

    if (texture == 0) {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = GL_NONE;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = 0;
        framebuffer->colorInfo.colorLevels[attachmentIndex] = 0;
        framebuffer->colorInfo.colorLayers[attachmentIndex] = 0;
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = 0;
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = 0;
    } else {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = texture;
        framebuffer->colorInfo.colorLevels[attachmentIndex] = level;
        framebuffer->colorInfo.colorLayers[attachmentIndex] = layer;
        
        GLenum textureFormat = trackedStates->activeTextureState->textureInternalFormats[texture];
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = ESUtils::getComponentTypeFromFormat(textureFormat);
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = ESUtils::isSRGBFormat(textureFormat) ? GL_SRGB : GL_LINEAR;
    }

    rebindFramebuffer(target, framebuffer, attachment);
}

static void glFramebufferRenderbufferHook(NEXT(glFramebufferRenderbuffer) next, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    auto framebuffer = getFramebufferObject(target);
    GLint attachmentIndex = getAttachmentIndex(attachment);

    if (!framebuffer || attachmentIndex == -1) {
        next(target, attachment, renderbuffertarget, renderbuffer);
        return;
    }

    if (renderbuffer == 0) {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = GL_NONE;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = 0;
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = 0;
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = 0;
    } else {
        framebuffer->colorInfo.colorTargets[attachmentIndex] = renderbuffertarget;
        framebuffer->colorInfo.colorObjects[attachmentIndex] = renderbuffer;
        
        GLenum renderbufferFormat = trackedStates->renderbufferInternalFormats[renderbuffer];
        framebuffer->colorInfo.colorComponentType[attachmentIndex] = ESUtils::getComponentTypeFromFormat(renderbufferFormat);
        framebuffer->colorInfo.colorEncoding[attachmentIndex] = ESUtils::isSRGBFormat(renderbufferFormat) ? GL_SRGB : GL_LINEAR;
    }

    rebindFramebuffer(target, framebuffer, attachment);
}

static void glGetFramebufferAttachmentParameterivHook(NEXT(glGetFramebufferAttachmentParameteriv) next, GLenum target, GLenum attachment, GLenum pname, GLint* params) {
    auto framebuffer = getFramebufferObject(target);
    GLint attachmentIndex = getAttachmentIndex(attachment);

    if (!framebuffer || attachmentIndex == -1) {
        next(target, attachment, pname, params);
        return;
    }

    getFramebufferAttachmentParameter(framebuffer, attachmentIndex, pname, params);
}

GLenum OV_glCheckFramebufferStatus(GLenum target) {
    return GL_FRAMEBUFFER_COMPLETE;
    /* GLenum framebufferStatus = glCheckFramebufferStatus(target);
    if (framebufferStatus == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
        auto framebuffer = getFramebufferObject(target);
        for (GLint i = 0; i < MAX_FBTARGETS; ++i) {
            if (framebuffer->colorInfo.colorTargets[i] != GL_NONE
             || framebuffer->colorInfo.colorObjects[i] != 0) return GL_FRAMEBUFFER_COMPLETE;
        }
    }

    return framebufferStatus; */
}
