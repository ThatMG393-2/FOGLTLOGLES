#include "common/es_utils.hpp"
#include "common/log.hpp"
#include "ext/ext.hpp"

#include <GLES3/gl32.h>

void Ext::register_ARB_timer_query() {
    LOGI("ES 3 is present and so GL_ARB_timer_query exposed.");
    ESUtils::fakeExtensions.insert("GL_ARB_timer_query");
}