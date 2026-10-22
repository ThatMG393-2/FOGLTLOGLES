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

static void HK_glTexImage2D(NEXT(glTexImage2D) next, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
static void HK_glTexSubImage2D(NEXT(glTexSubImage2D) next, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
static void HK_glCopyTexImage2D(NEXT(glCopyTexImage2D) next, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static void HK_glCopyTexSubImage2D(NEXT(glCopyTexSubImage2D) next, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

static void HK_glTexParameterf(NEXT(glTexParameterf) next, GLenum target, GLenum pname, GLfloat param);
static void HK_glTexParameteri(NEXT(glTexParameteri) next, GLenum target, GLenum pname, GLint param);

void Texture::registerTextureOverrides() {
    PROVIDE(glTexImage2D);
    HOOK(glTexImage2D);
    PROVIDE(glTexSubImage2D);
    HOOK(glTexSubImage2D);
    PROVIDE(glCopyTexImage2D);
    HOOK(glCopyTexImage2D);
    PROVIDE(glCopyTexSubImage2D);
    HOOK(glCopyTexSubImage2D);
    
    PROVIDE(glTexParameterf);
    HOOK(glTexParameterf);
    PROVIDE(glTexParameteri);
    HOOK(glTexParameteri);
}

static void HK_glTexImage2D(NEXT(glTexImage2D) next,
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

static void HK_glTexSubImage2D(NEXT(glTexSubImage2D) next,
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

static void HK_glCopyTexImage2D(NEXT(glCopyTexImage2D) next,
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

static void HK_glCopyTexSubImage2D(NEXT(glCopyTexSubImage2D) next,
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

static void HK_glTexParameterf(NEXT(glTexParameterf) next, GLenum target, GLenum pname, GLfloat param) {
    // LOGI("glTexParameterf: target=%u pname=%u param=%f", target, pname, param);
    if (FFPE::List::addCommand<CHAIN(glTexParameterf)>(target, pname, param)) return;

    selectProperTexParamf(target, pname, param);
    next(target, pname, param);
}

static void HK_glTexParameteri(NEXT(glTexParameteri) next, GLenum target, GLenum pname, GLint param) {
    // LOGI("glTexParameteri: target=%u pname=%u param=%d", target, pname, param);
    if (FFPE::List::addCommand<CHAIN(glTexParameteri)>(target, pname, param)) return;

    // selectProperTexParami(target, pname, param);
    next(target, pname, param);
}
