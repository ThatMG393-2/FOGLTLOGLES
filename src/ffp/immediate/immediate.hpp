#pragma once

#include "ffp/vertex/vertex.hpp"

#include <GLES3/gl32.h>
#include <vector>

namespace FFPE::Rendering::ImmediateMode {

using VertexData = FFPE::States::VertexData::VertexRepresentation<4, GLfloat, 4, GLfloat, 3, GLfloat, 4, GLfloat>;

namespace States {
    inline GLenum primitive;

    inline std::vector<VertexData> vertices;
}

inline bool isActive() {
    return States::primitive != GL_NONE;
}

void begin(GLenum primitive);
void advance();
void endInternal(const GLenum primitive, const std::vector<VertexData>& vertices);
void end();

}

namespace FFP {
    void registerImmediateFunctions();
}
