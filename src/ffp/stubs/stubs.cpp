#include "ffp/stubs/stubs.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>

typedef double GLdouble;

#define STUB_FUNC(name, args) void name args { LOGI("%s is not yet implemented!", #name); }
#define STUB_FUNC_RET(ret, name, args, retval) ret name args { LOGI("%s called!", #name); return retval; }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// Immediate Mode
STUB_FUNC(glArrayElement, (GLint i))
// STUB_FUNC(glColor3f, (GLfloat red, GLfloat green, GLfloat blue))
STUB_FUNC(glColor3fv, (const GLfloat *v))
// STUB_FUNC(glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
STUB_FUNC(glColor4fv, (const GLfloat *v))
STUB_FUNC(glEvalCoord1f, (GLfloat u))
STUB_FUNC(glEvalCoord1fv, (const GLfloat *u))
STUB_FUNC(glEvalCoord2f, (GLfloat u, GLfloat v))
STUB_FUNC(glEvalCoord2fv, (const GLfloat *u))
STUB_FUNC(glEvalMesh1, (GLenum mode, GLint i1, GLint i2))
STUB_FUNC(glEvalMesh2, (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2))
STUB_FUNC(glEvalPoint1, (GLint i))
STUB_FUNC(glEvalPoint2, (GLint i, GLint j))
STUB_FUNC(glFogCoordf, (GLfloat coord))
STUB_FUNC(glFogCoordfv, (const GLfloat *coord))
STUB_FUNC(glIndexi, (GLint c))
STUB_FUNC(glIndexiv, (const GLint *c))
STUB_FUNC(glIndexf, (GLfloat c))
STUB_FUNC(glIndexfv, (const GLfloat *c))
STUB_FUNC(glMaterialf, (GLenum face, GLenum pname, GLfloat param))
STUB_FUNC(glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params))
STUB_FUNC(glMultiTexCoord1f, (GLenum target, GLfloat s))
STUB_FUNC(glMultiTexCoord1fv, (GLenum target, const GLfloat *v))
STUB_FUNC(glMultiTexCoord2f, (GLenum target, GLfloat s, GLfloat t))
STUB_FUNC(glMultiTexCoord2fv, (GLenum target, const GLfloat *v))
STUB_FUNC(glMultiTexCoord3f, (GLenum target, GLfloat s, GLfloat t, GLfloat r))
STUB_FUNC(glMultiTexCoord3fv, (GLenum target, const GLfloat *v))
STUB_FUNC(glMultiTexCoord4f, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q))
STUB_FUNC(glMultiTexCoord4fv, (GLenum target, const GLfloat *v))
// STUB_FUNC(glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz))
STUB_FUNC(glNormal3fv, (const GLfloat *v))
STUB_FUNC(glSecondaryColor3f, (GLfloat red, GLfloat green, GLfloat blue))
STUB_FUNC(glSecondaryColor3fv, (const GLfloat *v))
STUB_FUNC(glTexCoord1f, (GLfloat s))
STUB_FUNC(glTexCoord1fv, (const GLfloat *v))
// STUB_FUNC(glTexCoord2f, (GLfloat s, GLfloat t))
STUB_FUNC(glTexCoord2fv, (const GLfloat *v))
STUB_FUNC(glTexCoord3f, (GLfloat s, GLfloat t, GLfloat r))
STUB_FUNC(glTexCoord3fv, (const GLfloat *v))
STUB_FUNC(glTexCoord4f, (GLfloat s, GLfloat t, GLfloat r, GLfloat q))
STUB_FUNC(glTexCoord4fv, (const GLfloat *v))

