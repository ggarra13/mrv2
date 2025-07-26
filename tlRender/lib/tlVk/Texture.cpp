// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlVk/Texture.h>
#include <tlVk/Vk.h>

#include <tlCore/Assert.h>

#include <FL/Fl_Vk_Utils.H>
#include <FL/vk_enum_string_helper.h>

#include <array>
#include <iostream>
#include <cstdint>
#include <stddef.h>

#define CHATGPT 1

namespace tl
{
    namespace vlk
    {
        namespace
        {
            inline uint32_t byteSwap32(uint32_t x) {
                return ((x >> 24) & 0x000000FF) |
                    ((x >> 8)  & 0x0000FF00) |
                    ((x << 8)  & 0x00FF0000) |
                    ((x << 24) & 0xFF000000);
            }
        
            // Convert from R10G10B10A2 to Vulkan A2R10G10B10 (ChatGPT)
            void convert_R10G10B10A2_to_A2R10G10B10(
                // DPX buffer (R10G10B10A2) - big endian
                const uint32_t* src,
                // Vulkan-compatible buffer (A2R10G10B10)
                uint32_t* dst,
                // number of pixels
                size_t num_pixels 
                )
            {
                for (size_t i = 0; i < num_pixels; ++i)
                {
                    uint32_t pixel = byteSwap32(src[i]);
                    
                    // Correct extraction for R10G10B10A2 layout
                    uint32_t R = (pixel >> 22) & 0x3FF;
                    uint32_t G = (pixel >> 12) & 0x3FF;
                    uint32_t B = (pixel >>  2) & 0x3FF;
                    uint32_t A = (pixel >>  0) & 0x3;

                    // Repack into Vulkan layout
                    dst[i] = (A << 30) | (R << 20) | (G << 10) | B;
                }
            }
        
        }
        
