#include "ffp/ffp.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

void glColorMaterial(GLenum face, GLenum mode);

void FFP::registerColorFunctions() {
    PROVIDE(glColor3f);
    PROVIDE(glColor4f);
    
    PROVIDE(glColorMaterial);
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
    if (!FFPE::Rendering::ImmediateMode::isActive()) {
        FFPE::List::addCommand<glColor3f>(red, green, blue);
        return;
    }

    FFPE::States::VertexData::set(
        glm::vec3(red, green, blue),
        FFPE::States::VertexData::color
    );
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    if (!FFPE::Rendering::ImmediateMode::isActive()) {
        FFPE::List::addCommand<glColor4f>(red, green, blue, alpha);
        return;
    }

    FFPE::States::VertexData::set(
        glm::vec4(red, green, blue, alpha),
        FFPE::States::VertexData::color
    );
}

void glColorMaterial(GLenum face, GLenum mode) {

}
