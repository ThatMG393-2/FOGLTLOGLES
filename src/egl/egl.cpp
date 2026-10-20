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

    REGISTER(eglBindAPI);
    REGISTER(eglChooseConfig);
    REGISTER(eglCreateContext);
    REGISTER(eglCreatePbufferSurface);
    REGISTER(eglCreateWindowSurface);
    REGISTER(eglDestroyContext);
    REGISTER(eglDestroySurface);
    REGISTER(eglGetConfigAttrib);
    REGISTER(eglGetCurrentContext);
    REGISTER(eglGetDisplay);
    REGISTER(eglGetError);
    REGISTER(eglGetProcAddress);
    REGISTER(eglInitialize);
    REGISTEROV(eglMakeCurrent);
    REGISTER(eglSwapBuffers);
    REGISTER(eglReleaseThread);
    REGISTER(eglSwapInterval);
    REGISTER(eglTerminate);
    REGISTER(eglGetCurrentSurface);
    REGISTER(eglQuerySurface);

    void* eglGetProcAddressSymbol = dlsym(eglLibHandle, "eglGetProcAddress");
    if (!eglLibHandle || !eglGetProcAddressSymbol) {
        LOGW("Failed to load libEGL.so/eglGetProcAddress! You may experience some issues...");
        return;
    }

    real_eglGetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(eglGetProcAddressSymbol);

    LOGI("Done initializing!");
}
