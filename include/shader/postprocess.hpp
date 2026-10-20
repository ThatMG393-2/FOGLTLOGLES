#pragma once

#include "shaderc/shaderc.hpp"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include "spirv_glsl.hpp"
#include "utils/log.hpp"

#include <shaderc/shaderc.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class SPVCExposed_CompilerGLSL : public spirv_cross::CompilerGLSL {
    public:
        using spirv_cross::CompilerGLSL::CompilerGLSL;
        using spirv_cross::CompilerGLSL::get;
};

namespace ShaderConverter::SPVCPostprocessor {
    enum {
        rLocation = 1 << 1,
        rBinding = 1 << 2,
        rDescSet = 1 << 3,
        rDefault = rLocation | rBinding | rDescSet
    };

    inline void removeLocationBindingAndDescriptorSets(
        SPVCExposed_CompilerGLSL& compiler,
        const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
        int flags = rDefault
    ) {
        for (const auto& resource : resources) {
            if (flags & rLocation) compiler.unset_decoration(resource.id, spv::DecorationLocation);
            if (flags & rBinding) compiler.unset_decoration(resource.id, spv::DecorationBinding);
            if (flags & rDescSet) compiler.unset_decoration(resource.id, spv::DecorationDescriptorSet);
        }
    }

    inline std::unordered_map<std::string, uint32_t> uniformBuffersBindingIndex;
    inline uint32_t currentBindingIndex = 0;

    // Bindings assigned or reused by the shader currently being converted.
    inline std::vector<std::pair<std::string, uint32_t>>* bindingSink = nullptr;

    inline void assignUniformBufferBindings(
        SPVCExposed_CompilerGLSL& compiler,
        const spirv_cross::SmallVector<spirv_cross::Resource>& resources
    ) {
        for (const auto& resource : resources) {
            std::string name = compiler.get_name(compiler.get_type(resource.base_type_id).self);

            auto it = uniformBuffersBindingIndex.find(name);
            if (it == uniformBuffersBindingIndex.end()) {
                it = uniformBuffersBindingIndex.emplace(name, currentBindingIndex++).first;
            }

            compiler.set_decoration(resource.id, spv::DecorationBinding, it->second);
            if (bindingSink) bindingSink->emplace_back(std::move(name), it->second);
        }
    }

    // Restores bindings baked into a cached shader. Fails without side effects on conflict.
    inline bool registerBindings(const std::vector<std::pair<std::string, uint32_t>>& bindings) {
        for (const auto& [name, binding] : bindings) {
            auto it = uniformBuffersBindingIndex.find(name);
            if (it != uniformBuffersBindingIndex.end()) {
                if (it->second != binding) return false;
                continue;
            }

            for (const auto& [otherName, otherBinding] : uniformBuffersBindingIndex) {
                if (otherBinding == binding) return false;
            }
        }

        for (const auto& [name, binding] : bindings) {
            uniformBuffersBindingIndex.emplace(name, binding);
            if (binding >= currentBindingIndex) currentBindingIndex = binding + 1;
        }

        return true;
    }

    inline void processSPVBytecode(SPVCExposed_CompilerGLSL& compiler, shaderc_shader_kind kind) {
        if (kind == shaderc_glsl_compute_shader) {
            LOGI("Compute shader processing is unimplemented right now...");
            return;
        }

        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        removeLocationBindingAndDescriptorSets(compiler, resources.separate_images);
        removeLocationBindingAndDescriptorSets(compiler, resources.sampled_images);
        removeLocationBindingAndDescriptorSets(compiler, resources.separate_samplers);

        removeLocationBindingAndDescriptorSets(compiler, resources.uniform_buffers);
        assignUniformBufferBindings(compiler, resources.uniform_buffers);

        removeLocationBindingAndDescriptorSets(compiler, resources.gl_plain_uniforms);

        removeLocationBindingAndDescriptorSets(compiler, resources.stage_inputs);
        
        int flags = rDefault;
        if (kind == shaderc_fragment_shader
         && resources.stage_outputs.size() > 1) flags = rDescSet | rBinding;
        removeLocationBindingAndDescriptorSets(compiler, resources.stage_outputs, flags);
    }
}; // namespace ShaderConverter::SPVCPostprocessor
