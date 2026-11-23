// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/WGPU.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Box.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>
#include <tlCore/Size.h>

#include <cstdint>
#include <memory>

namespace tl
{
    namespace wgpu
    {

        //! Default offscreen buffer color type.
        const image::PixelType offscreenColorDefault =
            image::PixelType::RGBA_U8;

        //! Offscreen buffer depth size.
        //!
        //! WebGPU only has Depth16Unorm, Depth24Plus and Depth32Float
        //! (plus combined depth/stencil variants), so the chosen format may
        //! be larger than requested.
        enum class OffscreenDepth {
            kNone,
            _16,
            _24,
            _32,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(OffscreenDepth);
        TLRENDER_ENUM_SERIALIZE(OffscreenDepth);

        //! Default offscreen buffer depth type.
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_16;

        //! Offscreen buffer stencil size.
        enum class OffscreenStencil {
            kNone,
            _8,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(OffscreenStencil);
        TLRENDER_ENUM_SERIALIZE(OffscreenStencil);

        //! Offscreen buffer multisampling.
        //!
        //! WebGPU only guarantees 1x and 4x.  Any value other than kNone is
        //! therefore treated as 4x (and 1x for color formats that cannot be
        //! multisampled, such as RGBA32Float).
        enum class OffscreenSampling {
            kNone,
            _2,
            _4,
            _8,
            _16,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(OffscreenSampling);
        TLRENDER_ENUM_SERIALIZE(OffscreenSampling);

        //! Offscreen buffer options.
        struct OffscreenBufferOptions
        {
            image::PixelType colorType = offscreenColorDefault;
            timeline::ImageFilters colorFilters;
            OffscreenDepth depth = OffscreenDepth::kNone;
            OffscreenStencil stencil = OffscreenStencil::kNone;
            OffscreenSampling sampling = OffscreenSampling::kNone;
            bool clear = false;
            image::Color4f clearColor = image::Color4f(0.F, 0.F, 0.F, 0.F);
            bool clearDepth = true;
            bool pbo = false;
            bool storeDepth = false;

            //! Kept for source compatibility.  The Vulkan version needed one
            //! depth image per frame-in-flight; WebGPU orders and
            //! synchronizes queue work implicitly, so a single depth texture
            //! is always allocated and this flag has no effect.
            bool multiFrameDepth = false;

            bool operator==(const OffscreenBufferOptions&) const;
            bool operator!=(const OffscreenBufferOptions&) const;
        };

        //! Viewport (WebGPU has no negative-height flip; NDC +Y is already
        //! up, so this is a plain top-left origin rectangle).
        struct Viewport
        {
            float x = 0.F;
            float y = 0.F;
            float width = 0.F;
            float height = 0.F;
            float minDepth = 0.F;
            float maxDepth = 1.F;
        };

        //! Scissor rectangle.
        struct ScissorRect
        {
            uint32_t x = 0;
            uint32_t y = 0;
            uint32_t width = 0;
            uint32_t height = 0;
        };

        //! Offscreen buffer.
        //!
        //! Unlike Vulkan there are no render pass objects, framebuffers or
        //! image layouts to manage: the begin*RenderPass() functions build a
        //! WGPURenderPassDescriptor on the fly and the implementation tracks
        //! resource state and inserts barriers itself.
        class OffscreenBuffer
            : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLRENDER_NON_COPYABLE(OffscreenBuffer);

        protected:
            void _init(const math::Size2i&, const OffscreenBufferOptions&);

            OffscreenBuffer(Fl_WebGPU_Context&);

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            //! \p device and \p queue are not owned and must outlive the
            //! buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                Fl_WebGPU_Context& ctx,
                const math::Size2i&,
                const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const math::Size2i& getSize() const;

            //! Get the offscreen buffer width.
            int getWidth() const;

            //! Get the offscreen buffer height.
            int getHeight() const;

            //! Get the number of channels.
            int getChannelCount() const;

            //! Returns true if the buffer has depth.
            bool hasDepth() const;

            //! Returns true if the buffer has stencil.
            bool hasStencil() const;

            //! Get the effective number of samples per pixel (1 or 4).
            uint32_t getSampleCount() const;

            //! Get the effective options (color type, sampling, etc. may
            //! differ from what was requested).
            const OffscreenBufferOptions& getOptions() const;

            //! Get the options exactly as they were requested.
            const OffscreenBufferOptions& getRequestedOptions() const;

            // ----------------------------------------------------------------
            // Frame index (no-ops, kept for source compatibility)
            // ----------------------------------------------------------------

            void setFrameIndex(uint32_t frameIndex);
            uint32_t getFrameIndex() const;

            // ----------------------------------------------------------------
            // WebGPU accessors
            // ----------------------------------------------------------------

            //! Get the (single-sample, resolved) color texture.  This is the
            //! texture to sample from, copy from, or display.
            WGPUTexture getTexture() const;

            //! Get the color texture view.
            WGPUTextureView getTextureView() const;

            //! Get the depth/stencil texture.
            WGPUTexture getDepthTexture() const;

            //! Get the depth/stencil view for use as a render attachment
            //! (all aspects).
            WGPUTextureView getDepthView() const;

