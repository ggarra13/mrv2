// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Texture.h>

#include <tlVk/Vk.h>

#include <tlCore/Assert.h>

#include <FL/Fl_Vk_Utils.H>

#include <array>
#include <iostream>

namespace tl
{
    namespace vlk
    {                           
        

        std::size_t getDataByteCount(const VkImageType type,
                                     uint32_t w,
                                     uint32_t h,
                                     uint32_t d,
                                     VkFormat format)
        {
            std::size_t out = 0;
            switch(type)
            {
            case VK_IMAGE_TYPE_1D:
                h = d = 1;
                break;
            case VK_IMAGE_TYPE_2D:
                d = 1;
                break;
            default:
                break;
            }
            
            switch (format)
            {
            case VK_FORMAT_R8_UNORM:
                out = w * h * d * sizeof(uint8_t);
                break;
            case VK_FORMAT_R16_UNORM:
                out = w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32_UINT:
                out = w * h * d * sizeof(uint32_t);
                break;
            case VK_FORMAT_R16_SFLOAT:
                out = w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32_SFLOAT:
                out = w * h * d * sizeof(float);
                break;

            case VK_FORMAT_R8G8_UNORM:
                out = 2 * w * h * d * sizeof(uint8_t);
                break;
            case VK_FORMAT_R16G16_UNORM:
                out = 2 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32_UINT:
                out = 2 * w * h * d * sizeof(uint32_t);
                break;
            case VK_FORMAT_R16G16_SFLOAT:
                out = 2 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32_SFLOAT:
                out = 2 * w * h * d * sizeof(float);
                break;
                
            case VK_FORMAT_R8G8B8_UNORM:
                out = 3 * w * h * d * sizeof(uint8_t);
                break;
            case VK_FORMAT_R16G16B16_UNORM:
                out = 3 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32B32_UINT:
                out = 3 * w * h * d * sizeof(uint32_t);
                break;
            case VK_FORMAT_R16G16B16_SFLOAT:
                out = 3 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32B32_SFLOAT:
                out = 3 * w * h * d * sizeof(float);
                break;
                
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                out = 32 * w * h * d;
                break;
                
            case VK_FORMAT_R8G8B8A8_UNORM:
                out = 4 * w * h * d * sizeof(uint8_t);
                break;
            case VK_FORMAT_R16G16B16A16_UNORM:
                out = 4 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32B32A32_UINT:
                out = 4 * w * h * d * sizeof(uint32_t);
                break;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                out = 4 * w * h * d * sizeof(uint16_t);
                break;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                out = 4 * w * h * d * sizeof(float);
                break;
                
            default:
                break;
            }
            return out;
        }
        

        VkFormat getTextureFormat(image::PixelType type)
        {
            const std::array<
                VkFormat, static_cast<std::size_t>(image::PixelType::Count)>
                data = {
                    VK_FORMAT_UNDEFINED,

                    VK_FORMAT_R8_UNORM,
                    VK_FORMAT_R16_UNORM,
                    VK_FORMAT_R32_UINT,
                    VK_FORMAT_R16_SFLOAT,
                    VK_FORMAT_R32_SFLOAT,

                    VK_FORMAT_R8G8_UNORM,
                    VK_FORMAT_R16G16_UNORM,
                    VK_FORMAT_R32G32_UINT,
                    VK_FORMAT_R16G16_SFLOAT,
                    VK_FORMAT_R32G32_SFLOAT,

                    VK_FORMAT_R8G8B8_UNORM,
                    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
                    VK_FORMAT_R16G16B16_UNORM,
                    VK_FORMAT_R32G32B32_UINT,
                    VK_FORMAT_R16G16B16_SFLOAT,
                    VK_FORMAT_R32G32B32_SFLOAT,

                    VK_FORMAT_R8G8B8A8_UNORM,
                    VK_FORMAT_R16G16B16A16_UNORM,
                    VK_FORMAT_R32G32B32A32_UINT,
                    VK_FORMAT_R16G16B16A16_SFLOAT,
                    VK_FORMAT_R32G32B32A32_SFLOAT,

                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM

                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM

                    VK_FORMAT_R8G8B8A8_UNORM};
            return data[static_cast<std::size_t>(type)];
        }

        bool TextureOptions::operator==(const TextureOptions& other) const
        {
            return filters == other.filters && pbo == other.pbo;
        }

        bool TextureOptions::operator!=(const TextureOptions& other) const
        {
            return !(*this == other);
        }

