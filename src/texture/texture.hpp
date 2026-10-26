#pragma once

#include "common/pointers.hpp"
#include "fogltlogles.hpp"
#include "state/state.hpp"

#include <memory>
#include <GLES3/gl32.h>
#include <GLES3/gl3.h>
#include <unordered_map>
#include <vector>

namespace Texture {
    void init();

    void registerTextureOverrides();
    void registerReadPixelsOverride();
    void registerQueryOverrides();
}

struct ProxyTexture {
    GLenum type;
    GLint width;
    GLint height;
    // GLint depth; // if GL_PROXY_TEXTURE_3D
    GLint level;
    GLint internalFormat;
};

inline std::shared_ptr<ProxyTexture> boundProxyTexture = MakeAggregateShared<ProxyTexture>();

inline bool isProxyTexture(GLenum target) {
    switch (target) {
        case 0x8063: // GL_PROXY_TEXTURE_1D
        case 0x8064: // GL_PROXY_TEXTURE_2D
        case 0x8070: // GL_PROXY_TEXTURE_3D
        case 0x84f5: // GL_PROXY_TEXTURE_RECTANGLE
        case 0x84f7: // GL_PROXY_TEXTURE_RECTANGLE_ARB
            return true;
        default:
            return false;
    }
}

inline GLint nlevel(GLint size, GLint level) {
    if (size) {
        size >>= level;
        if (!size) size = 1;
    }
    return size;
}

inline void getProxyTexLevelParameter(GLenum pname, GLint* params) {
    switch (pname) {
        case GL_TEXTURE_WIDTH:
            (*params) = nlevel(boundProxyTexture->width, boundProxyTexture->level);
            return;
        case GL_TEXTURE_HEIGHT:
            (*params) = nlevel(boundProxyTexture->height, boundProxyTexture->level);
            return;
        /* case GL_TEXTURE_DEPTH:
            if (boundProxyTexture->type != GL_TEXTURE_3D) return;
            (*params) = nlevel(boundProxyTexture->depth, boundProxyTexture->level);
            return; */
        case GL_TEXTURE_INTERNAL_FORMAT:
            (*params) = boundProxyTexture->internalFormat;
            return;
    }
}

enum SwizzleOperation {
    BGRA2RGBA,
    ENDIANNESS_SWAP
};

inline const std::unordered_map<SwizzleOperation, std::array<GLint, 4>> swizzleArrayMap = {
    { BGRA2RGBA,           { GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA } },
    { ENDIANNESS_SWAP,     { GL_ALPHA, GL_BLUE, GL_GREEN, GL_RED } }
};

// here to avoid recreating a new vector for each swizzles
// do not forget to resize this to match the
// number of swizzle operations
inline std::vector<SwizzleOperation> currentSwizzleOperations;

// hardcoded to 4 channels only
// avoid recreations!
inline std::array<GLint, 4> transformedComponents;

inline std::array<GLint, 4> transformComponents(
    const std::array<GLint, 4>& current,
    const std::array<GLint, 4>& transform
) {
    for (int i = 0; i < 4; i++) {
        GLint component = transform[i];
        if (component >= GL_RED && component <= GL_ALPHA) {
            // Map through the existing swizzle
            transformedComponents[i] = current[component - GL_RED];
        } else {
            // Pass through non-swizzle values
            transformedComponents[i] = component;
        }
    }

    return transformedComponents;
}

inline void doSwizzling(GLenum target) {
    if (currentSwizzleOperations.empty()) return;
    auto currentSwizzle = swizzleArrayMap.at(currentSwizzleOperations[0]);

    if (currentSwizzleOperations.size() == 1) goto swizzle;
    for (size_t i = 1; i < currentSwizzleOperations.size(); i++) {
        auto& nextTransform = swizzleArrayMap.at(currentSwizzleOperations[i]);
        currentSwizzle = transformComponents(currentSwizzle, nextTransform);
    }

swizzle:
    // Apply the final swizzle settings to the texture
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, currentSwizzle[0]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, currentSwizzle[1]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, currentSwizzle[2]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, currentSwizzle[3]);

    currentSwizzleOperations.clear();
}

inline void selectProperTexParamf(GLenum target, GLenum& pname, GLfloat& param) {
    switch (pname) {
        case 0x8501: // GL_TEXTURE_LOD_BIAS
            pname = GL_TEXTURE_MIN_LOD;
            return;
    }
}

inline void selectProperTexParami(GLenum target, GLenum& pname, GLint& param) {
    switch (pname) {

    }
}

inline bool isDepthFormat(GLenum format) {
    switch (format) {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case 0x81a7: // GL_DEPTH_COMPONENT_32 (this ones an edge case)
        case GL_DEPTH_COMPONENT32F:
            return true;
    }

    return false;
}

inline void swizzleBGRA(GLenum& type) {
    switch (type) {
        case 0x8035: // GL_UNSIGNED_INT_8_8_8_8
            currentSwizzleOperations.emplace_back(ENDIANNESS_SWAP);
            currentSwizzleOperations.emplace_back(BGRA2RGBA);
            type = GL_UNSIGNED_BYTE;
            break;

        case 0x8367: // GL_UNSIGNED_INT_8_8_8_8_REV
        case GL_UNSIGNED_BYTE:
            currentSwizzleOperations.emplace_back(BGRA2RGBA);
            type = GL_UNSIGNED_BYTE;
            break;

        default:
            break;
    }
}

void fixTexArguments(
    GLenum& target,
    GLint& internalFormat,
    GLenum& type,
    GLenum& format,
    bool isShadowMap
);
