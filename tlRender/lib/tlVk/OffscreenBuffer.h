// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

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
            None,
            _16,
            _24,
            _32,

            Count,
            First = None
        };
        TLRENDER_ENUM(OffscreenDepth);
        TLRENDER_ENUM_SERIALIZE(OffscreenDepth);

        //! Default offscreen buffer color type.
#if defined(TLRENDER_API_GL_4_1)
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_16;
#elif defined(TLRENDER_API_GLES_2)
        const OffscreenDepth offscreenDepthDefault = OffscreenDepth::_24;
#endif // TLRENDER_API_GL_4_1

        //! Offscreen buffer stencil size.
        enum class OffscreenStencil {
            None,
            _8,

            Count,
            First = None
        };
        TLRENDER_ENUM(OffscreenStencil);
        TLRENDER_ENUM_SERIALIZE(OffscreenStencil);

        //! Offscreen buffer multisampling.
        enum class OffscreenSampling {
            None,
            _2,
            _4,
            _8,
            _16,

            Count,
            First = None
        };
        TLRENDER_ENUM(OffscreenSampling);
        TLRENDER_ENUM_SERIALIZE(OffscreenSampling);

        //! Offscreen buffer options.
        struct OffscreenBufferOptions
        {
            image::PixelType colorType = image::PixelType::None;
            timeline::ImageFilters colorFilters;
            OffscreenDepth depth = OffscreenDepth::None;
            OffscreenStencil stencil = OffscreenStencil::None;
            OffscreenSampling sampling = OffscreenSampling::None;

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

            OffscreenBuffer();

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer>
            create(const math::Size2i&, const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const math::Size2i& getSize() const;

            //! Get the offscreen buffer width.
            int getWidth() const;

            //! Get the offscreen buffer height.
            int getHeight() const;

            //! Get the options.
            const OffscreenBufferOptions& getOptions() const;

            //! Get the offscreen buffer ID.
            unsigned int getID() const;

            //! Get the color texture ID.
            unsigned int getColorID() const;

            //! Bind the offscreen buffer.
            void bind();

        private:
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
