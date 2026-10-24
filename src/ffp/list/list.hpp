#pragma once

#include "common/enums.hpp"
#include "common/fast_map.hpp"
#include "state/state.hpp"

#include <GLES3/gl32.h>
#include <concepts>
#include <functional>
#include <memory>
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

inline constexpr GLsizei MAX_NESTING = 64;

struct Command {
    std::string name;
    std::function<void()> func;

    void operator() () const {
        this->func();
    }
};

class DisplayList {
private:
    std::vector<Command> commands;

public:
    template<typename... Args>
    void addCommand(Args&&... args) {
        commands.emplace_back(std::forward<Args>(args)...);
    }

    void execute() const {
        for (const auto& command : commands) {
            command();
        }
    }
};

namespace States {
    inline FastMapBI<GLuint, std::shared_ptr<const DisplayList>> displayLists;

    inline GLuint listBase = 0;

    inline GLuint recordingIndex = 0;
    inline GLenum recordingMode = GL_NONE;
    inline DisplayList recordingList;

    inline GLsizei executionDepth = 0;

    inline bool ignoreNextCallFlag;
}

inline bool isList(GLuint list) {
    return States::displayLists.find(list) != States::displayLists.end();
}

inline bool isRecording() {
    return States::recordingIndex != 0;
}

inline bool isExecuting() {
    return States::executionDepth > 0;
}

void raiseError(GLenum error);

GLuint genDisplayLists(GLsizei range);
void deleteDisplayLists(GLuint list, GLsizei range);
void startDisplayList(GLuint list, GLenum mode);
void endDisplayList();
void callDisplayList(GLuint list);
void callDisplayLists(GLsizei n, GLenum type, const void* lists);
void setListBase(GLuint base);
bool queryIsList(GLuint list);
void ignoreNextCall();

template<auto F, typename... Args>
requires std::invocable<decltype(F), Args...>
inline bool addCommand(Args&&... args) {
    if (!isRecording() || isExecuting()) return false;
    if (States::ignoreNextCallFlag) {
        States::ignoreNextCallFlag = false;
        return false;
    }

    States::recordingList.addCommand(
        getTypeNameFromTemplate<F>(),
        [a = std::make_tuple(args...)]() mutable {
            std::apply(F, std::move(a));
        }
    );

    return States::recordingMode == GL_COMPILE;
}

}

namespace FFP {
    void registerListsFunctions();
}
