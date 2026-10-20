#pragma once

#include "compilers.hpp"
#include "postprocess.hpp"
#include "preprocess.hpp"
#include "es/utils.hpp"
#include "gl/shader.hpp"
#include "shaderc/shaderc.h"
#include "shaderc/status.h"
#include "spirv_glsl.hpp"
#include "utils.hpp"

#include <stdexcept>
#include <string>
#include <shaderc/shaderc.hpp>

namespace ShaderConverter {
    inline shaderc::SpvCompilationResult compileToSPV(shaderc_shader_kind kind, std::string& source) {
        int shaderVersion = 0;
        std::string_view profile;
        getShaderVersion(source, shaderVersion, profile);

        GLSLPreprocessor::run(source, shaderVersion);

        if (SHADER_DUMP) {
            LOGE("Preprocessed source:");
            LOGE("%s", source.c_str());
        }

        if (shaderVersion < 330) {
            upgradeTo330(kind, source);
            shaderVersion = 330;
        }

        shaderc::CompileOptions options = generateGLSL2SPVOptions(shaderVersion);
        shaderc::SpvCompilationResult result = getShaderCompiler().CompileGlslToSpv(source, kind, "glslShader", options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOGE("Failed to compile shader: %s", result.GetErrorMessage().c_str());
            throw std::runtime_error("Shader compilation failed!");
        }

        return result;
    }

    inline void transpileSPV2ESSL(shaderc_shader_kind kind, const shaderc::SpvCompilationResult& module, std::string& target) {
        SPVCExposed_CompilerGLSL compiler(module.cbegin(), static_cast<size_t>(module.cend() - module.cbegin()));
        compiler.set_common_options(generateSPV2ESSLOptions(ESUtils::shadingVersion));

        SPVCPostprocessor::processSPVBytecode(compiler, kind);

        target = compiler.compile();

        if (SHADER_DUMP) {
            LOGI("Transpiled GLSL -> ESSL source:");
            LOGI("%s", target.c_str());
        }
    }

    // `ubos` receives the UBO bindings used by the converted shader.
    inline void convertAndFix(
        shaderc_shader_kind kind,
        std::string& source,
        std::vector<std::pair<std::string, uint32_t>>* ubos = nullptr
    ) {
        if (SHADER_DUMP) {
            LOGI("Converting and fixing %s!", getKindStringFromKind(kind));
            LOGI("Received GLSL source:");
            LOGI("%s", source.c_str());
        }

        shaderc::SpvCompilationResult spirv = compileToSPV(kind, source);

        SPVCPostprocessor::bindingSink = ubos;
        try {
            transpileSPV2ESSL(kind, spirv, source);
        } catch (...) {
            SPVCPostprocessor::bindingSink = nullptr;
            throw;
        }
        SPVCPostprocessor::bindingSink = nullptr;
    }
}; // namespace ShaderConverter
