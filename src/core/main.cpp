#include "core/main.hpp"
#include "common/log.hpp"

void Core::init() {
    LOGI("Core entrypoint!");

    Core::registerTranslatedFunctions();
    Core::registerBrandingOverride();
    Core::registerNoErrorOverride();
}
