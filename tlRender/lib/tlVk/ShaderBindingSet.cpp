
#include <tlVk/ShaderBindingSet.h>

namespace tl
{
    namespace vlk
    {

        ShaderBindingSet::ShaderBindingSet(VkDevice device) :
            device(device)
        {
            descriptorSets.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
            descriptorPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
        }
        
        ShaderBindingSet::~ShaderBindingSet()
        {
            destroy();
        }

        void ShaderBindingSet::updateUniform(
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
            vkMapMemory(
                device, it->second.memories[frameIndex], 0, size, 0,
                &mapped);
            memcpy(mapped, data, size);
            vkUnmapMemory(device, it->second.memories[frameIndex]);
                
            // We need to update the bufferInfo in the descriptor set for the
            // current frame. Alternatively, you could use dynamic uniform
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

            std::cerr << ">>>>>>>>>>>>> updated " << descriptorSet << std::endl;
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }

        void ShaderBindingSet::updateTexture(
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

            std::cerr << ">>>>>>>>>>>>> updated " << descriptorSet << std::endl;
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }

        void ShaderBindingSet::updateFBO(const std::string& name,
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

            std::cerr << ">>>>>>>>>>>>> updated " << descriptorSet << std::endl;
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
            
        void ShaderBindingSet::destroy()
        {
            std::cerr << __PRETTY_FUNCTION__ << " uniforms" << std::endl;
            for (auto& [_, ubo] : uniforms)
            {
                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                {
                    if (ubo.buffers[i] != VK_NULL_HANDLE)
                    {
                        std::cerr << "\tdestroy " << ubo.buffers[i] << std::endl;
                        vkDestroyBuffer(device,
                                        ubo.buffers[i],
                                        nullptr);
                        ubo.buffers[i] = VK_NULL_HANDLE;
                    }
                    if (ubo.memories[i] != VK_NULL_HANDLE)
                    {
                        vkFreeMemory(device,
                                     ubo.memories[i],
                                     nullptr);
                        ubo.memories[i] = VK_NULL_HANDLE;
                    }
                }
            }

            std::cerr << __PRETTY_FUNCTION__ << " descriptorPools" << std::endl;
            for (auto& pool : descriptorPools)
            {
                if (pool != VK_NULL_HANDLE)
                {
                    vkDestroyDescriptorPool(device, pool, nullptr);
                    pool = VK_NULL_HANDLE;
                }
            }

            descriptorSets.clear();
            descriptorPools.clear();
                    
            uniforms.clear();
            textures.clear();
            fbos.clear();
        }            

    } // namespace vlk
} // namespace tl
