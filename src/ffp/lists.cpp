#include "ffp/lists.hpp"
#include "common/es_utils.hpp"
#include "ffp/main.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"

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
    REGISTER(glGenLists);
    REGISTER(glDeleteLists);

    REGISTER(glNewList);
    REGISTER(glEndList);

    REGISTER(glCallList);
    REGISTER(glCallLists);

    REGISTER(glListBase);
    REGISTER(glIsList);
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
    ESUtils::TypeTraits::typeToPrimitive(type, [&]<typename T>() {
        FFPE::List::callDisplayLists(
            n, ESUtils::TypeTraits::asTypedArray<T>(lists)
        );
    });
}

void glListBase(GLuint base) {
    LOGI("glListBase: %u", base);
}

GLboolean glIsList(GLuint list) {
    return FFPE::List::isList(list);
}
