#pragma once

#include "shaderc/shaderc.hpp"
#include "spirv_glsl.hpp"
#include "utils/log.hpp"
#include "utils/env.hpp"

#include <stdexcept>

inline shaderc::Compiler& getShaderCompiler() {
    static thread_local shaderc::Compiler compiler;
    return compiler;
}

inline void upgradeTo330(shaderc_shader_kind kind, std::string& src) {
    static const shaderc::CompileOptions options = [] {
        shaderc::CompileOptions o;
        o.SetGenerateDebugInfo();
        o.SetSourceLanguage(shaderc_source_language_glsl);
        o.SetTargetEnvironment(shaderc_target_env_opengl, 330);
        o.SetForcedVersionProfile(330, shaderc_profile_core);
        o.SetOptimizationLevel(shaderc_optimization_level_zero);
        o.SetVulkanRulesRelaxed(true);
        o.SetAutoMapLocations(true);
        o.SetAutoBindUniforms(true);
        return o;
    }();

    shaderc::SpvCompilationResult module = getShaderCompiler().CompileGlslToSpv(src, kind, "shader_upgto330", options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error("Shader upgrade error: " + module.GetErrorMessage());
    }

    spirv_cross::CompilerGLSL glslCompiler(module.cbegin(), static_cast<size_t>(module.cend() - module.cbegin()));
    spirv_cross::CompilerGLSL::Options glslOptions;
    glslOptions.version = 330;
    glslOptions.es = false;
    glslOptions.vulkan_semantics = false;
    glslOptions.enable_420pack_extension = false;
    glslOptions.force_flattened_io_blocks = true;
    glslOptions.enable_storage_image_qualifier_deduction = false;
    glslCompiler.set_common_options(glslOptions);

    src = glslCompiler.compile();

    if (SHADER_DUMP) {
        LOGI("Upgraded shader source:");
        LOGI("%s", src.c_str());
    }
}
