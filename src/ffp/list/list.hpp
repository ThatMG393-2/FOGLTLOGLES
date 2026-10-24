#pragma once

#include "common/fast_map.hpp"
#include "state/state.hpp"

#include <GLES3/gl32.h>
#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
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

GLuint genDisplayLists(GLsizei range);
void startDisplayList(GLuint list, GLenum mode);
void endDisplayList();
void deleteDisplayLists(GLuint list, GLsizei range);
void callDisplayList(GLuint list);
void ignoreNextCall();

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

namespace FFP {
    void registerListsFunctions();
}
