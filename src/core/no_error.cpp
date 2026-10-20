#include "core/main.hpp"
#include "fogltlogles.hpp"
#include "common/env.hpp"

#include <GLES3/gl32.h>

GLenum OV_glGetError();

void Core::registerNoErrorOverride() {
    // PROVIDE(glGetError, OV_glGetError);
}

GLenum OV_glGetError() {
    if (ENV_NO_ERROR) return GL_NO_ERROR;
    return glGetError();
}