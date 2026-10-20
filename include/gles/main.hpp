#pragma once

#include "utils/env.hpp"
#include "wrappers/base.hpp"

#include <memory>

inline bool debugEnabled = SHADER_DUMP;

namespace GLES {
    class GLESWrapper : BaseWrapper {
        public:
            void init();
    };
    
    const inline std::shared_ptr<GLESWrapper> wrapper = std::make_shared<GLESWrapper>();
    
    void registerBrandingOverride();
    void registerTranslatedFunctions();
    void registerNoErrorOverride();
    void registerDrawOverride();
}