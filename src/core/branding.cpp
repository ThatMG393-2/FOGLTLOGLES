#include "build_info.hpp"
#include "ffp/lists.hpp"
#include "ffp/states.hpp"
#include "state/state_tracking.hpp"
#include "common/es_utils.hpp"
#include "ffp/enums.hpp"
#include "core/main.hpp"
#include "fogltlogles.hpp"
#include "common/strings.hpp"

#include <GLES3/gl32.h>
#include <string>

#ifndef CAST_TO_CUBYTE
#define CAST_TO_CUBYTE(str) reinterpret_cast<const GLubyte*>(str)
#endif

const GLubyte* OV_glGetString(GLenum name);
void OV_glGetIntegerv(GLenum pname, int* v);
const GLubyte* OV_glGetStringi(GLenum pname, int index);
void OV_glEnable(GLenum cap);
void OV_glDisable(GLenum cap);

inline std::string glVersion;
inline std::string rendererString;

inline std::string fakeExtensionsJoined;

void Core::registerBrandingOverride() {
    glVersion = string_format(
        "4.0 (on ES %i.%i)",
        ESUtils::version.first,
        ESUtils::version.second
    );

    rendererString = string_format(
        "FOGLTLOGLES %s (on %s)",
        RENDERER_VERSION,
        glGetString(GL_RENDERER)
    );

    PROVIDE(glGetString, OV_glGetString);
    PROVIDE(glGetIntegerv, OV_glGetIntegerv);
    PROVIDE(glGetStringi, OV_glGetStringi);
    PROVIDE(glEnable, OV_glEnable);
    PROVIDE(glDisable, OV_glDisable);
}

const GLubyte* OV_glGetString(GLenum name) {
    switch (name) {
        case GL_VERSION:
            return CAST_TO_CUBYTE(glVersion.c_str());
        
        case GL_SHADING_LANGUAGE_VERSION:
            return CAST_TO_CUBYTE("4.00 FOGLTLOGLES"); // 1.50 for GL3.2

        case GL_VENDOR:
            return CAST_TO_CUBYTE("ThatMG393");

        case GL_RENDERER:
            return CAST_TO_CUBYTE(rendererString.c_str());

        case GL_EXTENSIONS:
            fakeExtensionsJoined = join_set(ESUtils::fakeExtensions, " ");
            return CAST_TO_CUBYTE(fakeExtensionsJoined.c_str());

        default:
            return glGetString(name);
    }
}

void OV_glGetIntegerv(GLenum pname, int* v) {
    if (debugEnabled) LOGI("OV_glGetIntegerv: pname=%u", pname);
    switch (pname) {
        case GL_NUM_EXTENSIONS:
            (*v) = ESUtils::fakeExtensions.size();
            break;

        case 0x9126: // GL_CONTEXT_PROFILE_MASK
            (*v) = 0x2; // dawg
            break;

        case GL_MAJOR_VERSION:
            (*v) = 4;
            break;

        case GL_MINOR_VERSION:
            (*v) = 0;
            break;

        default:
            glGetIntegerv(pname, v);
            break;
    }
}

const GLubyte* OV_glGetStringi(GLenum pname, int index) {
    switch (pname) {
        case GL_EXTENSIONS:
            if (index < 0 || static_cast<size_t>(index) >= ESUtils::fakeExtensions.size()) return nullptr;
            {
                auto it = ESUtils::fakeExtensions.begin();
                std::advance(it, index);
                return CAST_TO_CUBYTE((*it).c_str());
            }

        default:
            return glGetStringi(pname, index);
    }
}

void OV_glEnable(GLenum cap) {
    if (FFPE::List::addCommand<OV_glEnable>(cap)) return;
    switch (cap) {
        case GL_ALPHA_TEST:
        case GL_TEXTURE_2D:
        case GL_FOG:
            FFPE::States::Manager::invalidateCurrentState();
        case GL_LIGHTING:
        case GL_COLOR_MATERIAL:
            break;

        case GL_DEBUG_OUTPUT:
        case GL_DEBUG_OUTPUT_SYNCHRONOUS:
            if (!debugEnabled) return; // dont allow debug
            // passthrough!

        default:
            if (debugEnabled) LOGI("glEnable : cap=%u", cap);
            glEnable(cap);
            break;
    }
    
    trackedStates->capabilitiesState[cap] = true;
}

void OV_glDisable(GLenum cap) {
    if (FFPE::List::addCommand<OV_glDisable>(cap)) return;
    switch (cap) {
        case GL_ALPHA_TEST:
        case GL_TEXTURE_2D:
        case GL_FOG:
            FFPE::States::Manager::invalidateCurrentState();
        case GL_LIGHTING:
        case GL_COLOR_MATERIAL:
            break;
    
        default:
            if (debugEnabled) LOGI("glDisable : cap=%u", cap);
            glDisable(cap);
            break;
    }

    trackedStates->capabilitiesState[cap] = false;
}

