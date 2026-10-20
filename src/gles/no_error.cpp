#include "gles/main.hpp"
#include "main.hpp"
#include "utils/env.hpp"

#include <GLES3/gl32.h>

GLenum OV_glGetError();

void GLES::registerNoErrorOverride() {
    // REGISTEROV(glGetError);
}

GLenum OV_glGetError() {
    if (ENV_NO_ERROR) return GL_NO_ERROR;
    return glGetError();
}