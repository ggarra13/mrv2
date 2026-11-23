// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlWGPU/OffscreenBuffer.h>

#include <tlWGPU/Texture.h> // getTextureFormat(), getTextureFilter()

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

// A browser cannot block waiting for a GPU callback: callbacks are
// delivered by the JS event loop, which a spinning main thread starves, so a
// wait loop always runs into its timeout.  Define TLRENDER_WGPU_CAN_BLOCK=1
// to force the blocking paths (e.g. with JSPI / Asyncify).
#ifndef TLRENDER_WGPU_CAN_BLOCK
#    if defined(__EMSCRIPTEN__)
#        define TLRENDER_WGPU_CAN_BLOCK 0
#    else
#        define TLRENDER_WGPU_CAN_BLOCK 1
#    endif
#endif

namespace tl
{
    namespace wgpu
    {
        TLRENDER_ENUM_IMPL(OffscreenDepth, "None", "16", "24", "32");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenDepth);

        TLRENDER_ENUM_IMPL(OffscreenStencil, "None", "8");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenStencil);

        TLRENDER_ENUM_IMPL(OffscreenSampling, "None", "2", "4", "8", "16");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenSampling);

        namespace
        {
            constexpr int NUM_PBO_BUFFERS = 3;

            // WebGPU requires texture-to-buffer copy rows to be a multiple of
            // 256 bytes (COPY_BYTES_PER_ROW_ALIGNMENT).
            constexpr uint32_t COPY_BYTES_PER_ROW_ALIGNMENT = 256;

            constexpr uint64_t alignUp(uint64_t value, uint64_t alignment)
            {
                return (value + alignment - 1) / alignment * alignment;
            }

            // WebGPU only guarantees 1x and 4x MSAA.  Some color formats
            // (the 32-bit float ones) cannot be multisampled at all.
            bool formatSupportsMultisample(WGPUTextureFormat format)
            {
                switch (format)
                {
                case WGPUTextureFormat_R32Float:
                case WGPUTextureFormat_RG32Float:
                case WGPUTextureFormat_RGBA32Float:
                    return false;
                default:
                    return true;
                }
            }

            bool formatIsFloat32(WGPUTextureFormat format)
            {
                return format == WGPUTextureFormat_R32Float ||
                       format == WGPUTextureFormat_RG32Float ||
                       format == WGPUTextureFormat_RGBA32Float;
            }

            OffscreenSampling clampSampling(
                OffscreenSampling requested, WGPUTextureFormat colorFormat)
            {
                if (OffscreenSampling::kNone == requested)
                    return requested;
                if (!formatSupportsMultisample(colorFormat))
                    return OffscreenSampling::kNone;
                return OffscreenSampling::_4;
            }

            // Chooses a depth/stencil format.  Unlike Vulkan there is nothing
            // to probe per format, except Depth32FloatStencil8 which is an
            // optional feature.
            WGPUTextureFormat getBufferInternalFormat(
                WGPUDevice device, OffscreenDepth depth, OffscreenStencil stencil)
            {
                const bool hasStencil = (stencil != OffscreenStencil::kNone);
                WGPUTextureFormat out = WGPUTextureFormat_Undefined;
                switch (depth)
                {
                case OffscreenDepth::kNone:
                    if (hasStencil)
                        out = WGPUTextureFormat_Stencil8;
                    break;
                case OffscreenDepth::_16:
                    // There is no D16+S8: use the combined 24+8 format.
                    out = hasStencil ? WGPUTextureFormat_Depth24PlusStencil8
                                     : WGPUTextureFormat_Depth16Unorm;
                    break;
                case OffscreenDepth::_24:
                    out = hasStencil ? WGPUTextureFormat_Depth24PlusStencil8
                                     : WGPUTextureFormat_Depth24Plus;
                    break;
                case OffscreenDepth::_32:
                    if (hasStencil)
                    {
                        out = wgpuDeviceHasFeature(
                                  device, WGPUFeatureName_Depth32FloatStencil8)
                                  ? WGPUTextureFormat_Depth32FloatStencil8
                                  : WGPUTextureFormat_Depth24PlusStencil8;
                    }
                    else
                    {
                        out = WGPUTextureFormat_Depth32Float;
                    }
                    break;
                default:
                    break;
                }
                return out;
            }

            // Process pending callbacks (buffer maps).
            // Define TLRENDER_WGPU_DAWN when building against Dawn.
            [[maybe_unused]] void pumpEvents(WGPUInstance instance)
            {
                wgpuInstanceProcessEvents(instance);
                // \@bug: neither function exists
// #if defined(TLRENDER_WGPU_DAWN)
//                 wgpuDeviceTick(device);
// #else
//                 wgpuDevicePoll(device, false, nullptr);
// #endif
            }

            //! Shared between the staging slot and the map callback, so the
            //! callback is safe even if the buffer is destroyed first.
            struct MapState
            {
                std::atomic<int> status{0}; // 0 = pending, 1 = ok, 2 = failed
            };

            void onBufferMapped(
                WGPUMapAsyncStatus status, WGPUStringView, void* userdata1,
                void*)
            {
                auto* state = static_cast<std::shared_ptr<MapState>*>(userdata1);
                (*state)->status =
                    (WGPUMapAsyncStatus_Success == status) ? 1 : 2;
                delete state;
            }

            struct StagingBuffer
            {
                enum class State
                {
                    Idle,       // free
                    Recorded,   // copy recorded, not yet mapping
                    MapPending, // submitted, mapAsync in flight
                    Ready       // pixels holds tightly packed data
                };

                WGPUBuffer buffer = nullptr;
                uint64_t size = 0;
                State state = State::Idle;
                std::shared_ptr<MapState> map;
                uint32_t width = 0;
                uint32_t height = 0;
                std::vector<uint8_t> pixels;
            };
        } // namespace

        bool OffscreenBufferOptions::operator==(
            const OffscreenBufferOptions& other) const
        {
            return (colorType == other.colorType &&
                    colorFilters == other.colorFilters &&
                    depth == other.depth &&
                    stencil == other.stencil && sampling == other.sampling &&
                    clearColor == other.clearColor &&
                    clearDepth == other.clearDepth &&
                    storeDepth == other.storeDepth &&
                    multiFrameDepth == other.multiFrameDepth &&
                    pbo == other.pbo);
        }

        bool OffscreenBufferOptions::operator!=(
            const OffscreenBufferOptions& other) const
        {
            return !(*this == other);
        }

        namespace
        {
            std::atomic<size_t> objectCount = 0;
            std::atomic<size_t> totalByteCount = 0;
        } // namespace

        size_t OffscreenBuffer::getTotalByteCount()
        {
            return totalByteCount;
        }

        size_t OffscreenBuffer::getObjectCount()
        {
            return objectCount;
        }

        struct OffscreenBuffer::Private
        {
            WGPUInstance instance = nullptr;
            WGPUDevice device = nullptr;

            uint32_t frameIndex = 0; // unused, see header

            math::Size2i size;
            OffscreenBufferOptions options;   // effective
            OffscreenBufferOptions requested; // as passed to create()

            WGPUTextureFormat colorFormat = WGPUTextureFormat_RGBA8Unorm;
            WGPUTextureFormat depthFormat = WGPUTextureFormat_Undefined;
            uint32_t bytesPerPixel = 4;
            size_t byteCount = 0;

            // Multisampled color render target (only when sampling > 1).
            WGPUTexture msColorTexture = nullptr;
            WGPUTextureView msColorView = nullptr;

            // Resolved single-sample color texture (public API texture).
            WGPUTexture resolveTexture = nullptr;
            WGPUTextureView resolveView = nullptr;

            WGPUTexture depthTexture = nullptr;
            WGPUTextureView depthView = nullptr;
            WGPUTextureView depthSampleView = nullptr;

            WGPUSampler sampler = nullptr;

            Viewport viewport;
            ScissorRect scissor;

            // Read-back ring.
            int writeIndex = 0; // where the next copy is recorded
            int readIndex = 0;  // where we'll next look for finished data
            std::vector<StagingBuffer> pboRing;

            uint32_t sampleCount() const
            {
                return options.sampling == OffscreenSampling::kNone ? 1u : 4u;
            }

            uint32_t paddedBytesPerRow(uint32_t width) const
            {
                return static_cast<uint32_t>(alignUp(
                    static_cast<uint64_t>(width) * bytesPerPixel,
                    COPY_BYTES_PER_ROW_ALIGNMENT));
            }

            // Wait (bounded) until the slot's map request has completed.
            // On Emscripten this cannot wait: it processes events once and
            // reports whether the map has completed *so far*.
            bool waitForMap(StagingBuffer& slot, int timeoutMs = 5000)
            {
#if !TLRENDER_WGPU_CAN_BLOCK
                (void)timeoutMs;
                pumpEvents(instance);
                return !slot.map || 0 != slot.map->status;
#else
                const auto start = std::chrono::steady_clock::now();
                while (slot.map && 0 == slot.map->status)
                {
                    pumpEvents(instance);
                    if (0 != slot.map->status)
                        break;
                    if (std::chrono::steady_clock::now() - start >
                        std::chrono::milliseconds(timeoutMs))
                    {
                        return false;
                    }
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(200));
                }
                return true;
#endif
            }

            // Advance a slot from MapPending to Ready (copying the data out
            // tightly packed and unmapping) if its map has completed.
            // Returns true when the slot is Ready.
            bool finishMap(StagingBuffer& slot, bool wait)
            {
                if (StagingBuffer::State::Ready == slot.state)
                    return true;
                if (StagingBuffer::State::MapPending != slot.state)
                    return false;

                if (wait)
                {
                    if (!waitForMap(slot))
                        return false;
                }
                else
                {
                    pumpEvents(instance);
                }

                const int status = slot.map ? slot.map->status.load() : 2;
                if (0 == status)
                    return false;

                if (1 != status)
                {
                    slot.state = StagingBuffer::State::Idle;
                    return false;
                }

                const uint32_t rowBytes = slot.width * bytesPerPixel;
                const uint32_t srcStride = paddedBytesPerRow(slot.width);
                const uint64_t mappedSize =
                    static_cast<uint64_t>(srcStride) * slot.height;
                const auto* src = static_cast<const uint8_t*>(
                    wgpuBufferGetConstMappedRange(slot.buffer, 0, mappedSize));
                if (src)
                {
                    slot.pixels.resize(static_cast<size_t>(rowBytes) * slot.height);
                    for (uint32_t row = 0; row < slot.height; ++row)
                    {
                        std::memcpy(
                            slot.pixels.data() +
                                static_cast<size_t>(row) * rowBytes,
                            src + static_cast<size_t>(row) * srcStride,
                            rowBytes);
                    }
                }
                // Unmap right away so the GPU can reuse the buffer.
                wgpuBufferUnmap(slot.buffer);
                slot.state = src ? StagingBuffer::State::Ready
                                 : StagingBuffer::State::Idle;
                return StagingBuffer::State::Ready == slot.state;
            }

            // Make a slot safe to be a copy destination again.  Returns
            // false if a previous map could not be completed in time.
            bool recycle(StagingBuffer& slot)
            {
                if (StagingBuffer::State::MapPending == slot.state)
                {
                    finishMap(slot, true);
                    if (StagingBuffer::State::MapPending == slot.state)
                        return false;
                }
                slot.state = StagingBuffer::State::Idle;
                return true;
            }

            void recordCopy(
                WGPUCommandEncoder encoder, StagingBuffer& slot, int32_t x,
                int32_t y, uint32_t w, uint32_t h)
            {
                x = std::max(0, std::min(x, size.w - 1));
                y = std::max(0, std::min(y, size.h - 1));
                const uint32_t maxW = static_cast<uint32_t>(size.w - x);
                const uint32_t maxH = static_cast<uint32_t>(size.h - y);
                w = (0 == w) ? maxW : std::min(w, maxW);
                h = (0 == h) ? maxH : std::min(h, maxH);

                WGPUTexelCopyTextureInfo src = {};
                src.texture = resolveTexture;
                src.mipLevel = 0;
                src.origin = {
                    static_cast<uint32_t>(x), static_cast<uint32_t>(y), 0};
                src.aspect = WGPUTextureAspect_All;

                WGPUTexelCopyBufferInfo dst = {};
                dst.buffer = slot.buffer;
                dst.layout.offset = 0;
                dst.layout.bytesPerRow = paddedBytesPerRow(w);
                dst.layout.rowsPerImage = h;

                const WGPUExtent3D extent = {w, h, 1};
                wgpuCommandEncoderCopyTextureToBuffer(
                    encoder, &src, &dst, &extent);

                slot.width = w;
                slot.height = h;
                slot.state = StagingBuffer::State::Recorded;
            }

            void requestMap(StagingBuffer& slot)
            {
                slot.map = std::make_shared<MapState>();

                WGPUBufferMapCallbackInfo info = {};
#if TLRENDER_WGPU_CAN_BLOCK
                info.mode = WGPUCallbackMode_AllowProcessEvents;
#else
                // Delivered by the browser's event loop; no pumping needed.
                info.mode = WGPUCallbackMode_AllowSpontaneous;
#endif
                info.callback = onBufferMapped;
                info.userdata1 = new std::shared_ptr<MapState>(slot.map);

                const uint64_t mapSize =
                    static_cast<uint64_t>(paddedBytesPerRow(slot.width)) *
                    slot.height;
                wgpuBufferMapAsync(
                    slot.buffer, WGPUMapMode_Read, 0, mapSize, info);
                slot.state = StagingBuffer::State::MapPending;
            }
        };

        // ====================================================================
        //  _init
        // ====================================================================

        void OffscreenBuffer::_init(
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            TLRENDER_P();

            p.instance = _instance;
            p.device = _device;
            p.requested = options;

            OffscreenBufferOptions offscreenBufferOptions = options;
            switch (offscreenBufferOptions.colorType)
            {
            case image::PixelType::RGB_U8:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                break;
            case image::PixelType::RGB_U16:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U16;
                break;
            case image::PixelType::RGB_F16:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F16;
                break;
            case image::PixelType::RGB_F32:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                break;
            default:
                break;
            };

            p.size = size;
            p.options = offscreenBufferOptions;
            p.colorFormat = getTextureFormat(p.options.colorType);
            p.depthFormat = getBufferInternalFormat(
                _device, p.options.depth, p.options.stencil);

            // Get maximum texture resolution for the device.
            uint32_t maxTextureSize = 8192;
            WGPULimits limits = {};
            if (wgpuDeviceGetLimits(_device, &limits) == WGPUStatus_Success)
                maxTextureSize = limits.maxTextureDimension2D;

            if (p.size.w > static_cast<int>(maxTextureSize))
                p.size.w = maxTextureSize;
            if (p.size.h > static_cast<int>(maxTextureSize))
                p.size.h = maxTextureSize;
            if (p.size.w <= 0 || p.size.h <= 0)
            {
                throw std::invalid_argument(
                    "tl::webgpu::OffscreenBuffer: invalid size");
            }

            // WebGPU: 1x or 4x only.  Write the effective value back so
            // getSampleCount() and every caller see it.
            p.options.sampling =
                clampSampling(p.options.sampling, p.colorFormat);

            const image::Info info(p.size.w, p.size.h, p.options.colorType);
            p.byteCount = image::getDataByteCount(info);
            p.bytesPerPixel = static_cast<uint32_t>(
                p.byteCount / (static_cast<size_t>(p.size.w) * p.size.h));
            totalByteCount += p.byteCount;
            ++objectCount;

            initialize();
        }

        OffscreenBuffer::OffscreenBuffer(Fl_WebGPU_Context& ctx) :
            _instance(ctx.instance),
            _device(ctx.device),
            _queue(ctx.queue()),
            _p(new Private)
        {
        }

        OffscreenBuffer::~OffscreenBuffer()
        {
            TLRENDER_P();

            --objectCount;
            totalByteCount -= p.byteCount;

            cleanup();
        }

        // ====================================================================
        //  cleanup
        //
        //  No vkDeviceWaitIdle needed: wgpu*Release() only drops our
        //  reference, the implementation keeps resources alive until any
        //  submitted work using them has finished.
        // ====================================================================

        void OffscreenBuffer::cleanup()
        {
            TLRENDER_P();

            if (p.sampler)
                wgpuSamplerRelease(p.sampler);
            p.sampler = nullptr;

            if (p.msColorView)
                wgpuTextureViewRelease(p.msColorView);
            if (p.msColorTexture)
                wgpuTextureRelease(p.msColorTexture);
            p.msColorView = nullptr;
            p.msColorTexture = nullptr;

            if (p.resolveView)
                wgpuTextureViewRelease(p.resolveView);
            if (p.resolveTexture)
                wgpuTextureRelease(p.resolveTexture);
            p.resolveView = nullptr;
            p.resolveTexture = nullptr;

            if (p.depthSampleView)
                wgpuTextureViewRelease(p.depthSampleView);
            if (p.depthView)
                wgpuTextureViewRelease(p.depthView);
            if (p.depthTexture)
                wgpuTextureRelease(p.depthTexture);
            p.depthSampleView = nullptr;
            p.depthView = nullptr;
            p.depthTexture = nullptr;

            // A pending map callback holds its own shared MapState, so it is
            // safe to release the buffer even while a map is in flight.
            for (auto& pbo : p.pboRing)
            {
                if (pbo.buffer)
                    wgpuBufferRelease(pbo.buffer);
                pbo.buffer = nullptr;
            }
            p.pboRing.clear();
        }

        // ====================================================================
        //  initialize
        // ====================================================================

        void OffscreenBuffer::initialize()
        {
            TLRENDER_P();

            createColorTextures();

            if (hasDepth() || hasStencil())
                createDepthTextures();

            setupViewportAndScissor();

            if (p.options.pbo)
                createStagingBuffers();

            createSampler();
        }

        // ====================================================================
        //  Frame-index selection (no-ops)
        // ====================================================================

        void OffscreenBuffer::setFrameIndex(uint32_t frameIndex)
        {
            _p->frameIndex = frameIndex;
        }

        uint32_t OffscreenBuffer::getFrameIndex() const
        {
            return _p->frameIndex;
        }

        // ====================================================================
        //  Texture creation
        // ====================================================================

        void OffscreenBuffer::createColorTextures()
        {
            TLRENDER_P();

            const uint32_t samples = p.sampleCount();
            const bool multisampled = (samples > 1);

            auto makeTexture = [&](uint32_t sampleCount, WGPUTextureUsage usage,
                                   const char* label)
            {
                WGPUTextureDescriptor desc = {};
                desc.label = {label, WGPU_STRLEN};
                desc.usage = usage;
                desc.dimension = WGPUTextureDimension_2D;
                desc.size = {
                    static_cast<uint32_t>(p.size.w),
                    static_cast<uint32_t>(p.size.h), 1};
                desc.format = p.colorFormat;
                desc.mipLevelCount = 1;
                desc.sampleCount = sampleCount;
                WGPUTexture texture = wgpuDeviceCreateTexture(_device, &desc);
                if (!texture)
                    throw std::runtime_error(
                        std::string("Failed to create ") + label);
                return texture;
            };

            // 1. Resolve texture (always single-sampled).
            p.resolveTexture = makeTexture(
                1,
                WGPUTextureUsage_RenderAttachment |
                    WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopySrc,
                "OffscreenBuffer: resolve color texture");
            p.resolveView = wgpuTextureCreateView(p.resolveTexture, nullptr);

            // 2. Multisampled color texture (only when needed).
            if (multisampled)
            {
                p.msColorTexture = makeTexture(
                    samples, WGPUTextureUsage_RenderAttachment,
                    "OffscreenBuffer: multisampled color texture");
                p.msColorView = wgpuTextureCreateView(p.msColorTexture, nullptr);
            }
        }

        void OffscreenBuffer::createDepthTextures()
        {
            TLRENDER_P();

            // One depth texture is enough: WebGPU serializes queue work, so
            // the next frame's opaque pass cannot overwrite depth before the
            // previous frame's OIT pass has read it.
            WGPUTextureDescriptor desc = {};
            desc.label = {"OffscreenBuffer: depth texture", WGPU_STRLEN};
            desc.usage = WGPUTextureUsage_RenderAttachment |
                         WGPUTextureUsage_TextureBinding;
            desc.dimension = WGPUTextureDimension_2D;
            desc.size = {
                static_cast<uint32_t>(p.size.w),
                static_cast<uint32_t>(p.size.h), 1};
            desc.format = p.depthFormat;
            desc.mipLevelCount = 1;
            desc.sampleCount = p.sampleCount();

            p.depthTexture = wgpuDeviceCreateTexture(_device, &desc);
            if (!p.depthTexture)
                throw std::runtime_error("Failed to create depth texture");

            // Attachment view: all aspects.
            p.depthView = wgpuTextureCreateView(p.depthTexture, nullptr);

            // Sampling view: a texture binding must see a single aspect.
            if (hasDepth())
            {
                WGPUTextureViewDescriptor vd = {};
                vd.label = {"OffscreenBuffer: depth sample view descriptor", WGPU_STRLEN};

                // Depth-only view of a combined depth/stencil texture needs the
                // aspect's own format (Depth24Plus / Depth32Float), not the texture's.
                WGPUTextureFormat sampleFormat = p.depthFormat;
                switch (p.depthFormat)
                {
                case WGPUTextureFormat_Depth24PlusStencil8:
                    sampleFormat = WGPUTextureFormat_Depth24Plus;
                    break;
                case WGPUTextureFormat_Depth32FloatStencil8:
                    sampleFormat = WGPUTextureFormat_Depth32Float;
                    break;
                default:
                    break; // Depth16Unorm, Depth24Plus, Depth32Float: unchanged
                }

                vd.format = sampleFormat;
                vd.dimension = WGPUTextureViewDimension_2D;
                vd.baseMipLevel = 0;
                vd.mipLevelCount = 1;
                vd.baseArrayLayer = 0;
                vd.arrayLayerCount = 1;
                vd.aspect = WGPUTextureAspect_DepthOnly;
                p.depthSampleView = wgpuTextureCreateView(p.depthTexture, &vd);
            }
        }

        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            Fl_WebGPU_Context& ctx, const math::Size2i& size,
            const OffscreenBufferOptions& options)
        {
            auto out = std::shared_ptr<OffscreenBuffer>(
                new OffscreenBuffer(ctx));
            out->_init(size, options);
            return out;
        }

        // ====================================================================
        //  Simple getters
        // ====================================================================

        const math::Size2i& OffscreenBuffer::getSize() const
        {
            return _p->size;
        }

        int OffscreenBuffer::getWidth() const
        {
            return _p->size.w;
        }

        int OffscreenBuffer::getHeight() const
        {
            return _p->size.h;
        }

        int OffscreenBuffer::getChannelCount() const
        {
            return image::getChannelCount(_p->options.colorType);
        }

        bool OffscreenBuffer::hasDepth() const
        {
            return (_p->options.depth != OffscreenDepth::kNone);
        }

        bool OffscreenBuffer::hasStencil() const
        {
            return (_p->options.stencil != OffscreenStencil::kNone);
        }

        uint32_t OffscreenBuffer::getSampleCount() const
        {
            return _p->sampleCount();
        }

        const OffscreenBufferOptions& OffscreenBuffer::getOptions() const
        {
            return _p->options;
        }

        const OffscreenBufferOptions&
        OffscreenBuffer::getRequestedOptions() const
        {
            return _p->requested;
        }

        WGPUTextureFormat OffscreenBuffer::getFormat() const
        {
            return _p->colorFormat;
        }

        WGPUTextureFormat OffscreenBuffer::getDepthFormat() const
        {
            return _p->depthFormat;
        }

        WGPUTexture OffscreenBuffer::getTexture() const
        {
            return _p->resolveTexture;
        }

        WGPUTextureView OffscreenBuffer::getTextureView() const
        {
            return _p->resolveView;
        }

        WGPUTexture OffscreenBuffer::getDepthTexture() const
        {
            return _p->depthTexture;
        }

        WGPUTextureView OffscreenBuffer::getDepthView() const
        {
            return _p->depthView;
        }

        WGPUTextureView OffscreenBuffer::getDepthSampleView() const
        {
            return _p->depthSampleView;
        }

        WGPUSampler OffscreenBuffer::getSampler() const
        {
            return _p->sampler;
        }

        WGPUExtent3D OffscreenBuffer::getExtent() const
        {
            return {
                static_cast<uint32_t>(_p->size.w),
                static_cast<uint32_t>(_p->size.h), 1};
        }

        Viewport OffscreenBuffer::getViewport() const
        {
            return _p->viewport;
        }

        ScissorRect OffscreenBuffer::getScissor() const
        {
            return _p->scissor;
        }

        // ====================================================================
        //  Viewport / scissor
        // ====================================================================

        void OffscreenBuffer::setupViewportAndScissor()
        {
            TLRENDER_P();

            // The Vulkan version used a negative-height viewport to flip Y.
            // WebGPU rejects negative heights, and its NDC +Y is already up
            // (matching what that flip produced), so this is a plain
            // full-size viewport.
            p.viewport = {};
            p.viewport.x = 0.F;
            p.viewport.y = 0.F;
            p.viewport.width = static_cast<float>(p.size.w);
            p.viewport.height = static_cast<float>(p.size.h);
            p.viewport.minDepth = 0.F;
            p.viewport.maxDepth = 1.F;

            p.scissor = {};
            p.scissor.x = 0;
            p.scissor.y = 0;
            p.scissor.width = static_cast<uint32_t>(p.size.w);
            p.scissor.height = static_cast<uint32_t>(p.size.h);
        }

        void OffscreenBuffer::setupScissor(const math::Box2i& value)
        {
            TLRENDER_P();

            if (value.w() > 0 && value.h() > 0)
            {
                // WebGPU validates that the scissor lies inside the render
                // target (Vulkan did not), so clamp.
                const int x = std::max(0, std::min(value.x(), p.size.w));
                const int y = std::max(0, std::min(value.y(), p.size.h));
                const int w = std::max(0, std::min(value.w(), p.size.w - x));
                const int h = std::max(0, std::min(value.h(), p.size.h - y));
                p.scissor.x = static_cast<uint32_t>(x);
                p.scissor.y = static_cast<uint32_t>(y);
                p.scissor.width = static_cast<uint32_t>(w);
                p.scissor.height = static_cast<uint32_t>(h);
            }
        }

        void OffscreenBuffer::setupViewportAndScissor(WGPURenderPassEncoder pass)
        {
            TLRENDER_P();

            setupViewportAndScissor();

            wgpuRenderPassEncoderSetViewport(
                pass, p.viewport.x, p.viewport.y, p.viewport.width,
                p.viewport.height, p.viewport.minDepth, p.viewport.maxDepth);
            wgpuRenderPassEncoderSetScissorRect(
                pass, p.scissor.x, p.scissor.y, p.scissor.width,
                p.scissor.height);
        }

        // ====================================================================
        //  Render passes
        //
        //  Replaces: the clear/load VkRenderPass objects, VkFramebuffers,
        //  layout tracking and all the image/depth barriers.
        // ====================================================================

        WGPURenderPassEncoder OffscreenBuffer::_beginRenderPass(
            WGPUCommandEncoder encoder, bool clear, bool depthReadOnly,
            const image::Color4f& clearColor)
        {
            TLRENDER_P();

            const bool multisampled = (p.sampleCount() > 1);

            // Color.  Always stored.  For MSAA the multisampled texture is
            // stored as well (not discarded) so that a later *load* pass
            // really continues from the previous contents, then resolved
            // into the single-sample texture.
            WGPURenderPassColorAttachment color = {};
            color.view = multisampled ? p.msColorView : p.resolveView;
            color.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
            color.resolveTarget = multisampled ? p.resolveView : nullptr;
            color.storeOp = WGPUStoreOp_Store;
            if (clear)
            {
                const image::Color4f& c = clearColor;
                color.loadOp = WGPULoadOp_Clear;
                color.clearValue = {c.r, c.g, c.b, c.a};
            }
            else
            {
                color.loadOp = WGPULoadOp_Load;
            }

            // Depth/stencil.  Ops must be set only for aspects the format
            // has, and must be left unset for a read-only aspect.
            WGPURenderPassDepthStencilAttachment ds = {};
            const bool hasDS = hasDepth() || hasStencil();
            if (hasDS)
            {
                ds.view = p.depthView;
                if (hasDepth())
                {
                    if (depthReadOnly && !clear)
                    {
                        ds.depthReadOnly = true;
                    }
                    else
                    {
                        ds.depthLoadOp =
                            clear ? WGPULoadOp_Clear : WGPULoadOp_Load;
                        ds.depthStoreOp = WGPUStoreOp_Store;
                        ds.depthClearValue = 1.F;
                    }
                }
                if (hasStencil())
                {
                    // Stencil is cleared in both passes (as in the original).
                    ds.stencilLoadOp = WGPULoadOp_Clear;
                    ds.stencilStoreOp = WGPUStoreOp_Store;
                    ds.stencilClearValue = 0;
                }
            }

            WGPURenderPassDescriptor desc = {};
            desc.label = {
                clear ? "OffscreenBuffer clear pass" : "OffscreenBuffer load pass",
                WGPU_STRLEN};
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &color;
            desc.depthStencilAttachment = hasDS ? &ds : nullptr;

            WGPURenderPassEncoder pass =
                wgpuCommandEncoderBeginRenderPass(encoder, &desc);

            setupViewportAndScissor(pass);
            return pass;
        }

        WGPURenderPassEncoder
        OffscreenBuffer::beginClearRenderPass(WGPUCommandEncoder encoder)
        {
            return _beginRenderPass(encoder, true, false, _p->options.clearColor);
        }

        WGPURenderPassEncoder OffscreenBuffer::beginClearRenderPass(
            WGPUCommandEncoder encoder, const image::Color4f& clearColor)
        {
            return _beginRenderPass(encoder, true, false, clearColor);
        }

        WGPURenderPassEncoder OffscreenBuffer::beginLoadRenderPass(
            WGPUCommandEncoder encoder, bool depthReadOnly)
        {
            return _beginRenderPass(encoder, false, depthReadOnly, _p->options.clearColor);
        }

        void OffscreenBuffer::endRenderPass(WGPURenderPassEncoder pass)
        {
            wgpuRenderPassEncoderEnd(pass);
            wgpuRenderPassEncoderRelease(pass);
        }

        // ====================================================================
        //  Sampler
        // ====================================================================

        void OffscreenBuffer::createSampler()
        {
            TLRENDER_P();

            WGPUFilterMode magFilter =
                getTextureFilter(p.options.colorFilters.magnify);
            WGPUFilterMode minFilter =
                getTextureFilter(p.options.colorFilters.minify);

            // 32-bit float textures are not filterable unless the
            // float32-filterable feature was enabled on the device.
            if (formatIsFloat32(p.colorFormat) &&
                !wgpuDeviceHasFeature(_device, WGPUFeatureName_Float32Filterable))
            {
                magFilter = WGPUFilterMode_Nearest;
                minFilter = WGPUFilterMode_Nearest;
            }

            WGPUSamplerDescriptor desc = {};
            desc.label = {"OffscreenBuffer sampler", WGPU_STRLEN};
            desc.addressModeU = WGPUAddressMode_ClampToEdge;
            desc.addressModeV = WGPUAddressMode_ClampToEdge;
            desc.addressModeW = WGPUAddressMode_ClampToEdge;
            desc.magFilter = magFilter;
            desc.minFilter = minFilter;
            desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
            desc.lodMinClamp = 0.F;
            desc.lodMaxClamp = 32.F;
            desc.compare = WGPUCompareFunction_Undefined;
            desc.maxAnisotropy = 1;

            p.sampler = wgpuDeviceCreateSampler(_device, &desc);
            if (!p.sampler)
            {
                throw std::runtime_error(
                    "Failed to create sampler for offscreen buffer.");
            }
        }

        // ====================================================================
        //  PBO / staging buffers
        // ====================================================================

        void OffscreenBuffer::createStagingBuffers()
        {
            TLRENDER_P();

            // Rows are padded to 256 bytes, so size for the padded layout.
            const uint64_t bufferSize =
                static_cast<uint64_t>(p.paddedBytesPerRow(p.size.w)) *
                p.size.h;

            p.pboRing.resize(NUM_PBO_BUFFERS);
            for (int i = 0; i < NUM_PBO_BUFFERS; ++i)
            {
                auto& pbo = p.pboRing[i];

                WGPUBufferDescriptor desc = {};
                desc.label = {"OffscreenBuffer staging buffer", WGPU_STRLEN};
                desc.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
                desc.size = bufferSize;
                desc.mappedAtCreation = false;

                pbo.buffer = wgpuDeviceCreateBuffer(_device, &desc);
                if (!pbo.buffer)
                    throw std::runtime_error("Failed to create staging buffer");
                pbo.size = bufferSize;
                pbo.state = StagingBuffer::State::Idle;
            }
            p.writeIndex = 0;
            p.readIndex = 0;
        }

        void OffscreenBuffer::readPixels(
            WGPUCommandEncoder encoder, int32_t x, int32_t y, uint32_t w,
            uint32_t h)
        {
            TLRENDER_P();

            if (p.pboRing.empty())
                return;

            // Equivalent of the Vulkan fence wait: make sure the slot's
            // previous use is finished and it is not mapped.
            auto& pbo = p.pboRing[p.writeIndex];
            if (!p.recycle(pbo))
            {
#if TLRENDER_WGPU_CAN_BLOCK
                std::cerr << "OffscreenBuffer: timed out waiting for the "
                             "previous read-back"
                          << std::endl;
#endif
                // Emscripten: the slot is still in flight; drop this
                // read-back instead of blocking.
                return;
            }

            p.recordCopy(encoder, pbo, x, y, w, h);
        }

        void OffscreenBuffer::submitReadback(WGPUCommandEncoder encoder)
        {
            TLRENDER_P();

            if (p.pboRing.empty())
                return;

            WGPUCommandBuffer commands =
                wgpuCommandEncoderFinish(encoder, nullptr);
            if (!commands)
            {
                std::cerr << "OffscreenBuffer::submitReadback: failed to "
                             "finish command encoder"
                          << std::endl;
                return;
            }
            wgpuQueueSubmit(_queue, 1, &commands);
            wgpuCommandBufferRelease(commands);

            // The map request is ordered after the submit above.
            auto& pbo = p.pboRing[p.writeIndex];
            if (StagingBuffer::State::Recorded == pbo.state)
                p.requestMap(pbo);

            p.writeIndex = (p.writeIndex + 1) % NUM_PBO_BUFFERS;
        }

        void OffscreenBuffer::readPixelsInline(
            WGPUCommandEncoder encoder, int32_t x, int32_t y, uint32_t w,
            uint32_t h)
        {
            TLRENDER_P();

            if (p.pboRing.empty())
                return;

            auto& pbo = p.pboRing[p.writeIndex];
            if (!p.recycle(pbo))
            {
#if TLRENDER_WGPU_CAN_BLOCK
                std::cerr << "OffscreenBuffer: timed out waiting for the "
                             "previous read-back"
                          << std::endl;
#endif
                // Emscripten: the slot is still in flight; drop this
                // read-back instead of blocking.
                return;
            }

            p.recordCopy(encoder, pbo, x, y, w, h);

            // Advance ring; the data will be in the slot we just recorded.
            p.writeIndex = (p.writeIndex + 1) % NUM_PBO_BUFFERS;
        }

        void* OffscreenBuffer::getInlineReadbackPtr()
        {
            TLRENDER_P();

            if (p.pboRing.empty())
                return nullptr;

            // The last-written slot is one behind the current writeIndex.
            const int slot =
                (p.writeIndex - 1 + NUM_PBO_BUFFERS) % NUM_PBO_BUFFERS;
            auto& pbo = p.pboRing[slot];

            // The caller has submitted by now, so the map can be requested.
            if (StagingBuffer::State::Recorded == pbo.state)
                p.requestMap(pbo);

            // Native: waits (bounded).  Emscripten: returns false while the
            // map is in flight; the caller polls again on a later frame.
            if (p.finishMap(pbo, true))
                return pbo.pixels.data();
            return nullptr;
        }

        bool OffscreenBuffer::getLatestReadPixels(void*& imageData)
        {
            TLRENDER_P();

            if (p.pboRing.empty())
                return false;

            auto& pbo = p.pboRing[p.readIndex];

            // Non-blocking, like the vkGetFenceStatus() check.
            if (p.finishMap(pbo, false))
            {
                imageData = pbo.pixels.data();
                p.readIndex = (p.readIndex + 1) % NUM_PBO_BUFFERS;
                return true;
            }
            return false;
        }

        // ====================================================================
        //  doCreate
        //
        //  Compares against the *requested* options: the effective options
        //  can legitimately differ (RGB -> RGBA, 8x MSAA -> 4x, ...), which
        //  would otherwise make this return true on every call.
        // ====================================================================

        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>& offscreenBuffer,
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            bool out = false;
            out |= size.isValid() && !offscreenBuffer;
            out |= size.isValid() && offscreenBuffer &&
                   offscreenBuffer->getSize() != size;
            out |= offscreenBuffer &&
                   offscreenBuffer->getRequestedOptions() != options;
            return out;
        }
    } // namespace webgpu
} // namespace tl
