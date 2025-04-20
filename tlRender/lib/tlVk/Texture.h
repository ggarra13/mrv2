// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlVk/Vk.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace vk
    {
        //! Get the Vulkan internal texture format.
        VkFormat    getTextureInternalFormat(image::PixelType);

        //! Vulkan texture options.
        struct TextureOptions
        {
            timeline::ImageFilters filters;
            bool pbo = false;

            bool operator==(const TextureOptions&) const;
            bool operator!=(const TextureOptions&) const;
        };

        //! Get the Vulkan texture filter.
        VkFilter getTextureFilter(timeline::ImageFilter);

        //! Vulkan texture.
        class Texture : public std::enable_shared_from_this<Texture>
        {
            TLRENDER_NON_COPYABLE(Texture);

        protected:
            void
            _init(const image::Info&, const TextureOptions& = TextureOptions());

            Texture();

        public:
            ~Texture();

            //! Create a new texture.
            static std::shared_ptr<Texture> create(
                const image::Info&, const TextureOptions& = TextureOptions());

            //! Get the Vulkan texture ID.
            unsigned int getID() const;

            //! Get the image information.
            const image::Info& getInfo() const;

            //! Get the size.
            const image::Size& getSize() const;

            //! Get the width.
            int getWidth() const;

            //! Get the height.
            int getHeight() const;

            //! Get the pixel type.
            image::PixelType getPixelType() const;

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void copy(const std::shared_ptr<image::Image>&);
            void copy(const std::shared_ptr<image::Image>&, int x, int y);
            void copy(const uint8_t*, const image::Info&);

            ///@}

            //! Bind the texture.
            void bind();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace vk
} // namespace tl
