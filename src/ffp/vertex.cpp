#include "ffp/states.hpp"
#include "ffp/immediate.hpp"
#include "ffp/enums.hpp"
#include "ffp/main.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

#pragma region Vertex Short Declarations
void glVertex2s(GLshort x, GLshort y);
void glVertex3s(GLshort x, GLshort y, GLshort z);
void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);

void glVertex2sv(const GLshort *v);
void glVertex3sv(const GLshort *v);
void glVertex4sv(const GLshort *v);
#pragma endregion

#pragma region Vertex Integer Declarations
void glVertex2i(GLint x, GLint y);
void glVertex3i(GLint x, GLint y, GLint z);
void glVertex4i(GLint x, GLint y, GLint z, GLint w);

void glVertex2iv(const GLint *v);
void glVertex3iv(const GLint *v);
void glVertex4iv(const GLint *v);
#pragma endregion

#pragma region Vertex Float Declarations
void glVertex2f(GLfloat x, GLfloat y);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

void glVertex2fv(const GLfloat *v);
void glVertex3fv(const GLfloat *v);
void glVertex4fv(const GLfloat *v);
#pragma endregion

#pragma region Vertex Double Declarations
void glVertex2d(GLdouble x, GLdouble y);
void glVertex3d(GLdouble x, GLdouble y, GLdouble z);
void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

void glVertex2dv(const GLdouble *v);
void glVertex3dv(const GLdouble *v);
void glVertex4dv(const GLdouble *v);
#pragma endregion

void FFP::registerVertexFunctions() {
#pragma region Vertex Registrations
    REGISTER(glVertex2s);
    REGISTER(glVertex3s);
    REGISTER(glVertex4s);

    REGISTER(glVertex2sv);
    REGISTER(glVertex3sv);
    REGISTER(glVertex4sv);
#pragma endregion

#pragma region Vertex Integer Registrations
    REGISTER(glVertex2i);
    REGISTER(glVertex3i);
    REGISTER(glVertex4i);

    REGISTER(glVertex2iv);
    REGISTER(glVertex3iv);
    REGISTER(glVertex4iv);
#pragma endregion

#pragma region Vertex Float Registrations
    REGISTER(glVertex2f);
    REGISTER(glVertex3f);
    REGISTER(glVertex4f);

    REGISTER(glVertex2fv);
    REGISTER(glVertex3fv);
    REGISTER(glVertex4fv);
#pragma endregion

#pragma region Vertex Double Registrations
    REGISTER(glVertex2d);
    REGISTER(glVertex3d);
    REGISTER(glVertex4d);

    REGISTER(glVertex2dv);
    REGISTER(glVertex3dv);
    REGISTER(glVertex4dv);
#pragma endregion
}

#pragma region Vertex Short Implementations
void glVertex2s(GLshort x, GLshort y) {
    glVertex2i(static_cast<GLint>(x), static_cast<GLint>(y));
}
void glVertex3s(GLshort x, GLshort y, GLshort z) {
    glVertex3i(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(z));
}
void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {
    glVertex4i(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(z), static_cast<GLint>(w));
}

void glVertex2sv(const GLshort *v) {
    glVertex2s(v[0], v[1]);
}
void glVertex3sv(const GLshort *v) {
    glVertex3s(v[0], v[1], v[2]);
}
void glVertex4sv(const GLshort *v) {
    glVertex4s(v[0], v[1], v[2], v[3]);
}
#pragma endregion

#pragma region Vertex Integer Implementations
void glVertex2i(GLint x, GLint y) {
    glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y));
}

void glVertex3i(GLint x, GLint y, GLint z) {
    glVertex3f(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}

void glVertex4i(GLint x, GLint y, GLint z, GLint w) {
    glVertex4f(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z), static_cast<GLfloat>(w));
}

void glVertex2iv(const GLint *v) {
    glVertex2i(v[0], v[1]);
}

void glVertex3iv(const GLint *v) {
    glVertex3i(v[0], v[1], v[2]);
}

void glVertex4iv(const GLint *v) {
    glVertex4i(v[0], v[1], v[2], v[3]);
}
#pragma endregion

#pragma region Vertex Float Implementations
void glVertex2f(GLfloat x, GLfloat y) {
    FFPE::States::VertexData::set(
        glm::vec2(x, y),
        FFPE::States::VertexData::position
    );
    FFPE::Rendering::ImmediateMode::advance();
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
    FFPE::States::VertexData::set(
        glm::vec3(x, y, z),
        FFPE::States::VertexData::position
    );
    FFPE::Rendering::ImmediateMode::advance();
}

void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    FFPE::States::VertexData::set(
        glm::vec4(x, y, z, w),
        FFPE::States::VertexData::position
    );
    FFPE::Rendering::ImmediateMode::advance();
}

void glVertex2fv(const GLfloat *v) {
    glVertex2f(v[0], v[1]);
}

void glVertex3fv(const GLfloat *v) {
    glVertex3f(v[0], v[1], v[2]);
}

void glVertex4fv(const GLfloat *v) {
    glVertex4f(v[0], v[1], v[2], v[3]);
}
#pragma endregion

#pragma region Vertex Double Implementations
void glVertex2d(GLdouble x, GLdouble y) {
    glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y));
}

void glVertex3d(GLdouble x, GLdouble y, GLdouble z) {
    glVertex3f(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}

void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    glVertex4f(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z), static_cast<GLfloat>(w));
}

void glVertex2dv(const GLdouble *v) {
    glVertex2d(v[0], v[1]);
}

void glVertex3dv(const GLdouble *v) {
    glVertex3d(v[0], v[1], v[2]);
}

void glVertex4dv(const GLdouble *v) {
    glVertex4d(v[0], v[1], v[2], v[3]);
}
#pragma endregion

