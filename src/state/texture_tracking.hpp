#pragma once

#include <GLES2/gl2.h>

void OV_glBindTexture(GLenum target, GLuint texture);
void OV_glDeleteTextures(GLsizei n, const GLuint *textures);

void OV_glActiveTexture(GLuint texture);