#pragma once

#include "common/raii_helpers.hpp"
#include "ffp/enums.hpp"
#include "common/fast_map.hpp"

#include <GLES3/gl32.h>
#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

template<auto Func>
constexpr std::string getTypeNameFromTemplate() {
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view fn = __PRETTY_FUNCTION__;
    constexpr std::string_view key = "Func = ";
    auto start = fn.find(key) + key.size();
    auto end = fn.find_first_of("];", start);
    return std::string(fn.substr(start, end - start));
#elif defined(_MSC_VER)
    constexpr std::string_view fn = __FUNCSIG__;
    constexpr std::string_view key = "Func=";
    auto start = fn.find(key) + key.size();
    auto end = fn.find_first_of(">,", start);
    return std::string(fn.substr(start, end - start));
#else
    return "unknown";
#endif
}

namespace FFPE::List {

struct Command {
    const std::string name;
    const std::function<void()> func;

    void operator() () const {
        this->func();
    }
};

// TODO: an optimize function
class DisplayList {
private:
    std::vector<Command> commands;

    GLenum mode = GL_NONE;

public:
    DisplayList() { }
    DisplayList(GLenum mode) : mode(mode) { }
    
    template<typename... Args>
    void addCommand(Args&&... args) {
        commands.emplace_back(std::forward<Args>(args)...);
    }
    
    void setMode(GLenum mode) {
        this->mode = mode;
    }

    void execute() const {
        for (const auto& command : commands) {
            command();
        }
    }

    GLenum getMode() const {
        return mode;
    }
};

namespace States {
    inline GLuint nextListIndex = 1;
    inline bool executingDisplayList;

    inline bool ignoreNextCallFlag;

    inline FastMapBI<GLuint, DisplayList> displayLists;

    inline GLuint activeDisplayListIndex;
    inline DisplayList activeDisplayList;
}

inline bool isList(GLuint list) {
    return States::displayLists.find(list) != States::displayLists.end();
}

inline bool isRecording() {
    return States::activeDisplayListIndex != 0;
}

inline bool isExecuting() {
    return States::executingDisplayList;
}

// i dont know if this is even correct
inline GLuint genDisplayLists(GLsizei range) {
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

inline void startDisplayList(GLuint list, GLenum mode) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    States::activeDisplayListIndex = list;
    States::activeDisplayList = DisplayList(mode);

    std::string dbgMes = "Recording list : " + std::to_string(list);
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, dbgMes.c_str());
}

inline void ignoreNextCall() {
    if (!isRecording() || isExecuting()) return;
    States::ignoreNextCallFlag = true;
}

template<auto F, typename... Args>
requires std::invocable<decltype(F), Args...>
inline bool addCommand(Args&&... args) {
    if (!isRecording() || isExecuting()) return false;
    if (States::ignoreNextCallFlag) {
        States::ignoreNextCallFlag = false;
        return false;
    }

    using DecayedF = std::decay_t<decltype(F)>;
    
    States::activeDisplayList.addCommand(
        getTypeNameFromTemplate<F>(),
        [a = std::make_tuple(args...)]() mutable {
            std::apply(DecayedF { }, std::move(a));
        }
    );

    return !(States::activeDisplayList.getMode() == GL_COMPILE_AND_EXECUTE);
}

inline void endDisplayList() {
    if (!isRecording() || isExecuting()) return;
    
    States::displayLists.insert({
        States::activeDisplayListIndex,
        States::activeDisplayList
    });

    States::activeDisplayListIndex = 0;
    States::activeDisplayList = DisplayList();

    glPopDebugGroup();
}

inline void deleteDisplayLists(GLuint list, GLsizei range) {
    if (range <= 0) return;

    for (GLsizei i = 0; i < range; ++i) {
        if (!isList(list)) continue;
        States::displayLists.erase(list + i);
    }
}

inline void callDisplayList(GLuint list) {
    if (isRecording() || isExecuting()) return;
    if (!isList(list)) return;

    GLDebugGroup gldg("List replay : " + std::to_string(list));
    
    States::executingDisplayList = true;
    States::displayLists[list].execute();
    States::executingDisplayList = false;
}

template<typename T>
inline void callDisplayLists(GLsizei n, const T* lists) {
    States::executingDisplayList = true;
    for (GLsizei i = 0; i < n; ++i) {
        if (!isList(i)) continue;
        GLDebugGroup gldg("List replay : " + std::to_string(lists[i]));
        
        States::displayLists[static_cast<GLuint>(lists[i])].execute();
    }
    States::executingDisplayList = false;
}

}
