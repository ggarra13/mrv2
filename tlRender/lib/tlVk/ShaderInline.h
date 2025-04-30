
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

            UBO ubo;
            ubo.size = sizeof(value);

            // Create buffer and memory for each frame (You need helper
            // functions here)
            ubo.buffers.resize(MAX_FRAMES_IN_FLIGHT);
            ubo.memories.resize(MAX_FRAMES_IN_FLIGHT);
            ubo.bufferInfos.resize(MAX_FRAMES_IN_FLIGHT);

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;

            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                VkBufferCreateInfo ubo_buf_info = {};
                ubo_buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                ubo_buf_info.size = sizeof(value);
                ubo_buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                ubo_buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VkMemoryRequirements mem_reqs;
                vkCreateBuffer(device, &ubo_buf_info, nullptr, &ubo.buffers[i]);

                vkGetBufferMemoryRequirements(
                    device, ubo.buffers[i], &mem_reqs);

                VkMemoryAllocateInfo ubo_alloc_info = {};
                ubo_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                ubo_alloc_info.allocationSize = mem_reqs.size;
                ubo_alloc_info.memoryTypeIndex = findMemoryType(
                    gpu, mem_reqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                vkAllocateMemory(
                    device, &ubo_alloc_info, nullptr, &ubo.memories[i]);
                vkBindBufferMemory(device, ubo.buffers[i], ubo.memories[i], 0);

                ubo.bufferInfos[i].buffer = ubo.buffers[i];
                ubo.bufferInfos[i].offset = 0;
                ubo.bufferInfos[i].range = sizeof(T);
            }

            ubo.layoutBinding.binding = current_binding_index++;
            ubo.layoutBinding.descriptorCount = 1;
            ubo.layoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo.layoutBinding.stageFlags = getVulkanShaderFlags(stageFlags);
            ubo.layoutBinding.pImmutableSamplers = nullptr;

            ubos.insert({name, ubo});
        }

        template <typename T>
        void Shader::setUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            auto it = ubos.find(name);
            if (it == ubos.end())
            {
                throw std::runtime_error(
                    shaderName + ": parameter '" + name + "' was not created");
            }

            VkDevice device = ctx.device;

            const UBO& ubo = it->second;
            if (ubo.size != sizeof(value))
            {
                throw std::runtime_error(
                    shaderName + ": parameter '" + name +
                    "' passed is different size than created ");
            }

            void* data;
            vkMapMemory(
                device, ubo.memories[frameIndex], 0, sizeof(T), 0, &data);
            memcpy(data, &value, sizeof(T));
            vkUnmapMemory(device, ubo.memories[frameIndex]);

            // We need to update the bufferInfo in the descriptor set for the
            // current frame Alternatively, you could use dynamic uniform
            // buffers.

            // For this approach, we re-write the descriptor set for this
            // binding and frame
            VkDescriptorBufferInfo bufferInfo =
                ubo.bufferInfos[frameIndex]; // Use the buffer info for this
                                             // frame

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet =
                descriptorSets[frameIndex]; // Update the set for this frame
            write.dstBinding = ubo.layoutBinding.binding;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(ctx.device, 1, &write, 0, nullptr);
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
