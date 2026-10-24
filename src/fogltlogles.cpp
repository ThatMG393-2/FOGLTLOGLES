#include "build_info.hpp"
#include "fogltlogles.hpp"
#include "common/es_utils.hpp"
#include "common/limits.hpp"
#include "common/log.hpp"
#include "core/core.hpp"
#include "draw/draw.hpp"
#include "ext/ext.hpp"
#include "ffp/ffp.hpp"
#include "framebuffer/framebuffer.hpp"
#include "shader/shader.hpp"
#include "state/state.hpp"
#include "texture/texture.hpp"
#include "vertex/vertex.hpp"

#include <GLES3/gl32.h>
#include <unordered_map>

inline std::unordered_map<std::string, FOGLTLOGLES::Entry> registeredFunctions;

FOGLTLOGLES::Entry& FOGLTLOGLES::entryFor(const std::string& name) {
    return registeredFunctions[name];
}

void FOGLTLOGLES::provide(const std::string& name, FunctionPtr function) {
    Entry& e = entryFor(name);

    if (e.base) {
        LOGI("Overriding %s", name.c_str());
    } else {
        LOGI("Registering %s", name.c_str());
    }

    e.base = function;
    if (!e.hooked) e.exposed = function;
}

FunctionPtr FOGLTLOGLES::getFunctionAddress(std::string name) {
    if (name.empty()) return nullptr; // fast path
    if (auto it = registeredFunctions.find(name);
        it != registeredFunctions.end()) {
        // LOGI("Found function named %s", name.c_str());
        return it->second.exposed;
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
    
    State::init();
    Core::init();
    FFP::init();
    Shader::init();
    Texture::init();
    Draw::init();
    FBO::init();
    Ext::init();
    Vertex::init();
}
