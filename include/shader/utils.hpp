#pragma once

#include "es/utils.hpp"
#include "utils/types.hpp"
#include "shaderc/shaderc.hpp"
#include "utils/log.hpp"

#include <GLES3/gl31.h>
#include <stdexcept>
#include <string>
#include <string_view>

inline void combineSources(GLsizei count, const GLchar *const* sources, const GLint* lengths, std::string& destination) {
    for (GLsizei i = 0; i < count; i++) {
        if (!sources[i]) continue;

        if (lengths && lengths[i] >= 0) destination.append(sources[i], lengths[i]);
        else destination.append(sources[i]);
    }
}

inline shaderc_shader_kind getKindFromShader(GLuint shader) {
    GLint shaderType;
    glGetShaderiv(shader, GL_SHADER_TYPE, &shaderType);

    switch (shaderType) {
        case GL_FRAGMENT_SHADER:
            return shaderc_fragment_shader;
        case GL_VERTEX_SHADER:
            return shaderc_vertex_shader;
        case GL_COMPUTE_SHADER:
            if (ESUtils::version.first == 3 && ESUtils::version.second >= 1) return shaderc_compute_shader;
            throw std::runtime_error("You need OpenGL ES 3.1 or newer for compute shaders!");

        default:
            LOGI("%u", shader);
            throw std::runtime_error("Received an unsupported shader type!");
    }
}

inline str getKindStringFromKind(shaderc_shader_kind kind) {
    switch (kind) {
        case shaderc_vertex_shader:
            return "VertexShader";
        case shaderc_fragment_shader:
            return "FragmentShader";
        case shaderc_compute_shader:
            return "ComputeShader";
        default: return "";
    }
}

inline bool getShaderVersion(std::string_view source, int& version, std::string_view& profile) {
    constexpr std::string_view token = "#version";

    size_t pos = source.find(token);
    if (pos == std::string_view::npos) return false;

    size_t i = pos + token.size();
    while (i < source.size() && (source[i] == ' ' || source[i] == '\t')) i++;

    size_t digits = i;
    int value = 0;
    while (i < source.size() && source[i] >= '0' && source[i] <= '9') {
        value = value * 10 + (source[i] - '0');
        i++;
    }
    if (i == digits) return false;

    version = value;
    profile = {};

    while (i < source.size() && (source[i] == ' ' || source[i] == '\t')) i++;
    for (std::string_view candidate : { std::string_view("compatibility"), std::string_view("core"), std::string_view("es") }) {
        if (source.compare(i, candidate.size(), candidate) == 0) {
            size_t after = i + candidate.size();
            if (after >= source.size() || !(source[after] == '_' || (source[after] >= '0' && source[after] <= '9')
                || (source[after] >= 'a' && source[after] <= 'z') || (source[after] >= 'A' && source[after] <= 'Z'))) {
                profile = candidate;
                break;
            }
        }
    }

    return true;
}