// GL2 Rasterization
STUB_FUNC(glBitmap, (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap))
STUB_FUNC(glClearIndex, (GLfloat c))
STUB_FUNC(glClipPlane, (GLenum plane, const GLdouble *equation))
STUB_FUNC(glCopyPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type))
STUB_FUNC(glDrawPixels, (GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
STUB_FUNC(glFeedbackBuffer, (GLsizei size, GLenum type, GLfloat *buffer))
// STUB_FUNC(glFogf, (GLenum pname, GLfloat param))
// STUB_FUNC(glFogfv, (GLenum pname, const GLfloat *params))
// STUB_FUNC(glFogi, (GLenum pname, GLint param))
STUB_FUNC(glFogiv, (GLenum pname, const GLint *params))
STUB_FUNC(glGetClipPlane, (GLenum plane, GLdouble *equation))
STUB_FUNC(glGetMapdv, (GLenum target, GLenum query, GLdouble *v))
STUB_FUNC(glGetMapfv, (GLenum target, GLenum query, GLfloat *v))
STUB_FUNC(glGetMapiv, (GLenum target, GLenum query, GLint *v))
STUB_FUNC(glGetPixelMapfv, (GLenum map, GLfloat *values))
STUB_FUNC(glGetPixelMapuiv, (GLenum map, GLuint *values))
STUB_FUNC(glGetPixelMapusv, (GLenum map, GLushort *values))
STUB_FUNC(glGetPolygonStipple, (GLubyte *mask))
STUB_FUNC(glInitNames, (void))
STUB_FUNC(glLineStipple, (GLint factor, GLushort pattern))
STUB_FUNC(glLoadName, (GLuint name))
STUB_FUNC(glMap1f, (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points))
STUB_FUNC(glMap2f, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points))
STUB_FUNC(glMapGrid1f, (GLint un, GLfloat u1, GLfloat u2))
STUB_FUNC(glMapGrid2f, (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2))
STUB_FUNC(glPassThrough, (GLfloat token))
STUB_FUNC(glPixelZoom, (GLfloat xfactor, GLfloat yfactor))
STUB_FUNC(glPolygonStipple, (const GLubyte *mask))
STUB_FUNC(glPopName, (void))
STUB_FUNC(glPushName, (GLuint name))
STUB_FUNC(glRasterPos2f, (GLfloat x, GLfloat y))
STUB_FUNC(glRasterPos2fv, (const GLfloat *v))
STUB_FUNC(glRasterPos3f, (GLfloat x, GLfloat y, GLfloat z))
STUB_FUNC(glRasterPos3fv, (const GLfloat *v))
STUB_FUNC(glRasterPos4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w))
STUB_FUNC(glRasterPos4fv, (const GLfloat *v))
STUB_FUNC(glRectf, (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2))
STUB_FUNC(glRectfv, (const GLfloat *v1, const GLfloat *v2))
STUB_FUNC_RET(GLint, glRenderMode, (GLenum mode), 0)
STUB_FUNC(glSelectBuffer, (GLsizei size, GLuint *buffer))
STUB_FUNC(glWindowPos2f, (GLfloat x, GLfloat y))
STUB_FUNC(glWindowPos2fv, (const GLfloat *v))
STUB_FUNC(glWindowPos3f, (GLfloat x, GLfloat y, GLfloat z))
STUB_FUNC(glWindowPos3fv, (const GLfloat *v))

// Client Arrays
// STUB_FUNC(glColorPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer))
// STUB_FUNC(glDisableClientState, (GLenum array))
// STUB_FUNC(glEnableClientState, (GLenum array))
STUB_FUNC(glFogCoordPointer, (GLenum type, GLsizei stride, const void *pointer))
STUB_FUNC(glIndexPointer, (GLenum type, GLsizei stride, const void *pointer))
STUB_FUNC(glInterleavedArrays, (GLenum format, GLsizei stride, const void *pointer))
// STUB_FUNC(glNormalPointer, (GLenum type, GLsizei stride, const void *pointer))
STUB_FUNC(glPopClientAttrib, (void))
STUB_FUNC(glPushClientAttrib, (GLbitfield mask))
STUB_FUNC(glSecondaryColorPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer))
// STUB_FUNC(glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer))
// STUB_FUNC(glVertexPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer))

// Fixed Function
STUB_FUNC(glAccum, (GLenum op, GLfloat value))
// STUB_FUNC(glAlphaFunc, (GLenum func, GLclampf ref))
STUB_FUNC(glClearAccum, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
STUB_FUNC(glEdgeFlag, (GLboolean flag))
STUB_FUNC(glEdgeFlagPointer, (GLsizei stride, const void *pointer))
STUB_FUNC(glGetLightfv, (GLenum light, GLenum pname, GLfloat *params))
STUB_FUNC(glGetLightiv, (GLenum light, GLenum pname, GLint *params))
STUB_FUNC(glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params))
STUB_FUNC(glGetMaterialiv, (GLenum face, GLenum pname, GLint *params))
STUB_FUNC(glGetTexEnvfv, (GLenum target, GLenum pname, GLfloat *params))
STUB_FUNC(glGetTexEnviv, (GLenum target, GLenum pname, GLint *params))
STUB_FUNC(glGetTexGendv, (GLenum coord, GLenum pname, GLdouble *params))
STUB_FUNC(glGetTexGenfv, (GLenum coord, GLenum pname, GLfloat *params))
STUB_FUNC(glGetTexGeniv, (GLenum coord, GLenum pname, GLint *params))
STUB_FUNC(glIndexMask, (GLuint mask))
STUB_FUNC(glLightf, (GLenum light, GLenum pname, GLfloat param))
// STUB_FUNC(glLightfv, (GLenum light, GLenum pname, const GLfloat *params))
STUB_FUNC(glLighti, (GLenum light, GLenum pname, GLint param))
STUB_FUNC(glLightiv, (GLenum light, GLenum pname, const GLint *params))
STUB_FUNC(glLightModelf, (GLenum pname, GLfloat param))
// STUB_FUNC(glLightModelfv, (GLenum pname, const GLfloat *params))
STUB_FUNC(glLightModeli, (GLenum pname, GLint param))
STUB_FUNC(glLightModeliv, (GLenum pname, const GLint *params))
STUB_FUNC(glPopAttrib, (void))
STUB_FUNC(glPushAttrib, (GLbitfield mask))
// STUB_FUNC(glShadeModel, (GLenum mode))
STUB_FUNC(glTexEnvf, (GLenum target, GLenum pname, GLfloat param))
STUB_FUNC(glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params))
STUB_FUNC(glTexEnvi, (GLenum target, GLenum pname, GLint param))
STUB_FUNC(glTexEnviv, (GLenum target, GLenum pname, const GLint *params))
STUB_FUNC(glTexGenf, (GLenum coord, GLenum pname, GLfloat param))
STUB_FUNC(glTexGenfv, (GLenum coord, GLenum pname, const GLfloat *params))
STUB_FUNC(glTexGeni, (GLenum coord, GLenum pname, GLint param))
STUB_FUNC(glTexGeniv, (GLenum coord, GLenum pname, const GLint *params))

