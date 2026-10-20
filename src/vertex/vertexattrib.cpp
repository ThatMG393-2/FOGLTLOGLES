/**
 * OpenGL 4.5 glVertexAttrib* wrappers for OpenGL ES 3.2
 *
 * This file implements as many of the OpenGL 4.5 glVertexAttrib* functions
 * as possible by mapping them to their OpenGL ES 3.2 equivalents.
 *
 * Note:
 *  - Double-precision (L*) versions are not supported in OpenGL ES. (will workaround soon)
 *  - Packed attribute functions (glVertexAttribP*ui) are not available in OpenGL ES.
 *
 *
 * For functions that take non‐float scalar parameters (short, double, etc.), we
 * simply cast (or convert) the value and call the corresponding ES function.
 *
 * For vector versions, we dereference the pointer and call the scalar version.
*/

#include "vertex/vertexattrib.hpp"

#include "vertex/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl3.h>

typedef double GLdouble;

/* -------------------------------
Scalar Versions – Float, Short, Double, and Integer
-------------------------------*/

/* Already available in ES: glVertexAttrib1f, glVertexAttrib2f, glVertexAttrib3f, glVertexAttrib4f*/

/* glVertexAttrib1s: convert GLshort to GLfloat*/
void glVertexAttrib1s(GLuint index, GLshort v0) {
    glVertexAttrib1f(index, static_cast<GLfloat>(v0));
}
 
/* glVertexAttrib1d: convert GLdouble to GLfloat*/
void glVertexAttrib1d(GLuint index, GLdouble v0) {
    glVertexAttrib1f(index, static_cast<GLfloat>(v0));
}
 
/* glVertexAttribI1i: use glVertexAttribI4i with defaults*/
void glVertexAttribI1i(GLuint index, GLint v0) {
    glVertexAttribI4i(index, v0, 0, 0, 1);
}
 
/* glVertexAttribI1ui: use glVertexAttribI4ui with defaults*/
void glVertexAttribI1ui(GLuint index, GLuint v0) {
    glVertexAttribI4ui(index, v0, 0, 0, 1);
}
 
/* glVertexAttrib2s:*/
void glVertexAttrib2s(GLuint index, GLshort v0, GLshort v1) {
    glVertexAttrib2f(index, static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
 
/* glVertexAttrib2d:*/
void glVertexAttrib2d(GLuint index, GLdouble v0, GLdouble v1) {
    glVertexAttrib2f(index, static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
 
/* glVertexAttribI2i:*/
void glVertexAttribI2i(GLuint index, GLint v0, GLint v1) {
    glVertexAttribI4i(index, v0, v1, 0, 1);
}
 
/* glVertexAttribI2ui:*/
void glVertexAttribI2ui(GLuint index, GLuint v0, GLuint v1) {
    glVertexAttribI4ui(index, v0, v1, 0, 1);
}
 
/* glVertexAttrib3s:*/
void glVertexAttrib3s(GLuint index, GLshort v0, GLshort v1, GLshort v2) {
    glVertexAttrib3f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2)
    );
}
 
/* glVertexAttrib3d:*/
void glVertexAttrib3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2) {
    glVertexAttrib3f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2)
    );
}
 
/* glVertexAttribI3i:*/
void glVertexAttribI3i(GLuint index, GLint v0, GLint v1, GLint v2) {
    glVertexAttribI4i(index, v0, v1, v2, 1);
}
 
/* glVertexAttribI3ui:*/
void glVertexAttribI3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2) {
    glVertexAttribI4ui(index, v0, v1, v2, 1);
}
 
/* glVertexAttrib4f is provided by ES*/
 
/* glVertexAttrib4s:*/
void glVertexAttrib4s(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3) {
    glVertexAttrib4f(index, 
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2),
        static_cast<GLfloat>(v3)
    );
}
 
/* glVertexAttrib4d:*/
void glVertexAttrib4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) {
    glVertexAttrib4f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2),
        static_cast<GLfloat>(v3)
    );
}
 
/* glVertexAttrib4Nub: for normalized unsigned bytes*/
void glVertexAttrib4Nub(GLuint index, GLubyte v0, GLubyte v1, GLubyte v2, GLubyte v3) {
    glVertexAttrib4f(index,
        v0 / 255.0f,
        v1 / 255.0f,
        v2 / 255.0f,
        v3 / 255.0f
    );
}
 
/* glVertexAttribI4i and glVertexAttribI4ui are provided by ES*/

void glVertexAttribL1d(GLuint index, GLdouble v0) {
    glVertexAttrib1f(index, static_cast<GLfloat>(v0));
}
 
