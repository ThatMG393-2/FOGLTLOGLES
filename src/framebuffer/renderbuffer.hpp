#pragma once

#include "state/state_tracking.hpp"

#include <GLES2/gl2.h>

inline void trackRenderbufferFormat(GLenum& internalFormat) {
    trackedStates->renderbufferInternalFormats.insert({ trackedStates->boundRenderbuffer, internalFormat });
}

inline void fixStorageParams(GLenum& internalFormat) {
    switch (internalFormat) {
        case GL_DEPTH_COMPONENT:
            internalFormat = GL_DEPTH_COMPONENT16;
            break;
    }
}
