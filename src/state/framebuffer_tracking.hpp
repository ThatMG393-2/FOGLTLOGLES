#pragma once

#include <GLES2/gl2.h>

void OV_glBindFramebuffer(GLenum target, GLuint framebuffer);
void OV_glGenFramebuffers(GLsizei n, GLuint* framebuffers);
void OV_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);