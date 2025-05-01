#pragma once

#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include <memory>
#include <string>

namespace tl
{
    namespace vlk
    {

        struct ShaderBindingSet
        {
            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<VkDescriptorPool> descriptorPools;

            struct Uniform
            {
                std::vector<VkBuffer> buffers;
                std::vector<VkDeviceMemory> memories;
                std::vector<VkDescriptorBufferInfo> infos;
                VkDescriptorSetLayoutBinding layoutBinding;
                size_t size = 0;
            };
            std::map<std::string, Uniform> uniforms;

            struct Texture
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
            };
            std::map<std::string, Texture> textures;

            struct FBO
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
            };
            std::map<std::string, FBO> fbos;

            void updateUniform(
                VkDevice device, const std::string& name, const void* data,
                size_t size, size_t frameIndex)
            {
                auto it = uniforms.find(name);
                if (it == uniforms.end())
                    throw std::runtime_error("Uniform not found: " + name);
                if (it->second.size != size)
                    throw std::runtime_error("Uniform size mismatch");

                void* mapped;
                vkMapMemory(
                    device, it->second.memories[frameIndex], 0, size, 0,
                    &mapped);
                memcpy(mapped, data, size);
                vkUnmapMemory(device, it->second.memories[frameIndex]);
            }

            void updateTexture(
                VkDevice device, const std::string& name,
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
                texture->transition();
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

            void updateFBO(
                VkDevice device, const std::string& name,
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

            VkDescriptorSet get(size_t frameIndex) const
            {
                return descriptorSets.at(frameIndex);
            }
        };

    } // namespace vlk
} // namespace tl
