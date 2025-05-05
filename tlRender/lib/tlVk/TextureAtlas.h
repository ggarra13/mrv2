// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlVk/Texture.h>
#include <tlVk/Vk.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>
#include <tlCore/Range.h>

namespace tl
{
    namespace vlk
    {
        //! Texture atlas item.
        struct TextureAtlasItem
        {
            int w = 0;
            int h = 0;
            uint8_t textureIndex = 0;
            math::FloatRange textureU;
            math::FloatRange textureV;
        };

        //! Texture atlas item ID.
        typedef uint64_t TextureAtlasID;

        //! OpenGL texture atlas.
        class TextureAtlas : public std::enable_shared_from_this<TextureAtlas>
        {
            TLRENDER_NON_COPYABLE(TextureAtlas);

        protected:
            void _init(
                size_t textureCount, int textureSize,
                image::PixelType textureType, timeline::ImageFilter filter,
                int border);

            TextureAtlas(Fl_Vk_Context& ctx);

        public:
            ~TextureAtlas();

            //! Create a new texture atlas.
            static std::shared_ptr<TextureAtlas> create(
                Fl_Vk_Context& ctx, size_t textureCount, int textureSize,
                image::PixelType textureType,
                timeline::ImageFilter filter = timeline::ImageFilter::Linear,
                int border = 1);

            size_t getTextureCount() const;
            int getTextureSize() const;
            image::PixelType getTextureType() const;
            std::vector<std::shared_ptr<vlk::Texture> > getTextures() const;

            bool getItem(TextureAtlasID, TextureAtlasItem&);

            TextureAtlasID
            addItem(const std::shared_ptr<image::Image>&, TextureAtlasItem&);

            float getPercentageUsed() const;

        private:
            Fl_Vk_Context& ctx;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl
