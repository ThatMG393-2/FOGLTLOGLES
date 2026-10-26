#include "common/fast_map.hpp"
#include "common/log.hpp"
#include "texture/desktop_enums.hpp"
#include "texture/texture.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <unordered_set>

namespace {
    namespace Desktop = GLDesktop;

    struct FormatType {
        GLenum format;
        GLenum type;
    };

    struct ResolveRule {
        GLenum format;
        GLenum type;
        GLenum internalFormat;
        GLenum resultType;
    };

    struct BgraRule {
        GLenum sourceFormat;
        GLenum internalFormat;
        GLenum format;
    };

    enum class Field : uint64_t {
        InternalFormat,
        Format,
        Type
    };

    constexpr const char* fieldNames[] = { "internalFormat", "format", "type" };

    const FastMapBI<GLenum, GLenum> desktopToEsInternalFormat = {
        { Desktop::DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F },

        { Desktop::RGBA2,         GL_RGBA4 },
        { Desktop::RGBA12,        GL_RGBA16F },
        { Desktop::RGBA16,        GL_RGBA16F },
        { Desktop::RGBA16_SNORM,  GL_RGBA16F },

        { Desktop::R3_G3_B2,      GL_RGB565 },
        { Desktop::RGB4,          GL_RGB565 },
        { Desktop::RGB5,          GL_RGB565 },
        { Desktop::RGB10,         GL_RGB16F },
        { Desktop::RGB12,         GL_RGB16F },
        { Desktop::RGB16,         GL_RGB16F },
        { Desktop::RGB16_SNORM,   GL_RGB16F },

        { Desktop::R16,           GL_R16F },
        { Desktop::R16_SNORM,     GL_R16F },
        { Desktop::RG16,          GL_RG16F },
        { Desktop::RG16_SNORM,    GL_RG16F },

        { Desktop::ALPHA4,        GL_ALPHA },
        { Desktop::ALPHA8,        GL_ALPHA },
        { Desktop::ALPHA12,       GL_ALPHA },
        { Desktop::ALPHA16,       GL_ALPHA },

        { Desktop::LUMINANCE4,    GL_LUMINANCE },
        { Desktop::LUMINANCE8,    GL_LUMINANCE },
        { Desktop::LUMINANCE12,   GL_LUMINANCE },
        { Desktop::LUMINANCE16,   GL_LUMINANCE },

        { Desktop::LUMINANCE4_ALPHA4,   GL_LUMINANCE_ALPHA },
        { Desktop::LUMINANCE6_ALPHA2,   GL_LUMINANCE_ALPHA },
        { Desktop::LUMINANCE8_ALPHA8,   GL_LUMINANCE_ALPHA },
        { Desktop::LUMINANCE12_ALPHA4,  GL_LUMINANCE_ALPHA },
        { Desktop::LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA },
        { Desktop::LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA },

        { Desktop::SRGB,                  GL_SRGB8 },
        { Desktop::SRGB_ALPHA,            GL_SRGB8_ALPHA8 },
        { Desktop::COMPRESSED_RED,        GL_R8 },
        { Desktop::COMPRESSED_RG,         GL_RG8 },
        { Desktop::COMPRESSED_RGB,        GL_RGB8 },
        { Desktop::COMPRESSED_RGBA,       GL_RGBA8 },
        { Desktop::COMPRESSED_SRGB,       GL_SRGB8 },
        { Desktop::COMPRESSED_SRGB_ALPHA, GL_SRGB8_ALPHA8 }
    };

    const FastMapBI<GLenum, GLenum> driverInternalFormatFixes = {
        { GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8 },

        { GL_R8_SNORM,    GL_R16F },
        { GL_RG8_SNORM,   GL_RG16F },
        { GL_RGBA8_SNORM, GL_RGBA16F },

        { GL_RGB8I,       GL_R11F_G11F_B10F },
        { GL_RGB16I,      GL_R11F_G11F_B10F },
        { GL_RGB32I,      GL_R11F_G11F_B10F },
        { GL_RGB8_SNORM,  GL_R11F_G11F_B10F },
        { GL_RGB16F,      GL_R11F_G11F_B10F },
        { GL_RGB32F,      GL_R11F_G11F_B10F },

        { GL_RGB8UI,      GL_RGB8 }
    };

