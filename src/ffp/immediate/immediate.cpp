#include "ffp/immediate/immediate.hpp"
#include "ffp/arrays/arrays.hpp"
#include "ffp/list/list.hpp"
#include "draw/draw.hpp"
#include "common/es_utils.hpp"
#include "common/log.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>
#include <cstddef>

void FFP::registerImmediateFunctions() {
    FOGLTLOGLES::provide("glBegin", TO_FUNCPTR(FFPE::Rendering::ImmediateMode::begin));
    FOGLTLOGLES::provide("glEnd", TO_FUNCPTR(FFPE::Rendering::ImmediateMode::end));
}

namespace FFPE::Rendering::ImmediateMode {

void begin(GLenum primitive) {
    if (isActive()) {
        LOGE("glBegin has not been ended yet!");
        return;
    }

    switch (primitive) {
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case 0x7: // GL_QUADS
        case 0x8: // GL_QUAD_STRIP
        case 0x9: // GL_POLYGON
            break;
        default:
            return;
    }

    if (FFPE::List::addCommand<begin>(primitive)) return;

    States::primitive = primitive;
}

void advance() {
    if (!isActive()) return;
    States::vertices.emplace_back(
        FFPE::States::VertexData::position,
        FFPE::States::VertexData::color,
        FFPE::States::VertexData::normal,
        FFPE::States::VertexData::texCoord
    );
}

// TODO: merge with other immediates on display list end record
void endInternal(
    const GLenum primitive,
    const std::vector<VertexData>& vertices
) {
    if (!isActive()) {
        LOGE("glEnd has not been called yet!");
        return;
    }

    if (vertices.size() == 0) {
        LOGW("glEnd called with no vertices??");
        LOGW("Ignoring!!");
        return;
    }

    if (FFPE::List::addCommand<endInternal>(
        primitive,
        vertices
    )) return;

    GLDebugGroup gldg("Immediate mode rendering");

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    const auto* base = reinterpret_cast<const char*>(vertices.data());

    glVertexPointer(
        decltype(VertexData::position)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::position
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data()
    );

    glColorPointer(
        decltype(VertexData::color)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::color
            )::value_type
        >::value,
        sizeof(VertexData), base + offsetof(VertexData, color)
    );

    glNormalPointer(
        decltype(VertexData::normal)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::normal
            )::value_type
        >::value,
        sizeof(VertexData), base + offsetof(VertexData, normal)
    );

    glTexCoordPointer(
        decltype(VertexData::texCoord)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::texCoord
            )::value_type
        >::value,
        sizeof(VertexData), base + offsetof(VertexData, texCoord)
    );

    FFPE::List::ignoreNextCall();
    OV_glDrawArrays(primitive, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void end() {
    endInternal(States::primitive, States::vertices);

    States::primitive = GL_NONE;
    States::vertices.clear();
}

}