        std::size_t getDataByteCount(
            const VkImageType type, uint32_t w, uint32_t h, uint32_t d,
            VkFormat format)
        {
            std::size_t out = 0;
            switch (type)
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

            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                out = w * h * d * sizeof(uint32_t);
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

                    // YUV_*P_U8
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM

                    // YUV_*P_U10
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM
                    
                    // YUV_*P_U12
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
                    VK_FORMAT_UNDEFINED, // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM

                    // YUV_*P_U16
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

        uint64_t                       Texture::numTextures = 0;
        std::unique_ptr<SamplersCache> Texture::samplersCache;
        
        struct Texture::Private
        {
            image::Info info;

            TextureOptions options;

            std::string name;
            uint32_t arrayLayers = 1; // unused.

            VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkMemoryMapFlags memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            VkImageType imageType = VK_IMAGE_TYPE_2D;
            uint32_t depth = 1;
            VkImage image = VK_NULL_HANDLE;

#ifdef MRV2_NO_VMA
            VkDeviceMemory memory = VK_NULL_HANDLE;
#else
            VmaAllocation allocation = VK_NULL_HANDLE;
#endif

            VkCommandPool commandPool = VK_NULL_HANDLE;
            
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

            createCommandPool();
            createImage();
            allocateMemory();
            createImageView();
            createSampler();
        }

        void Texture::_init(
            const VkImageType type,
            const uint32_t width, const uint32_t height, const uint32_t depth,
            const VkFormat format, const std::string& name,
            const TextureOptions& options)
        {
            TLRENDER_P();

            p.imageType = type;
            p.info.size.w = width;
            p.info.size.h = height;
            p.depth = depth;
            p.format = p.internalFormat = format;
            p.options = options;
            p.name = name;

            switch(p.imageType)
            {
            case VK_IMAGE_TYPE_3D:
            case VK_IMAGE_TYPE_1D:
                p.options.tiling = VK_IMAGE_TILING_OPTIMAL;
                break;
            default:
                break;
            }

            if (p.options.tiling == VK_IMAGE_TILING_OPTIMAL)
                p.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            createCommandPool();
            createImage();
            allocateMemory();
            createImageView();
            createSampler();
        }

        Texture::Texture(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
            if (!samplersCache)
            {
                samplersCache = std::make_unique<SamplersCache>(ctx.device);
            }
            ++numTextures;
        }

        Texture::~Texture()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            if (p.imageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.imageView, nullptr);

#ifdef MRV2_NO_VMA
            if (p.memory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.memory, nullptr);

            if (p.image != VK_NULL_HANDLE)
                vkDestroyImage(device, p.image, nullptr);
#else
            if (p.image != VK_NULL_HANDLE && p.allocation != VK_NULL_HANDLE)
                vmaDestroyImage(ctx.allocator, p.image, p.allocation);
#endif
            if (p.commandPool != VK_NULL_HANDLE)
                vkDestroyCommandPool(device, p.commandPool, nullptr);
            
            --numTextures;
            if (numTextures == 0)
            {
                samplersCache.reset();
            }
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
            const std::string& name, const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
            out->_init(type, width, height, depth, format, name, options);
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

        const std::string& Texture::getName() const
        {
            return _p->name;
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

        void Texture::transition(
            VkCommandBuffer cmd,
            VkImageLayout newLayout, VkAccessFlags srcAccessMask,
            VkPipelineStageFlags srcStageMask, VkAccessFlags dstAccessMask,
            VkPipelineStageFlags dstStageMask)
        {
            TLRENDER_P();

            if (newLayout == p.currentLayout)
                return;

            VkDevice device = ctx.device;

            
            // Determine proper masks
            if (p.currentLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                srcAccessMask = 0;
                dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (p.currentLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                     newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                srcAccessMask = 0;
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (p.currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                     newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (p.currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
                     newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
    
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

            p.currentLayout = newLayout;
        }

        void Texture::transitionToShaderRead(VkCommandBuffer cmd)
        {
            transition(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_ACCESS_SHADER_READ_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        }

        void Texture::transitionToColorAttachment(VkCommandBuffer cmd)
        {
            transition(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_ACCESS_SHADER_READ_BIT, 0,
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0);
        }
        
        void Texture::copy(const std::shared_ptr<image::Image>& data, int x, int y)
        {
            TLRENDER_P();

            const auto& info = data->getInfo();
            const uint8_t* srcData = data->getData();
            const std::size_t size = data->getDataByteCount();

            VkDevice device = ctx.device;
            VkQueue queue = ctx.queue();

#ifdef MRV2_NO_VMA    
            // Create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            VkBufferCreateInfo bufInfo = {};
            bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufInfo.size = size;
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VK_CHECK(vkCreateBuffer(device, &bufInfo, nullptr, &stagingBuffer));

            VkMemoryRequirements memReqs;
            vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                ctx.gpu, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory));
            VK_CHECK(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

            // Copy image data to buffer, respecting alignment
            void* mapped;
            VK_CHECK(vkMapMemory(device, stagingMemory, 0, size, 0, &mapped));
            if (info.layout.alignment == 1) {
                std::memcpy(mapped, srcData, size);
            } else {
                size_t pixelSize = image::getBitDepth(info.pixelType) / 8 * image::getChannelCount(info.pixelType);
                size_t srcRowBytes = info.size.w * pixelSize;
                size_t dstRowBytes = (srcRowBytes + info.layout.alignment - 1) & ~(info.layout.alignment - 1);
                for (uint32_t y = 0; y < info.size.h; ++y) {
                    std::memcpy(
                        static_cast<uint8_t*>(mapped) + y * dstRowBytes,
                        srcData + y * srcRowBytes,
                        srcRowBytes);
                }
            }
            vkUnmapMemory(device, stagingMemory);
#else
            // Create staging buffer
            VkBuffer stagingBuffer;
            VmaAllocation stagingAllocation;

            VkBufferCreateInfo bufInfo = {};
            bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufInfo.size = size;
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            
            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            VK_CHECK(vmaCreateBuffer(ctx.allocator, &bufInfo, &allocInfo,
                                     &stagingBuffer, &stagingAllocation, nullptr));

            // Copy image data to buffer, respecting alignment
            void* mapped;
            VK_CHECK(vmaMapMemory(ctx.allocator, stagingAllocation, &mapped));
            if (info.layout.alignment == 1) {
                std::memcpy(mapped, srcData, size);
            } else {
                size_t pixelSize = image::getBitDepth(info.pixelType) / 8 * image::getChannelCount(info.pixelType);
                size_t srcRowBytes = info.size.w * pixelSize;
                size_t dstRowBytes = (srcRowBytes + info.layout.alignment - 1) & ~(info.layout.alignment - 1);
                for (uint32_t y = 0; y < info.size.h; ++y) {
                    std::memcpy(
                        static_cast<uint8_t*>(mapped) + y * dstRowBytes,
                        srcData + y * srcRowBytes,
                        srcRowBytes);
                }
            }
            vmaUnmapMemory(ctx.allocator, stagingAllocation);
#endif
            // Begin command buffer
            VkCommandBuffer cmd = beginSingleTimeCommands(device,
                                                          p.commandPool);

            // Transition image to transfer dst
            transition(cmd,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT);

            // Prepare region copy
            VkBufferImageCopy region = {};
            region.bufferOffset = 0;
            region.bufferRowLength = info.layout.alignment > 1 ?
                                     ((info.size.w + info.layout.alignment - 1) & ~(info.layout.alignment - 1)) : 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {x, y, 0};
            region.imageExtent = {
                static_cast<uint32_t>(info.size.w),
                static_cast<uint32_t>(info.size.h),
                1};

            vkCmdCopyBufferToImage(
                cmd, stagingBuffer, p.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region);

            // Transition to shader-readable
            transition(cmd,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            {
                std::lock_guard<std::mutex> lock(ctx.queue_mutex());
                endSingleTimeCommands(cmd, device, p.commandPool, queue);
            }
            
#ifdef MRV2_NO_VMA
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingMemory, nullptr);
#else
            vmaDestroyBuffer(ctx.allocator, stagingBuffer, stagingAllocation);
#endif
            p.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        void Texture::copy(const uint8_t* upload, const std::size_t size,
                           const int rowPitch)
        {
            TLRENDER_P();

            uint8_t* data = const_cast<uint8_t*>(upload);
            
            // Assuming 'data' is a pointer to your tightly packed
            // source pixel data
            size_t pixel_size =
                image::getBitDepth(p.info.pixelType) / 8 *
                image::getChannelCount(p.info.pixelType);
            if (p.info.pixelType == image::PixelType::RGB_U10)
            {
                pixel_size = sizeof(uint32_t);
                data = new uint8_t[size];
                unsigned numPixels = p.info.size.w * p.info.size.h;
                const uint32_t* src = reinterpret_cast<const uint32_t*>(upload);
                uint32_t* dst = reinterpret_cast<uint32_t*>(data);
                convert_R10G10B10A2_to_A2R10G10B10(src, dst, numPixels);
            }
            
            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;
            VkQueue queue = ctx.queue();

            // First, check if the memory is host visible
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(device, p.image, &memReqs);

            VkMemoryPropertyFlags memFlags =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            if ((p.memoryFlags & memFlags) == memFlags)
            {
                VkImageSubresource subresource = {};
                subresource.aspectMask =
                    VK_IMAGE_ASPECT_COLOR_BIT; // Or appropriate aspect
                subresource.mipLevel = 0;      // The mip level you are mapping
                subresource.arrayLayer = 0; // The array layer you are mapping

                VkSubresourceLayout subresourceLayout;
                vkGetImageSubresourceLayout(
                    device, p.image, &subresource, &subresourceLayout);

                void* mapped;
                if (rowPitch == 0 && size == subresourceLayout.size)
                {
#ifdef MRV2_NO_VMA
                    // Host-visible upload (like glTexSubImage2D)
                    VK_CHECK(
                        vkMapMemory(device, p.memory, 0, size, 0, &mapped));
                    std::memcpy(mapped, data, size);
                    vkUnmapMemory(device, p.memory);
#else
                    // Host-visible upload (like glTexSubImage2D)
                    VK_CHECK(vmaMapMemory(ctx.allocator, p.allocation, &mapped));
                    std::memcpy(mapped, data, size);
                    vmaUnmapMemory(ctx.allocator, p.allocation);
                    vmaFlushAllocation(ctx.allocator, p.allocation, 0,
                                       VK_WHOLE_SIZE); // Ensure flush
#endif
                }
                else
                {
#ifdef MRV2_NO_VMA
                    // Map based on layout offset and size
                    VK_CHECK(vkMapMemory(
                        device, p.memory, subresourceLayout.offset,
                        subresourceLayout.size, 0, &mapped));
                    const uint32_t src_row_pitch = rowPitch > 0 ? rowPitch : (p.info.size.w * pixel_size);
                    const uint32_t dst_row_size = p.info.size.w * pixel_size;
                    for (uint32_t y = 0;
                         y < static_cast<uint32_t>(p.info.size.h); ++y)
                    {
                        // Source row start in your tightly packed data
                        const void* src_row = static_cast<const uint8_t*>(data) + y * src_row_pitch;

                        // Destination row start in the mapped memory, using the
                        // rowPitch
                        void* dst_row = static_cast<uint8_t*>(mapped) +
                                        y * subresourceLayout.rowPitch;

                        // Copy the actual pixel data for this row (excluding
                        // padding)
                        std::memcpy(dst_row, src_row, dst_row_size);
                    }
                    
                    vkUnmapMemory(device, p.memory);
#else
                    // Map based on layout offset and size
                    VK_CHECK(vmaMapMemory(ctx.allocator, p.allocation, &mapped));

                    // Assuming 'data' is a pointer to your tightly packed
                    // source pixel data
                    const uint32_t src_row_pitch = rowPitch > 0 ? rowPitch : (p.info.size.w * pixel_size);
                    const uint32_t dst_row_size = p.info.size.w * pixel_size;
                    for (uint32_t y = 0;
                         y < static_cast<uint32_t>(p.info.size.h); ++y)
                    {
                        // Source row start in your tightly packed data
                        const void* src_row = static_cast<const uint8_t*>(data) + y * src_row_pitch;

                        // Destination row start in the mapped memory, using the
                        // rowPitch
                        void* dst_row = static_cast<uint8_t*>(mapped) +
                                        y * subresourceLayout.rowPitch;

                        // Copy the actual pixel data for this row (excluding
                        // padding)
                        std::memcpy(dst_row, src_row, dst_row_size);
                    }

                    vmaUnmapMemory(ctx.allocator, p.allocation);
                    vmaFlushAllocation(ctx.allocator, p.allocation, 0,
                                       VK_WHOLE_SIZE); // Ensure flush
#endif
                }
            }
            else
            {
                // Use a staging buffer
                VkBuffer stagingBuffer;

                VkBufferCreateInfo bufInfo = {};
                bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufInfo.size = size;
                bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

#ifdef MRV2_NO_VMA
                VkDeviceMemory stagingMemory;
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
#else
                VmaAllocation stagingAllocation;
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                
                VK_CHECK(vmaCreateBuffer(ctx.allocator, &bufInfo, &allocInfo,
                                         &stagingBuffer, &stagingAllocation, nullptr));

                void* mapped;
                VK_CHECK(vmaMapMemory(ctx.allocator, stagingAllocation, &mapped));
                std::memcpy(mapped, data, size);
                vmaUnmapMemory(ctx.allocator, stagingAllocation);
#endif

                // Transition image for copy
                VkCommandBuffer cmd =
                    beginSingleTimeCommands(device, p.commandPool);
                
                transition(
                    cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);

                // Copy from staging buffer to image

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


                // Transition back to shader layout
                transition(
                    cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


                {
                    std::lock_guard<std::mutex> lock(ctx.queue_mutex());
                    endSingleTimeCommands(cmd, device, p.commandPool, queue);
                }

#ifdef MRV2_NO_VMA
                vkDestroyBuffer(device, stagingBuffer, nullptr);
                vkFreeMemory(device, stagingMemory, nullptr);
#else
                vmaDestroyBuffer(ctx.allocator, stagingBuffer, stagingAllocation);
#endif
                
                p.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            
            if (p.info.pixelType == image::PixelType::RGB_U10)
            {
                delete [] data;
            }
            
        }

        void Texture::copy(const uint8_t* data, const image::Info& info,
                           const int rowPitch)
        {
            const size_t size = image::getDataByteCount(info);
            copy(data, size, rowPitch);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data)
        {
            TLRENDER_P();
            copy(data, 0, 0);
        }

        void Texture::createImage()
        {
            TLRENDER_P();

            VkPhysicalDeviceImageFormatInfo2 formatInfo = {};
            formatInfo.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
            formatInfo.format = p.format;
            formatInfo.type = p.imageType;
            formatInfo.tiling = p.options.tiling;
            formatInfo.usage =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            formatInfo.flags = 0;

            VkImageFormatProperties2 imageProperties = {};
            imageProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

            VkPhysicalDevice gpu = ctx.gpu;

            VkResult result = vkGetPhysicalDeviceImageFormatProperties2(
                gpu, &formatInfo, &imageProperties);

            if (result != VK_SUCCESS)
            {
                switch (p.format)
                {
                case VK_FORMAT_R8G8B8_UNORM:
                    p.internalFormat = VK_FORMAT_R8G8B8A8_UNORM;
                    p.info.pixelType = image::PixelType::RGBA_U8;
                    break;
                case VK_FORMAT_R16G16B16_UNORM:
                    p.internalFormat = VK_FORMAT_R16G16B16A16_UNORM;
                    p.info.pixelType = image::PixelType::RGBA_U16;
                    break;
                case VK_FORMAT_R16G16B16_SFLOAT:
                    p.internalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
                    p.info.pixelType = image::PixelType::RGBA_F16;
                    break;
                default:
                    std::string err = "tl::vlk::Texture Invalid VK_FORMAT: ";
                    throw std::runtime_error(err + string_VkFormat(p.format));
                    break;
                }
#ifndef NDEBUG
                std::cerr << this << ": tried to create texture "
                          << string_VkFormat(p.format)
                          << " but created "
                          << string_VkFormat(p.internalFormat)
                          << std::endl;
#endif
            }

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = p.imageType;
            imageInfo.extent.width = p.info.size.w;
            imageInfo.extent.height = p.info.size.h;
            imageInfo.extent.depth = p.depth == 0 ? p.depth = 1 : p.depth;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = p.arrayLayers;
            imageInfo.format = p.internalFormat;
            imageInfo.tiling = p.options.tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage =
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

#ifdef MRV2_NO_VMA
            VK_CHECK(vkCreateImage(ctx.device, &imageInfo, nullptr, &p.image));
#else
            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = (p.memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                              ? VMA_MEMORY_USAGE_GPU_ONLY
                              : VMA_MEMORY_USAGE_CPU_TO_GPU;

            VK_CHECK(vmaCreateImage(ctx.allocator, &imageInfo, &allocInfo,
                                    &p.image, &p.allocation, nullptr));
#endif
        }

        void Texture::allocateMemory()
        {
            TLRENDER_P();

#ifdef MRV2_NO_VMA
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
#endif
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

        void Texture::createCommandPool()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            // Per-thread command pool
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = ctx.queueFamilyIndex;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            
            vkCreateCommandPool(device, &poolInfo, nullptr, &p.commandPool);
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

            p.sampler = samplersCache->getOrCreateSampler(samplerInfo);
        }

    } // namespace vlk
} // namespace tl
