
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

            // VMA specific handles
            VmaAllocation allocation = VK_NULL_HANDLE;

            void* mappedPtr = nullptr;
        };

        Buffer::~Buffer()
        {
            
            TLRENDER_P();

            VkDevice device = ctx.device;
  
            if (p.buffer != VK_NULL_HANDLE && p.allocation != VK_NULL_HANDLE)
            {
                vmaDestroyBuffer(ctx.allocator, p.buffer, p.allocation);
            }
        }

        void Buffer::createBuffer(VkDeviceSize bufferSize)
        {
            TLRENDER_P();
            
            VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = p.bufferSize = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            // VMA Setup
            VmaAllocationCreateInfo allocCreateInfo = {};
            
            // VMA_MEMORY_USAGE_CPU_TO_GPU guarantees HOST_VISIBLE and HOST_COHERENT where available.
            // On macOS M1 (Unified Memory), this handles the shared memory selection correctly.
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            
            // PERISISTENT MAPPING:
            // We ask VMA to map the memory immediately and keep it mapped.
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo allocInfo = {};
            
            if (vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocCreateInfo, &p.buffer,
                                &p.allocation, &allocInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create vertex buffer with VMA");
            }

            // Retrieve the persistent pointer provided by VMA
            p.mappedPtr = allocInfo.pMappedData;
            
            if (!p.mappedPtr)
            {
                // In rare cases (e.g. if memory was not HOST_VISIBLE), mapping might fail, 
                // but CPU_TO_GPU usage ensures it should be mappable.
                throw std::runtime_error("Failed to get mapped pointer from VMA");
            }
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

            if (!p.mappedPtr)
            {
                throw std::runtime_error("Attempting to upload to an unmapped buffer");
            }

            const VkDeviceSize size = p.bufferSize;
            
            // 1. Copy data to the persistently mapped pointer
            std::memcpy(p.mappedPtr, data, size);

            // 2. Flush the cache
            // Even if memory is Coherent (common on M1), flushing explicitly via VMA 
            // is safer for MoltenVK wrapper consistency across different Mac hardware.
            //
            // NOTE: The previous code called vmaUnmapMemory here. That was a bug.
            // We must NOT unmap if we used VMA_ALLOCATION_CREATE_MAPPED_BIT.
            
            VkResult result = vmaFlushAllocation(ctx.allocator, p.allocation, 0, size);
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("vmaFlushAllocation failed");
            }
        }
        
    } // namespace vlk
} // namespace tl
