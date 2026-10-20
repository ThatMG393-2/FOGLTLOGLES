#include "texture/main.hpp"
#include "common/log.hpp"

void Texture::init() {
    LOGI("Texture entrypoint!");

    Texture::registerTextureOverrides();
    Texture::registerReadPixelsOverride();
    Texture::registerQueryOverrides();
}
