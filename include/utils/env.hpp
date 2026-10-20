#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>

inline std::string getEnvironmentVar(std::string const& key, std::string const& fallback = "") {
    char* val = getenv( key.c_str() );
    return val == NULL ? fallback : std::string(val);
}

inline bool envFlag(const char* key, const char* fallback) {
    return getEnvironmentVar(key, fallback) == "1";
}

inline const bool SHADER_DUMP = envFlag("LIBGL_VGPU_DUMP", "0");
inline const bool ENV_NO_ERROR = envFlag("LIBGL_NO_ERROR", "0");

inline bool ENABLE_SHADER_CACHE = envFlag("FOGLE_ENABLE_SHADER_CACHE", "1");
inline bool ENABLE_BINARY_CACHE = envFlag("FOGLE_ENABLE_BINARY_CACHE", "1");

inline const std::string SHADER_CACHE_DIR = getEnvironmentVar("MESA_GLSL_CACHE_DIR");
inline const uint64_t SHADER_RETENTION_DAYS = std::strtoull(getEnvironmentVar("FOGLE_CACHE_RETENTION_DAY", "30").c_str(), nullptr, 10);
