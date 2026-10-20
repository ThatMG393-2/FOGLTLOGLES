#pragma once

#include "ffp/lists.hpp"
#include "ffp/states.hpp"
#include "common/raii_helpers.hpp"
#include "common/es_utils.hpp"
#include "draw/draw_overrides.hpp"
#include "ffp/arrays.hpp"
#include "ffp/enums.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>
#include <cstddef>
#include <vector>

using VertexData = FFPE::States::VertexData::VertexRepresentation<4, GLfloat, 4, GLfloat, 3, GLfloat, 4, GLfloat>;

namespace FFPE::Rendering::ImmediateMode {

namespace States {
    inline GLenum primitive;

    inline std::vector<VertexData> vertices;
}

inline bool isActive() {
    return States::primitive != GL_NONE;
}

inline void begin(GLenum primitive) {
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

    // LOGI("glBegin()!");

    States::primitive = primitive;
}

inline void advance() {
    if (!isActive()) return;
    States::vertices.emplace_back(
        FFPE::States::VertexData::position,
        FFPE::States::VertexData::color,
        FFPE::States::VertexData::normal,
        FFPE::States::VertexData::texCoord
    );
}

// TODO: merge with other immediates on display list end record
inline void endInternal(
    const GLenum primitive,
    const std::vector<VertexData>& vertices
) {
    if (!isActive()) {
        LOGE("glEnd has not been called yet!");
        return;
    }

    // LOGI("glEnd()!");

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

    /* SaveClientArray sca1(GL_VERTEX_ARRAY);
    SaveClientArray sca2(GL_COLOR_ARRAY);
    SaveTexCoordClientArray stcca(GL_TEXTURE0); */

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
        sizeof(VertexData), vertices.data() + offsetof(VertexData, color)
    );

    glNormalPointer(
        decltype(VertexData::normal)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::normal
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data() + offsetof(VertexData, normal)
    );

    glTexCoordPointer(
        decltype(VertexData::texCoord)::length(),
        ESUtils::TypeTraits::GLTypeEnum<
            decltype(
                VertexData::texCoord
            )::value_type
        >::value,
        sizeof(VertexData), vertices.data() + offsetof(VertexData, texCoord)
    );

    FFPE::List::ignoreNextCall();
    OV_glDrawArrays(primitive, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

inline void end() {
    endInternal(States::primitive, States::vertices);

    States::primitive = GL_NONE;
    States::vertices.clear();
}

}
