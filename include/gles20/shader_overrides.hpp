#pragma once

#include <GLES2/gl2.h>

void OV_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
void OV_glCompileShader(GLuint shader);
void OV_glDeleteShader(GLuint shader);
void OV_glAttachShader(GLuint program, GLuint shader);
void OV_glDetachShader(GLuint program, GLuint shader);
void OV_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);
void OV_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
void OV_glLinkProgram(GLuint program);
void OV_glUseProgram(GLuint program);
void OV_glDeleteProgram(GLuint program);
