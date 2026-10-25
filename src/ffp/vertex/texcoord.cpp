#include "ffp/vertex/vertex.hpp"
#include "ffp/immediate/immediate.hpp"
#include "ffp/list/list.hpp"
#include "glm/ext/vector_float2.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glTexCoord2f(GLfloat s, GLfloat t);
void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);

void FFP::registerTexCoordFunctions() {
    PROVIDE(glTexCoord2f);
    PROVIDE(glMultiTexCoord2f);
}

void glTexCoord2f(GLfloat s, GLfloat t) {
    if (!FFPE::Rendering::ImmediateMode::isActive()) {
        FFPE::List::addCommand<glTexCoord2f>(s, t);
        return;
    }

    FFPE::States::VertexData::set(
        glm::vec2(s, t),
        FFPE::States::VertexData::texCoord
    );
}

void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) {
    if (!FFPE::Rendering::ImmediateMode::isActive()) {
        FFPE::List::addCommand<glMultiTexCoord2f>(target, s, t);
        return;
    }

    if (target != GL_TEXTURE0) return;

    FFPE::States::VertexData::set(
        glm::vec2(s, t),
        FFPE::States::VertexData::texCoord
    );
}
