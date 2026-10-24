#include "ffp/ffp.hpp"
#include "ffp/alphatest/alphatest.hpp"
#include "ffp/arrays/arrays.hpp"
#include "ffp/fog/fog.hpp"
#include "ffp/immediate/immediate.hpp"
#include "ffp/light/light.hpp"
#include "ffp/list/list.hpp"
#include "ffp/matrix/matrix.hpp"
#include "ffp/shading/shading.hpp"
#include "ffp/stubs/stubs.hpp"
#include "ffp/vertex/vertex.hpp"
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
