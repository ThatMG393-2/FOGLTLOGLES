#include "common/es_utils.hpp"
#include "common/log.hpp"
#include "ext/ext.hpp"

void Ext::register_EXT_framebuffer_object() {
    LOGI("ES 3 is present and so GL_EXT_framebuffer_object exposed.");
    
    ESUtils::fakeExtensions.insert("GL_EXT_framebuffer_object");
}
