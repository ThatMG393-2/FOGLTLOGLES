#include "ffp/lists.hpp"
#include "framebuffer/framebuffer.hpp"
#include "common/limits.hpp"
#include "texture/proxy.hpp"
#include "state/state_tracking.hpp"
#include "texture/swizzling.hpp"
#include "texture/texture.hpp"
#include "texture/texparam.hpp"
#include "core/main.hpp"
#include "texture/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"
#include "common/pointers.hpp"

#include <GLES2/gl2.h>

static void glTexImage2DHook(NEXT(glTexImage2D) next, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
static void glTexSubImage2DHook(NEXT(glTexSubImage2D) next, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
static void glCopyTexImage2DHook(NEXT(glCopyTexImage2D) next, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static void glCopyTexSubImage2DHook(NEXT(glCopyTexSubImage2D) next, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

static void glTexParameterfHook(NEXT(glTexParameterf) next, GLenum target, GLenum pname, GLfloat param);
static void glTexParameteriHook(NEXT(glTexParameteri) next, GLenum target, GLenum pname, GLint param);

void Texture::registerTextureOverrides() {
    PROVIDE(glTexImage2D);
    HOOK(glTexImage2D, glTexImage2DHook);
    PROVIDE(glTexSubImage2D);
    HOOK(glTexSubImage2D, glTexSubImage2DHook);
    PROVIDE(glCopyTexImage2D);
    HOOK(glCopyTexImage2D, glCopyTexImage2DHook);
    PROVIDE(glCopyTexSubImage2D);
    HOOK(glCopyTexSubImage2D, glCopyTexSubImage2DHook);
    
    PROVIDE(glTexParameterf);
    HOOK(glTexParameterf, glTexParameterfHook);
    PROVIDE(glTexParameteri);
    HOOK(glTexParameteri, glTexParameteriHook);
}

static void glTexImage2DHook(NEXT(glTexImage2D) next,
    GLenum target,
    GLint level,
    GLint internalFormat,
    GLsizei width, GLsizei height,
    GLint border, GLenum format,
    GLenum type, const void* pixels
) {
    if (isProxyTexture(target)) {
        boundProxyTexture = MakeAggregateShared<ProxyTexture>(
            target,
            (( width << level ) > ESLimits::MAX_TEXTURE_SIZE) ? 0 : width,
            (( height << level ) > ESLimits::MAX_TEXTURE_SIZE) ? 0 : height,
            level,
            internalFormat
        );
    } else {
        if (debugEnabled) LOGI("glTexImage2D (before): internalformat=%i border=%i format=%i type=%u", internalFormat, border, format, type);
        
        fixTexArguments(target, internalFormat, type, format, (width == height));
        next(
            target, level, internalFormat, 
            width, height,
            border, format, type, pixels
        );

        if (debugEnabled) LOGI("glTexImage2D (after): internalformat=%i border=%i format=%i type=%u", internalFormat, border, format, type);
    }

    trackTextureFormat(internalFormat);
}

static void glTexSubImage2DHook(NEXT(glTexSubImage2D) next,
    GLenum target, GLint level, 
    GLint xOffset, GLint yOffset, 
    GLsizei width, GLsizei height, 
    GLenum format, GLenum type,
    const void* pixels
) {
    if (format == 0x80e1) { // GL_BGRA
        swizzleBGRA(type);
        doSwizzling(target);

        format = GL_RGBA;
    }

    if (isDepthFormat(format)
        && width == fakeDepthbuffer->width
        && height == fakeDepthbuffer->height
        && fakeDepthbuffer->data == pixels
    ) {
        fakeDepthbuffer->blitFakeReadToFakeDraw(target, level, xOffset, yOffset, width, height);
        return;
    }

    next(
        target, level,
        xOffset, yOffset,
        width, height,
        format, type,
        pixels
    );
}

static void glCopyTexImage2DHook(NEXT(glCopyTexImage2D) next,
    GLenum target, GLint level,
    GLenum internalformat,
    GLint x, GLint y,
    GLsizei width, GLsizei height, 
    GLint border
) {
    if (isDepthFormat(internalformat)) {
        CALL(glTexImage2D,
            target, level, trackedStates->activeTextureState->getTextureInternalFormat(GL_TEXTURE_2D),
            width, height, border,
            GL_DEPTH_COMPONENT32F, GL_FLOAT,
            nullptr
        );

        fakeDepthbuffer->blitCurrentReadToFakeDraw(target, level, x, y, width, height);
        return;
    }
    
    next(
        target,  level, internalformat,
        x, y,
        width, height, border
    );
}

static void glCopyTexSubImage2DHook(NEXT(glCopyTexSubImage2D) next,
    GLenum target, GLint level,
    GLint xoffset, GLint yoffset,
    GLint x, GLint y,
    GLsizei width, GLsizei height
) {
    glGetError();
    next(target, level, xoffset, yoffset, x, y, width, height);
    if (glGetError() == GL_INVALID_OPERATION) {
        fakeDepthbuffer->blitCurrentReadToFakeDraw(target, level, x, y, width, height);
    }
}

static void glTexParameterfHook(NEXT(glTexParameterf) next, GLenum target, GLenum pname, GLfloat param) {
    // LOGI("glTexParameterf: target=%u pname=%u param=%f", target, pname, param);
    if (FFPE::List::addCommand<CHAIN(glTexParameterf)>(target, pname, param)) return;

    selectProperTexParamf(target, pname, param);
    next(target, pname, param);
}

static void glTexParameteriHook(NEXT(glTexParameteri) next, GLenum target, GLenum pname, GLint param) {
    // LOGI("glTexParameteri: target=%u pname=%u param=%d", target, pname, param);
    if (FFPE::List::addCommand<CHAIN(glTexParameteri)>(target, pname, param)) return;

    // selectProperTexParami(target, pname, param);
    next(target, pname, param);
}
