#pragma once

#include <GLES2/gl2.h>

void OV_glBindBuffer(GLenum target, GLuint buffer);
void OV_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);