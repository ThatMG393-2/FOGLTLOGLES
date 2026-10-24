#pragma once

#include "common/env.hpp"

#include <memory>

inline bool debugEnabled = SHADER_DUMP;

namespace Core {
    void init();

    void registerBrandingOverride();
    void registerTranslatedFunctions();
    void registerNoErrorOverride();
}
