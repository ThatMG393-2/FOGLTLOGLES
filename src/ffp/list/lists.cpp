#include "ffp/list/list.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

GLuint glGenLists(GLsizei range);
void glDeleteLists(GLuint list, GLsizei range);

void glNewList(GLuint list, GLenum mode);
void glEndList();

void glCallList(GLuint list);
void glCallLists(GLsizei n, GLenum type, const void* lists);

void glListBase(GLuint base);
GLboolean glIsList(GLuint list);

void FFP::registerListsFunctions() {
    PROVIDE(glGenLists);
    PROVIDE(glDeleteLists);

    PROVIDE(glNewList);
    PROVIDE(glEndList);

    PROVIDE(glCallList);
    PROVIDE(glCallLists);

    PROVIDE(glListBase);
    PROVIDE(glIsList);
}

GLuint glGenLists(GLsizei range) {
    return FFPE::List::genDisplayLists(range);
}

void glDeleteLists(GLuint list, GLsizei range) {
    FFPE::List::deleteDisplayLists(list, range);
}

void glNewList(GLuint list, GLenum mode) {
    FFPE::List::startDisplayList(list, mode);
}

void glEndList() {
    FFPE::List::endDisplayList();
}

void glCallList(GLuint list) {
    FFPE::List::callDisplayList(list);
}

void glCallLists(GLsizei n, GLenum type, const void* lists) {
    FFPE::List::callDisplayLists(n, type, lists);
}

void glListBase(GLuint base) {
    FFPE::List::setListBase(base);
}

GLboolean glIsList(GLuint list) {
    return FFPE::List::queryIsList(list) ? GL_TRUE : GL_FALSE;
}
