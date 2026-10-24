#include "ffp/list/list.hpp"

#include <GLES3/gl32.h>
#include <utility>

namespace FFPE::List {

// i dont know if this is even correct
GLuint genDisplayLists(GLsizei range) {
    if (range <= 0) return 0;

    for (GLsizei i = 0; i < range; ++i) {
        States::displayLists.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(i + States::nextListIndex),
            std::forward_as_tuple(DisplayList())
        );
    }

    return std::exchange(States::nextListIndex, States::nextListIndex += range);
}

void startDisplayList(GLuint list, GLenum mode) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    States::activeDisplayListIndex = list;
    States::activeDisplayList = DisplayList(mode);

    std::string dbgMes = "Recording list : " + std::to_string(list);
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, dbgMes.c_str());
}

void ignoreNextCall() {
    if (!isRecording() || isExecuting()) return;
    States::ignoreNextCallFlag = true;
}

void endDisplayList() {
    if (!isRecording() || isExecuting()) return;

    States::displayLists.insert({
        States::activeDisplayListIndex,
        States::activeDisplayList
    });

    States::activeDisplayListIndex = 0;
    States::activeDisplayList = DisplayList();

    glPopDebugGroup();
}

void deleteDisplayLists(GLuint list, GLsizei range) {
    if (range <= 0) return;

    for (GLsizei i = 0; i < range; ++i) {
        if (!isList(list)) continue;
        States::displayLists.erase(list + i);
    }
}

void callDisplayList(GLuint list) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    GLDebugGroup gldg("List replay : " + std::to_string(list));

    States::executingDisplayList = true;
    States::displayLists[list].execute();
    States::executingDisplayList = false;
}

}
