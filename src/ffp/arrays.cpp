#include "ffp/lists.hpp"
#include "ffp/states.hpp"
#include "ffp/draw.hpp"
#include "common/es_utils.hpp"
#include "ffp/enums.hpp"
#include "ffp/main.hpp"
#include "ffp/arrays.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void FFP::registerArrayFunctions() {
    PROVIDE(glEnableClientState);
    PROVIDE(glDisableClientState);

    PROVIDE(glVertexPointer);
    PROVIDE(glColorPointer);
    PROVIDE(glNormalPointer);
    PROVIDE(glTexCoordPointer);

    FFPE::Rendering::Arrays::init();
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    // LOGI("glVertexPointer : size=%i mode=%u stride=%i start=%p", size, type, stride, pointer);
    
    auto* vertex = FFPE::States::ClientState::Arrays::getArray(GL_VERTEX_ARRAY);
    if (vertex->parameters.size != size) FFPE::States::Manager::invalidateCurrentState();
    
    vertex->parameters = {
        !stride, size, type, GL_FALSE,
        stride ? stride : size * ESUtils::TypeTraits::getTypeSize(type),
        pointer
    };
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    // LOGI("glColorPointer : size=%i mode=%u stride=%i start=%p", size, type, stride, pointer);
    
    auto* color = FFPE::States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    if (color->parameters.size != size) FFPE::States::Manager::invalidateCurrentState();
    
    color->parameters = {
        !stride, size, type, GL_TRUE,
        stride ? stride : size * ESUtils::TypeTraits::getTypeSize(type),
        pointer
    };
}

void glNormalPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    // LOGI("glNormalPointer : size=%i mode=%u stride=%i start=%p", size, type, stride, pointer);
    
    auto* normal = FFPE::States::ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    if (normal->parameters.size != size) FFPE::States::Manager::invalidateCurrentState();

    normal->parameters = {
        !stride, 3, type, GL_FALSE,
        stride ? stride : size * ESUtils::TypeTraits::getTypeSize(type),
        pointer
    };
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {
    // LOGI("glTexCoordPointer : size=%i mode=%u stride=%i start=%p", size, type, stride, pointer);

    auto* texCoord = FFPE::States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);
    if (texCoord->parameters.size != size) FFPE::States::Manager::invalidateCurrentState();
    
    texCoord->parameters = {
        !stride, size, type, GL_FALSE,
        stride ? stride : size * ESUtils::TypeTraits::getTypeSize(type),
        pointer
    };
}

void glEnableClientState(GLenum array) {
    if (FFPE::List::addCommand<glEnableClientState>(array)) return;
    FFPE::States::Manager::invalidateCurrentState();

    switch (array) {
        case GL_TEXTURE_COORD_ARRAY:
            FFPE::States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0)->enabled = true;
        return;

        default:
            FFPE::States::ClientState::Arrays::getArray(array)->enabled = true;
        return;
    }
}

void glDisableClientState(GLenum array) {
    if (FFPE::List::addCommand<glDisableClientState>(array)) return;
    FFPE::States::Manager::invalidateCurrentState();

    switch (array) {
        case GL_TEXTURE_COORD_ARRAY:
            FFPE::States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0)->enabled = false;
        return;

        default:
            FFPE::States::ClientState::Arrays::getArray(array)->enabled = false;
        return;
    }
}

