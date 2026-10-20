#include "framebuffer/framebuffer.hpp"
#include "texture/main.hpp"
#include "texture/readpixels.hpp"
#include "fogltlogles.hpp"

void Texture::registerReadPixelsOverride() {
    PROVIDE(glReadPixels, OV_glReadPixels);
}

void OV_glReadPixels(
    GLint x, GLint y,
    GLsizei width, GLsizei height,
    GLenum format, GLenum type, void* pixels
) {
    if (format == GL_DEPTH_COMPONENT) {
        fakeDepthbuffer->storeDepthToFakeDraw(x, y, width, height, pixels);
    } else {
        glReadPixels(
            x, y,
            width, height,
            format, type, pixels
        );
    }
}