    const FastMapBI<GLenum, FormatType> canonicalPairs = {
        { GL_RGB,             { GL_RGB,             GL_UNSIGNED_BYTE } },
        { GL_RGBA,            { GL_RGBA,            GL_UNSIGNED_BYTE } },
        { GL_LUMINANCE_ALPHA, { GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE } },
        { GL_LUMINANCE,       { GL_LUMINANCE,       GL_UNSIGNED_BYTE } },
        { GL_ALPHA,           { GL_ALPHA,           GL_UNSIGNED_BYTE } },

        { GL_R8,       { GL_RED, GL_UNSIGNED_BYTE } },
        { GL_R8_SNORM, { GL_RED, GL_BYTE } },
        { GL_R16F,     { GL_RED, GL_HALF_FLOAT } },
        { GL_R32F,     { GL_RED, GL_FLOAT } },

        { GL_R8UI,  { GL_RED_INTEGER, GL_UNSIGNED_BYTE } },
        { GL_R8I,   { GL_RED_INTEGER, GL_BYTE } },
        { GL_R16UI, { GL_RED_INTEGER, GL_UNSIGNED_SHORT } },
        { GL_R16I,  { GL_RED_INTEGER, GL_SHORT } },
        { GL_R32UI, { GL_RED_INTEGER, GL_UNSIGNED_INT } },
        { GL_R32I,  { GL_RED_INTEGER, GL_INT } },

        { GL_RG8,       { GL_RG, GL_UNSIGNED_BYTE } },
        { GL_RG8_SNORM, { GL_RG, GL_BYTE } },
        { GL_RG16F,     { GL_RG, GL_HALF_FLOAT } },
        { GL_RG32F,     { GL_RG, GL_FLOAT } },

        { GL_RG8UI,  { GL_RG_INTEGER, GL_UNSIGNED_BYTE } },
        { GL_RG8I,   { GL_RG_INTEGER, GL_BYTE } },
        { GL_RG16UI, { GL_RG_INTEGER, GL_UNSIGNED_SHORT } },
        { GL_RG16I,  { GL_RG_INTEGER, GL_SHORT } },
        { GL_RG32UI, { GL_RG_INTEGER, GL_UNSIGNED_INT } },
        { GL_RG32I,  { GL_RG_INTEGER, GL_INT } },

        { GL_RGB8,            { GL_RGB, GL_UNSIGNED_BYTE } },
        { GL_SRGB8,           { GL_RGB, GL_UNSIGNED_BYTE } },
        { GL_RGB565,          { GL_RGB, GL_UNSIGNED_BYTE } },
        { GL_RGB8_SNORM,      { GL_RGB, GL_BYTE } },
        { GL_R11F_G11F_B10F,  { GL_RGB, GL_FLOAT } },
        { GL_RGB9_E5,         { GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV } },
        { GL_RGB16F,          { GL_RGB, GL_HALF_FLOAT } },
        { GL_RGB32F,          { GL_RGB, GL_FLOAT } },

        { GL_RGB8UI,  { GL_RGB_INTEGER, GL_UNSIGNED_BYTE } },
        { GL_RGB8I,   { GL_RGB_INTEGER, GL_BYTE } },
        { GL_RGB16UI, { GL_RGB_INTEGER, GL_UNSIGNED_SHORT } },
        { GL_RGB16I,  { GL_RGB_INTEGER, GL_SHORT } },
        { GL_RGB32UI, { GL_RGB_INTEGER, GL_UNSIGNED_INT } },
        { GL_RGB32I,  { GL_RGB_INTEGER, GL_INT } },

        { GL_RGBA8,          { GL_RGBA, GL_UNSIGNED_BYTE } },
        { GL_SRGB8_ALPHA8,   { GL_RGBA, GL_UNSIGNED_BYTE } },
        { GL_RGBA8_SNORM,    { GL_RGBA, GL_BYTE } },
        { GL_RGB5_A1,        { GL_RGBA, GL_UNSIGNED_BYTE } },
        { GL_RGBA4,          { GL_RGBA, GL_UNSIGNED_BYTE } },
        { GL_RGB10_A2,       { GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV } },
        { GL_RGBA16F,        { GL_RGBA, GL_HALF_FLOAT } },
        { GL_RGBA32F,        { GL_RGBA, GL_FLOAT } },

        { GL_RGBA8UI,     { GL_RGBA_INTEGER, GL_UNSIGNED_BYTE } },
        { GL_RGBA8I,      { GL_RGBA_INTEGER, GL_BYTE } },
        { GL_RGB10_A2UI,  { GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV } },
        { GL_RGBA16UI,    { GL_RGBA_INTEGER, GL_UNSIGNED_SHORT } },
        { GL_RGBA16I,     { GL_RGBA_INTEGER, GL_SHORT } },
        { GL_RGBA32UI,    { GL_RGBA_INTEGER, GL_UNSIGNED_INT } },
        { GL_RGBA32I,     { GL_RGBA_INTEGER, GL_INT } }
    };

    constexpr BgraRule bgraRules[] = {
        { Desktop::BGRA, GL_R8,    GL_RED },
        { Desktop::BGRA, GL_RG8,   GL_RG },
        { Desktop::BGRA, GL_RGBA,  GL_RGBA },
        { Desktop::BGRA, GL_RGBA8, GL_RGBA },
        { Desktop::BGR,  GL_RGB,   GL_RGB },
        { Desktop::BGR,  GL_RGB8,  GL_RGB }
    };