        VkFilter getTextureFilter(timeline::ImageFilter value)
        {
            const std::array<
                VkFilter,
                static_cast<std::size_t>(timeline::ImageFilter::Count)>
                data = {VK_FILTER_NEAREST, VK_FILTER_LINEAR};
            return data[static_cast<std::size_t>(value)];
        }

        struct Texture::Private
        {
            image::Info info;

            TextureOptions options;

            uint32_t arrayLayers = 1; // unused.

            VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkMemoryMapFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            VkImageType imageType = VK_IMAGE_TYPE_2D;
            uint32_t depth = 1;
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            VkSampler sampler = VK_NULL_HANDLE;

            VkFormat format = VK_FORMAT_UNDEFINED;
            VkFormat internalFormat = VK_FORMAT_UNDEFINED;

            bool hostVisible = true;
        };

        void
        Texture::_init(const image::Info& info, const TextureOptions& options)
        {
            TLRENDER_P();
            p.info = info;
            if (!p.info.isValid())
            {
                throw std::runtime_error("Invalid texture");
            }
            p.format = p.internalFormat = getTextureFormat(info.pixelType);
            p.options = options;

            createImage();
            allocateMemory();
            createImageView();
            createSampler();
        }

        void Texture::_init(
            const uint32_t width, const uint32_t height, const uint32_t depth,
            const VkFormat format, const TextureOptions& options)
        {
            TLRENDER_P();

            p.info.size.w = width;
            p.info.size.h = height;
            p.depth = depth;
            p.format = p.internalFormat = format;
            p.options = options;

            if (depth == 0 && height == 0 && width > 0)
                p.imageType = VK_IMAGE_TYPE_1D;
            else if (depth > 0 && height > 0 && width > 0)
                p.imageType = VK_IMAGE_TYPE_3D;
            else if (height > 0 && width > 0)
                p.imageType = VK_IMAGE_TYPE_2D;
            else
                throw std::runtime_error("Invalid width, height and depth.");

            createImage();
            allocateMemory();
            createImageView();
            createSampler();
        }

        Texture::Texture(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        Texture::~Texture()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            if (p.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, p.sampler, nullptr);

            if (p.imageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.imageView, nullptr);

            if (p.memory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.memory, nullptr);

            if (p.image != VK_NULL_HANDLE)
                vkDestroyImage(device, p.image, nullptr);
        }

        std::shared_ptr<Texture> Texture::create(
            Fl_Vk_Context& ctx, const image::Info& info,
            const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
            out->_init(info, options);
            return out;
        }

        std::shared_ptr<Texture> Texture::create(
            Fl_Vk_Context& ctx, const VkImageType type, const uint32_t width,
            const uint32_t height, const uint32_t depth, const VkFormat format,
            const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
            out->_init(width, height, depth, format, options);
            return out;
        }

        const image::Info& Texture::getInfo() const
        {
            return _p->info;
        }

        const image::Size& Texture::getSize() const
        {
            return _p->info.size;
        }

        int Texture::getWidth() const
        {
            return _p->info.size.w;
        }

        int Texture::getHeight() const
        {
            return _p->info.size.h;
        }

        int Texture::getDepth() const
        {
            return _p->depth;
        }

        image::PixelType Texture::getPixelType() const
        {
            return _p->info.pixelType;
        }

        VkFormat Texture::getSourceFormat() const
        {
            return _p->format;
        }
        
        VkFormat Texture::getInternalFormat() const
        {
            return _p->internalFormat;
        }
        
        VkImageView Texture::getImageView() const
        {
            return _p->imageView;
        }

        VkSampler Texture::getSampler() const
        {
            return _p->sampler;
        }

        VkImage Texture::getImage() const
        {
            return _p->image;
        }

        VkDescriptorImageInfo Texture::getDescriptorInfo() const
        {
            TLRENDER_P();

            VkDescriptorImageInfo out = {};
            out.sampler = p.sampler;
            out.imageView = p.imageView;
            out.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            return out;
        }

        VkImageLayout Texture::getImageLayout() const
        {
            return _p->currentLayout;
        }

        unsigned int Texture::getID() const
        {
            return 0;
        }

        void Texture::transition(
            VkImageLayout newLayout, VkAccessFlags srcAccessMask,
            VkPipelineStageFlags srcStageMask, VkAccessFlags dstAccessMask,
            VkPipelineStageFlags dstStageMask)
        {
            TLRENDER_P();

            if (newLayout == p.currentLayout)
                return;

            VkDevice device = ctx.device;
            VkCommandPool commandPool = ctx.commandPool;
            VkQueue queue = ctx.queue;

            VkCommandBuffer cmd = beginSingleTimeCommands(device, commandPool);

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = p.currentLayout;
            barrier.newLayout = newLayout;
            barrier.srcAccessMask = srcAccessMask;
            barrier.dstAccessMask = dstAccessMask;
            barrier.image = p.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            if (p.imageType == VK_IMAGE_TYPE_3D)
                barrier.subresourceRange.layerCount = 1;
            else
                barrier.subresourceRange.layerCount = p.arrayLayers;

            vkCmdPipelineBarrier(
                cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
                &barrier);

            endSingleTimeCommands(cmd, device, commandPool, queue);

            p.currentLayout = newLayout;
        }

