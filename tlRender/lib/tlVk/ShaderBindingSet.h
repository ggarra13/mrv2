// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlVk/Buffer.h>
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
                std::size_t size = 0;
                std::vector<VmaAllocation> allocation;
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

            struct StorageBufferParameter {
                std::vector<std::shared_ptr<vlk::Buffer> > buffers;
                uint32_t binding;
                VkShaderStageFlags stageFlags;

                // Track the current allocated size per frame to
                // avoid unnecessary reallocation.
                std::vector<VkDeviceSize> currentSizes;
            };
            std::map<std::string, StorageBufferParameter> storageBuffers;

            struct StorageImageParameter {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
            };
            std::map<std::string, StorageImageParameter> storageImages;
            
            VkDevice device;
            VmaAllocator allocator;
            
            ShaderBindingSet(VkDevice device, VmaAllocator vmaAllocator) :
                device(device),
                allocator(vmaAllocator)
                {
                    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                    descriptorPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
                }

            ~ShaderBindingSet()
                {
                    destroy();
                }

            void updateStorageBuffer(
                Fl_Vk_Context& ctx,
                const std::string& name,
                const uint8_t* data,
                const std::size_t size,
                const uint32_t frameIndex)
                {
                    auto it = storageBuffers.find(name); // You'll need to define this map in ShaderBindingSet
                    if (it == storageBuffers.end()) throw std::runtime_error("Storage Buffer not found: " + name);

                    auto& param = it->second;

                    // 1. Check if we need to (re)allocate
                    if (!param.buffers[frameIndex] ||
                        param.currentSizes[frameIndex] < size)
                    {
                        // Recreate the buffer with the new, larger size
                        param.buffers[frameIndex] = vlk::Buffer::create(ctx, size);
                        param.currentSizes[frameIndex] = size;

                        // 2. CRITICAL: Because the VkBuffer handle changed, 
                        // we MUST update the Descriptor Set for this frame.
                        VkDescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = param.buffers[frameIndex]->getBuffer();
                        bufferInfo.offset = 0;
                        bufferInfo.range  = size; // Or VK_WHOLE_SIZE

                        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                        write.dstSet = descriptorSets[frameIndex];
                        write.dstBinding = param.binding;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        write.descriptorCount = 1;
                        write.pBufferInfo = &bufferInfo;

                        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
                    }

                    // 3. Upload the data (this handles the map/memcpy/unmap internally)
                    param.buffers[frameIndex]->upload(data);
                }
                                   
            void updateStorageImage(
                const std::string& name,
                VkDescriptorSet descriptorSet,
                const std::shared_ptr<Texture>& texture)
                {
                    auto it = storageImages.find(name);
                    if (it == storageImages.end()) throw std::runtime_error("Storage Image not found: " + name +
                                                                            " storeImages.size()=" + std::to_string(storageImages.size()));

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageView   = texture->getImageView();
                    // Storage images do not use samplers in the shader (imageStore/imageLoad)
                    imageInfo.sampler     = VK_NULL_HANDLE; 
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; 

                    VkWriteDescriptorSet write{};
                    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet          = descriptorSet;
                    write.dstBinding      = it->second.binding;
                    write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    write.descriptorCount = 1;
                    write.pImageInfo      = &imageInfo;

                    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
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

                void* mapped;
                vmaMapMemory(
                    allocator, it->second.allocation[frameIndex], &mapped);
                memcpy(mapped, data, size);
                vmaUnmapMemory(allocator, it->second.allocation[frameIndex]);
                
                // We need to update the bufferInfo in the descriptor set for the
                // current frame.  Alternatively, you could use dynamic uniform
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
                            if (ubo.buffers[i] != VK_NULL_HANDLE &&
                                ubo.allocation[i] != VK_NULL_HANDLE)
                                vmaDestroyBuffer(allocator, ubo.buffers[i],
                                                 ubo.allocation[i]);
                        }
                    }
                    
                    for (auto& [_, sb] : storageBuffers)
                    {
                        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                        {
                            sb.buffers[i].reset();
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
                    storageBuffers.clear();
                    storageImages.clear();
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