    constexpr ResolveRule resolveRules[] = {
        { GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE,  GL_DEPTH_COMPONENT16,  GL_UNSIGNED_SHORT },
        { GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, GL_DEPTH_COMPONENT16,  GL_UNSIGNED_SHORT },
        { GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,   GL_DEPTH_COMPONENT16,  GL_UNSIGNED_INT },
        { GL_DEPTH_COMPONENT, GL_FLOAT,          GL_DEPTH_COMPONENT32F, GL_FLOAT },

        { GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,              GL_DEPTH24_STENCIL8,  GL_UNSIGNED_INT_24_8 },
        { GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_DEPTH32F_STENCIL8, GL_FLOAT_32_UNSIGNED_INT_24_8_REV },

        { GL_RED, GL_BYTE,          GL_R8_SNORM, GL_BYTE },
        { GL_RED, GL_UNSIGNED_BYTE, GL_R8,       GL_UNSIGNED_BYTE },
        { GL_RED, GL_HALF_FLOAT,    GL_R16F,     GL_HALF_FLOAT },
        { GL_RED, GL_FLOAT,         GL_R32F,     GL_FLOAT },

        { GL_RG, GL_BYTE,          GL_RG8_SNORM, GL_BYTE },
        { GL_RG, GL_UNSIGNED_BYTE, GL_RG8,       GL_UNSIGNED_BYTE },
        { GL_RG, GL_HALF_FLOAT,    GL_RG16F,     GL_HALF_FLOAT },
        { GL_RG, GL_FLOAT,         GL_RG32F,     GL_FLOAT }
    };

    constexpr GLenum esDepthInternalFormats[] = {
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH24_STENCIL8,
        GL_DEPTH32F_STENCIL8
    };

    constexpr GLenum esFormats[] = {
        GL_RED, GL_RG, GL_RGB, GL_RGBA,
        GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER,
        GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_ALPHA,
        GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
    };

    constexpr GLenum esTypes[] = {
        GL_BYTE, GL_UNSIGNED_BYTE,
        GL_SHORT, GL_UNSIGNED_SHORT,
        GL_INT, GL_UNSIGNED_INT,
        GL_HALF_FLOAT, GL_FLOAT,
        GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1,
        GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV,
        GL_UNSIGNED_INT_24_8, GL_FLOAT_32_UNSIGNED_INT_24_8_REV
    };

    template <typename Map>
    const typename Map::mapped_type* lookup(const Map& map, GLenum key) {
        const auto it = map.find(key);
        return it == map.end() ? nullptr : &it->second;
    }

    template <std::size_t N>
    bool contains(const GLenum (&values)[N], GLenum value) {
        return std::find(std::begin(values), std::end(values), value) != std::end(values);
    }

    void warnOnce(Field field, GLenum value) {
        static thread_local std::unordered_set<uint64_t> reported;
        const uint64_t key = (static_cast<uint64_t>(field) << 32) | value;
        if (!reported.insert(key).second) return;

        LOGW("no ES 3.2 equivalent for %s 0x%x", fieldNames[static_cast<std::size_t>(field)], value);
    }

    GLenum toEsInternalFormat(GLenum internalFormat, bool isShadowMap) {
        if (internalFormat == GL_DEPTH_COMPONENT && isShadowMap) return GL_DEPTH_COMPONENT32F;

        if (const auto* es = lookup(desktopToEsInternalFormat, internalFormat)) internalFormat = *es;
        if (const auto* fixed = lookup(driverInternalFormatFixes, internalFormat)) internalFormat = *fixed;

        return internalFormat;
    }

    void applyCanonicalPair(GLenum internalFormat, GLenum& format, GLenum& type) {
        const auto* pair = lookup(canonicalPairs, internalFormat);
        if (!pair) return;

        format = pair->format;
        type = pair->type;
    }

    void applyBgraSwizzle(GLenum sourceFormat, GLenum sourceType, GLenum internalFormat, GLenum& format, GLenum& type) {
        for (const auto& rule : bgraRules) {
            if (rule.sourceFormat != sourceFormat || rule.internalFormat != internalFormat) continue;

            format = rule.format;
            type = sourceType;
            swizzleBGRA(type);
            return;
        }
    }

    void resolveInternalFormat(GLenum& internalFormat, GLenum format, GLenum& type) {
        for (const auto& rule : resolveRules) {
            if (rule.format != format || rule.type != type) continue;

            internalFormat = rule.internalFormat;
            type = rule.resultType;
            return;
        }
    }

    void reportUnsupported(GLenum internalFormat, GLenum format, GLenum type) {
        if (!lookup(canonicalPairs, internalFormat) && !contains(esDepthInternalFormats, internalFormat)) {
            warnOnce(Field::InternalFormat, internalFormat);
        }

        if (!contains(esFormats, format)) warnOnce(Field::Format, format);
        if (!contains(esTypes, type)) warnOnce(Field::Type, type);
    }
}

void fixTexArguments(
    GLenum& target,
    GLint& internalFormat,
    GLenum& type,
    GLenum& format,
    bool isShadowMap
) {
    const GLenum sourceFormat = format;
    const GLenum sourceType = type;

    GLenum esInternalFormat = toEsInternalFormat(static_cast<GLenum>(internalFormat), isShadowMap);

    applyCanonicalPair(esInternalFormat, format, type);
    applyBgraSwizzle(sourceFormat, sourceType, esInternalFormat, format, type);
    resolveInternalFormat(esInternalFormat, format, type);
    reportUnsupported(esInternalFormat, format, type);

    internalFormat = static_cast<GLint>(esInternalFormat);
    doSwizzling(target);
}
