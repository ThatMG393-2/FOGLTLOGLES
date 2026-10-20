#include "build_info.hpp"
#include "fogltlogles.hpp"
#include "common/limits.hpp"
#include "common/es_utils.hpp"
#include "core/main.hpp"
#include "draw/main.hpp"
#include "ext/main.hpp"
#include "ffp/main.hpp"
#include "framebuffer/main.hpp"
#include "shader/cache.hpp"
#include "shader/main.hpp"
#include "state/main.hpp"
#include "texture/main.hpp"
#include "vertex/main.hpp"
#include "common/log.hpp"

#include <GLES3/gl32.h>
#include <unordered_map>

inline std::unordered_map<std::string, FunctionPtr> registeredFunctions;

void FOGLTLOGLES::registerFunction(std::string name, FunctionPtr function) {
    if (registeredFunctions.find(name) != registeredFunctions.end()) {
        LOGI("Overriding %s", name.c_str());
    } else {
        LOGI("Registering %s", name.c_str());
    }
    
    registeredFunctions.insert({ name, function });
}

FunctionPtr FOGLTLOGLES::getFunctionAddress(std::string name) {
    if (name.empty()) return nullptr; // fast path
    if (auto it = registeredFunctions.find(name);
        it != registeredFunctions.end()) {
        // LOGI("Found function named %s", name.c_str());
        return it->second;
    }

    // LOGW("Function named %s not found", name.c_str());
    return nullptr;
}

void FOGLTLOGLES::init() {
    LOGI("Using FOGLTLOGLES %s", RENDERER_VERSION);

    ESUtils::init();
    ESLimits::init();

    LOGI("FOGLTLOGLES launched on:");
    if (ESUtils::isAngle)
        LOGI(
            "ANGLE %i.%i.%i",
            std::get<0>(ESUtils::angleVersion),
            std::get<1>(ESUtils::angleVersion),
            std::get<2>(ESUtils::angleVersion)
        );
    
    LOGI("GLES : %i.%i", ESUtils::version.first, ESUtils::version.second);
    LOGI("ESSL : %i", ESUtils::shadingVersion);

    ShaderConverter::Cache::init();
    
    Core::init();
    FFP::init();
    State::init();
    Shader::init();
    Texture::init();
    Draw::init();
    FBO::init();
    Ext::init();
    Vertex::init();
}
