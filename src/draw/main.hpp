#pragma once

#include <memory>

namespace Draw {
    void init();

    void registerDrawOverride();
    void registerMultiDrawEmulation();
    void registerBatchedMultiDraw();
    void registerBaseVertexFunction();
}
