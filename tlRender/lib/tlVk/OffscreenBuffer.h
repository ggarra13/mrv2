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
    namespace vk
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

            bool operator==(const OffscreenBufferOptions&) const;
            bool operator!=(const OffscreenBufferOptions&) const;
        };

        //! Offscreen buffer.
        class OffscreenBuffer
            : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLRENDER_NON_COPYABLE(OffscreenBuffer);

        protected:
            void _init(const math::Size2i&,
                       const OffscreenBufferOptions&);

            OffscreenBuffer(Fl_Vk_Context& context);

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer>
            create(Fl_Vk_Context& context, const math::Size2i&,
                   const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const math::Size2i& getSize() const;

            //! Get the offscreen buffer width.
            int getWidth() const;

            //! Get the offscreen buffer height.
            int getHeight() const;

            //! Get the options.
            const OffscreenBufferOptions& getOptions() const;

            //! Vulkan Accessors
            VkImageView      getImageView() const;
            VkImage          getImage() const;
            VkFramebuffer    getFramebuffer() const;
            VkRenderPass     getRenderPass() const;
            VkExtent2D       getExtent() const;
            
        private:
            Fl_Vk_Context& ctx;
            
            void cleanup();
            void initialize();
            void createImage();
            void createImageView();
            void createDepthImage();
            void createDepthImageView();
            void createRenderPass();
            void createFramebuffer();

            uint32_t findMemoryType(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties);
            
            TLRENDER_PRIVATE();
        };

        //! Check whether the offscreen buffer should be created or re-created.
        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>&, const math::Size2i&,
            const OffscreenBufferOptions&);

        //! Offscreen buffer binding.
        class OffscreenBufferBinding
        {
        public:
            explicit OffscreenBufferBinding(
                const std::shared_ptr<OffscreenBuffer>&);

            ~OffscreenBufferBinding();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace vk
} // namespace tl
