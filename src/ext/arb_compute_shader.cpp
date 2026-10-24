#include "common/es_utils.hpp"
#include "common/log.hpp"
#include "ext/ext.hpp"

void Ext::register_ARB_compute_shader() {
    LOGI("Not exposing GL_ARB_compute_shader.");

    // ESUtils::fakeExtensions.insert("GL_ARB_compute_shader");
}
