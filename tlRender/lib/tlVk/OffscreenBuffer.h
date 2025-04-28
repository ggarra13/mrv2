// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Vk.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>
#include <tlCore/Size.h>

namespace tl
{
    namespace vlk
    {
        //! Default offscreen buffer color type.
        const image::PixelType offscreenColorDefault =
            image::PixelType::RGBA_U8;

        //! Offscreen buffer depth size.
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

        //! Default offscreen buffer color type.
#if defined(TLRENDER_API_GL_4_1)
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_16;
#elif defined(TLRENDER_API_GLES_2)
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_24;
#else
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_16;
#endif // TLRENDER_API_GL_4_1

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
            image::PixelType colorType = image::PixelType::kNone;
            timeline::ImageFilters colorFilters;
            OffscreenDepth depth = OffscreenDepth::kNone;
            OffscreenStencil stencil = OffscreenStencil::kNone;
            OffscreenSampling sampling = OffscreenSampling::kNone;
            bool      allowCompositing = false;

            bool operator==(const OffscreenBufferOptions&) const;
            bool operator!=(const OffscreenBufferOptions&) const;
        };

        //! Offscreen buffer.
        class OffscreenBuffer
            : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLRENDER_NON_COPYABLE(OffscreenBuffer);

        protected:
            void _init(const math::Size2i&, const OffscreenBufferOptions&);

            OffscreenBuffer(Fl_Vk_Context& context);

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                Fl_Vk_Context& context, const math::Size2i&,
                const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const math::Size2i& getSize() const;

            //! Get the offscreen buffer width.
            int getWidth() const;

            //! Get the offscreen buffer height.
            int getHeight() const;

            //! Returns true if the buffer has depth.
            bool hasDepth() const;

            //! Returns true if the buffer has depth.
            bool hasStencil() const;

            //! Get the options.
            const OffscreenBufferOptions& getOptions() const;

            //! Vulkan Accessors
            VkImageLayout getImageLayout() const;

            VkImageLayout getDepthLayout() const;
            
            VkImageView getImageView() const;
            VkImage getImage() const;
            
            VkFramebuffer getFramebuffer() const;
            VkRenderPass getRenderPass() const;
            
            VkFramebuffer getCompositingFramebuffer() const;
            VkRenderPass getCompositingRenderPass() const;
            
            VkSampler getSampler() const;
            VkExtent2D getExtent() const;
            VkViewport getViewport() const;
            VkRect2D getScissor() const;
            
            //! Start/end a normal render pass.
            void beginRenderPass(VkCommandBuffer cmd,
                                 VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
            void endRenderPass(VkCommandBuffer cmd);

            //! Start/end a compositing pass.
            void beginCompositingRenderPass(VkCommandBuffer cmd,
                                            VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
            void endCompositingRenderPass(VkCommandBuffer cmd);
            
            //! Offscreen image transitions.
            void transitionToShaderRead(VkCommandBuffer cmd);
            void transitionToColorAttachment(VkCommandBuffer cmd);
            
            void transitionDepthToShaderRead(VkCommandBuffer cmd);
            
            //! Set up the internal viewport and scissor.
            void setupViewportAndScissor();

            //! Apply internal viewport and scissor to command buffer.
            void setupViewportAndScissor(VkCommandBuffer cmd);

        private:
            Fl_Vk_Context& ctx;

            void cleanup();
            void initialize();
            void createImage();
            void createImageView();
            void createDepthImage();
            void createDepthImageView();
            void createRenderPass(VkAttachmentLoadOp colorLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                  VkAttachmentStoreOp colorStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                                  VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                  VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
            void createCompositingRenderPass();
            void createCompositingFramebuffer();
            void createFramebuffer();
            void createSampler();

            uint32_t findMemoryType(
                uint32_t typeFilter, VkMemoryPropertyFlags properties);

            TLRENDER_PRIVATE();
        };

        //! Check whether the offscreen buffer should be created or re-created.
        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>&, const math::Size2i&,
            const OffscreenBufferOptions&);
    } // namespace vlk
} // namespace tl
