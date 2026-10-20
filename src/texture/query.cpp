#include "framebuffer/framebuffer.hpp"
#include "texture/proxy.hpp"
#include "state/state_tracking.hpp"
#include "state/framebuffer_tracking.hpp"
#include "texture/readpixels.hpp"
#include "texture/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels);

static void glGetTexLevelParameterivHook(NEXT(glGetTexLevelParameteriv) next, GLenum target, GLint level, GLenum pname, GLint* params);
static void glGetTexLevelParameterfvHook(NEXT(glGetTexLevelParameterfv) next, GLenum target, GLint level, GLenum pname, GLfloat* params);

// this should be in gles3.1 but idrc
void Texture::registerQueryOverrides() {
    PROVIDE(glGetTexImage);

    PROVIDE(glGetTexLevelParameteriv);
    HOOK(glGetTexLevelParameteriv, glGetTexLevelParameterivHook);
    PROVIDE(glGetTexLevelParameterfv);
    HOOK(glGetTexLevelParameterfv, glGetTexLevelParameterfvHook);
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    if (format != GL_RGBA
     && format != GL_RGBA_INTEGER
     && type != GL_UNSIGNED_BYTE
     && type != GL_UNSIGNED_INT
     && type != GL_INT
     && type != GL_FLOAT) return;

    CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, fakeDepthbuffer->readFramebuffer);
    glFramebufferTexture2D(
        GL_READ_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        target,
        trackedStates->activeTextureState->boundTextures[target],
        level
    );

    GLsizei width, height;
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);

    glReadPixels(0, 0, width, height, format, type, pixels);
    
    glFramebufferRenderbuffer(
        GL_READ_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_RENDERBUFFER,
        0
    );
}

static void glGetTexLevelParameterivHook(NEXT(glGetTexLevelParameteriv) next, GLenum target, GLint level, GLenum pname, GLint* params) {
    LOGI("glGetTexLevelParameteriv: target=%u level=%i pname=%u", target, level, pname);
    
    if (isProxyTexture(target)) {
        getProxyTexLevelParameter(pname, params);
        return;
    }

    next(target, level, pname, params);
}

static void glGetTexLevelParameterfvHook(NEXT(glGetTexLevelParameterfv) next, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    LOGI("glGetTexLevelParameterfv: target=%u level=%i pname=%u", target, level, pname);
    
    if (isProxyTexture(target)) {
        GLint tmp = 0;
        getProxyTexLevelParameter(pname, &tmp);

        (*params) = static_cast<GLfloat>(tmp);
        return;
    }

    next(target, level, pname, params);
}