// Matrix State
STUB_FUNC(glLoadTransposeMatrixf, (const GLfloat *m))
STUB_FUNC(glLoadTransposeMatrixd, (const GLdouble *m))
STUB_FUNC(glMultTransposeMatrixf, (const GLfloat *m))
STUB_FUNC(glMultTransposeMatrixd, (const GLdouble *m))

// GL2 Textures
STUB_FUNC_RET(GLboolean, glAreTexturesResident, (GLsizei n, const GLuint *textures, GLboolean *residences), GL_TRUE)
// STUB_FUNC(glClientActiveTexture, (GLenum texture))
STUB_FUNC(glPixelMapfv, (GLenum map, GLsizei mapsize, const GLfloat *values))
STUB_FUNC(glPixelMapuiv, (GLenum map, GLsizei mapsize, const GLuint *values))
STUB_FUNC(glPixelMapusv, (GLenum map, GLsizei mapsize, const GLushort *values))
STUB_FUNC(glPixelTransferf, (GLenum pname, GLfloat param))
STUB_FUNC(glPixelTransferi, (GLenum pname, GLint param))
STUB_FUNC(glPrioritizeTextures, (GLsizei n, const GLuint *textures, const GLclampf *priorities))

#pragma GCC diagnostic pop

void FFP::registerStubFunctions() {
    // Immediate Mode
    PROVIDE(glArrayElement);
    // PROVIDE(glColor3f);
    PROVIDE(glColor3fv);
    // PROVIDE(glColor4f);
    PROVIDE(glColor4fv);
    PROVIDE(glEvalCoord1f);
    PROVIDE(glEvalCoord1fv);
    PROVIDE(glEvalCoord2f);
    PROVIDE(glEvalCoord2fv);
    PROVIDE(glEvalMesh1);
    PROVIDE(glEvalMesh2);
    PROVIDE(glEvalPoint1);
    PROVIDE(glEvalPoint2);
    PROVIDE(glFogCoordf);
    PROVIDE(glFogCoordfv);
    PROVIDE(glIndexi);
    PROVIDE(glIndexiv);
    PROVIDE(glIndexf);
    PROVIDE(glIndexfv);
    PROVIDE(glMaterialf);
    PROVIDE(glMaterialfv);
    PROVIDE(glMultiTexCoord1f);
    PROVIDE(glMultiTexCoord1fv);
    PROVIDE(glMultiTexCoord2f);
    PROVIDE(glMultiTexCoord2fv);
    PROVIDE(glMultiTexCoord3f);
    PROVIDE(glMultiTexCoord3fv);
    PROVIDE(glMultiTexCoord4f);
    PROVIDE(glMultiTexCoord4fv);
    // PROVIDE(glNormal3f);
    PROVIDE(glNormal3fv);
    PROVIDE(glSecondaryColor3f);
    PROVIDE(glSecondaryColor3fv);
    PROVIDE(glTexCoord1f);
    PROVIDE(glTexCoord1fv);
    // PROVIDE(glTexCoord2f);
    PROVIDE(glTexCoord2fv);
    PROVIDE(glTexCoord3f);
    PROVIDE(glTexCoord3fv);
    PROVIDE(glTexCoord4f);
    PROVIDE(glTexCoord4fv);
    
    // GL2 Rasterization
    PROVIDE(glBitmap);
    PROVIDE(glClearIndex);
    PROVIDE(glClipPlane);
    PROVIDE(glCopyPixels);
    PROVIDE(glDrawPixels);
    PROVIDE(glFeedbackBuffer);
    // PROVIDE(glFogf);
    // PROVIDE(glFogfv);
    // PROVIDE(glFogi);
    PROVIDE(glFogiv);
    PROVIDE(glGetClipPlane);
    PROVIDE(glGetMapdv);
    PROVIDE(glGetMapfv);
    PROVIDE(glGetMapiv);
    PROVIDE(glGetPixelMapfv);
    PROVIDE(glGetPixelMapuiv);
    PROVIDE(glGetPixelMapusv);
    PROVIDE(glGetPolygonStipple);
    PROVIDE(glInitNames);
    PROVIDE(glLineStipple);
    PROVIDE(glLoadName);
    PROVIDE(glMap1f);
    PROVIDE(glMap2f);
    PROVIDE(glMapGrid1f);
    PROVIDE(glMapGrid2f);
    PROVIDE(glPassThrough);
    PROVIDE(glPixelZoom);
    PROVIDE(glPolygonStipple);
    PROVIDE(glPopName);
    PROVIDE(glPushName);
    PROVIDE(glRasterPos2f);
    PROVIDE(glRasterPos2fv);
    PROVIDE(glRasterPos3f);
    PROVIDE(glRasterPos3fv);
    PROVIDE(glRasterPos4f);
    PROVIDE(glRasterPos4fv);
    PROVIDE(glRectf);
    PROVIDE(glRectfv);
    PROVIDE(glRenderMode);
    PROVIDE(glSelectBuffer);
    PROVIDE(glWindowPos2f);
    PROVIDE(glWindowPos2fv);
    PROVIDE(glWindowPos3f);
    PROVIDE(glWindowPos3fv);
    
    // Client Arrays
    // PROVIDE(glColorPointer);
    // PROVIDE(glDisableClientState);
    // PROVIDE(glEnableClientState);
    PROVIDE(glFogCoordPointer);
    PROVIDE(glIndexPointer);
    PROVIDE(glInterleavedArrays);
    // PROVIDE(glNormalPointer);
    PROVIDE(glPopClientAttrib);
    PROVIDE(glPushClientAttrib);
    PROVIDE(glSecondaryColorPointer);
    // PROVIDE(glTexCoordPointer);
    // PROVIDE(glVertexPointer);
    
    // Fixed Function
    PROVIDE(glAccum);
    // PROVIDE(glAlphaFunc);
    PROVIDE(glClearAccum);
    // PROVIDE(glColorMaterial);
    PROVIDE(glEdgeFlag);
    PROVIDE(glEdgeFlagPointer);
    PROVIDE(glGetLightfv);
    PROVIDE(glGetLightiv);
    PROVIDE(glGetMaterialfv);
    PROVIDE(glGetMaterialiv);
    PROVIDE(glGetTexEnvfv);
    PROVIDE(glGetTexEnviv);
    PROVIDE(glGetTexGendv);
    PROVIDE(glGetTexGenfv);
    PROVIDE(glGetTexGeniv);
    PROVIDE(glIndexMask);
    PROVIDE(glLightf);
    // PROVIDE(glLightfv);
    PROVIDE(glLighti);
    PROVIDE(glLightiv);
    PROVIDE(glLightModelf);
    // PROVIDE(glLightModelfv);
    PROVIDE(glLightModeli);
    PROVIDE(glLightModeliv);
    PROVIDE(glPopAttrib);
    PROVIDE(glPushAttrib);
    // PROVIDE(glShadeModel);
    PROVIDE(glTexEnvf);
    PROVIDE(glTexEnvfv);
    PROVIDE(glTexEnvi);
    PROVIDE(glTexEnviv);
    PROVIDE(glTexGenf);
    PROVIDE(glTexGenfv);
    PROVIDE(glTexGeni);
    PROVIDE(glTexGeniv);
    
    // Matrix State
    PROVIDE(glLoadTransposeMatrixf);
    PROVIDE(glLoadTransposeMatrixd);
    PROVIDE(glMultTransposeMatrixf);
    PROVIDE(glMultTransposeMatrixd);
    
    // GL2 Textures
    PROVIDE(glAreTexturesResident);
    // PROVIDE(glClientActiveTexture);
    PROVIDE(glPixelMapfv);
    PROVIDE(glPixelMapuiv);
    PROVIDE(glPixelMapusv);
    PROVIDE(glPixelTransferf);
    PROVIDE(glPixelTransferi);
    PROVIDE(glPrioritizeTextures);
}