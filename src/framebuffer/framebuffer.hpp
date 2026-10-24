#pragma once

#include "state/state.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>
#include <memory>
#include <unordered_map>
#include <GLES2/gl2.h>

// Based on:
// https://github.com/artdeell/LTW/blob/master/ltw/src/main/tinywrapper/framebuffer.c




struct FakeDepthFramebuffer;

inline std::shared_ptr<FakeDepthFramebuffer> fakeDepthbuffer;

inline std::shared_ptr<Framebuffer> getFramebufferObject(GLenum target) {
    GLuint framebuffer = 0;
    switch (target) {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            framebuffer = trackedStates->framebufferState.boundDrawFramebuffer;
            break;
        case GL_READ_FRAMEBUFFER:
            framebuffer = trackedStates->framebufferState.boundReadFramebuffer;
            break;
    }
    
    return trackedStates->framebufferState.trackedFramebuffers[framebuffer];
}

inline GLint getAttachmentIndex(GLenum attachment) {
    switch (attachment) {
        case GL_NONE:
        case GL_DEPTH_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            return -1;
    }

    return attachment - GL_COLOR_ATTACHMENT0;
}

inline GLenum getAttachmentObjectType(GLenum framebufferTarget) {
    switch (framebufferTarget) {
        case GL_NONE: return GL_NONE;
        case GL_RENDERBUFFER: return GL_RENDERBUFFER;
        default: return GL_TEXTURE;
    }
}

inline GLenum getMapAttachment(std::shared_ptr<Framebuffer> framebuffer, GLenum attachment) {
    if (framebuffer->bufferAmount == 0) return GL_NONE;

    for (GLsizei i = 0; i < framebuffer->bufferAmount; ++i) {
        if (framebuffer->virtualDrawbuffers[i] == attachment) {
            return i + GL_COLOR_ATTACHMENT0;
        }
    }

    return GL_NONE;
}

inline void rebindFramebuffer(GLenum target, std::shared_ptr<Framebuffer> framebuffer, GLenum virtualAttachment) {
    GLint virtualIndex = getAttachmentIndex(virtualAttachment);
    if (virtualIndex == -1) return;

    GLenum physicalAttachment = getMapAttachment(framebuffer, virtualAttachment);
    if (physicalAttachment == GL_NONE) return;

    switch (framebuffer->colorInfo.colorTargets[virtualIndex]) {
        case GL_NONE:
            glFramebufferRenderbuffer(
                target, 
                physicalAttachment, 
                GL_RENDERBUFFER, 
                0
            );
            break;
        case GL_RENDERBUFFER:
            glFramebufferRenderbuffer(
                target, 
                physicalAttachment, 
                GL_RENDERBUFFER, 
                framebuffer->colorInfo.colorObjects[virtualIndex]
            );
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            glFramebufferTextureLayer(
                target, 
                physicalAttachment,
                framebuffer->colorInfo.colorObjects[virtualIndex],
                framebuffer->colorInfo.colorLevels[virtualIndex],
                framebuffer->colorInfo.colorLayers[virtualIndex]
            );
            break;
        default:
            glFramebufferTexture2D(
                target, 
                physicalAttachment,
                framebuffer->colorInfo.colorTargets[virtualIndex],
                framebuffer->colorInfo.colorObjects[virtualIndex],
                framebuffer->colorInfo.colorLevels[virtualIndex]
            );
            break;
    }
}

