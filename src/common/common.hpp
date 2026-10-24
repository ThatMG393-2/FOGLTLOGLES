#pragma once

#include <unordered_set>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <cstdio>
#include <android/log.h>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <atomic>
#include <GLES3/gl31.h>

typedef void (*FunctionPtr)(void);

typedef char const* str;

#ifndef EXTERN
#define EXTERN extern "C"
#endif

#ifndef PUBLIC
#define PUBLIC __attribute__((visibility("default")))
#endif

#ifndef PUBLIC_API
#define PUBLIC_API PUBLIC EXTERN
#endif

#ifndef DEFINE_API
#define DEFINE_API(type, func, ...) PUBLIC_API type func __VA_ARGS__
#endif

namespace FOGLTLOGLES {
} /* namespace FOGLTLOGLES */

template<typename T, typename... Args>
inline std::shared_ptr<T> MakeAggregateShared(Args&&... args) {
    if constexpr (std::is_aggregate_v<T>) {
        // aggregate initialization
        return std::make_shared<T>(T{std::forward<Args>(args)...});
    } else {
        // plain-ol constructor
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char ubyte;

inline float clampf(const float f, const float min, const float max) {
  const float t = f < min ? min : f;
  return t > max ? max : t;
}

inline uint float_to_uint(const float x) {
    return static_cast<uint>(x);
}

inline float uint_to_float(const uint x) {
    return static_cast<float>(x);
}

inline float half_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint e = (x & 0x7C00) >> 10; // exponent
    const uint m = (x & 0x03FF) << 13; // mantissa
    const uint v = float_to_uint((float) m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
    return uint_to_float((x & 0x8000) << 16
			| (e != 0) * ((e+112) << 23 | m)
			| ((e == 0) & (m != 0)) * ((v - 37) << 23
			| ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
}

inline ubyte float_to_ubyte(const float f) {
	return static_cast<ubyte>(clampf(f, 0.0f, 1.0f) * 255.0f);
}

inline uint32_t float_to_bits(float f) {
    uint32_t out;
    std::memcpy(&out, &f, sizeof(float));
    return out;
}

template<typename ... Args>
inline std::string string_format(const std::string& format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if( size_s <= 0 ) throw std::runtime_error("Error during formatting.");
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[ size ]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

inline std::string join_set(const std::unordered_set<std::string>& uset, const std::string& delimiter) {
    std::string result;
    for (auto it = uset.begin(); it != uset.end(); ++it) {
        result += *it;
        if (std::next(it) != uset.end()) {
            result += delimiter;
        }
    }
    return result;
}

#ifndef TAG
#define TAG "FOGLTLOGLES"
#endif

#define PRINTF_LOG

#ifdef PRINTF_LOG
    #define LOGE(...) std::printf( TAG " E: " __VA_ARGS__ ), std::printf("\n")
    #define LOGW(...) std::printf( TAG " W: " __VA_ARGS__ ), std::printf("\n")
    #define LOGI(...) std::printf( TAG " I: " __VA_ARGS__ ), std::printf("\n")
    #define LOGD(...) std::printf( TAG " D: " __VA_ARGS__ ), std::printf("\n")
#else
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#endif

inline std::string getEnvironmentVar(std::string const& key, std::string const& fallback = "") {
    char* val = getenv( key.c_str() );
    return val == NULL ? fallback : std::string(val);
}

inline bool envFlag(const char* key, const char* fallback) {
    return getEnvironmentVar(key, fallback) == "1";
}

inline const bool SHADER_DUMP = envFlag("LIBGL_VGPU_DUMP", "0");
inline const bool ENV_NO_ERROR = envFlag("LIBGL_NO_ERROR", "0");

inline bool ENABLE_SHADER_CACHE = envFlag("FOGLE_ENABLE_SHADER_CACHE", "1");
inline bool ENABLE_BINARY_CACHE = envFlag("FOGLE_ENABLE_BINARY_CACHE", "1");

inline const std::string SHADER_CACHE_DIR = getEnvironmentVar("MESA_GLSL_CACHE_DIR");
inline const uint64_t SHADER_RETENTION_DAYS = std::strtoull(getEnvironmentVar("FOGLE_CACHE_RETENTION_DAY", "30").c_str(), nullptr, 10);

template<typename K>
class FastHasher {
public:
    size_t operator()(const K& key) const {
        return key; // LLVM method 💀
        // i dont even think we'll have collisions
        // since we return the key and its always unique
    }
};

// Keys should be builtins only!
#define FAST_MAP_BI(key, value) std::unordered_map<key, value, FastHasher<key>>

template <typename Key, typename Value>
using FastMapBI = std::unordered_map<Key, Value, FastHasher<Key>>;

#define GETVAL(type, name, dest) glGet##type##v(name, dest)

namespace ESLimits {
    inline GLint MAX_TEXTURE_SIZE = 0;
    inline GLfloat MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0;

    inline void init() {
        GETVAL(Integer, GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
        GETVAL(Float, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MAX_TEXTURE_MAX_ANISOTROPY_EXT);

        LOGI("GL_MAX_TEXTURE_SIZE: %d", MAX_TEXTURE_SIZE);
        LOGI("GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT: %f", MAX_TEXTURE_MAX_ANISOTROPY_EXT);
    }
}

typedef double   GLdouble;
typedef uint64_t GLbitfield64;

// types
#define GL_DOUBLE 0x140a

// States

// color
#define GL_COLOR_MATERIAL 0xb57

// FFP

// arrays
#define GL_COLOR_ARRAY         0x8076
#define GL_NORMAL_ARRAY        0x8075
#define GL_TEXTURE_COORD_ARRAY 0x8078

// alpha
#define GL_ALPHA_TEST 0xbc0

#define GL_NEVER    0x0200
#define GL_LESS     0x0201
#define GL_EQUAL    0x0202
#define GL_LEQUAL   0x0203
#define GL_GREATER  0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL   0x0206
#define GL_ALWAYS   0x0207

// shading
#define GL_FLAT   0x1d00
#define GL_SMOOTH 0x1d0

// fog
#define GL_FOG         0x0B60
#define GL_FOG_MODE    0x0B65
#define GL_FOG_COLOR   0x0B66
#define GL_FOG_START   0x0B63
#define GL_FOG_END     0x0B64
#define GL_FOG_DENSITY 0x0B62

#define GL_EXP  0x0800
#define GL_EXP2 0x0801

// lighting
#define GL_LIGHTING 0xb50

// draw array
// #define GL_QUADS 0x7

// Matrices
#define GL_MODELVIEW  0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE    0x1702

// Lists
#define GL_COMPILE             0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301

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
