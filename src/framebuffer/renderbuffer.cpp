#include "framebuffer/renderbuffer.hpp"
#include "framebuffer/main.hpp"
#include "fogltlogles.hpp"

#include <GLES2/gl2.h>

void OV_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

void FBO::registerRenderbufferWorkaround() {
    REGISTEROV(glRenderbufferStorage);
}

void OV_glRenderbufferStorage(
    GLenum target,
    GLenum internalformat,
    GLsizei width,
    GLsizei height
) {
    fixStorageParams(internalformat);
    glRenderbufferStorage(target, internalformat, width, height);

    trackRenderbufferFormat(internalformat);
}
