#pragma once

#include "common/pointers.hpp"
#include <GLES3/gl32.h>
#include <memory>

struct ProxyTexture {
    GLenum type;
    GLint width;
    GLint height;
    // GLint depth; // if GL_PROXY_TEXTURE_3D
    GLint level;
    GLint internalFormat;
};

inline std::shared_ptr<ProxyTexture> boundProxyTexture = MakeAggregateShared<ProxyTexture>();

inline bool isProxyTexture(GLenum target) {
    switch (target) {
        case 0x8063: // GL_PROXY_TEXTURE_1D
        case 0x8064: // GL_PROXY_TEXTURE_2D
        case 0x8070: // GL_PROXY_TEXTURE_3D
        case 0x84f5: // GL_PROXY_TEXTURE_RECTANGLE
        case 0x84f7: // GL_PROXY_TEXTURE_RECTANGLE_ARB
            return true;
        default:
            return false;
    }
}

inline GLint nlevel(GLint size, GLint level) {
    if (size) {
        size >>= level;
        if (!size) size = 1;
    }
    return size;
}

inline void getProxyTexLevelParameter(GLenum pname, GLint* params) {
    switch (pname) {
        case GL_TEXTURE_WIDTH:
            (*params) = nlevel(boundProxyTexture->width, boundProxyTexture->level);
            return;
        case GL_TEXTURE_HEIGHT:
            (*params) = nlevel(boundProxyTexture->height, boundProxyTexture->level);
            return;
        /* case GL_TEXTURE_DEPTH:
            if (boundProxyTexture->type != GL_TEXTURE_3D) return;
            (*params) = nlevel(boundProxyTexture->depth, boundProxyTexture->level);
            return; */
        case GL_TEXTURE_INTERNAL_FORMAT:
            (*params) = boundProxyTexture->internalFormat;
            return;
    }
}
