#pragma once

#include <GLES3/gl32.h>

namespace GLDesktop {
    inline constexpr GLenum BGR  = 0x80e0;
    inline constexpr GLenum BGRA = 0x80e1;

    inline constexpr GLenum ALPHA4  = 0x803b;
    inline constexpr GLenum ALPHA8  = 0x803c;
    inline constexpr GLenum ALPHA12 = 0x803d;
    inline constexpr GLenum ALPHA16 = 0x803e;

    inline constexpr GLenum LUMINANCE4  = 0x803f;
    inline constexpr GLenum LUMINANCE8  = 0x8040;
    inline constexpr GLenum LUMINANCE12 = 0x8041;
    inline constexpr GLenum LUMINANCE16 = 0x8042;

    inline constexpr GLenum LUMINANCE4_ALPHA4   = 0x8043;
    inline constexpr GLenum LUMINANCE6_ALPHA2   = 0x8044;
    inline constexpr GLenum LUMINANCE8_ALPHA8   = 0x8045;
    inline constexpr GLenum LUMINANCE12_ALPHA4  = 0x8046;
    inline constexpr GLenum LUMINANCE12_ALPHA12 = 0x8047;
    inline constexpr GLenum LUMINANCE16_ALPHA16 = 0x8048;

    inline constexpr GLenum R3_G3_B2 = 0x2a10;
    inline constexpr GLenum RGB4     = 0x804f;
    inline constexpr GLenum RGB5     = 0x8050;
    inline constexpr GLenum RGB10    = 0x8052;
    inline constexpr GLenum RGB12    = 0x8053;
    inline constexpr GLenum RGB16    = 0x8054;
    inline constexpr GLenum RGBA2    = 0x8055;
    inline constexpr GLenum RGBA12   = 0x805a;
    inline constexpr GLenum RGBA16   = 0x805b;

    inline constexpr GLenum R16  = 0x822a;
    inline constexpr GLenum RG16 = 0x822c;

    inline constexpr GLenum R16_SNORM    = 0x8f98;
    inline constexpr GLenum RG16_SNORM   = 0x8f99;
    inline constexpr GLenum RGB16_SNORM  = 0x8f9a;
    inline constexpr GLenum RGBA16_SNORM = 0x8f9b;

    inline constexpr GLenum SRGB       = 0x8c40;
    inline constexpr GLenum SRGB_ALPHA = 0x8c42;

    inline constexpr GLenum COMPRESSED_RED        = 0x8225;
    inline constexpr GLenum COMPRESSED_RG         = 0x8226;
    inline constexpr GLenum COMPRESSED_RGB        = 0x84ed;
    inline constexpr GLenum COMPRESSED_RGBA       = 0x84ee;
    inline constexpr GLenum COMPRESSED_SRGB       = 0x8c48;
    inline constexpr GLenum COMPRESSED_SRGB_ALPHA = 0x8c49;

    inline constexpr GLenum DEPTH_COMPONENT32 = 0x81a7;
}
