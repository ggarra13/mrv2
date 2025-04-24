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

            for (auto& [_, ubo] : ubos)
            {
                vkDestroyBuffer(device, ubo.buffer, nullptr);
                vkFreeMemory(device, ubo.memory, nullptr);
            }
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

        const VkDescriptorSet& Shader::getDescriptorSet() const
        {
            return descriptorSet;
        }

        const VkDescriptorSetLayout& Shader::getDescriptorSetLayout() const
        {
            return descriptorSetLayout;
        }

        const VkDescriptorPool& Shader::getDescriptorPool() const
        {
            return descriptorPool;
        }

        void Shader::bind()
        {
            // glUseProgram(_p->program);
        }

        void Shader::addTexture(
            const std::string& name, const VkShaderStageFlags stageFlags)
        {
            TextureBinding t;
            t.binding = current_binding_index++;
            t.stageFlags = stageFlags;
            textureBindings.insert(std::make_pair(name, t));
        }

        void Shader::setTexture(
            const std::string& name, const std::shared_ptr<Texture>& texture)
        {

            auto it = textureBindings.find(name);
            if (it == textureBindings.end())
            {
                std::string err =
                    "Could not find " + name + " in texture bindings";
                throw std::runtime_error(err);
            }

            VkDevice device = ctx.device;

            uint32_t binding = it->second.binding;

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = texture->getImageView();
            imageInfo.sampler = texture->getSampler();
            imageInfo.imageLayout = texture->getImageLayout();

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet; // previously created set
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
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
                VkDescriptorPoolSize poolSize = {};
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSize.descriptorCount = 1; // or more if you support arrays
                poolSizes.push_back(poolSize);
            }

            // Samplers

            for (const auto& [_, texture] : textureBindings)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};

                layoutBinding.binding = texture.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                layoutBinding.pImmutableSamplers = nullptr;

                bindings.push_back(layoutBinding);

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
                writes.push_back(write);
            }

            for (const auto& [_, texBinding] : textureBindings)
            {
                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = texBinding.binding;
                write.dstArrayElement = 0;
                write.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = 1;

                VkDescriptorImageInfo imageInfo =
                    texBinding.texture->getDescriptorInfo();
                write.pImageInfo = &imageInfo;

                writes.push_back(write);
            }

            vkUpdateDescriptorSets(
                device, static_cast<uint32_t>(writes.size()), writes.data(), 0,
                nullptr);
        }
    } // namespace vk
} // namespace tl
