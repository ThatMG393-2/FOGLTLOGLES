#include "ffp/lists.hpp"
#include "ffp/matrices.hpp"
#include "ffp/enums.hpp"
#include "ffp/main.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glMatrixMode(GLenum mode);

void glPushMatrix();
void glPopMatrix();
void glLoadIdentity();

void glLoadMatrixd(const GLdouble *m);
void glLoadMatrixf(const GLfloat *m);

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val);
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);

void glMultMatrixd(const GLdouble *m);
void glMultMatrixf(const GLfloat *m);

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

void glScaled(GLdouble x, GLdouble y, GLdouble z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);

void glTranslated(GLdouble x, GLdouble y, GLdouble z);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);

void FFP::registerMatrixFunctions() {
    REGISTER(glMatrixMode);

    REGISTER(glPushMatrix);
    REGISTER(glPopMatrix);
    REGISTER(glLoadIdentity);

    REGISTER(glLoadMatrixd);
    REGISTER(glLoadMatrixf);

    REGISTER(glOrtho);
    REGISTER(glOrthof);
    REGISTER(glFrustum);

    REGISTER(glMultMatrixd);
    REGISTER(glMultMatrixf);

    REGISTER(glRotated);
    REGISTER(glRotatef);

    REGISTER(glScaled);
    REGISTER(glScalef);

    REGISTER(glTranslated);
    REGISTER(glTranslatef);
}

void glMatrixMode(GLenum mode) {
    switch (mode) {
        case GL_MODELVIEW:
        case GL_PROJECTION:
        case GL_TEXTURE:
            break;
        default:
            return;
    }

    if (FFPE::List::addCommand<glMatrixMode>(mode)) return;
    
    FFPE::Rendering::Matrices::setCurrentMatrix(mode);
}

void glPushMatrix() {
    if (FFPE::List::addCommand<glPushMatrix>()) return;

    FFPE::Rendering::Matrices::pushCurrentMatrix();
}

void glPopMatrix() {
    if (FFPE::List::addCommand<glPopMatrix>()) return;

    FFPE::Rendering::Matrices::popTopMatrix();
}

void glLoadIdentity() {
    if (FFPE::List::addCommand<glLoadIdentity>()) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([](glm::mat4) { return glm::mat4(1.0f); });
}

void glLoadMatrixd(const GLdouble *m) {
    if (FFPE::List::addCommand<glLoadMatrixd>(m)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4) {
        return glm::mat4(
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        );
    });

}

void glLoadMatrixf(const GLfloat *m) {
    if (FFPE::List::addCommand<glLoadMatrixf>(m)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4) {
        return glm::mat4(
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        );
    });
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {
    glOrthof(left, right, bottom, top, near_val, far_val);
}

void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val)  {
    if (FFPE::List::addCommand<glOrthof>(left, right, bottom, top, near_val, far_val)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return currentMatrix * glm::ortho(
            left, right,
            bottom, top,
            near_val, far_val
        );
    });
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {
    if (FFPE::List::addCommand<glFrustum>(left, right, bottom, top, near_val, far_val)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return currentMatrix * glm::frustum(
            (GLfloat) left, (GLfloat) right,
            (GLfloat) bottom, (GLfloat) top,
            (GLfloat) near_val, (GLfloat) far_val
        );
    });
}

void glMultMatrixd(const GLdouble *m) {
    if (FFPE::List::addCommand<glMultMatrixd>(m)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return currentMatrix * glm::mat4(
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        );
    });
}

void glMultMatrixf(const GLfloat *m) {
    if (FFPE::List::addCommand<glMultMatrixf>(m)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return currentMatrix * glm::mat4(
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        );
    });
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    glRotatef(static_cast<float>(angle), static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    if (FFPE::List::addCommand<glRotatef>(angle, x, y, z)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return glm::rotate(currentMatrix, glm::radians(angle), glm::vec3(x, y, z));
    });
}

void glScaled(GLdouble x, GLdouble y, GLdouble z) {
    glScalef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {
    if (FFPE::List::addCommand<glScalef>(x, y, z)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) {
        return glm::scale(currentMatrix, glm::vec3(x, y, z));
    });
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z) {
    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    if (FFPE::List::addCommand<glTranslatef>(x, y, z)) return;

    FFPE::Rendering::Matrices::modifyCurrentMatrix([&](glm::mat4 currentMatrix) { 
        return glm::translate(currentMatrix, glm::vec3(x, y, z));
    });
}
