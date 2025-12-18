
#include <tlVk/Buffer.h>
#include <tlVk/Vk.h>

#include <memory>
#include <stdexcept>
#include <cstring>

namespace tl
{
    namespace vlk
    {
        struct Buffer::Private
        {
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceSize bufferSize = 0;
            
#ifdef MRV2_NO_VMA
            VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
#else
            VmaAllocation allocation = VK_NULL_HANDLE;
#endif
        };

        Buffer::~Buffer()
        {
            
            TLRENDER_P();

            VkDevice device = ctx.device;

#ifdef MRV2_NO_VMA
            if (p.buffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, p.buffer, nullptr);
#else            
            if (p.buffer != VK_NULL_HANDLE && p.allocation != VK_NULL_HANDLE)
            {
                vmaDestroyBuffer(ctx.allocator, p.buffer, p.allocation);
            }
#endif
        }

        void Buffer::createBuffer(VkDeviceSize bufferSize)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;
            
            VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#ifdef MRV2_NO_VMA
            if (vkCreateBuffer(device, &bufferInfo, nullptr, &p.buffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create persistent vertex buffer");

#else
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // Maps to Host Visible + Coherent

            VmaAllocationInfo allocInfo = {};
            if (vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocCreateInfo, &p.buffer,
                                &p.allocation, &allocInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create vertex buffer with VMA");
            }
#endif
        }
        
        void Buffer::_init(VkDeviceSize bufferSize)
        {
            createBuffer(bufferSize);
        }
        
        Buffer::Buffer(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }
        
        std::shared_ptr<Buffer> Buffer::create(Fl_Vk_Context& context, VkDeviceSize bufferSize)
        {
            auto out = std::shared_ptr<Buffer>(new Buffer(context));
            out->_init(bufferSize);
            return out;
        }
        
        VkBuffer Buffer::getBuffer() const { return _p->buffer; }

        VkDeviceSize Buffer::getBufferSize() { return _p->bufferSize; }
        
        void Buffer::upload(const uint8_t* data)
        {
            TLRENDER_P();
            VkDevice device = ctx.device;
            const VkDeviceSize size = p.bufferSize;
            
            void* mapped = nullptr;
#ifdef MRV2_NO_VMA
            // Host-visible upload (like glTexSubImage2D)
            VK_CHECK(vkMapMemory(device, p.memory, 0, size, 0, &mapped));
            std::memcpy(mapped, data, size);
            vkUnmapMemory(device, p.memory);
#else
            // Host-visible upload (like glTexSubImage2D)
            VK_CHECK(vmaMapMemory(ctx.allocator, p.allocation, &mapped));
            std::memcpy(mapped, data, size);
            vmaUnmapMemory(ctx.allocator, p.allocation);
            vmaFlushAllocation(ctx.allocator, p.allocation, 0, VK_WHOLE_SIZE); // Ensure flush
#endif
        }
        
    } // namespace vlk
} // namespace tl
