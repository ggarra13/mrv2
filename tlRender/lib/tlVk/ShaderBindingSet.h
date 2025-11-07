// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlVk/Texture.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Vk.h>

#include <map>
#include <vector>
#include <memory>
#include <string>

namespace tl
{
    namespace vlk
    {
        //! All allocation of these structures happen in
        //! Shader::createBingingSet()
        struct ShaderBindingSet
        {
            std::string shaderName;
            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<VkDescriptorPool> descriptorPools;

            struct UniformParameter
            {
                std::vector<VkBuffer> buffers;
                std::vector<VkDescriptorBufferInfo> infos;
                VkDescriptorSetLayoutBinding layoutBinding;
                size_t size = 0;
#ifndef MRV2_NO_VMA
                std::vector<VmaAllocation> allocation;
#else
                std::vector<VkDeviceMemory> memories;
#endif
            };
            std::map<std::string, UniformParameter> uniforms;

            struct TextureParameter
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
            };
            std::map<std::string, TextureParameter> textures;

            struct FBOParameter
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
            };
            std::map<std::string, FBOParameter> fbos;

#ifdef MRV2_NO_VMA
            VkDevice device;
            
            ShaderBindingSet(VkDevice device) :
                device(device)
                {
                    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                    descriptorPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                }
#else
            VkDevice device;
            VmaAllocator allocator;
            
            ShaderBindingSet(VkDevice device, VmaAllocator vmaAllocator) :
                device(device),
                allocator(vmaAllocator)
                {
                    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                    descriptorPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                }
#endif

            ~ShaderBindingSet()
                {
                    destroy();
                }

            void updateUniform(
                const std::string& name, const void* data,
                size_t size, size_t frameIndex)
            {
                auto it = uniforms.find(name);
                if (it == uniforms.end())
                {
                    throw std::runtime_error("Uniform Parameter not found: " + name);
                }
                if (it->second.size != size)
                    throw std::runtime_error("Uniform size mismatch");

#ifdef MRV2_NO_VMA
                void* mapped;
                vkMapMemory(
                    device, it->second.memories[frameIndex], 0, size, 0,
                    &mapped);
                memcpy(mapped, data, size);
                vkUnmapMemory(device, it->second.memories[frameIndex]);
#else
                void* mapped;
                vmaMapMemory(
                    allocator, it->second.allocation[frameIndex], &mapped);
                memcpy(mapped, data, size);
                vmaUnmapMemory(allocator, it->second.allocation[frameIndex]);
#endif
                
                // We need to update the bufferInfo in the descriptor set for the
                // current frame Alternatively, you could use dynamic uniform
                // buffers.

                auto descriptorSet = descriptorSets[frameIndex]; // Update the set for this frame
                // For this approach, we re-write the descriptor set for this
                // binding and frame
                VkDescriptorBufferInfo bufferInfo = it->second.infos[frameIndex];
                
                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = it->second.layoutBinding.binding;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &bufferInfo;
                
                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }

            void updateTexture(
                const std::string& name,
                VkDescriptorSet descriptorSet,
                const std::shared_ptr<Texture>& texture)
            {
                auto it = textures.find(name);
                if (it == textures.end())
                    throw std::runtime_error(
                        "Texture binding not found: " + name);
                if (!texture)
                    throw std::runtime_error("Null texture for: " + name);

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageView = texture->getImageView();
                imageInfo.sampler = texture->getSampler();
                imageInfo.imageLayout = texture->getImageLayout();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = it->second.binding;
                write.dstArrayElement = 0;
                write.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }

            void updateFBO(const std::string& name,
                VkDescriptorSet descriptorSet,
                const std::shared_ptr<OffscreenBuffer>& fbo)
            {
                auto it = fbos.find(name);
                if (it == fbos.end())
                    throw std::runtime_error("FBO binding not found: " + name);
                if (!fbo)
                    throw std::runtime_error("Null texture for: " + name);

                uint32_t binding = it->second.binding;

                VkDescriptorImageInfo imageInfo{};
                imageInfo.sampler = fbo->getSampler();
                imageInfo.imageView = fbo->getImageView();
                imageInfo.imageLayout =
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = binding;
                write.dstArrayElement = 0;
                write.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }
            
            void destroy()
                {
                    for (auto& [_, ubo] : uniforms)
                    {
                        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                        {
#ifdef MRV2_NO_VMA
                            if (ubo.buffers[i] != VK_NULL_HANDLE)
                                vkDestroyBuffer(device,
                                                ubo.buffers[i],
                                                nullptr);
                            if (ubo.memories[i] != VK_NULL_HANDLE)
                                vkFreeMemory(device,
                                             ubo.memories[i],
                                             nullptr);
#else
                            if (ubo.buffers[i] != VK_NULL_HANDLE &&
                                ubo.allocation[i] != VK_NULL_HANDLE)
                                vmaDestroyBuffer(allocator, ubo.buffers[i],
                                                 ubo.allocation[i]);
#endif
                        }
                    }

                    for (auto& pool : descriptorPools)
                    {
                        if (pool != VK_NULL_HANDLE)
                            vkDestroyDescriptorPool(device, pool, nullptr);
                    }

                    descriptorSets.clear();
                    descriptorPools.clear();
                    
                    uniforms.clear();
                    textures.clear();
                    fbos.clear();
                }
            
            VkDescriptorPool getDescriptorPool(size_t frameIndex) const
            {
                return descriptorPools.at(frameIndex);
            }
            
            VkDescriptorSet getDescriptorSet(size_t frameIndex) const
            {
                return descriptorSets.at(frameIndex);
            }
        };

    } // namespace vlk
} // namespace tl
