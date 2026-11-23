#include <tlWGPU/Buffer.h>

#include <stdexcept>

namespace tl
{
    namespace wgpu
    {
        struct Buffer::Private
        {
            WGPUDevice device = nullptr;
            WGPUQueue  queue  = nullptr;

            WGPUBuffer buffer = nullptr;
            uint64_t bufferSize = 0;
        };

        Buffer::~Buffer()
        {
            TLRENDER_P();

            if (p.buffer)
            {
                wgpuBufferDestroy(p.buffer);
                wgpuBufferRelease(p.buffer);
                p.buffer = nullptr;
            }
        }

        void Buffer::createBuffer(uint64_t bufferSize)
        {
            TLRENDER_P();

            if (p.buffer)
            {
                wgpuBufferDestroy(p.buffer);
                wgpuBufferRelease(p.buffer);
                p.buffer = nullptr;
            }

            if (!p.device)
            {
                throw std::runtime_error("Invalid WebGPU device");
            }

            // Note: WebGPU buffer sizes for many operations/bindings are typically
            // multiples of 4 bytes; ensure caller-provided bufferSize satisfies alignment.
            WGPUBufferDescriptor bufferDesc = {};
            bufferDesc.label = { "tl::wgpu::Buffer", WGPU_STRLEN };
            bufferDesc.size = bufferSize;
            bufferDesc.usage = WGPUBufferUsage_Storage |
                               WGPUBufferUsage_CopyDst;
            bufferDesc.mappedAtCreation = false;

            p.buffer = wgpuDeviceCreateBuffer(p.device, &bufferDesc);
            if (!p.buffer)
            {
                throw std::runtime_error("Failed to create WebGPU storage buffer");
            }

            p.bufferSize = bufferSize;
        }

        void Buffer::_init(uint64_t bufferSize)
        {
            createBuffer(bufferSize);
        }

        Buffer::Buffer(Fl_WebGPU_Context& ctx) :
            _p(new Private)
        {
            TLRENDER_P();

            p.device = ctx.device;
            p.queue  = ctx.queue();
        }

        std::shared_ptr<Buffer> Buffer::create(Fl_WebGPU_Context& ctx,
                                               uint64_t bufferSize)
        {
            auto out = std::shared_ptr<Buffer>(new Buffer(ctx));
            out->_init(bufferSize);
            return out;
        }

        const WGPUBuffer& Buffer::getBuffer() const
        {
            return _p->buffer;
        }

        uint64_t Buffer::getBufferSize() const
        {
            return _p->bufferSize;
        }

        void Buffer::upload(const uint8_t* data)
        {
            TLRENDER_P();

            if (!p.buffer)
            {
                throw std::runtime_error("Attempting to upload to an uninitialized buffer");
            }

            if (!data)
            {
                throw std::runtime_error("Attempting to upload from a null data pointer");
            }

            if (!p.queue)
            {
                throw std::runtime_error("Invalid WebGPU queue");
            }

            if (p.bufferSize > 0)
            {
                // Dawn stages and copies data into the buffer on the queue
                // timeline, avoiding manual host mapping or cache flushes.
                wgpuQueueWriteBuffer(p.queue, p.buffer, 0, data, p.bufferSize);
            }
        }

    } // namespace wgpu
} // namespace tl
