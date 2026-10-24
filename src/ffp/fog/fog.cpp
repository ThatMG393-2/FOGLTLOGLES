#include "ffp/fog/fog.hpp"
#include "ffp/list/list.hpp"
#include "common/enums.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glFogi(GLenum pname, GLint param);

void glFogf(GLenum pname, GLfloat param);
void glFogfv(GLenum pname, const GLfloat* params);

void FFP::registerFogFunctions() {
    PROVIDE(glFogi);

    PROVIDE(glFogf);
    PROVIDE(glFogfv);
}

void glFogi(GLenum pname, GLint param) {
    if (FFPE::List::addCommand<glFogi>(pname, param)) return;

    switch (pname) {
        case GL_FOG_MODE:
            FFPE::States::Fog::fogMode = param;
        break;

        case GL_FOG_START:
        case GL_FOG_END:
        case GL_FOG_DENSITY:
            glFogf(pname, param);
        return;
    }
}

void glFogf(GLenum pname, GLfloat param) {
    if (FFPE::List::addCommand<glFogf>(pname, param)) return;

    switch (pname) {
        case GL_FOG_START:
            FFPE::States::Fog::Linear::fogStart = param;
        break;

        case GL_FOG_END:
            FFPE::States::Fog::Linear::fogEnd = param;
        break;

        case GL_FOG_DENSITY:
            FFPE::States::Fog::Exp::fogDensity = param;
        break;

        case GL_FOG_MODE:
            glFogi(pname, param);
        return;
    }
}


void glFogfv(GLenum pname, const GLfloat* param) {
    if (FFPE::List::addCommand<glFogfv>(pname, param)) return;

    switch (pname) {
        case GL_FOG_COLOR:
            FFPE::States::Fog::fogColor = glm::make_vec4(param);
        break;
    }
}
