#pragma once

#include <memory>

namespace FFP {
    void init();

    void registerStubFunctions();
    void registerImmediateFunctions();
    void registerColorFunctions();
    void registerTexCoordFunctions();
    void registerVertexFunctions();
    void registerMatrixFunctions();
    void registerListsFunctions();
    void registerAlphaTestFunctions();
    void registerTextureFunctions();
    void registerArrayFunctions();
    void registerShadingFunction();
    void registerFogFunctions();
    void registerNormalFunctions();
    void registerLightFunctions();
}
