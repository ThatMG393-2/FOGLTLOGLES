#pragma once

#include <GLES2/gl2.h>
#include <memory>

void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);

#define GETFUNCEXT(name, suffix) TO_FUNCPTR(real_eglGetProcAddress(#name suffix))

namespace Ext {
    void init();

    void register_EXT_buffer_storage();
    void register_OES_mapbuffer();
    void register_EXT_blend_func_extended();
    void register_ARB_compute_shader();
    void register_ARB_timer_query();
    void register_EXT_framebuffer_object();
}
