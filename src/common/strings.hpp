#pragma once

#include "common/types.hpp"
#include <stdexcept>
#include <unordered_set>

template<typename ... Args>
inline std::string string_format(const std::string& format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if( size_s <= 0 ) throw std::runtime_error("Error during formatting.");
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[ size ]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

inline std::string join_set(const std::unordered_set<std::string>& uset, const std::string& delimiter) {
    std::string result;
    for (auto it = uset.begin(); it != uset.end(); ++it) {
        result += *it;
        if (std::next(it) != uset.end()) {
            result += delimiter;
        }
    }
    return result;
}