        //! Used in TextureAtlas
        void
        Texture::copy(const std::shared_ptr<image::Image>& data, int x, int y)
        {
            TLRENDER_P();
#if defined(TLRENDER_API_GL_4_1)
            // if (p.pbo)
            // {
            // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
            // if (void* buffer =
            //         glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
            // {
            //     memcpy(buffer, data->getData(),
            //     data->getDataByteCount());
            //     glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            //     const auto& info = data->getInfo();
            //     glBindTexture(GL_TEXTURE_2D, p.id);
            //     glPixelStorei(GL_UNPACK_ALIGNMENT,
            //     info.layout.alignment); glPixelStorei(
            //         GL_UNPACK_SWAP_BYTES,
            //         info.layout.endian != memory::getEndian());
            //     glTexSubImage2D(
            //         GL_TEXTURE_2D, 0, x, y, info.size.w, info.size.h,
            //         getTextureFormat(info.pixelType),
            //         getTextureType(info.pixelType), NULL);
            // }
            // glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            // }
            // else
#endif // TLRENDER_API_GL_4_1
            {
                const auto& info = data->getInfo();
                //                 glBindTexture(GL_TEXTURE_2D, p.id);
                //                 glPixelStorei(GL_UNPACK_ALIGNMENT,
                //                 info.layout.alignment);
                // #if defined(TLRENDER_API_GL_4_1)
                //                 glPixelStorei(
                //                     GL_UNPACK_SWAP_BYTES,
                //                     info.layout.endian !=
                //                     memory::getEndian());
                // #endif // TLRENDER_API_GL_4_1
                //                 glTexSubImage2D(
                //                     GL_TEXTURE_2D, 0, x, y, info.size.w,
                //                     info.size.h,
                //                     getTextureFormat(info.pixelType),
                //                     getTextureType(info.pixelType),
                //                     data->getData());
            }
        }

        void Texture::copy(const uint8_t* data, const std::size_t size)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            VkCommandPool commandPool = ctx.commandPool;
            VkPhysicalDevice gpu = ctx.gpu;
            VkQueue queue = ctx.queue;

            // First, check if the memory is host visible
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(device, p.image, &memReqs);

