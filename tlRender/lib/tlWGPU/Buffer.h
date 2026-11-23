#pragma once

#include <tlWGPU/WGPU.h>

#include <tlCore/Util.h>

#include <cstdint>
#include <memory>

namespace tl
{
    namespace wgpu
    {
        class Buffer : public std::enable_shared_from_this<Buffer>
        {
            TLRENDER_NON_COPYABLE(Buffer);

        protected:
            void _init(uint64_t bufferSize);
            Buffer(Fl_WebGPU_Context&);

        public:
            ~Buffer();

            //! Create a new object.
            static std::shared_ptr<Buffer> create(Fl_WebGPU_Context&,
                                                  uint64_t bufferSize);

            //! Get the WebGPU buffer.
            const WGPUBuffer& getBuffer() const;

            uint64_t getBufferSize() const;

            void upload(const uint8_t* data);

        private:
            void createBuffer(uint64_t bufferSize);

            TLRENDER_PRIVATE();
        };
    } // namespace wgpu_backend
} // namespace tl