            //! Get a depth-only view for sampling the depth buffer in a
            //! shader (e.g. an OIT pass).  Null when there is no depth.
            //! Use texture_depth_2d / texture_depth_multisampled_2d in WGSL.
            WGPUTextureView getDepthSampleView() const;

            //! Get Sampler.
            WGPUSampler getSampler() const;

            //! Get Extents (same as getSize()).
            WGPUExtent3D getExtent() const;

            //! Get viewport.
            Viewport getViewport() const;

            //! Get scissor.
            ScissorRect getScissor() const;

            //! Get the color format.
            WGPUTextureFormat getFormat() const;

            //! Get the depth/stencil format (Undefined when there is none).
            WGPUTextureFormat getDepthFormat() const;

            //! Begin a render pass that clears color (to the clear color),
            //! depth (to 1) and stencil (to 0).  Pipelines used inside must
            //! match getFormat(), getDepthFormat() and getSampleCount().
            WGPURenderPassEncoder beginClearRenderPass(WGPUCommandEncoder);

            //! Same as above, but clears color to the given value instead of
            //! the buffer's configured clear color.
            WGPURenderPassEncoder beginClearRenderPass(
                WGPUCommandEncoder, const image::Color4f& clearColor);

            //! Begin a render pass that loads color and depth and clears
            //! stencil.  When @p depthReadOnly is true the depth aspect is
            //! bound read-only, which allows the same depth texture to be
            //! sampled in the pass (via getDepthSampleView()) while also
            //! being used for depth testing.
            WGPURenderPassEncoder beginLoadRenderPass(
                WGPUCommandEncoder, bool depthReadOnly = false);

            //! End (and release) a render pass started by one of the
            //! begin*RenderPass() functions.
            void endRenderPass(WGPURenderPassEncoder);

            //! Set up the internal scissor.
            void setupScissor(const math::Box2i&);

            //! Reset the internal viewport and scissor to the full buffer.
            void setupViewportAndScissor();

            //! Reset and apply the internal viewport and scissor to a pass.
            void setupViewportAndScissor(WGPURenderPassEncoder);

            // ----------------------------------------------------------------
            // Read-back (PBO-like)
            //
            // Differences from Vulkan:
            //  * Mapping is asynchronous; results appear after the device's
            //    events are processed (done internally on each call).
            //  * On Emscripten nothing here ever blocks (the browser
            //    delivers map callbacks from its event loop, so a wait loop
            //    would only run into its timeout).  A readback whose ring
            //    slot is still in flight is silently dropped, and
            //    getInlineReadbackPtr() returns nullptr until the data has
            //    arrived: poll it once per frame.
            //  * Texture-to-buffer copies need 256-byte aligned rows.  The
            //    pointers returned below are repacked to tightly packed rows,
            //    so callers see the same layout as before.
            //  * Call these outside of a render pass.
            // ----------------------------------------------------------------

            //! Allocate the staging buffer ring.
            void createStagingBuffers();

            //! Record a copy of a region of the color texture into the next
            //! staging buffer.  Pass w/h of 0 for the full size.  Follow with
            //! submitReadback().
            void readPixels(
                WGPUCommandEncoder, int32_t x = 0, int32_t y = 0,
                uint32_t w = 0, uint32_t h = 0);

            //! Finish and submit the encoder, then start mapping the staging
            //! buffer written by readPixels().  The encoder must not be used
            //! afterwards (the caller still releases it).
            void submitReadback(WGPUCommandEncoder);

            //! Get the oldest outstanding read-back, if it has completed.
            //! Returns true and sets @p ptr (valid until the ring wraps
            //! around to that slot) when ready; false if not ready yet.
            bool getLatestReadPixels(void*& ptr);

            //! Record a copy like readPixels(), but the caller submits the
            //! encoder itself.
            void readPixelsInline(
                WGPUCommandEncoder, int32_t x, int32_t y, uint32_t w,
                uint32_t h);

            //! Get the pixels of the latest readPixelsInline().  Call after
            //! the caller's queue submit.
            //!
            //! Native: blocks (bounded) until the data is mapped; returns
            //! nullptr on timeout/failure.
            //!
            //! Emscripten: never blocks.  Returns nullptr while the data is
            //! still in flight; call again on a later frame (do not record
            //! another readPixelsInline() in between, it would move on to
            //! the next ring slot).
            void* getInlineReadbackPtr();

            static size_t getObjectCount();

            static size_t getTotalByteCount();

        private:
            WGPUInstance _instance = nullptr;
            WGPUDevice _device = nullptr;
            WGPUQueue _queue = nullptr;

            void cleanup();
            void initialize();
            void createColorTextures();
            void createDepthTextures();
            void createSampler();
            WGPURenderPassEncoder _beginRenderPass(
                WGPUCommandEncoder, bool clear, bool depthReadOnly,
                const image::Color4f& clearColor);

            TLRENDER_PRIVATE();
        };

        //! Check whether the offscreen buffer should be created or re-created.
        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>&, const math::Size2i&,
            const OffscreenBufferOptions&);
    } // namespace webgpu
} // namespace tl
