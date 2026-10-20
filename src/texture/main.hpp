#pragma once

#include <memory>

namespace Texture {
    void init();

    void registerTextureOverrides();
    void registerReadPixelsOverride();
    void registerQueryOverrides();
}
