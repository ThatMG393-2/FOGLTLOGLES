#include "gles20/main.h"
#include "es/proxy.h"
#include "es/texture.h"
#include "main.h"
#include "utils/log.h"

#include <GLES2/gl2.h>

// TODO: texture.cpp shader.cpp separation

void glClearDepth(double d);

void OV_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
void OV_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);

static GLint maxTextureSize = 0;

void GLES20::registerTranslatedFunctions() {
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    LOGI("GL_MAX_TEXTURE_SIZE is %i", maxTextureSize);

    REGISTER(glClearDepth);
    REGISTEROV(glTexImage2D);
    REGISTEROV(glGetTexLevelParameteriv);
}

void glClearDepth(double d) {
    glClearDepthf(static_cast<float>(d));
}

GLint proxyWidth, proxyHeight, proxyInternalFormat;

void OV_glTexImage2D(
    GLenum target,
    GLint level,
    GLint internalFormat,
    GLsizei width, GLsizei height,
    GLint border, GLenum format,
    GLenum type, const void* pixels
) {
    LOGI("glTexImage2D: internalformat=%i border=%i format=%i type=%u", internalFormat, border, format, type);
    if (isProxyTexture(target)) {
        LOGI("yes its a proxy tex");
        proxyWidth = (( width << level ) > maxTextureSize) ? 0 : width;
        proxyHeight = (( height << level ) > maxTextureSize) ? 0 : height;
        proxyInternalFormat = internalFormat;
    } else {
        selectProperTexType(internalFormat, type);
        glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    }
}

void OV_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
    LOGI("glGetTexLevelParameteriv: target=%u level=%i pname=%u", target, level, pname);
    
    if (isProxyTexture(target)) {
        switch (pname) {
            case GL_TEXTURE_WIDTH:
                (*params) = nlevel(proxyWidth, level);
                return;
            case GL_TEXTURE_HEIGHT:
                (*params) = nlevel(proxyHeight, level);
                return;
            case GL_TEXTURE_INTERNAL_FORMAT:
                (*params) = proxyInternalFormat;
                return;
        }
    } else {
        glGetTexLevelParameteriv(target, level, pname, params);
    }
}
