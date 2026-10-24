#include "egl/egl.hpp"
#include "state/state.hpp"
#include "common/es_utils.hpp"
#include "ext/ext.hpp"
#include "fogltlogles.hpp"

#include <GLES2/gl2ext.h>

inline PFNGLBUFFERSTORAGEEXTPROC glBufferStorageEXT = nullptr;

void Ext::register_EXT_buffer_storage() {
    if (!ESUtils::isExtensionSupported("GL_EXT_buffer_storage")) return;
    LOGI("EXT_buffer_storage is present and so used.");

    ESUtils::fakeExtensions.insert("GL_ARB_buffer_storage");

    glBufferStorageEXT = reinterpret_cast<PFNGLBUFFERSTORAGEEXTPROC>(GETFUNCEXT(glBufferStorage, "EXT"));
    PROVIDE(glBufferStorage);
}

void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) {
    glBufferStorageEXT(target, size, data, flags);

    trackedStates->boundBuffers[target].size = size;
}