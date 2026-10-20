#pragma once

#include <GLES3/gl3.h>

// Unpacks a 32‑bit packed value for GL_UNSIGNED_INT_2_10_10_10_REV or
// GL_INT_2_10_10_10_REV into 4 floating‐point components.
// The 'normalized' flag indicates whether the result should be normalized.
// The 'isSigned' flag distinguishes between signed and unsigned packing.
inline void unpack_2_10_10_10_REV(GLuint value, GLfloat out[4],
                                GLboolean normalized, GLboolean isSigned) {
    if (!isSigned) {
        // Unsigned case.
        if (normalized) {
            out[0] = (value & 0x3FF) / 1023.0f;
            out[1] = ((value >> 10) & 0x3FF) / 1023.0f;
            out[2] = ((value >> 20) & 0x3FF) / 1023.0f;
            out[3] = ((value >> 30) & 0x3) / 3.0f;
        } else {
            out[0] = static_cast<GLfloat>(value & 0x3FF);
            out[1] = static_cast<GLfloat>((value >> 10) & 0x3FF);
            out[2] = static_cast<GLfloat>((value >> 20) & 0x3FF);
            out[3] = static_cast<GLfloat>((value >> 30) & 0x3);
        }
    } else {
        // Signed case: sign-extend 10-bit values.
        auto extend = [](GLuint comp) -> GLint {
            return (comp & 0x200) ? static_cast<GLint>(comp) - 1024 : static_cast<GLint>(comp);
        };
        if (normalized) {
            GLint r = extend(value & 0x3FF);
            GLint g = extend((value >> 10) & 0x3FF);
            GLint b = extend((value >> 20) & 0x3FF);
            // Typically the 2-bit field (resultColorAlpha) remains unsigned.
            GLint a = (value >> 30) & 0x3;
            out[0] = r / 511.0f;
            out[1] = g / 511.0f;
            out[2] = b / 511.0f;
            out[3] = a / 3.0f;
        } else {
            GLint r = extend(value & 0x3FF);
            GLint g = extend((value >> 10) & 0x3FF);
            GLint b = extend((value >> 20) & 0x3FF);
            GLint a = (value >> 30) & 0x3;
            out[0] = static_cast<GLfloat>(r);
            out[1] = static_cast<GLfloat>(g);
            out[2] = static_cast<GLfloat>(b);
            out[3] = static_cast<GLfloat>(a);
        }
    }
}

// General unpack function for supported packed types.
// Currently supports GL_UNSIGNED_INT_2_10_10_10_REV and GL_INT_2_10_10_10_REV.
// Extend this switch as additional packed formats are needed.
inline void unpackPackedValue(GLenum type, GLuint value, GLfloat out[4], GLboolean normalized) {
    switch (type) {
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            unpack_2_10_10_10_REV(value, out, normalized, GL_FALSE);
            break;
        case GL_INT_2_10_10_10_REV:
            unpack_2_10_10_10_REV(value, out, normalized, GL_TRUE);
            break;
        default:
            // For unsupported types, set to zeros.
            out[0] = out[1] = out[2] = out[3] = 0.0f;
            break;
    }
}
