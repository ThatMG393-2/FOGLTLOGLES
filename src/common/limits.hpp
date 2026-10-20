#pragma once

#include "common/log.hpp"

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

#define GETVAL(type, name, dest) glGet##type##v(name, dest)

namespace ESLimits {
    inline GLint MAX_TEXTURE_SIZE = 0;
    inline GLfloat MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0;

    inline void init() {
        GETVAL(Integer, GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
        GETVAL(Float, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MAX_TEXTURE_MAX_ANISOTROPY_EXT);

        LOGI("GL_MAX_TEXTURE_SIZE: %d", MAX_TEXTURE_SIZE);
        LOGI("GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT: %f", MAX_TEXTURE_MAX_ANISOTROPY_EXT);
    }
}