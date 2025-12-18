#pragma once

#include <tlVk/Vk.h>

#include <tlCore/Util.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace vlk
    {
        class Buffer : public std::enable_shared_from_this<Buffer>
        {
            TLRENDER_NON_COPYABLE(Buffer);

        protected:
            void _init(VkDeviceSize bufferSize);
            Buffer(Fl_Vk_Context& ctx);

        public:
            ~Buffer();
            
            //! Create a new object.
            static std::shared_ptr<Buffer> create(Fl_Vk_Context& ctx, VkDeviceSize bufferSize);

            //!  Get the buffer.
            VkBuffer getBuffer() const;

            VkDeviceSize getBufferSize();

            void upload(const uint8_t* data);
            
        private:
            Fl_Vk_Context& ctx;

            void createBuffer(VkDeviceSize);
            void allocateMemory();
            
            TLRENDER_PRIVATE();
        };
    }
}
