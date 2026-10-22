#include "egl/egl.hpp"
#include "common/es_utils.hpp"
#include "ext/main.hpp"
#include "fogltlogles.hpp"

void Ext::register_EXT_blend_func_extended() {
    if (!ESUtils::isExtensionSupported("GL_EXT_blend_func_extended")) return;
    LOGI("EXT_blend_func_extended is present and so used.");
    
    ESUtils::fakeExtensions.insert("ARB_blend_func_extended");

    FOGLTLOGLES::provide("glBindFragDataLocation", GETFUNCEXT(glBindFragDataLocation, "EXT"));
    FOGLTLOGLES::provide("glBindFragDataLocationIndexed", GETFUNCEXT(glBindFragDataLocationIndexed, "EXT"));
}