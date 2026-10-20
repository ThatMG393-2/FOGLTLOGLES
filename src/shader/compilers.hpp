#pragma once

#include <shaderc/shaderc.hpp>

#include "shaderc/env.h"
#include "spirv_glsl.hpp"

inline shaderc::CompileOptions generateGLSL2SPVOptions(int glslVersion) {
    shaderc::CompileOptions options;
    options.SetGenerateDebugInfo();
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_opengl, glslVersion);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetVulkanRulesRelaxed(true);

    options.SetAutoMapLocations(true);
    options.SetAutoBindUniforms(true);
    
    return options;
}

inline spirv_cross::CompilerGLSL::Options generateSPV2ESSLOptions(int esslVersion) {
    spirv_cross::CompilerGLSL::Options options;
    options.version = esslVersion;
    options.es = true;
    options.vulkan_semantics = false;
    options.enable_420pack_extension = false;
    options.force_flattened_io_blocks = true;
    options.enable_storage_image_qualifier_deduction = false;

    return options;
}
