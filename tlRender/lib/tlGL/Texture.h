// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace gl
    {
        //! Get the OpenGL texture format.
        unsigned int getTextureFormat(image::PixelType);

        //! Get the OpenGL internal texture format.
        unsigned int getTextureInternalFormat(image::PixelType);

        //! Get the OpenGL texture type.
        unsigned int getTextureType(image::PixelType);

        //! OpenGL texture options.
        struct TextureOptions
        {
            timeline::ImageFilters filters;
            bool pbo = false;

            bool operator==(const TextureOptions&) const;
            bool operator!=(const TextureOptions&) const;
        };

        //! Get the OpenGL texture filter.
        unsigned int getTextureFilter(timeline::ImageFilter);

        //! OpenGL texture.
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

            //! Get the OpenGL texture ID.
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
            void copy(const uint8_t*, const image::Info&, const int rowStride);

            ///@}

            //! Bind the texture.
            void bind();

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace gl
} // namespace tl
