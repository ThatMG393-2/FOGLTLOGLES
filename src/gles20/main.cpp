#include "gles20/main.h"
#include "utils/log.h"

void GLES20::GLES20Wrapper::init() {
    LOGI("GLES 2.0 overrides entrypoint!");

    GLES20::registerTranslatedFunctions();
    GLES20::registerShaderOverrides();
    GLES20::registerTextureOverrides();
    GLES20::registerMultiDrawEmulation();
    GLES20::registerBufferWorkarounds();
    GLES20::registerBindFunctions();
}