inline void getFramebufferAttachmentParameter(std::shared_ptr<Framebuffer> framebuffer, GLint attachmentIndex, GLenum pname, GLint* params) {
    GLenum framebufferTarget = framebuffer->colorInfo.colorTargets[attachmentIndex];
    GLenum targetObjectType = getAttachmentObjectType(framebufferTarget);
    
    switch (pname) {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            (*params) = targetObjectType;
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            if (targetObjectType == GL_RENDERBUFFER
             || targetObjectType == GL_TEXTURE) (*params) = framebuffer->colorInfo.colorObjects[attachmentIndex];
            else (*params) = 0;
            break;

        // attachment object type is not NONE
        case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            if (targetObjectType == GL_NONE) (*params) = 0;
            else (*params) = framebuffer->colorInfo.colorComponentType[attachmentIndex];
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            if (targetObjectType == GL_NONE) (*params) = 0;
            else (*params) = framebuffer->colorInfo.colorEncoding[attachmentIndex];
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
        case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
            LOGI("GL_FRAMEBUFFER_ATTACHMENT_*_SIZE are not yet implemented.");
            break;

        // attachment object type is a texture
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            if (targetObjectType == GL_TEXTURE) (*params) = framebuffer->colorInfo.colorLevels[attachmentIndex];
            else (*params) = 0;
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            if (framebufferTarget == GL_TEXTURE) (*params) = framebuffer->colorInfo.colorLayers[attachmentIndex];
            else (*params) = 0;
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        if (targetObjectType != GL_TEXTURE
         || framebufferTarget < GL_TEXTURE_CUBE_MAP_POSITIVE_X
         || framebufferTarget > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
         || framebuffer->colorInfo.colorObjects[attachmentIndex] != GL_TEXTURE_CUBE_MAP) (*params) = 0;
        else (*params) = framebufferTarget;
        break;
    }
}

inline void clearFramebuffer(GLenum buffer, GLint& drawBuffer) {
    auto framebuffer = getFramebufferObject(GL_DRAW_FRAMEBUFFER);
    if (framebuffer.get() != nullptr && buffer == GL_COLOR) {
        GLenum attachment = getMapAttachment(framebuffer, GL_COLOR_ATTACHMENT0 + drawBuffer);
        drawBuffer = attachment - GL_COLOR_ATTACHMENT0;
    }
}

struct FakeDepthFramebuffer {
    GLuint drawFramebuffer;
    GLuint drawFramebufferTexture;

    GLuint readFramebuffer;

    GLsizei width, height;
    GLvoid* data;

    bool ready;

    FakeDepthFramebuffer() {
        glGenTextures(1, &drawFramebufferTexture);
        glGenFramebuffers(1, &drawFramebuffer);
        glGenFramebuffers(1, &readFramebuffer);

        SaveBoundedTexture sbt(GL_TEXTURE_2D);

        CALL(glBindTexture, GL_TEXTURE_2D, drawFramebufferTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        ready = (glGetError() == GL_NO_ERROR);
        LOGI("FakeDepthFramebuffer: ready=%s", ready ? "true" : "false");
    }

    ~FakeDepthFramebuffer() {
        CALL(glDeleteTextures, 1, &drawFramebufferTexture);
        glDeleteFramebuffers(1, &drawFramebuffer);
        glDeleteFramebuffers(1, &readFramebuffer);
    }

    void storeDepthToFakeDraw(GLint x, GLint y, GLsizei w, GLsizei h, GLvoid* pxs) {
        if (!ready) return;
        width = w;
        height = h;
        data = pxs;

        SaveBoundedTexture sbt(GL_TEXTURE_2D);
        CALL(glBindTexture, GL_TEXTURE_2D, drawFramebufferTexture);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
            w, h, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
        );

        SaveBoundedFramebuffer sbf1(GL_DRAW_FRAMEBUFFER);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);

        glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, drawFramebufferTexture, 0
        );

        glBlitFramebuffer(
            x, y,
            x + w, y + h,
            0, 0,
            w, h,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
    }

    void blitCurrentReadToFakeDraw(GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h) {
        if (!ready) return;
        
        SaveBoundedFramebuffer sbf1(GL_DRAW_FRAMEBUFFER);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);

        glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target,
            trackedStates->activeTextureState->boundTextures[target], level
        );

        glBlitFramebuffer(
            0, 0,
            w, h,
            x, y,
            x + w, y + h,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
    }

    void blitFakeReadToFakeDraw(GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h) {
        if (!ready) return;

        SaveBoundedFramebuffer sbf1(GL_DRAW_FRAMEBUFFER);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);

        SaveBoundedFramebuffer sbf2(GL_READ_FRAMEBUFFER);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);

        glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target,
            trackedStates->activeTextureState->boundTextures[target], level
        );

        glBlitFramebuffer(
            0, 0,
            w, h,
            x, y,
            x + w, y + h,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
    }
};

namespace FBO {
    void init();

    void registerRenderbufferWorkaround();
    void registerFramebufferOverrides();
}

inline void fixStorageParams(GLenum& internalFormat) {
    switch (internalFormat) {
        case GL_DEPTH_COMPONENT:
            internalFormat = GL_DEPTH_COMPONENT16;
            break;
    }
}
