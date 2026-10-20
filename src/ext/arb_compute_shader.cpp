#include "common/es_utils.hpp"
#include "ext/main.hpp"
#include "common/log.hpp"

void Ext::register_ARB_compute_shader() {
    LOGI("Not exposing GL_ARB_compute_shader.");

    // ESUtils::fakeExtensions.insert("GL_ARB_compute_shader");
}