void glVertexAttribL2d(GLuint index, GLdouble v0, GLdouble v1) {
    glVertexAttrib2f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1)
    );
}
 
void glVertexAttribL3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2) {
    glVertexAttrib3f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2)
    );
}
 
void glVertexAttribL4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) {
    glVertexAttrib4f(index,
        static_cast<GLfloat>(v0),
        static_cast<GLfloat>(v1),
        static_cast<GLfloat>(v2),
        static_cast<GLfloat>(v3)
    );
}
 
 
/*  -------------------------------
    Vector Versions (pointer based)
    -------------------------------

    For float vectors, the ES functions exist; for other types, call the scalar version
    for each element.
*/
 
/* glVertexAttrib1fv is provided natively in ES but we implement a wrapper for consistency*/
void glVertexAttrib1fv(GLuint index, const GLfloat *v) {
    glVertexAttrib1f(index, v[0]);
}
 
void glVertexAttrib1sv(GLuint index, const GLshort *v) {
    glVertexAttrib1s(index, v[0]);
}
 
void glVertexAttrib1dv(GLuint index, const GLdouble *v) {
    glVertexAttrib1d(index, v[0]);
}

void glVertexAttribI1iv(GLuint index, const GLint *v) {
    glVertexAttribI1i(index, v[0]);
}

void glVertexAttribI1uiv(GLuint index, const GLuint *v) {
    glVertexAttribI1ui(index, v[0]);
}

void glVertexAttrib2fv(GLuint index, const GLfloat *v) {
    glVertexAttrib2f(index, v[0], v[1]);
}

void glVertexAttrib2sv(GLuint index, const GLshort *v) {
    glVertexAttrib2s(index, v[0], v[1]);
}
 
void glVertexAttrib2dv(GLuint index, const GLdouble *v) {
    glVertexAttrib2d(index, v[0], v[1]);
}

void glVertexAttribI2iv(GLuint index, const GLint *v) {
    glVertexAttribI2i(index, v[0], v[1]);
}
 
void glVertexAttribI2uiv(GLuint index, const GLuint *v) {
    glVertexAttribI2ui(index, v[0], v[1]);
}
 
void glVertexAttrib3fv(GLuint index, const GLfloat *v) {
    glVertexAttrib3f(index, v[0], v[1], v[2]);
}
 
void glVertexAttrib3sv(GLuint index, const GLshort *v) {
    glVertexAttrib3s(index, v[0], v[1], v[2]);
}
 
void glVertexAttrib3dv(GLuint index, const GLdouble *v) {
    glVertexAttrib3d(index, v[0], v[1], v[2]);
}
 
void glVertexAttribI3iv(GLuint index, const GLint *v) {
    glVertexAttribI3i(index, v[0], v[1], v[2]);
}
 
void glVertexAttribI3uiv(GLuint index, const GLuint *v) {
    glVertexAttribI3ui(index, v[0], v[1], v[2]);
}
 
