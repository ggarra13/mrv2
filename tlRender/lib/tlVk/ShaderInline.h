
#include <FL/Fl_Vk_Utils.H>

namespace tl
{
    namespace vlk
    {

        template <typename T>
        void Shader::createUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            auto it = ubos.find(name);
            if (it != ubos.end())
            {
                throw std::runtime_error(
                    name + " for shader " + shaderName + " already created.");
            }

            UBOBinding ubo;
            ubo.size = sizeof(value);

            ubo.layoutBinding.binding = current_binding_index++;
            ubo.layoutBinding.descriptorCount = 1;
            ubo.layoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo.layoutBinding.stageFlags = getVulkanShaderFlags(stageFlags);
            ubo.layoutBinding.pImmutableSamplers = nullptr;

            ubos[name] = ubo;
        }

        template <typename T>
        void Shader::setUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + name);
            activeBindingSet->updateUniform(name, &value, sizeof(value), frameIndex);
            
            // void* data;
            // vkMapMemory(
            //     device, ubo.memories[frameIndex], 0, sizeof(T), 0, &data);
            // memcpy(data, &value, sizeof(T));
            // vkUnmapMemory(device, ubo.memories[frameIndex]);

            // We need to update the bufferInfo in the descriptor set for the
            // current frame Alternatively, you could use dynamic uniform
            // buffers.

            // // For this approach, we re-write the descriptor set for this
            // // binding and frame
            // VkDescriptorBufferInfo bufferInfo =
            //     ubo.infos[frameIndex]; // Use the buffer info for this
            //                                  // frame

            // VkWriteDescriptorSet write{};
            // write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            // write.dstSet =
            //     descriptorSets[frameIndex]; // Update the set for this frame
            // write.dstBinding = ubo.layoutBinding.binding;
            // write.dstArrayElement = 0;
            // write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            // write.descriptorCount = 1;
            // write.pBufferInfo = &bufferInfo;

            // vkUpdateDescriptorSets(ctx.device, 1, &write, 0, nullptr);
        }

        template <typename T>
        void Shader::addPush(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            pushSize = sizeof(T);
            pushStageFlags = getVulkanShaderFlags(stageFlags);
        }

    } // namespace vlk
} // namespace tl
