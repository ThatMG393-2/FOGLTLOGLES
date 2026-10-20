#pragma once

#include <GLES3/gl3.h>

void OV_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);