#pragma once

#include "ffp/states.hpp"
#include "state/state_tracking.hpp"
#include "ffp/arrays.hpp"
#include "state/buffer_tracking.hpp"
#include "state/framebuffer_tracking.hpp"
#include "shader/overrides.hpp"
#include "state/texture_tracking.hpp"

#include <GLES2/gl2ext.h>
#include <GLES3/gl32.h>
#include <string_view>

// TODO:
// What if we keep track of bounded textures, buffer, etc ourselves
// as we can intercept calls and because glGets are expensive

// PROGRESS: PARTIAL

template <typename Derived>
class Restorable {
    bool restored = false;

protected:
    Restorable() = default;
    ~Restorable() = default;

public:
    Restorable(const Restorable&) = delete;
    Restorable& operator=(const Restorable&) = delete;

    void restore() noexcept {
        if (restored) return;
        restored = true;
        static_cast<Derived*>(this)->_internal_restore();
    }

    bool isRestored() const noexcept {
        return restored;
    }
};

class SaveActiveTextureUnit : public Restorable<SaveActiveTextureUnit> {
    friend class Restorable<SaveActiveTextureUnit>;
    GLuint activeTextureUnit;

    void _internal_restore() {
        CALL(glActiveTexture, activeTextureUnit);
    }

public:
    SaveActiveTextureUnit() : activeTextureUnit(trackedStates->activeTextureUnit) { }
    ~SaveActiveTextureUnit() { restore(); }
};

class SaveBoundedTexture : public Restorable<SaveBoundedTexture> {
    friend class Restorable<SaveBoundedTexture>;
    GLenum textureType;
    GLuint boundedTexture;

    void _internal_restore() {
        CALL(glBindTexture, textureType, boundedTexture);
    }

public:
    explicit SaveBoundedTexture(GLenum textureType)
        : textureType(textureType),
          boundedTexture(trackedStates->activeTextureState->boundTextures[textureType]) { }
    ~SaveBoundedTexture() { restore(); }
};

class SaveBoundedBuffer : public Restorable<SaveBoundedBuffer> {
    friend class Restorable<SaveBoundedBuffer>;
    GLenum bufferType;
    GLuint boundedBuffer;

    void _internal_restore() {
        CALL(glBindBuffer, bufferType, boundedBuffer);
    }

public:
    explicit SaveBoundedBuffer(GLenum bufferType)
        : bufferType(bufferType),
          boundedBuffer(trackedStates->boundBuffers[bufferType].buffer) { }
    ~SaveBoundedBuffer() { restore(); }

    GLuint saved() const noexcept { return boundedBuffer; }
};

class SaveBoundedFramebuffer : public Restorable<SaveBoundedFramebuffer> {
    friend class Restorable<SaveBoundedFramebuffer>;
    GLenum framebufferType;
    GLuint boundedFramebuffer;

    static GLuint queryBound(GLenum type) {
        return type == GL_READ_FRAMEBUFFER
            ? trackedStates->framebufferState.boundReadFramebuffer
            : trackedStates->framebufferState.boundDrawFramebuffer;
    }

    void _internal_restore() {
        CALL(glBindFramebuffer, framebufferType, boundedFramebuffer);
    }

public:
    explicit SaveBoundedFramebuffer(GLenum framebufferType)
        : framebufferType(framebufferType),
          boundedFramebuffer(queryBound(framebufferType)) { }
    ~SaveBoundedFramebuffer() { restore(); }
};

class SaveUsedProgram : public Restorable<SaveUsedProgram> {
    friend class Restorable<SaveUsedProgram>;
    GLuint activeProgram;

    void _internal_restore() {
        CALL(glUseProgram, activeProgram);
    }

public:
    SaveUsedProgram() : activeProgram(trackedStates->currentlyUsedProgram) { }
    ~SaveUsedProgram() { restore(); }
};

class GLDebugGroup : public Restorable<GLDebugGroup> {
    friend class Restorable<GLDebugGroup>;

    void _internal_restore() {
        glPopDebugGroup();
    }

public:
    explicit GLDebugGroup(std::string_view name) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION_KHR, 0, static_cast<GLsizei>(name.size()), name.data());
    }
    ~GLDebugGroup() { restore(); }
};

class SaveClientArray : public Restorable<SaveClientArray> {
    friend class Restorable<SaveClientArray>;
    GLenum targetArray;
    bool isEnabled;

    void _internal_restore() {
        if (isEnabled) glEnableClientState(targetArray);
        else glDisableClientState(targetArray);
    }

public:
    explicit SaveClientArray(GLenum array)
        : targetArray(array),
          isEnabled(FFPE::States::ClientState::Arrays::getArray(array)->enabled) { }
    ~SaveClientArray() { restore(); }
};

class SaveTexCoordClientArray : public Restorable<SaveTexCoordClientArray> {
    friend class Restorable<SaveTexCoordClientArray>;
    GLenum targetUnit;
    bool isEnabled;

    void _internal_restore() {
        FFPE::States::ClientState::Arrays::getTexCoordArray(targetUnit)->enabled = isEnabled;
    }

public:
    explicit SaveTexCoordClientArray(GLenum unit)
        : targetUnit(unit),
          isEnabled(FFPE::States::ClientState::Arrays::getTexCoordArray(unit)->enabled) { }
    ~SaveTexCoordClientArray() { restore(); }
};
