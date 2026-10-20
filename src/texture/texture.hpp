#pragma once

#include "state/state_tracking.hpp"
#include "texture/swizzling.hpp"

#include <GLES3/gl32.h>
#include <unordered_map>

inline void trackTextureFormat(GLint& internalFormat) {
    trackedStates->activeTextureState->textureInternalFormats.emplace(trackedStates->activeTextureState->boundTextures[GL_TEXTURE_2D], internalFormat);
}

// nuke the format fixer
// rewrite it to be more
// extensible and readable
// frozen lib constexpr ???

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
        
            // fall through
        case 0x8367: // GL_UNSIGNED_INT_8_8_8_8_REV
            type = GL_UNSIGNED_BYTE;
            break;

        default:
            break;
    }
}

inline void fixTexArguments(
    GLenum& target,
    GLint& internalFormat,
    GLenum& type,
    GLenum& format,
    bool isShadowMap
) {
    // Workarounds
    // https://github.com/artdeell/LTW/blob/0a0567659fe4e2ade04313061ac8c9aaa5240e53/ltw/src/main/tinywrapper/glformats.c#L77
    switch (internalFormat) {
        case 0x805a: // GL_RGBA12
        case 0x805b: // GL_RGBA16
            internalFormat = GL_RGBA16F;
            break;

        case GL_DEPTH_COMPONENT:
            if (isShadowMap) internalFormat = GL_DEPTH_COMPONENT32F;
            break;

        case 0x81a7: // GL_DEPTH_COMPONENT32
            internalFormat = GL_DEPTH_COMPONENT32F;
            break;

        case GL_DEPTH_STENCIL:
            internalFormat = GL_DEPTH24_STENCIL8;
            break;

        case GL_R8_SNORM:
            internalFormat = GL_R16F;
            break;

        case GL_RG8_SNORM:
            internalFormat = GL_RG16F;
            break;

        case GL_RGBA8_SNORM:
            internalFormat = GL_RGBA16F;
            break;

        case GL_RGB8I:
        case GL_RGB16I:
        case GL_RGB32I:
        case GL_RGB8_SNORM:
        case 0x8053: // GL_RGB12
        case 0x8054: // GL_RGB16
        case GL_RGB16F:
        case GL_RGB32F:
            internalFormat = GL_R11F_G11F_B10F;
            break;

        case GL_RGB8UI:
            internalFormat = GL_RGB8;
            break;
    }

    // Cover the full mapping for color and sized formats.
    switch (internalFormat) {
        // Unsized color formats
        case GL_RGB:
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGBA:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_LUMINANCE_ALPHA:
            format = GL_LUMINANCE_ALPHA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_LUMINANCE:
            format = GL_LUMINANCE;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_ALPHA:
            format = GL_ALPHA;
            type = GL_UNSIGNED_BYTE;
            break;
        // Sized red formats
        case GL_R8:
            format = GL_RED;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_R8_SNORM:
            format = GL_RED;
            type = GL_BYTE;
            break;
        case GL_R16F:
            format = GL_RED;
            type = GL_HALF_FLOAT;
            break;
        case GL_R32F:
            format = GL_RED;
            type = GL_FLOAT;
            break;
        case GL_R8UI:
            format = GL_RED_INTEGER;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_R8I:
            format = GL_RED_INTEGER;
            type = GL_BYTE;
            break;
        case GL_R16UI:
            format = GL_RED_INTEGER;
            type = GL_UNSIGNED_SHORT;
            break;
        case GL_R16I:
            format = GL_RED_INTEGER;
            type = GL_SHORT;
            break;
        case GL_R32UI:
            format = GL_RED_INTEGER;
            type = GL_UNSIGNED_INT;
            break;
        case GL_R32I:
            format = GL_RED_INTEGER;
            type = GL_INT;
            break;
        // Sized RG formats
        case GL_RG8:
            format = GL_RG;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RG8_SNORM:
            format = GL_RG;
            type = GL_BYTE;
            break;
        case GL_RG16F:
            format = GL_RG;
            type = GL_HALF_FLOAT;
            break;
        case GL_RG32F:
            format = GL_RG;
            type = GL_FLOAT;
            break;
        case GL_RG8UI:
            format = GL_RG_INTEGER;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RG8I:
            format = GL_RG_INTEGER;
            type = GL_BYTE;
            break;
        case GL_RG16UI:
            format = GL_RG_INTEGER;
            type = GL_UNSIGNED_SHORT;
            break;
        case GL_RG16I:
            format = GL_RG_INTEGER;
            type = GL_SHORT;
            break;
        case GL_RG32UI:
            format = GL_RG_INTEGER;
            type = GL_UNSIGNED_INT;
            break;
        case GL_RG32I:
            format = GL_RG_INTEGER;
            type = GL_INT;
            break;
        // Sized RGB formats
        case GL_RGB8:
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_SRGB8:
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGB565:
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGB8_SNORM:
            format = GL_RGB;
            type = GL_BYTE;
            break;
        case GL_R11F_G11F_B10F:
            format = GL_RGB;
            type = GL_FLOAT;
            break;
        case GL_RGB9_E5:
            format = GL_RGB;
            type = GL_UNSIGNED_INT_5_9_9_9_REV;
            break;
        case GL_RGB16F:
            format = GL_RGB;
            type = GL_HALF_FLOAT;
            break;
        case GL_RGB32F:
            format = GL_RGB;
            type = GL_FLOAT;
            break;
        case GL_RGB8UI:
            format = GL_RGB_INTEGER;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGB8I:
            format = GL_RGB_INTEGER;
            type = GL_BYTE;
            break;
        case GL_RGB16UI:
            format = GL_RGB_INTEGER;
            type = GL_UNSIGNED_SHORT;
            break;
        case GL_RGB16I:
            format = GL_RGB_INTEGER;
            type = GL_SHORT;
            break;
        case GL_RGB32UI:
            format = GL_RGB_INTEGER;
            type = GL_UNSIGNED_INT;
            break;
        case GL_RGB32I:
            format = GL_RGB_INTEGER;
            type = GL_INT;
            break;
        // Sized RGBA formats
        case GL_RGBA8:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_SRGB8_ALPHA8:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGBA8_SNORM:
            format = GL_RGBA;
            type = GL_BYTE;
            break;
        case GL_RGB5_A1:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGBA4:
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGB10_A2:
            format = GL_RGBA;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            break;
        case GL_RGBA16F:
            format = GL_RGBA;
            type = GL_HALF_FLOAT;
            break;
        case GL_RGBA32F:
            format = GL_RGBA;
            type = GL_FLOAT;
            break;
        case GL_RGBA8UI:
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_RGBA8I:
            format = GL_RGBA_INTEGER;
            type = GL_BYTE;
            break;
        case GL_RGB10_A2UI:
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_INT_2_10_10_10_REV;
            break;
        case GL_RGBA16UI:
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_SHORT;
            break;
        case GL_RGBA16I:
            format = GL_RGBA_INTEGER;
            type = GL_SHORT;
            break;
        case GL_RGBA32I:
            format = GL_RGBA_INTEGER;
            type = GL_INT;
            break;
        case GL_RGBA32UI:
            format = GL_RGBA_INTEGER;
            type = GL_UNSIGNED_INT;
            break;
        
        default:
            break;
    }

    // Finally, apply BGRA swizzling adjustments.
    switch (internalFormat) {
        case GL_R8: {
            if (format == 0x80e1) { // GL_BGRA
                format = GL_RED;
                swizzleBGRA(type);
            }
            break;
        }
        case GL_RG8: {
            if (format == 0x80e1) { // GL_BGRA
                format = GL_RG;
                swizzleBGRA(type);
            }
            break;
        }
        case GL_RGBA:
        case GL_RGBA8: {
            if (format == 0x80e1) { // GL_BGRA
                format = GL_RGBA;
                swizzleBGRA(type);
            }
            break;
        }
    }

    // Special cases for depth, depth-stencil, red and RG unsized formats.
    switch (format) {
        case GL_DEPTH_COMPONENT:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    type = GL_UNSIGNED_SHORT;
                    // byte to short eta?
                case GL_UNSIGNED_SHORT:
                case GL_UNSIGNED_INT:
                    internalFormat = GL_DEPTH_COMPONENT16;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_DEPTH_COMPONENT32F;
                    break;
                default:
                    break;
            }
            break;
        
        case GL_DEPTH_STENCIL: {
            switch (type) {
                case GL_UNSIGNED_INT_24_8:
                    internalFormat = GL_DEPTH24_STENCIL8;
                    break;
                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                    internalFormat = GL_DEPTH32F_STENCIL8;
                    break;
                default:
                    break;
            }
            break;
        }
        // Single-channel (red) unsized format
        case GL_RED: {
            switch (type) {
                case GL_BYTE:
                    internalFormat = GL_R8_SNORM;
                    break;
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_R8;
                    break;
                case GL_HALF_FLOAT:
                    internalFormat = GL_R16F;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_R32F;
                    break;
                default:
                    break;
            }
            break;
        }
        // Two-channel (RG) unsized format
        case GL_RG: {
            switch (type) {
                case GL_BYTE:
                    internalFormat = GL_RG8_SNORM;
                    break;
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_RG8;
                    break;
                case GL_HALF_FLOAT:
                    internalFormat = GL_RG16F;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_RG32F;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            // Other internal formats will be handled in the large mapping below.
            break;
    }

    doSwizzling(target);
}
