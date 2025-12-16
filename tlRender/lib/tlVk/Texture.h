// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/SamplersCache.h>
#include <tlVk/Vk.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

#include <memory>

namespace tl
{
    namespace vlk
    {
        //! Get the Vulkan's source texture format.
        VkFormat getTextureFormat(image::PixelType);

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
                const VkImageType type,
                const uint32_t width, const uint32_t height,
                const uint32_t depth, const VkFormat format,
                const std::string& name,
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
                const std::string& name = "sampler1",
                const TextureOptions& = TextureOptions());

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

            //! Get the name.
            const std::string& getName() const;

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void setRGBToRGBA(bool t);
            
            void copy(const std::shared_ptr<image::Image>&);
            void copy(const uint8_t*, const image::Info&,
                      const int rowPitch = 0);
            void copy(const uint8_t*, const std::size_t,
                      const int rowPitch = 0);

            //! \@todo:
            void copy(const std::shared_ptr<image::Image>&, int x, int y);

            ///@}
            
            void transition(
                VkCommandBuffer cmd,
                VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VkAccessFlags srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT,
                VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VkAccessFlags dstAccess = VK_ACCESS_SHADER_READ_BIT,
                VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            void transitionToShaderRead(VkCommandBuffer cmd);

            void transitionToColorAttachment(VkCommandBuffer cmd);

            VkDescriptorImageInfo getDescriptorInfo() const;

            //! Vulkan's internal format representation.
            VkFormat getInternalFormat() const;

            //! Image's original source format.
            VkFormat getSourceFormat() const;
            
            VkImageView getImageView() const;

            VkSampler getSampler() const;

            VkImage getImage() const;

            VkImageLayout getImageLayout() const;

        private:
            Fl_Vk_Context& ctx;

            void createImage();
            void allocateMemory();
            void createCommandPool();
            void createImageView();
            void createSampler();

            static uint64_t numTextures;
            static std::unique_ptr<SamplersCache> samplersCache;
            
            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl
