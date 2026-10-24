#pragma once

#include "common/enums.hpp"
#include "common/log.hpp"
#include "common/types.hpp"

#include <atomic>
#include <GLES3/gl31.h>
#include <unordered_set>

inline std::atomic_bool esUtilsInitialized = false;

namespace ESUtils {

inline void initExtensionsES3();

inline std::pair<int, int> version = std::make_pair(0, 0); // major, minor
inline int shadingVersion; // (major * 100) + (minor * 10)

inline std::unordered_set<std::string> realExtensions;
inline std::unordered_set<std::string> fakeExtensions;

inline bool isAngle = false;
inline std::tuple<int, int, int> angleVersion = std::make_tuple(0, 0, 0);

inline void init() {
    if (esUtilsInitialized.load()) return;

    str versionStr = reinterpret_cast<str>(glGetString(GL_VERSION));
    if (!versionStr) {
        throw std::runtime_error("Failed to get OpenGL ES version! Is the context current or is there no context at all?");
    }

    int major = 0, minor = 0;
    if (sscanf(versionStr, "OpenGL ES %d.%d", &major, &minor) != 2) {
        throw std::runtime_error("Failed to get OpenGL ES version! Is the formatting different? (" + std::string(versionStr) + ")");
    }

    version = std::make_pair(major, minor);
    shadingVersion = (major * 100) + (minor * 10); // 3 -> 300, 2 -> 20 = 320 = 3.2

    int angleMajor = 0, angleMinor = 0, anglePatch = 0; // ts just made up
    if (sscanf(versionStr, "(ANGLE %d.%d.%d", &angleMajor, &angleMinor, &anglePatch) == 3) {
        isAngle = true;
        angleVersion = std::make_tuple(angleMajor, angleMinor, anglePatch);
    }

    switch (major) {
        case 1:
            throw std::runtime_error("OpenGL ES 1.0 is NOT supported");
        case 2:
            throw std::runtime_error("OpenGL ES 2.0 is NOT supported");
        case 3:
            switch (minor) {
                case 2:
                    initExtensionsES3();
                    break;
                default:
                    throw std::runtime_error("OpenGL ES >3.0 & <3.2 is NOT supported");
        }
    }

    fakeExtensions = realExtensions;

    esUtilsInitialized.store(true);
}

inline bool isExtensionSupported(std::string name) {
    if (!esUtilsInitialized.load()) {
        LOGW("Extension set wasn't initialized!");
        ESUtils::init();
    }
    return realExtensions.find(name) != realExtensions.end();
}


inline void initExtensionsES3() {
    GLint extensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

    for (GLint i = 0; i < extensionCount; ++i) {
        str extension = reinterpret_cast<str>(glGetStringi(GL_EXTENSIONS, i));
        if (extension) ESUtils::realExtensions.insert(std::string(extension));
    }
}

inline GLenum getComponentTypeFromFormat(GLint format) {
    // Per OpenGL ES 3.0 spec, map internal formats to component types
    // TODO: Update to GLES 3.2 spec, add GL 4.6 formats
    switch (format) {
        // Float formats
        case GL_R32F:
        case GL_RG32F:
        case GL_RGB32F:
        case GL_RGBA32F:
        case GL_R16F:
        case GL_RG16F:
        case GL_RGB16F:
        case GL_RGBA16F:
            return GL_FLOAT;

        // Integer formats
        case GL_R8I:
        case GL_R16I:
        case GL_R32I:
        case GL_RG8I:
        case GL_RG16I:
        case GL_RG32I:
        case GL_RGB8I:
        case GL_RGB16I:
        case GL_RGB32I:
        case GL_RGBA8I:
        case GL_RGBA16I:
        case GL_RGBA32I:
            return GL_INT;

        // Unsigned integer formats
        case GL_R8UI:
        case GL_R16UI:
        case GL_R32UI:
        case GL_RG8UI:
        case GL_RG16UI:
        case GL_RG32UI:
        case GL_RGB8UI:
        case GL_RGB16UI:
        case GL_RGB32UI:
        case GL_RGBA8UI:
        case GL_RGBA16UI:
        case GL_RGBA32UI:
            return GL_UNSIGNED_INT;

        // Normalized formats
        case GL_R8:
        case GL_RG8:
        case GL_RGB8:
        case GL_RGBA8:
        case 0x822a: // GL_R16
        case 0x822c: // GL_RG16
        case 0x8050: // GL_RGB16
        case 0x805b: // GL_RGBA16
        case GL_RGB10_A2:
            return GL_UNSIGNED_NORMALIZED;

        // Signed normalized formats
        case GL_R8_SNORM:
        case GL_RG8_SNORM:
        case GL_RGB8_SNORM:
        case GL_RGBA8_SNORM:
            return GL_SIGNED_NORMALIZED;

        // Depth formats
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
            return GL_FLOAT;

        default:
            // Default to unsigned normalized for common formats
            return GL_UNSIGNED_NORMALIZED;
    }
}

inline bool isSRGBFormat(GLint format) {
    switch (format) {
        case GL_SRGB:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case 0x8c48: // GL_COMPRESSED_SRGB
        case 0x8c49: // GL_COMPRESSED_SRGB_ALPHA
            return true;
        default:
            return false;
    }
}

namespace TypeTraits {

template <typename T>
struct GLTypeEnum;

template<GLenum T>
struct GLPrimitive;

#define GL_TYPE_ENUM(podt, glt) template<> struct GLTypeEnum<podt> { static constexpr GLenum value = glt; }
#define GL_PRIMITIVE(glt, podt) template<> struct GLPrimitive<glt> { using type = podt; }

GL_TYPE_ENUM(GLubyte, GL_UNSIGNED_BYTE);
GL_TYPE_ENUM(GLuint, GL_UNSIGNED_INT);
GL_TYPE_ENUM(GLshort, GL_SHORT);
GL_TYPE_ENUM(GLint, GL_INT);
GL_TYPE_ENUM(GLfloat, GL_FLOAT);
GL_TYPE_ENUM(GLdouble, GL_DOUBLE);

GL_PRIMITIVE(GL_UNSIGNED_BYTE, GLubyte);
GL_PRIMITIVE(GL_UNSIGNED_INT, GLuint);
GL_PRIMITIVE(GL_SHORT, GLshort);
GL_PRIMITIVE(GL_INT, GLint);
GL_PRIMITIVE(GL_FLOAT, GLfloat);
GL_PRIMITIVE(GL_DOUBLE, GLdouble);

inline GLsizei getTypeSize(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
        case GL_UNSIGNED_INT: return sizeof(GLuint);
        case GL_SHORT: return sizeof(GLshort);
        case GL_INT: return sizeof(GLint);
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_DOUBLE: return sizeof(GLdouble);
        default:
            LOGE("Unhandled type! (type=%u)", type);
            throw std::runtime_error("getTypeSize : Unsupported GL type");
    }
}

template<typename Func>
void typeToPrimitive(GLenum type, const Func&& func) {
    switch (type) {
        case GL_UNSIGNED_BYTE:
            func.template operator()<GLPrimitive<GL_UNSIGNED_BYTE>::type>();
            break;

        case GL_UNSIGNED_INT:
            func.template operator()<GLPrimitive<GL_UNSIGNED_INT>::type>();
            break;

        case GL_SHORT:
            func.template operator()<GLPrimitive<GL_SHORT>::type>();
            break;

        case GL_INT:
            func.template operator()<GLPrimitive<GL_INT>::type>();
            break;

        case GL_FLOAT:
            func.template operator()<GLPrimitive<GL_FLOAT>::type>();
            break;

        case GL_DOUBLE:
            func.template operator()<GLPrimitive<GL_DOUBLE>::type>();
            break;

        default:
            LOGE("Unhandled type! (type=%u)", type);
            throw std::runtime_error("typeToPrimitive : Unsupported GL type");
    }
}

template<typename T>
inline const T* asTypedArray(const void* array) {
    return static_cast<const T*>(array);
}

}

}
