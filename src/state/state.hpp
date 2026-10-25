#pragma once

#include "fogltlogles.hpp"
#include "common/fast_map.hpp"

#include <GLES2/gl2.h>
#include <memory>
#include <GLES3/gl3.h>
#include <unordered_map>
#include <utility>
#include <GLES2/gl2ext.h>
#include <GLES3/gl32.h>
#include <string_view>

namespace State {
    void init();
}

#ifndef MAX_FBTARGETS
#define MAX_FBTARGETS 8
#endif

#ifndef MAX_DRAWBUFFERS
#define MAX_DRAWBUFFERS 8
#endif

struct Framebuffer;
struct Buffer;

struct TextureStates {
    std::pair<GLenum, GLuint> recentlyBoundTexture;
    bool texture2DEnabled = false;

    // Type, Texture
    FAST_MAP_BI(GLenum, GLuint) boundTextures;

    // Texture, Internal Format
    FAST_MAP_BI(GLuint, GLenum) textureInternalFormats;

    void bindTextureToTarget(GLenum target, GLuint texture) {
        boundTextures[target] = texture;
        recentlyBoundTexture = std::make_pair(target, texture);
    }

    void trackFormat(GLenum target, GLenum internalFormat) {
        textureInternalFormats.emplace(boundTextures[target], internalFormat);
    }

    GLenum getTextureInternalFormat(GLenum target) {
        return textureInternalFormats[boundTextures[target]];
    }
};

struct FramebufferStates {
    GLuint boundReadFramebuffer;
    GLuint boundDrawFramebuffer;

    // Type, Framebuffer
    std::pair<GLenum, GLuint> recentlyBoundFramebuffer; // (either read or draw) (or both? GL_FRAMEBUFFER)

    // Framebuffer, Framebuffer struct
    //     | |
    // since we are using the framebuffer object
    // we need to make sure it gets erased here
    FAST_MAP_BI(GLuint, std::shared_ptr<Framebuffer>) trackedFramebuffers;

};

struct TrackedStates {
    FAST_MAP_BI(GLenum, bool) capabilitiesState;

    GLuint activeTextureUnit = GL_TEXTURE0;
    TextureStates* activeTextureState;

    // Unit, TextureStates
    // Unit is GL_TEXTUREi, where i is 0 to max texture units
    FAST_MAP_BI(GLuint, TextureStates) textureUnits;

    FramebufferStates framebufferState;

    GLuint boundRenderbuffer;

    // Renderbuffer, Internal Format
    FAST_MAP_BI(GLuint, GLenum) renderbufferInternalFormats;

    // Type, Buffer
    // NOTE: Type is NOT a binding enum
    FAST_MAP_BI(GLenum, Buffer) boundBuffers;

    // FAST_MAP_BI(GLenum, FAST_MAP_BI(GLuint, Buffer)) boundBuffers2;
    // Buffer Type, Buffer ID, Buffer

    GLuint boundVertexArray = 0;

    // VAO, Element Array Buffer
    FAST_MAP_BI(GLuint, GLuint) vertexArrayElementBuffers;

    GLuint currentlyUsedProgram;

    TrackedStates() {
        activeTextureState = &textureUnits[GL_TEXTURE0];
    }

    void setActiveTextureUnit(GLuint unit) {
        activeTextureUnit = unit;
        activeTextureState = &textureUnits[unit];
    }

    bool isCapabilityEnabled(GLenum capability) {
        return capabilitiesState[capability];
    }
};

inline std::shared_ptr<TrackedStates> trackedStates;

struct FramebufferColorInfo {
    GLuint colorTargets[MAX_FBTARGETS];
    GLuint colorObjects[MAX_FBTARGETS];
    GLuint colorComponentType[MAX_FBTARGETS];
    GLuint colorEncoding[MAX_FBTARGETS];

    // if GL_TEXTURE
    GLuint colorLevels[MAX_FBTARGETS];
    GLuint colorLayers[MAX_FBTARGETS];
};

struct Framebuffer {
    FramebufferColorInfo colorInfo;
    GLenum virtualDrawbuffers[MAX_DRAWBUFFERS] = { GL_COLOR_ATTACHMENT0 };
    GLenum physicalDrawbuffers[MAX_DRAWBUFFERS];
    GLsizei bufferAmount = 1;
};

struct Buffer {
    GLenum target;
    GLuint buffer;
    GLsizeiptr size;
};

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
