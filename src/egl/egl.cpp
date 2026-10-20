#include "egl/egl.hpp"
#include "fogltlogles.hpp"
#include "common/log.hpp"
#include "common/types.hpp"

#include <EGL/egl.h>
#include <dlfcn.h>
#include <mutex>

FunctionPtr eglGetProcAddress(str procname) {
    std::call_once(eglInitFlag, eglInit);
    FunctionPtr tmp = FOGLTLOGLES::getFunctionAddress(procname);
    
    if (tmp) return tmp;
    if (real_eglGetProcAddress) return real_eglGetProcAddress(procname);
    
    return nullptr;
}

EGLBoolean OV_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    EGLBoolean result = eglMakeCurrent(dpy, draw, read, ctx);
    std::call_once(rendererInitFlag, FOGLTLOGLES::init);
    return result;
}

inline void eglInit() {
    LOGI("Initializing EGL functions...");

    PROVIDE(eglBindAPI);
    PROVIDE(eglChooseConfig);
    PROVIDE(eglCreateContext);
    PROVIDE(eglCreatePbufferSurface);
    PROVIDE(eglCreateWindowSurface);
    PROVIDE(eglDestroyContext);
    PROVIDE(eglDestroySurface);
    PROVIDE(eglGetConfigAttrib);
    PROVIDE(eglGetCurrentContext);
    PROVIDE(eglGetDisplay);
    PROVIDE(eglGetError);
    PROVIDE(eglGetProcAddress);
    PROVIDE(eglInitialize);
    PROVIDE(eglMakeCurrent, OV_eglMakeCurrent);
    PROVIDE(eglSwapBuffers);
    PROVIDE(eglReleaseThread);
    PROVIDE(eglSwapInterval);
    PROVIDE(eglTerminate);
    PROVIDE(eglGetCurrentSurface);
    PROVIDE(eglQuerySurface);

    void* eglGetProcAddressSymbol = dlsym(eglLibHandle, "eglGetProcAddress");
    if (!eglLibHandle || !eglGetProcAddressSymbol) {
        LOGW("Failed to load libEGL.so/eglGetProcAddress! You may experience some issues...");
        return;
    }

    real_eglGetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(eglGetProcAddressSymbol);

    LOGI("Done initializing!");
}
