#include "gl/gl.hpp"
#include "egl/egl.hpp"
#include "fogltlogles.hpp"
#include "common/env.hpp"
#include "common/log.hpp"

#include <mutex>
#include <GLES3/gl32.h>

inline std::once_flag initDebugFlag;

void OV_glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);

FunctionPtr glXGetProcAddress(const GLchar* pn) {
    std::call_once(eglInitFlag, eglInit);
    std::call_once(rendererInitFlag, FOGLTLOGLES::init);
    std::call_once(initDebugFlag, initDebug);

    FunctionPtr tmp = FOGLTLOGLES::getFunctionAddress(pn);
    
    if (tmp) return tmp;
    if (real_eglGetProcAddress) return real_eglGetProcAddress(pn);

    return nullptr;
}

void customDebugCallback(
    GLenum, GLenum,
    GLuint, GLenum,
    GLsizei, const GLchar*,
    const void*
);

void initDebug() {
    if (SHADER_DUMP) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(customDebugCallback, nullptr);
    }

    // PROVIDE(glDebugMessageCallback, OV_glDebugMessageCallback);
}

void customDebugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    if (severity != GL_DEBUG_SEVERITY_HIGH) return;

    LOGD("[GL DEBUG] Source: %u, Type: %u, ID: %u, Severity: GL_DEBUG_SEVERITY_HIGH", source, type, id);
    LOGD("[GL DEBUG] Message: %s", message);
}

void OV_glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam) {
    LOGI("Intercepted glDebugMessageCallback!");
    glDebugMessageCallback(customDebugCallback, userParam);
}
