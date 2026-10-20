#include "framebuffer/main.hpp"
#include "common/log.hpp"

void FBO::init() {
    LOGI("Framebuffer entrypoint!");

    FBO::registerRenderbufferWorkaround();
    FBO::registerFramebufferOverrides();
}
