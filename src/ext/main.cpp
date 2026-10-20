#include "ext/main.hpp"
#include "common/log.hpp"

void Ext::init() {
    LOGI("Extensions entrypoint!");

    Ext::register_EXT_buffer_storage();
    Ext::register_OES_mapbuffer();
    Ext::register_EXT_blend_func_extended();
    Ext::register_ARB_compute_shader();
    Ext::register_ARB_timer_query();
    Ext::register_EXT_framebuffer_object();
}
