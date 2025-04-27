#pragma once

#include "es/state_tracking.h"
#include "gles20/buffer_tracking.h"
#include "gles20/framebuffer_tracking.h"
#include "gles20/shader_overrides.h"
#include "gles20/texture_tracking.h"

#include <GLES2/gl2.h>

// TODO:
// What if we keep track of bounded textures, buffer, etc ourselves
// as we can intercept calls and because glGets are expensive

// PROGRESS: PARTIAL

class Restorable {
protected:
    bool restored;

    virtual void _internal_restore() {
        throw std::runtime_error("Restorable::_internal_restore() not implemented!");
    }

public:
    virtual ~Restorable() = default;

    void restore() {
        if (restored) return;
        _internal_restore();
        restored = true;
    }

    bool isRestored() {
        return restored;
    }
};

class SaveActiveTextureUnit : public Restorable {
    GLuint activeTextureUnit;

    SaveActiveTextureUnit() {
        activeTextureUnit = trackedStates->activeTextureUnit;
    }

    ~SaveActiveTextureUnit() {
        restore();
    }

protected:
    void _internal_restore() override {
        OV_glActiveTexture(activeTextureUnit);
    }
};

struct SaveBoundedTexture : public Restorable  {
    GLuint boundedTexture;
    GLenum textureType;

    SaveBoundedTexture(GLenum textureType) : textureType(textureType) {
        boundedTexture = trackedStates->activeTextureState->boundTextures[textureType];
    }

protected:
    void _internal_restore() override {
        OV_glBindTexture(textureType, boundedTexture);
    }
};

struct SaveBoundedBuffer : public Restorable  {
    GLuint boundedBuffer;
    GLenum bufferType;

    SaveBoundedBuffer(GLenum bufferType) : bufferType(bufferType) {
        boundedBuffer = trackedStates->boundBuffers[bufferType].buffer;
    }

    ~SaveBoundedBuffer() {
        restore();
    }

protected:
    void _internal_restore() override {
        OV_glBindBuffer(bufferType, boundedBuffer);
    }
};

struct SaveBoundedFramebuffer : public Restorable  {
    GLenum framebufferType;
    GLuint boundedFramebuffer;

    SaveBoundedFramebuffer(GLenum framebufferType) : framebufferType(framebufferType) {
        switch (framebufferType) {
            case GL_FRAMEBUFFER:
            case GL_DRAW_FRAMEBUFFER:
                boundedFramebuffer = trackedStates->framebufferState.boundDrawFramebuffer;
            break;

            case GL_READ_FRAMEBUFFER:
                boundedFramebuffer = trackedStates->framebufferState.boundReadFramebuffer;
            break;
        }
    }

    ~SaveBoundedFramebuffer() {
        restore();
    }

protected:
    void _internal_restore() override {
        OV_glBindFramebuffer(framebufferType, boundedFramebuffer);
    }
};

struct SaveUsedProgram : public Restorable {
    GLuint activeProgram;

    SaveUsedProgram() {
        activeProgram = trackedStates->lastUsedProgram;
    }

    ~SaveUsedProgram() {
        restore();
    }

protected:
    void _internal_restore() override {
        OV_glUseProgram(activeProgram);
    }
};
