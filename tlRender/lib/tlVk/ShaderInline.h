
#include <FL/Fl_Vk_Utils.H>

namespace tl
{
    namespace vk
    {

        template <typename T>
        void Shader::createUniform(
            const std::string& name, const T& value,
            const VkShaderStageFlags stageFlags)
        {
            UBO ubo;

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;

            VkBufferCreateInfo ubo_buf_info = {};
            ubo_buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            ubo_buf_info.size = sizeof(value);
            ubo_buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            ubo_buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkMemoryRequirements mem_reqs;
            vkCreateBuffer(device, &ubo_buf_info, nullptr, &ubo.buffer);
            vkGetBufferMemoryRequirements(device, ubo.buffer, &mem_reqs);

            VkMemoryAllocateInfo ubo_alloc_info = {};
            ubo_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            ubo_alloc_info.allocationSize = mem_reqs.size;
            ubo_alloc_info.memoryTypeIndex = findMemoryType(
                gpu, mem_reqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            vkAllocateMemory(device, &ubo_alloc_info, nullptr, &ubo.memory);
            vkBindBufferMemory(device, ubo.buffer, ubo.memory, 0);

            ubo.size = sizeof(value);

            ubo.bufferInfo.buffer = ubo.buffer;
            ubo.bufferInfo.offset = 0;
            ubo.bufferInfo.range = sizeof(T);

            // Default to vertex shader; you can override this later if needed
            ubo.layoutBinding.binding = current_binding_index++;
            ubo.layoutBinding.descriptorCount = 1;
            ubo.layoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo.layoutBinding.stageFlags = stageFlags;
            ubo.layoutBinding.pImmutableSamplers = nullptr;

            ubos.insert({name, ubo});
        }

        template <typename T>
        void Shader::setUniform(
            const std::string& name, const T& value,
            const VkShaderStageFlags stageFlags)
        {
            auto i = ubos.find(name);
            if (i == ubos.end())
            {
                createUniform<T>(name, value, stageFlags);
            }
            else
            {
                VkDevice device = ctx.device;

                void* data;
                vkMapMemory(device, i->second.memory, 0, sizeof(T), 0, &data);
                memcpy(data, &value, sizeof(T));
                vkUnmapMemory(device, i->second.memory);
            }
        }

    } // namespace vk
} // namespace tl
