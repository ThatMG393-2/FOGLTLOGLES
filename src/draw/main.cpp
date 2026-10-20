#include "draw/main.hpp"
#include "common/log.hpp"

void Draw::init() {
    LOGI("Draw entrypoint!");

    Draw::registerDrawOverride();
    Draw::registerMultiDrawEmulation();
    Draw::registerBatchedMultiDraw();
    Draw::registerBaseVertexFunction();
}
