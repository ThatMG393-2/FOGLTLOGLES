#include "framebuffer/framebuffer.hpp"
#include "texture/main.hpp"
#include "texture/readpixels.hpp"
#include "fogltlogles.hpp"

static void HK_glReadPixels(NEXT(glReadPixels) next, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);

void Texture::registerReadPixelsOverride() {
    PROVIDE(glReadPixels);
    HOOK(glReadPixels);
}

static void HK_glReadPixels(NEXT(glReadPixels) next,
    GLint x, GLint y,
    GLsizei width, GLsizei height,
    GLenum format, GLenum type, void* pixels
) {
    if (format == GL_DEPTH_COMPONENT) {
        fakeDepthbuffer->storeDepthToFakeDraw(x, y, width, height, pixels);
    } else {
        next(
            x, y,
            width, height,
            format, type, pixels
        );
    }
}