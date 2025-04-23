// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_Vk_Utils.H>

#include <iostream>

namespace tl
{
    namespace vk
    {
        struct Shader::Private
        {
            std::string vertexSource;
            std::string fragmentSource;
            VkShaderModule vertex = VK_NULL_HANDLE;
            VkShaderModule fragment = VK_NULL_HANDLE;
        };

        void Shader::_init()
        {
            TLRENDER_P();

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.vertexSource,
                    shaderc_vertex_shader, // Shader type
                    "vertex_shader.glsl"   // Filename for error reporting
                );

                p.vertex = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                p.vertex = VK_NULL_HANDLE;
            }

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.fragmentSource,
                    shaderc_fragment_shader, // Shader type
                    "frag_shader.glsl"       // Filename for error reporting
                );

                // Assuming you have a VkDevice 'device' already created
                p.fragment = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                p.fragment = VK_NULL_HANDLE;
            }
        }

        Shader::Shader(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        Shader::~Shader()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            if (p.fragment != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.fragment, nullptr);

            if (p.vertex != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.vertex, nullptr);

            if (descriptorSetLayout != VK_NULL_HANDLE)
                vkDestroyDescriptorSetLayout(
                    device, descriptorSetLayout, nullptr);

            if (descriptorPool != VK_NULL_HANDLE)
                vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx, const std::string& vertexSource,
            const std::string& fragmentSource)
        {
            auto out = std::shared_ptr<Shader>(new Shader(ctx));
            out->_p->vertexSource = vertexSource;
            out->_p->fragmentSource = fragmentSource;
            out->_init();
            return out;
        }

        const VkShaderModule& Shader::getVertex() const
        {
            return _p->vertex;
        }

        const VkShaderModule& Shader::getFragment() const
        {
            return _p->fragment;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _p->vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _p->fragmentSource;
        }

        void Shader::bind()
        {
            // glUseProgram(_p->program);
        }

        void Shader::setTexture(
            const std::string& name, VkImageView imageView, VkSampler sampler,
            VkShaderStageFlags stageFlags)
        {
            SamplerBinding sb;
            sb.imageView = imageView;
            sb.sampler = sampler;
            sb.binding = current_binding_index++;

            sb.imageInfo.imageView = imageView;
            sb.imageInfo.sampler = sampler;
            sb.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            sb.layoutBinding.binding = sb.binding;
            sb.layoutBinding.descriptorCount = 1;
            sb.layoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sb.layoutBinding.stageFlags = stageFlags;
            sb.layoutBinding.pImmutableSamplers = nullptr;

            samplers.insert({name, sb});
        }

        const VkDescriptorSet& Shader::getDescriptorSet() const
        {
            return descriptorSet;
        }

        void Shader::createDescriptorSet()
        {
            VkDevice device = ctx.device;

            std::vector<VkDescriptorSetLayoutBinding> bindings;
            std::vector<VkDescriptorPoolSize> poolSizes;

            // UBOs
            for (const auto& [_, ubo] : ubos)
            {
                bindings.push_back(ubo.layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSize.descriptorCount = 1; // or more if you support arrays
                poolSizes.push_back(poolSize);
            }

            // Samplers
            for (const auto& [_, sampler] : samplers)
            {
                bindings.push_back(sampler.layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSize.descriptorCount = 1;
                poolSizes.push_back(poolSize);
            }

            // Create descriptor pool.
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1; // adjust if allocating multiple sets at once

            vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

            // Create descriptor set layout.
            VkDescriptorSetLayoutCreateInfo layout_info = {};
            layout_info.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
            layout_info.pBindings = bindings.data();

            vkCreateDescriptorSetLayout(
                device, &layout_info, nullptr, &descriptorSetLayout);

            // Allocate descriptor set.
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorSetLayout;

            vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

            std::vector<VkWriteDescriptorSet> writes;

            // Allocate descriptor set and update with buffer infos
            for (const auto& [_, ubo] : ubos)
            {
                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = ubo.layoutBinding.binding;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.pBufferInfo = &ubo.bufferInfo;
                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }

            // Samplers
            for (const auto& [_, sampler] : samplers)
            {
                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = sampler.layoutBinding.binding;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;
                write.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &sampler.imageInfo;
                writes.push_back(write);
            }

            vkUpdateDescriptorSets(
                device, static_cast<uint32_t>(writes.size()), writes.data(), 0,
                nullptr);
        }
    } // namespace vk
} // namespace tl
