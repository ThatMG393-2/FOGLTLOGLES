#pragma once

#include <GLES3/gl32.h>

inline void selectProperTexParamf(GLenum target, GLenum& pname, GLfloat& param) {
    switch (pname) {
        case 0x8501: // GL_TEXTURE_LOD_BIAS
            pname = GL_TEXTURE_MIN_LOD;
            return;
    }
}

inline void selectProperTexParami(GLenum target, GLenum& pname, GLint& param) {
    switch (pname) {
        
    }
}