void glVertexAttrib4fv(GLuint index, const GLfloat *v) {
    glVertexAttrib4f(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttrib4sv(GLuint index, const GLshort *v) {
    glVertexAttrib4s(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttrib4dv(GLuint index, const GLdouble *v) {
    glVertexAttrib4d(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttrib4iv(GLuint index, const GLint *v) {
    // No direct ES equivalent; use glVertexAttribI4i wrapper
    glVertexAttribI4i(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttrib4bv(GLuint index, const GLbyte *v) {
    // Translate byte values by converting to GLint
    glVertexAttribI4i(index,
        static_cast<GLint>(v[0]),
        static_cast<GLint>(v[1]),
        static_cast<GLint>(v[2]),
        static_cast<GLint>(v[3])
    );
}
 
void glVertexAttrib4ubv(GLuint index, const GLubyte *v) {
    glVertexAttribI4ui(index,
        static_cast<GLuint>(v[0]),
        static_cast<GLuint>(v[1]),
        static_cast<GLuint>(v[2]),
        static_cast<GLuint>(v[3])
    );
}
 
void glVertexAttrib4usv(GLuint index, const GLushort *v) {
    // Promote to GLuint via cast and call glVertexAttribI4ui
    glVertexAttribI4ui(index,
        static_cast<GLuint>(v[0]),
        static_cast<GLuint>(v[1]),
        static_cast<GLuint>(v[2]),
        static_cast<GLuint>(v[3])
    );
}
 
void glVertexAttrib4uiv(GLuint index, const GLuint *v) {
    // Use glVertexAttribI4ui directly
    glVertexAttribI4ui(index, v[0], v[1], v[2], v[3]);
}
 
/* Normalized attribute versions: convert values to floats in [0,1] (or [-1,1] for signed)*/
 
/* glVertexAttrib4Nbv: for signed bytes*/
inline float bnormalize(GLbyte x) {
    return x < 0 ? x / 128.0f : x / 127.0f;
}
 
void glVertexAttrib4Nbv(GLuint index, const GLbyte *v) {
    glVertexAttrib4f(index, bnormalize(v[0]), bnormalize(v[1]), bnormalize(v[2]), bnormalize(v[3]));
}
 
void glVertexAttrib4Nsv(GLuint index, const GLshort *v) {
    glVertexAttrib4f(index,
        v[0] / 32767.0f,
        v[1] / 32767.0f,
        v[2] / 32767.0f,
        v[3] / 32767.0f
    );
}
 
void glVertexAttrib4Niv(GLuint index, const GLint *v) {
    glVertexAttrib4f(index,
        v[0] / 2147483647.0f,
        v[1] / 2147483647.0f,
        v[2] / 2147483647.0f,
        v[3] / 2147483647.0f
    );
}
 
void glVertexAttrib4Nubv(GLuint index, const GLubyte *v) {
    glVertexAttrib4Nub(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttrib4Nusv(GLuint index, const GLushort *v) {
    glVertexAttrib4f(index,
        v[0] / 65535.0f,
        v[1] / 65535.0f,
        v[2] / 65535.0f,
        v[3] / 65535.0f
    );
}
 
void glVertexAttrib4Nuiv(GLuint index, const GLuint *v) {
    // Note: Floating-point precision may be limited.
    glVertexAttrib4f(index,
        v[0] / 4294967295.0f,
        v[1] / 4294967295.0f,
        v[2] / 4294967295.0f,
        v[3] / 4294967295.0f
    );
}
 
void glVertexAttribI4bv(GLuint index, const GLbyte *v) {
    glVertexAttribI4i(index,
        static_cast<GLint>(v[0]),
        static_cast<GLint>(v[1]),
        static_cast<GLint>(v[2]),
        static_cast<GLint>(v[3])
    );
}
 
void glVertexAttribI4ubv(GLuint index, const GLubyte *v) {
    glVertexAttribI4ui(index,
        static_cast<GLuint>(v[0]),
        static_cast<GLuint>(v[1]),
        static_cast<GLuint>(v[2]),
        static_cast<GLuint>(v[3])
    );
}
 
void glVertexAttribI4sv(GLuint index, const GLshort *v) {
    glVertexAttribI4i(index,
        static_cast<GLint>(v[0]),
        static_cast<GLint>(v[1]),
        static_cast<GLint>(v[2]),
        static_cast<GLint>(v[3])
    );
}
 
void glVertexAttribI4usv(GLuint index, const GLushort *v) {
    glVertexAttribI4ui(index,
        static_cast<GLuint>(v[0]),
        static_cast<GLuint>(v[1]),
        static_cast<GLuint>(v[2]),
        static_cast<GLuint>(v[3])
    );
}
 
void glVertexAttribI4iv(GLuint index, const GLint *v) {
    glVertexAttribI4i(index, v[0], v[1], v[2], v[3]);
}
 
void glVertexAttribI4uiv(GLuint index, const GLuint *v) {
    glVertexAttribI4ui(index, v[0], v[1], v[2], v[3]);
}
 
/*  -------------------------------
    Double-Precision Vector Versions (L*)
    -------------------------------
    Not Supported in ES, use our implementation: glVertexAttribL*d(...)
*/
void glVertexAttribL1dv(GLuint index, const GLdouble *v) {
    glVertexAttribL1d(index, v[0]);
}
 
void glVertexAttribL2dv(GLuint index, const GLdouble *v) {
    glVertexAttribL2d(index, v[0], v[1]);
}
 
void glVertexAttribL3dv(GLuint index, const GLdouble *v) {
    glVertexAttribL3d(index, v[0], v[1], v[2]);
}
 
void glVertexAttribL4dv(GLuint index, const GLdouble *v) {
    glVertexAttribL4d(index, v[0], v[1], v[2], v[3]);
}
 
/*  -------------------------------
    Packed Attribute Versions (glVertexAttribP*ui)
    -------------------------------
    These functions are not available in OpenGL ES 3.2 but can be
    partially emulated.
*/
void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    GLfloat comps[4];
    unpackPackedValue(type, value, comps, normalized);
    if (normalized) {
        glVertexAttrib1f(index, comps[0]);
   } else {
        glVertexAttribI4ui(index, static_cast<GLuint>(comps[0]), 0, 0, 1);
   }
}
 
void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    GLfloat comps[4];
    unpackPackedValue(type, value, comps, normalized);
    if (normalized) {
        glVertexAttrib2f(index, comps[0], comps[1]);
   } else {
        glVertexAttribI4ui(index,
            static_cast<GLuint>(comps[0]),
            static_cast<GLuint>(comps[1]), 0, 1
        );
   }
}
 
void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    GLfloat comps[4];
    unpackPackedValue(type, value, comps, normalized);
    if (normalized) {
        glVertexAttrib3f(index, comps[0], comps[1], comps[2]);
   } else {
        glVertexAttribI4ui(index,
            static_cast<GLuint>(comps[0]),
            static_cast<GLuint>(comps[1]),
            static_cast<GLuint>(comps[2]), 1
        );
   }
}
 
void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    GLfloat comps[4];
    unpackPackedValue(type, value, comps, normalized);
    if (normalized) {
        glVertexAttrib4f(index, comps[0], comps[1], comps[2], comps[3]);
   } else {
        // For unsigned types, use the unsigned integer version.
        if (type == GL_UNSIGNED_INT_2_10_10_10_REV) {
            glVertexAttribI4ui(index,
                static_cast<GLuint>(comps[0]),
                static_cast<GLuint>(comps[1]),
                static_cast<GLuint>(comps[2]),
                static_cast<GLuint>(comps[3])
            );
        } else {
            glVertexAttribI4i(index,
                static_cast<GLint>(comps[0]),
                static_cast<GLint>(comps[1]),
                static_cast<GLint>(comps[2]),
                static_cast<GLint>(comps[3])
            );
        }
   }
}
 
void Vertex::init() {
    REGISTER(glVertexAttrib1s);
    REGISTER(glVertexAttrib1d);
    REGISTER(glVertexAttribI1i);
    REGISTER(glVertexAttribI1ui);
    
    REGISTER(glVertexAttrib2s);
    REGISTER(glVertexAttrib2d);
    REGISTER(glVertexAttribI2i);
    REGISTER(glVertexAttribI2ui);
    
    REGISTER(glVertexAttrib3s);
    REGISTER(glVertexAttrib3d);
    REGISTER(glVertexAttribI3i);
    REGISTER(glVertexAttribI3ui);
    
    REGISTER(glVertexAttrib4s);
    REGISTER(glVertexAttrib4d);
    REGISTER(glVertexAttrib4Nub);
    
    REGISTER(glVertexAttrib1fv);
    REGISTER(glVertexAttrib1sv);
    REGISTER(glVertexAttrib1dv);
    REGISTER(glVertexAttribI1iv);
    REGISTER(glVertexAttribI1uiv);
 
    REGISTER(glVertexAttrib2fv);
    REGISTER(glVertexAttrib2sv);
    REGISTER(glVertexAttrib2dv);
    REGISTER(glVertexAttribI2iv);
    REGISTER(glVertexAttribI2uiv);
 
    REGISTER(glVertexAttrib3fv);
    REGISTER(glVertexAttrib3sv);
    REGISTER(glVertexAttrib3dv);
    REGISTER(glVertexAttribI3iv);
    REGISTER(glVertexAttribI3uiv);
 
    REGISTER(glVertexAttrib4fv);
    REGISTER(glVertexAttrib4sv);
    REGISTER(glVertexAttrib4dv);
    REGISTER(glVertexAttrib4iv);
    REGISTER(glVertexAttrib4bv);
    REGISTER(glVertexAttrib4ubv);
    REGISTER(glVertexAttrib4usv);
    REGISTER(glVertexAttrib4uiv);
 
    REGISTER(glVertexAttrib4Nbv);
    REGISTER(glVertexAttrib4Nsv);
    REGISTER(glVertexAttrib4Niv);
    REGISTER(glVertexAttrib4Nubv);
    REGISTER(glVertexAttrib4Nusv);
    REGISTER(glVertexAttrib4Nuiv);
 
    REGISTER(glVertexAttribP1ui);
    REGISTER(glVertexAttribP2ui);
    REGISTER(glVertexAttribP3ui);
    REGISTER(glVertexAttribP4ui);

    REGISTER(glVertexAttribL1dv);
    REGISTER(glVertexAttribL2dv);
    REGISTER(glVertexAttribL3dv);
    REGISTER(glVertexAttribL4dv);
}
 