            VkMemoryPropertyFlags memFlags =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            if ((p.memoryFlags & memFlags) == memFlags)
            {
                VkImageSubresource subresource = {};
                subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Or appropriate aspect
                subresource.mipLevel = 0; // The mip level you are mapping
                subresource.arrayLayer = 0; // The array layer you are mapping

                VkSubresourceLayout subresourceLayout;
                vkGetImageSubresourceLayout(device, p.image, &subresource,
                                            &subresourceLayout);
                
                void* mapped;
                if (size == subresourceLayout.size)
                {
                    // Host-visible upload (like glTexSubImage2D)
                    VK_CHECK(vkMapMemory(device, p.memory, 0, size, 0, &mapped));
                    std::memcpy(mapped, data, size);
                    vkUnmapMemory(device, p.memory);
                }
                else
                {
                    // Map based on layout offset and size
                    VK_CHECK(vkMapMemory(device, p.memory, subresourceLayout.offset,
                                         subresourceLayout.size, 0, &mapped));

                    // Assuming 'data' is a pointer to your tightly packed source pixel data
                    size_t pixel_size = image::getBitDepth(p.info.pixelType) / 8 *
                                        image::getChannelCount(p.info.pixelType);
                    uint32_t row_byte_size = static_cast<uint32_t>(p.info.size.w) * pixel_size;
                    for (uint32_t y = 0; y < static_cast<uint32_t>(p.info.size.h); ++y) {
                        // Source row start in your tightly packed data
                        const void* src_row = static_cast<const uint8_t*>(data) + y * row_byte_size;

                        // Destination row start in the mapped memory, using the rowPitch
                        void* dst_row = static_cast<uint8_t*>(mapped) + y * subresourceLayout.rowPitch;

                        // Copy the actual pixel data for this row (excluding padding)
                        std::memcpy(dst_row, src_row, row_byte_size);
                    }

                    vkUnmapMemory(device, p.memory);
                }
            }
            else
            {
                // Use a staging buffer
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingMemory;

                VkBufferCreateInfo bufInfo = {};
                bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufInfo.size = size;
                bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

                vkCreateBuffer(device, &bufInfo, nullptr, &stagingBuffer);

                VkMemoryRequirements bufMemReqs;
                vkGetBufferMemoryRequirements(
                    device, stagingBuffer, &bufMemReqs);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = bufMemReqs.size;
                allocInfo.memoryTypeIndex = findMemoryType(
                    gpu, bufMemReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory);
                vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

                void* mapped;
                vkMapMemory(device, stagingMemory, 0, size, 0, &mapped);
                std::memcpy(mapped, data, size);
                vkUnmapMemory(device, stagingMemory);

                // Transition image for copy
                transition(
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);

                // Copy from staging buffer to image
                VkCommandBuffer cmd =
                    beginSingleTimeCommands(device, commandPool);

                VkBufferImageCopy region = {};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;

                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;

                region.imageOffset = {0, 0, 0};
                region.imageExtent = {
                    static_cast<uint32_t>(p.info.size.w),
                    static_cast<uint32_t>(p.info.size.h), p.depth};

                vkCmdCopyBufferToImage(
                    cmd, stagingBuffer, p.image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                endSingleTimeCommands(cmd, device, commandPool, queue);

                // Transition back to shader layout
                transition(
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

                vkDestroyBuffer(device, stagingBuffer, nullptr);
                vkFreeMemory(device, stagingMemory, nullptr);
            }
        }

        void Texture::copy(const uint8_t* data, const image::Info& info)
        {
            const size_t size = image::getDataByteCount(info);
            copy(data, size);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data)
        {
            TLRENDER_P();
            copy(data->getData(), data->getDataByteCount());
        }
        
        void Texture::bind()
        {
            // glBindTexture(GL_TEXTURE_2D, _p->id);
        }

        void Texture::createImage()
        {
            TLRENDER_P();

            VkPhysicalDeviceImageFormatInfo2 formatInfo = {};
            formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
            formatInfo.format = p.format;
            formatInfo.type = p.imageType;
            formatInfo.tiling = p.options.tiling;
            formatInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                               VK_IMAGE_USAGE_SAMPLED_BIT;
            formatInfo.flags = 0;

            VkImageFormatProperties2 imageProperties = {};
            imageProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

            VkPhysicalDevice gpu = ctx.gpu;
            
            VkResult result = vkGetPhysicalDeviceImageFormatProperties2(
                gpu,
                &formatInfo,
                &imageProperties
                );
            
            if (result != VK_SUCCESS)
            {
                switch(p.format)
                {
                case VK_FORMAT_R8G8B8_UNORM:
                    p.internalFormat = VK_FORMAT_R8G8B8A8_UNORM;
                    break;
                case VK_FORMAT_R16G16B16_UNORM:
                    p.internalFormat = VK_FORMAT_R16G16B16A16_UNORM;
                    break;
                case VK_FORMAT_R16G16B16_SFLOAT:
                    p.internalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
                    break;
                }
            }
            
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = p.imageType;
            imageInfo.extent.width = p.info.size.w;
            imageInfo.extent.height = p.info.size.h;
            imageInfo.extent.depth = p.depth;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = p.arrayLayers;
            imageInfo.format = p.internalFormat;
            imageInfo.tiling = p.options.tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage =
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VK_CHECK(vkCreateImage(ctx.device, &imageInfo, nullptr, &p.image));
        }

        void Texture::allocateMemory()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;

            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(device, p.image, &memReqs);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;

            allocInfo.memoryTypeIndex =
                findMemoryType(gpu, memReqs.memoryTypeBits, p.memoryFlags);

            VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &p.memory));
            VK_CHECK(vkBindImageMemory(device, p.image, p.memory, 0));
        }

        void Texture::createImageView()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = p.image;

            switch (p.imageType)
            {
            case VK_IMAGE_TYPE_1D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
                break;
            case VK_IMAGE_TYPE_2D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                break;
            case VK_IMAGE_TYPE_3D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            default:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                break;
            }

            viewInfo.format = p.internalFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VK_CHECK(
                vkCreateImageView(device, &viewInfo, nullptr, &p.imageView));
        }

        void Texture::createSampler()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = getTextureFilter(p.options.filters.magnify);
            samplerInfo.minFilter = getTextureFilter(p.options.filters.minify);
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            VK_CHECK(
                vkCreateSampler(device, &samplerInfo, nullptr, &p.sampler));
        }

    } // namespace vlk
} // namespace tl
