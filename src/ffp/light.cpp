#include "ffp/main.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>

void glLightfv(GLenum light, GLenum pname, const GLfloat *params);

void glLightModelfv(GLenum pname, const GLfloat *params);

void FFP::registerLightFunctions() {
    PROVIDE(glLightfv);

    PROVIDE(glLightModelfv);
}

void glLightfv(GLenum light, GLenum pname, const GLfloat *params) {

}

void glLightModelfv(GLenum pname, const GLfloat *params) {

}
