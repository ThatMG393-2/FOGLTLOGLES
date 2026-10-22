#include "framebuffer/renderbuffer.hpp"
#include "framebuffer/main.hpp"
#include "fogltlogles.hpp"

#include <GLES2/gl2.h>

static void HK_glRenderbufferStorage(NEXT(glRenderbufferStorage) next, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

void FBO::registerRenderbufferWorkaround() {
    PROVIDE(glRenderbufferStorage);
    HOOK(glRenderbufferStorage);
}

static void HK_glRenderbufferStorage(NEXT(glRenderbufferStorage) next,
    GLenum target,
    GLenum internalformat,
    GLsizei width,
    GLsizei height
) {
    fixStorageParams(internalformat);
    next(target, internalformat, width, height);

    trackRenderbufferFormat(internalformat);
}
