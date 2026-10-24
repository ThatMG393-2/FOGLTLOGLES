#include "ffp/list/list.hpp"
#include "ffp/immediate/immediate.hpp"
#include "state/state.hpp"

#include <GLES3/gl32.h>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

namespace FFPE::List {

namespace {

constexpr GLenum LIST_2_BYTES = 0x1407;
constexpr GLenum LIST_3_BYTES = 0x1408;
constexpr GLenum LIST_4_BYTES = 0x1409;

struct DepthGuard {
    DepthGuard() { ++States::executionDepth; }
    ~DepthGuard() { --States::executionDepth; }
};

bool isInsideBeginEnd() {
    return FFPE::Rendering::ImmediateMode::isActive();
}

void executeList(GLuint list) {
    if (States::executionDepth >= MAX_NESTING) return;

    auto it = States::displayLists.find(list);
    if (it == States::displayLists.end()) return;

    auto displayList = it->second;
    GLDebugGroup gldg("List replay : " + std::to_string(list));

    DepthGuard guard;
    displayList->execute();
}

void callDisplayListNames(const std::vector<GLuint>& names) {
    for (GLuint name : names) executeList(States::listBase + name);
}

template<typename T>
void decodeTyped(GLsizei n, const unsigned char* src, std::vector<GLuint>& out) {
    for (GLsizei i = 0; i < n; ++i) {
        T value;
        std::memcpy(&value, src + static_cast<size_t>(i) * sizeof(T), sizeof(T));

        if constexpr (std::is_floating_point_v<T>) out.push_back(static_cast<GLuint>(static_cast<GLint>(value)));
        else out.push_back(static_cast<GLuint>(value));
    }
}

void decodePacked(GLsizei n, GLuint width, const unsigned char* src, std::vector<GLuint>& out) {
    for (GLsizei i = 0; i < n; ++i) {
        GLuint value = 0;
        for (GLuint b = 0; b < width; ++b) {
            value = (value << 8) | src[static_cast<size_t>(i) * width + b];
        }
        out.push_back(value);
    }
}

bool decodeNames(GLsizei n, GLenum type, const void* lists, std::vector<GLuint>& out) {
    const auto* src = static_cast<const unsigned char*>(lists);
    out.reserve(n);

    switch (type) {
        case GL_BYTE: decodeTyped<GLbyte>(n, src, out); return true;
        case GL_UNSIGNED_BYTE: decodeTyped<GLubyte>(n, src, out); return true;
        case GL_SHORT: decodeTyped<GLshort>(n, src, out); return true;
        case GL_UNSIGNED_SHORT: decodeTyped<GLushort>(n, src, out); return true;
        case GL_INT: decodeTyped<GLint>(n, src, out); return true;
        case GL_UNSIGNED_INT: decodeTyped<GLuint>(n, src, out); return true;
        case GL_FLOAT: decodeTyped<GLfloat>(n, src, out); return true;
        case LIST_2_BYTES: decodePacked(n, 2, src, out); return true;
        case LIST_3_BYTES: decodePacked(n, 3, src, out); return true;
        case LIST_4_BYTES: decodePacked(n, 4, src, out); return true;
        default: return false;
    }
}

}

// GL errors are surfaced by provoking the driver with a call that always fails
void raiseError(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM: glEnable(0); break;
        case GL_INVALID_VALUE: glDeleteBuffers(-1, nullptr); break;
        case GL_INVALID_OPERATION: glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE); break;
        default: break;
    }
}

GLuint genDisplayLists(GLsizei range) {
    if (isInsideBeginEnd()) {
        raiseError(GL_INVALID_OPERATION);
        return 0;
    }

    if (range < 0) {
        raiseError(GL_INVALID_VALUE);
        return 0;
    }

    if (range == 0) return 0;

    GLuint first = 1;
    GLsizei found = 0;
    while (found < range) {
        if (first > std::numeric_limits<GLuint>::max() - static_cast<GLuint>(range)) return 0;

        GLuint name = first + found;
        if (isList(name) || name == States::recordingIndex) {
            first = name + 1;
            found = 0;
        } else ++found;
    }

    static const auto emptyList = std::make_shared<const DisplayList>();
    for (GLsizei i = 0; i < range; ++i) {
        States::displayLists[first + i] = emptyList;
    }

    return first;
}

void deleteDisplayLists(GLuint list, GLsizei range) {
    if (isInsideBeginEnd()) {
        raiseError(GL_INVALID_OPERATION);
        return;
    }

    if (range < 0) {
        raiseError(GL_INVALID_VALUE);
        return;
    }

    if (static_cast<size_t>(range) > States::displayLists.size()) {
        for (auto it = States::displayLists.begin(); it != States::displayLists.end();) {
            if (static_cast<GLuint>(it->first - list) < static_cast<GLuint>(range)) it = States::displayLists.erase(it);
            else ++it;
        }
        return;
    }

    for (GLsizei i = 0; i < range; ++i) {
        States::displayLists.erase(list + i);
    }
}

void startDisplayList(GLuint list, GLenum mode) {
    if (isInsideBeginEnd() || isRecording()) {
        raiseError(GL_INVALID_OPERATION);
        return;
    }

    if (list == 0) {
        raiseError(GL_INVALID_VALUE);
        return;
    }

    if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE) {
        raiseError(GL_INVALID_ENUM);
        return;
    }

    States::recordingIndex = list;
    States::recordingMode = mode;
    States::recordingList = DisplayList();

    std::string dbgMes = "Recording list : " + std::to_string(list);
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, dbgMes.c_str());
}

void ignoreNextCall() {
    if (!isRecording() || isExecuting()) return;
    States::ignoreNextCallFlag = true;
}

void endDisplayList() {
    if (isInsideBeginEnd() || !isRecording()) {
        raiseError(GL_INVALID_OPERATION);
        return;
    }

    States::displayLists[States::recordingIndex] = std::make_shared<const DisplayList>(std::move(States::recordingList));

    States::recordingIndex = 0;
    States::recordingMode = GL_NONE;
    States::recordingList = DisplayList();
    States::ignoreNextCallFlag = false;

    glPopDebugGroup();
}

void callDisplayList(GLuint list) {
    if (addCommand<callDisplayList>(list)) return;

    executeList(list);
}

void callDisplayLists(GLsizei n, GLenum type, const void* lists) {
    if (n < 0) {
        raiseError(GL_INVALID_VALUE);
        return;
    }

    std::vector<GLuint> names;
    if (n > 0 && lists != nullptr) {
        if (!decodeNames(n, type, lists, names)) {
            raiseError(GL_INVALID_ENUM);
            return;
        }
    } else if (n > 0) return;

    if (addCommand<callDisplayListNames>(names)) return;

    callDisplayListNames(names);
}

void setListBase(GLuint base) {
    if (isInsideBeginEnd()) {
        raiseError(GL_INVALID_OPERATION);
        return;
    }

    States::listBase = base;
}

bool queryIsList(GLuint list) {
    if (isInsideBeginEnd()) {
        raiseError(GL_INVALID_OPERATION);
        return false;
    }

    return list != 0 && isList(list);
}

}
