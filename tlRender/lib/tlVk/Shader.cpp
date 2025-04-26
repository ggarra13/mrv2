// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_Vk_Utils.H>

#include <iostream>
#include <stdexcept>

namespace tl
{
    namespace vlk
    {
        struct Shader::Private
        {
            std::string name;
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
                std::cerr << p.name << " failed compilation " << std::endl
                          << e.what() << " for " << std::endl;
                const auto& lines = tl::string::split(p.vertexSource, '\n',
                                                      string::SplitOptions::KeepEmpty);
                uint32_t i = 1;
                for (const auto& line : lines)
                {
                    std::cerr << i++ << ": " << line << std::endl;
                }
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
                std::cerr << p.name << " failed compilation " << std::endl
                          << e.what() << " for " << std::endl;
                auto lines = tl::string::split(p.fragmentSource, '\n',
                                               string::SplitOptions::KeepEmpty);
                uint32_t i = 1;
                for (const auto& line : lines)
                {
                    std::cerr << i++ << ": " << line << std::endl;
                }
                p.fragment = VK_NULL_HANDLE;
            }
        }

        Shader::Shader(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
            descriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
            descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
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
            {
                vkDestroyDescriptorSetLayout(
                    device, descriptorSetLayout, nullptr);
            }
            
            for (auto& pool : descriptorPools) // Destroy all pools
            {
                 if (pool != VK_NULL_HANDLE)
                 {
                    vkResetDescriptorPool(device, pool, 0);
                    vkDestroyDescriptorPool(device, pool, nullptr);
                 }
            }
            
            for (auto& [_, ubo] : ubos)
            {
                for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) // Destroy buffers and memories for all frames
                {
                    if (ubo.buffers[i] != VK_NULL_HANDLE)
                        vkDestroyBuffer(device, ubo.buffers[i], nullptr);
                    if (ubo.memories[i] != VK_NULL_HANDLE)
                        vkFreeMemory(device, ubo.memories[i], nullptr);
                }
            }
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx, const std::string& vertexSource,
            const std::string& fragmentSource, const std::string& name)
        {
            auto out = std::shared_ptr<Shader>(new Shader(ctx));
            out->_p->vertexSource = vertexSource;
            out->_p->fragmentSource = fragmentSource;
            out->_p->name = name;
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

        const VkDescriptorSet& Shader::getDescriptorSet(int frameIndex) const
        {
            if (frameIndex < 0 || frameIndex >= MAX_FRAMES_IN_FLIGHT)
            {
                throw std::out_of_range("Invalid frame index");
            }
            return descriptorSets[frameIndex];
        }

        const VkDescriptorSetLayout& Shader::getDescriptorSetLayout() const
        {
            return descriptorSetLayout;
        }

        const VkDescriptorPool& Shader::getDescriptorPool(int frameIndex) const
        {
            if (frameIndex < 0 || frameIndex >= MAX_FRAMES_IN_FLIGHT)
            {
                throw std::out_of_range("Invalid frame index");
            }
            return descriptorPools[frameIndex];
        }

        void Shader::bind()
        {
            // glUseProgram(_p->program);
        }

        void Shader::addTexture(
            const std::string& name, const ShaderFlags stageFlags)
        {
            TextureBinding t;
            t.binding = current_binding_index++;
            t.stageFlags = getVulkanShaderFlags(stageFlags);
            textureBindings.insert(std::make_pair(name, t));
        }

        void Shader::setTexture(
            const std::string& name, const std::shared_ptr<Texture>& texture,
            const int frameIndex,
            const ShaderFlags stageFlags)
        {
            if (frameIndex < 0 || frameIndex > MAX_FRAMES_IN_FLIGHT)
            {
                throw std::runtime_error("Invalid frame index in setTexture");
            }
            
            auto it = textureBindings.find(name);
            if (it == textureBindings.end())
            {
                // If texture is added after descriptor sets are created,
                // you might need to recreate descriptor sets or use dynamic descriptors.
                // For simplicity here, we assume textures are added before createDescriptorSets.
                addTexture(name, stageFlags);
                it = textureBindings.find(name); // Find again after adding
                if (it == textureBindings.end()) return; // Should not happen
            }

            VkDevice device = ctx.device;

            uint32_t binding = it->second.binding;

            VkDescriptorImageInfo imageInfo{};
            if (texture)
            {
                imageInfo.imageView = texture->getImageView();
                imageInfo.sampler = texture->getSampler();
                imageInfo.imageLayout = texture->getImageLayout();
            }
            else
            {
                // Provide placeholder/dummy info if the texture is not yet available
                // This depends on your specific use case and whether you can have unbound descriptors.
                // For simplicity, assuming a valid texture is provided when setTexture is called.
                throw std::runtime_error(tl::string::Format("Cannot set texture '{0}': provided texture pointer is null.").arg(name));
            }
            
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[frameIndex];
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }


        void Shader::addFBO(
            const std::string& name, const ShaderFlags stageFlags)
        {
            FBOBinding binding;
            binding.binding = current_binding_index++;
            binding.stageFlags = getVulkanShaderFlags(stageFlags);
            fboBindings.insert(std::make_pair(name, binding));
        }
        
        void Shader::setFBO(
            const std::string& name, const std::shared_ptr<OffscreenBuffer>& fbo,
            const int frameIndex,
            const ShaderFlags stageFlags)
        {
            if (frameIndex < 0 || frameIndex >= MAX_FRAMES_IN_FLIGHT)
            {
                throw std::out_of_range("Invalid frame index in setFBO");
            }
            
            if (!fbo)
            {
                throw std::runtime_error(tl::string::Format("Cannot set FBO '{0}': provided FBO pointer is null.").arg(name));
            }
             
            auto it = fboBindings.find(name);
            if (it == fboBindings.end())
            {
                // If FBO is added after descriptor sets are created,
                // you might need to recreate descriptor sets or use dynamic descriptors.
                // For simplicity here, we assume FBOs are added before createDescriptorSets.
                addFBO(name, stageFlags);
                it = fboBindings.find(name); // Find again after adding
                if (it == fboBindings.end()) return; // Should not happen
            }
            
            VkDevice device = ctx.device;

            uint32_t binding = it->second.binding;
            
            VkDescriptorImageInfo imageInfo{};
            imageInfo.sampler = fbo->getSampler();
            imageInfo.imageView = fbo->getImageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[frameIndex]; // previously created set
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
        
        void Shader::createDescriptorSets()
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
                poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT; // or more if you support arrays
                poolSizes.push_back(poolSize);
            }
            
            // Textures
            for (const auto& [_, texture] : textureBindings)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};

                layoutBinding.binding = texture.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.stageFlags = texture.stageFlags;
                layoutBinding.pImmutableSamplers = nullptr;

                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                // One texture descriptor per frame per binding
                poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT; 
                poolSizes.push_back(poolSize);
            }
            
            // FBOs
            for (const auto& [_, element] : fboBindings)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};

                layoutBinding.binding = element.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.stageFlags = element.stageFlags;
                layoutBinding.pImmutableSamplers = nullptr;

                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                // One FBO descriptor per frame per binding                
                poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT; 
                poolSizes.push_back(poolSize);
            }

            // Create descriptor pool. (One pool per frame, or a larger single pool)
            // Using a pool per frame allows for easier management and less risk of pool fragmentation,
            // especially if descriptor counts vary significantly between frames, although less common for shaders.
            // A single large pool is also a valid strategy. Sticking to pool per frame as discussed.
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            // maxSets should be 1 if allocating 1 set per pool.
            // was: poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT; // Max sets is now number of frames in flight
            poolInfo.maxSets = 1;
            
            // Create a descriptor pool for each frame
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPools[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create descriptor pool!");
                }
            }

            // Create descriptor set layout. (Layout can be shared across frames)
            VkDescriptorSetLayoutCreateInfo layout_info = {};
            layout_info.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
            layout_info.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
            
            // Allocate descriptor sets for each frame, from their respective pools
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorSetCount = 1; // Allocating one set at a time
            allocInfo.pSetLayouts = layouts.data(); // Still use the same layout

            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                 allocInfo.descriptorPool = descriptorPools[i]; // Allocate from the pool for this frame
                 if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[i]) != VK_SUCCESS)
                 {
                      throw std::runtime_error("failed to allocate descriptor set for frame " + std::to_string(i) + "!");
                 }
            }

            // Initial Update of descriptor sets for UBOs
            // Textures and FBOs will be updated by setTexture/setFBO when the resources are ready.
            // An initial write is often required to transition the descriptor set to a valid state.
            // For image descriptors, you could potentially write a dummy image view/sampler if unbound descriptors
            // are not supported or you need to avoid validation errors. However, the simplest is to ensure
            // setTexture/setFBO is called for each frame's descriptor set before that set is first bound in a command buffer.

            // Update descriptor sets for each frame
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::vector<VkWriteDescriptorSet> writes;

                // Allocate descriptor set and update with buffer infos
                for (const auto& [_, ubo] : ubos)
                {
                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptorSets[i];
                    write.dstBinding = ubo.layoutBinding.binding;
                    write.dstArrayElement = 0;
                    write.descriptorCount = 1;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    write.pBufferInfo = &ubo.bufferInfos[i];
                    writes.push_back(write);
                }

                if (!writes.empty())
                {
                    vkUpdateDescriptorSets(
                        device, static_cast<uint32_t>(writes.size()), writes.data(), 0,
                        nullptr);
                }
            }
        }

        void Shader::debugVertexDescriptorSets()
        {
            
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.layoutBinding.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding << " = UNIFORM_BUFFER" << std::endl;
            }
            
            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
            
            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                if ((fbo.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
        }

        void Shader::debugFragmentDescriptorSets()
        {   
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.layoutBinding.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding << " = UNIFORM_BUFFER" << std::endl;
            }
            
            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
            
            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                if ((fbo.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
        }

        void Shader::debugDescriptorSets()
        {
            if (ubos.empty() && textureBindings.empty() && fboBindings.empty())
            {
                std::cerr << "Shader (" << this << "): has NO bindings" << std::endl;
                return;
            }
            else
            {
                std::cerr << "Shader (" << this << "): has these bindings" << std::endl;
            }
            
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding << " = UNIFORM_BUFFER" << std::endl;
                std::cerr << "\t        " << (ubo.layoutBinding.stageFlags & VK_SHADER_STAGE_VERTEX_BIT ? "vertex" : " ")
                          << (ubo.layoutBinding.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT ? "fragment" : " ")
                          << std::endl;
            }
            
            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
                std::cerr << "\t        " << (texture.stageFlags & VK_SHADER_STAGE_VERTEX_BIT ? "vertex" : " ")
                          << (texture.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT ? "fragment" : " ")
                          << std::endl;
            }
            
            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding << " = COMBINED_IMAGE_SAMPLER" << std::endl;
                std::cerr << "\t        " << (fbo.stageFlags & VK_SHADER_STAGE_VERTEX_BIT ? "vertex" : " ")
                          << (fbo.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT ? "fragment" : " ")
                          << std::endl;
            }
        }

        void Shader::debugPointers()
        {
            std::cerr << "--- Descriptor Sets ---" << std::endl;
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::cerr << "Frame " << i << ": " << descriptorSets[i] << std::endl;
            }
            
            std::cerr << "--- Descriptor Pools ---" << std::endl;
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::cerr << "Frame " << i << " Pool: " << descriptorPools[i] << std::endl;
            }
        }

        void Shader::debug()
        {
            std::cerr << "====================================" << std::endl;
            std::cerr << getVertexSource() << std::endl;
            debugVertexDescriptorSets();
            std::cerr << "------------------------------------" << std::endl;
            std::cerr << getFragmentSource() << std::endl;
            std::cerr << "------------------------------------" << std::endl;
            debugFragmentDescriptorSets();
            // debugPointers();
        }

        
    } // namespace vlk
} // namespace tl
