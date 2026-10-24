#include "ffp/ffp.hpp"
#include "common/log.hpp"

void FFP::init() {
    LOGI("FFP entrypoint!");

    FFP::registerStubFunctions();
    FFP::registerImmediateFunctions();
    FFP::registerColorFunctions();
    FFP::registerTexCoordFunctions();
    FFP::registerVertexFunctions();
    FFP::registerMatrixFunctions();
    FFP::registerListsFunctions();
    FFP::registerAlphaTestFunctions();
    FFP::registerTextureFunctions();
    FFP::registerArrayFunctions();
    FFP::registerShadingFunction();
    FFP::registerFogFunctions();
    FFP::registerNormalFunctions();
    FFP::registerLightFunctions();
}
