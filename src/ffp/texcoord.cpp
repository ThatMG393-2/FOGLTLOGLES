#include "ffp/lists.hpp"
#include "ffp/states.hpp"
#include "ffp/immediate.hpp"
#include "ffp/main.hpp"
#include "glm/ext/vector_float2.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glTexCoord2f(GLfloat s, GLfloat t);

void FFP::registerTexCoordFunctions() {
    PROVIDE(glTexCoord2f);
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
