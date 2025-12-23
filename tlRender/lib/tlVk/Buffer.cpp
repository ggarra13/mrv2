
#include <tlVk/Buffer.h>
#include <tlVk/Vk.h>

#include <FL/Fl_Vk_Utils.H>

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

            void* mappedPtr = nullptr;
        };

        Buffer::~Buffer()
        {
            
            TLRENDER_P();

            VkDevice device = ctx.device;

#ifdef MRV2_NO_VMA
            if (p.buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(device, p.buffer, nullptr);
            }
            
            if (p.bufferMemory != VK_NULL_HANDLE)
            {
                vkFreeMemory(device, p.bufferMemory, nullptr);
            }
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
            bufferInfo.size = p.bufferSize = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#ifdef MRV2_NO_VMA
            if (vkCreateBuffer(device, &bufferInfo, nullptr, &p.buffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create persistent vertex buffer");

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, p.buffer, &memRequirements);
            
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                gpu, memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &p.bufferMemory) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate persistent buffer memory");

            vkBindBufferMemory(device, p.buffer, p.bufferMemory, 0);
            
            // Persistently map
            if (vkMapMemory(device, p.bufferMemory, 0, memRequirements.size, 0,
                            &p.mappedPtr) != VK_SUCCESS)
                throw std::runtime_error("Failed to map persistent mapped ptr");
#else
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // Maps to Host Visible + Coherent
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; // Persistent mapping

            VmaAllocationInfo allocInfo = {};
            if (vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocCreateInfo, &p.buffer,
                                &p.allocation, &allocInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create vertex buffer with VMA");
            }

            p.mappedPtr = allocInfo.pMappedData;
            if (!p.mappedPtr)
            {
                throw std::runtime_error("Failed to get mapped pointer from VMA");
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
            
            // Host-visible upload (like glTexSubImage2D)
            std::memcpy(p.mappedPtr, data, size);
#ifdef MRV2_NO_VMA
            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = p.bufferMemory;
            range.offset = 0;
            range.size = size;

            VkResult result = vkFlushMappedMemoryRanges(device, 1, &range);
            if (result != VK_SUCCESS)
                throw std::runtime_error("vkFlushMappedMemoryRanges failed on macOS");
#else
            vmaUnmapMemory(ctx.allocator, p.allocation);
            if (vmaFlushAllocation(ctx.allocator, p.allocation, 0, size) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("vmaFlushAllocation failed");
            }
#endif
        }
        
    } // namespace vlk
} // namespace tl
