#include "build_info.hpp"
#include "ffp/manager/manager.hpp"
#include "state/state.hpp"
#include "common/enums.hpp"
#include "common/es_utils.hpp"
#include "common/strings.hpp"
#include "core/core.hpp"
#include "fogltlogles.hpp"

#include <GLES3/gl32.h>
#include <string>

#ifndef CAST_TO_CUBYTE
#define CAST_TO_CUBYTE(str) reinterpret_cast<const GLubyte*>(str)
#endif

static const GLubyte* HK_glGetString(NEXT(glGetString) next, GLenum name);
static void HK_glGetIntegerv(NEXT(glGetIntegerv) next, GLenum pname, int* v);
static const GLubyte* HK_glGetStringi(NEXT(glGetStringi) next, GLenum pname, int index);
static void HK_glEnable(NEXT(glEnable) next, GLenum cap);
static void HK_glDisable(NEXT(glDisable) next, GLenum cap);

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

    HOOK(glGetString);
    HOOK(glGetIntegerv);
    HOOK(glGetStringi);
    HOOK(glEnable);
    HOOK(glDisable);
}

static const GLubyte* HK_glGetString(NEXT(glGetString) next, GLenum name) {
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
            return next(name);
    }
}

static void HK_glGetIntegerv(NEXT(glGetIntegerv) next, GLenum pname, int* v) {
    if (debugEnabled) LOGI("HK_glGetIntegerv: pname=%u", pname);
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
            next(pname, v);
            break;
    }
}

static const GLubyte* HK_glGetStringi(NEXT(glGetStringi) next, GLenum pname, int index) {
    switch (pname) {
        case GL_EXTENSIONS:
            if (index < 0 || static_cast<size_t>(index) >= ESUtils::fakeExtensions.size()) return nullptr;
            {
                auto it = ESUtils::fakeExtensions.begin();
                std::advance(it, index);
                return CAST_TO_CUBYTE((*it).c_str());
            }

        default:
            return next(pname, index);
    }
}

static void HK_glEnable(NEXT(glEnable) next, GLenum cap) {
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
            next(cap);
            break;
    }
}

static void HK_glDisable(NEXT(glDisable) next, GLenum cap) {
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
            next(cap);
            break;
    }
}

