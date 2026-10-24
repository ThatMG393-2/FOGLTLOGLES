#pragma once

#include "common/enums.hpp"
#include <GLES3/gl32.h>

namespace FFPE::States::ShadeModel {
    inline GLenum type = GL_SMOOTH;
}

namespace FFP {
    void registerShadingFunction();
}
