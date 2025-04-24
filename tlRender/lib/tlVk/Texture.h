// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlVk/Vk.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace vlk
    {
        //! Get the Vulkan internal texture format.
        VkFormat getTextureInternalFormat(image::PixelType);

        //! Vulkan texture options.
        struct TextureOptions
        {
            timeline::ImageFilters filters;
            bool pbo = false;
            VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;

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

            void _init(
                const uint32_t width, const uint32_t height,
                const uint32_t depth, const VkFormat format,
                const TextureOptions& = TextureOptions());

            Texture(Fl_Vk_Context& ctx);

        public:
            ~Texture();

            //! Create a new texture.
            static std::shared_ptr<Texture> create(
                Fl_Vk_Context& ctx, const image::Info&,
                const TextureOptions& = TextureOptions());

            static std::shared_ptr<Texture> create(
                Fl_Vk_Context& ctx, const VkImageType type,
                const uint32_t width, const uint32_t height,
                const uint32_t depth, const VkFormat format,
                const TextureOptions& = TextureOptions());

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

            //! Get the depth.
            int getDepth() const;

            //! Get the pixel type.
            image::PixelType getPixelType() const;

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void copy(const std::shared_ptr<image::Image>&);
            void copy(const uint8_t*, const image::Info&);
            void copy(const uint8_t*, const std::size_t);

            //! \@todo:
            void copy(const std::shared_ptr<image::Image>&, int x, int y);

            ///@}
            
            void transition(
                VkImageLayout newLayout, VkAccessFlags srcAccess,
                VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                VkPipelineStageFlags dstStage);

            VkDescriptorImageInfo getDescriptorInfo() const;

            VkFormat getFormat() const;
            
            VkImageView getImageView() const;

            VkSampler getSampler() const;

            VkImage getImage() const;

            VkImageLayout getImageLayout() const;


            //! Bind the texture.
            void bind();

        private:
            Fl_Vk_Context& ctx;

            void createImage();
            void allocateMemory();
            void createImageView();
            void createSampler();

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl
