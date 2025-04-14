// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_LIBPLACEBO)
extern "C"
{
#    include <libplacebo/shaders/colorspace.h>
#    include <libplacebo/shaders.h>
}
#endif

#include <array>
#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

namespace
{
    VkFormat to_vk_format(pl_fmt fmt)
    {
        int size = fmt->internal_size / fmt->num_components;

        switch (fmt->num_components)
        {
        case 1:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16_SFLOAT : VK_FORMAT_R32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32_UINT
                                 : (size == 2 ? VK_FORMAT_R16_UINT
                                              : VK_FORMAT_R8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32_SINT
                                 : (size == 2 ? VK_FORMAT_R16_SINT
                                              : VK_FORMAT_R8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16_UNORM : VK_FORMAT_R8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16_SNORM : VK_FORMAT_R8_SNORM;
            }
            break;
        case 2:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16_SFLOAT
                                 : VK_FORMAT_R32G32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16_UINT
                                              : VK_FORMAT_R8G8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16_SINT
                                              : VK_FORMAT_R8G8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_UNORM
                                 : VK_FORMAT_R8G8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16_SNORM
                                 : VK_FORMAT_R8G8_SNORM;
            }
            break;
        case 3:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SFLOAT
                                 : VK_FORMAT_R32G32B32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16_UINT
                                              : VK_FORMAT_R8G8B8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16_SINT
                                              : VK_FORMAT_R8G8B8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_UNORM
                                 : VK_FORMAT_R8G8B8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16_SNORM
                                 : VK_FORMAT_R8G8B8_SNORM;
            }
            break;
        case 4:
            if (fmt->type == PL_FMT_FLOAT)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SFLOAT
                                 : VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            else if (fmt->type == PL_FMT_UINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_UINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16A16_UINT
                                              : VK_FORMAT_R8G8B8A8_UINT);
            }
            else if (fmt->type == PL_FMT_SINT)
            {
                return size == 4 ? VK_FORMAT_R32G32B32A32_SINT
                                 : (size == 2 ? VK_FORMAT_R16G16B16A16_SINT
                                              : VK_FORMAT_R8G8B8A8_SINT);
            }
            else if (fmt->type == PL_FMT_UNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_UNORM
                                 : VK_FORMAT_R8G8B8A8_UNORM;
            }
            else if (fmt->type == PL_FMT_SNORM)
            {
                return size == 2 ? VK_FORMAT_R16G16B16A16_SNORM
                                 : VK_FORMAT_R8G8B8A8_SNORM;
            }
            break;
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkImage createImage(
        VkImageType imageType, uint32_t width, uint32_t height, uint32_t depth,
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
    {

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = imageType;
        imageInfo.format = format;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image;
        if (vkCreateImage(ctx.device, &imageInfo, nullptr, &image) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image");
        }
        return image;
    }

    VkDeviceMemory allocateAndBindImageMemory(
        VkPhysicalDevice gpu, VkDevice device, VkImage image)
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

        uint32_t memoryTypeIndex = 0;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            {
                memoryTypeIndex = i;
                break;
            }
        }

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        VkDeviceMemory imageMemory;
        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
        return imageMemory;
    }

    VkCommandBuffer NDIView::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_cmd_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(ctx.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void NDIView::endSingleTimeCommands(
        VkDevice device, VkQueue queue, VkCommandPool commandPool,
        VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void transitionImageLayout(
        VkDevice device, VkQueue queue, VkCommandPool commandPool,
        VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (
            oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::runtime_error("Unsupported layout transition");
        }

        vkCmdPipelineBarrier(
            commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0,
            nullptr, 1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

}

namespace tl
{
    namespace timeline_vk
    {
        namespace
        {
            const int pboSizeMin = 1024;
        }

        std::vector<std::shared_ptr<vk::Texture> > getTextures(
            const image::Info& info, const timeline::ImageFilters& imageFilters,
            size_t offset)
        {
            std::vector<std::shared_ptr<vk::Texture> > out;
            vk::TextureOptions options;
            options.filters = imageFilters;
            options.pbo =
                info.size.w >= pboSizeMin || info.size.h >= pboSizeMin;
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U8);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vk::Texture::create(infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U8);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h / 2),
                    image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(
                    image::Size(info.size.w / 2, info.size.h),
                    image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                out.push_back(vk::Texture::create(ctx, infoTmp, options));
                break;
            }
            default:
            {
                auto texture = vk::Texture::create(ctx, info, options);
                out.push_back(texture);
                break;
            }
            }
            return out;
        }

        void copyTextures(
            const std::shared_ptr<image::Image>& image,
            const std::vector<std::shared_ptr<vk::Texture> >& textures,
            size_t offset)
        {
            const auto& info = image->getInfo();
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h2),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w2 * h),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    textures[1]->copy(
                        image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) + (w * h),
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(
                        image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) * 2 + (w2 * h2) * 2,
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(
                        image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) * 2 + (w2 * h) * 2,
                        textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    textures[1]->copy(
                        image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(
                        image->getData() + (w * h) * 2 + (w * h) * 2,
                        textures[2]->getInfo());
                }
                break;
            }
            default:
                if (1 == textures.size())
                {
                    textures[0]->copy(image);
                }
                break;
            }
        }

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<vk::Texture> >& textures,
            size_t offset)
        {
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U8:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U8:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_420P_U16:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U16:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U16:
                if (3 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // textures[1]->bind();
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // textures[2]->bind();
                }
                break;
            default:
                if (1 == textures.size())
                {
                    // glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 +
                    // offset)); textures[0]->bind(); glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    // glBindTexture(GL_TEXTURE_2D, 0);
                    // glActiveTexture(
                    //     static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    // glBindTexture(GL_TEXTURE_2D, 0);
                }
                break;
            }
        }

#if defined(TLRENDER_OCIO) || defined(TLRENDER_LIBPLACEBO)
        OCIOTexture::OCIOTexture(
            unsigned id, std::string name, std::string sampler, unsigned type) :
            id(id),
            name(name),
            sampler(sampler),
            type(type)
        {
        }
#endif

#if defined(TLRENDER_OCIO)
        OCIOData::~OCIOData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                // glDeleteTextures(1, &textures[i].id);
            }
        }

        OCIOLUTData::~OCIOLUTData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                // glDeleteTextures(1, &textures[i].id);
            }
        }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        LibPlaceboData::LibPlaceboData()
        {
            log = pl_log_create(ctx, PL_API_VER, NULL);
            if (!log)
            {
                throw std::runtime_error("log creation failed");
            }

            struct pl_gpu_dummy_params gpu_dummy;
            memset(&gpu_dummy, 0, sizeof(pl_gpu_dummy_params));

            gpu_dummy.glsl.version = 410;
            gpu_dummy.glsl.gles = false;
            gpu_dummy.glsl.vulkan = false;
            gpu_dummy.glsl.compute = false;

            gpu_dummy.limits.callbacks = false;
            gpu_dummy.limits.thread_safe = true;
            /* pl_buf */
            gpu_dummy.limits.max_buf_size = SIZE_MAX;
            gpu_dummy.limits.max_ubo_size = SIZE_MAX;
            gpu_dummy.limits.max_ssbo_size = SIZE_MAX;
            gpu_dummy.limits.max_vbo_size = SIZE_MAX;
            gpu_dummy.limits.max_mapped_size = SIZE_MAX;
            gpu_dummy.limits.max_buffer_texels = UINT64_MAX;
            /* pl_tex */
            gpu_dummy.limits.max_tex_1d_dim = UINT32_MAX;
            gpu_dummy.limits.max_tex_2d_dim = UINT32_MAX;
            gpu_dummy.limits.max_tex_3d_dim = UINT32_MAX;
            gpu_dummy.limits.buf_transfer = true;
            gpu_dummy.limits.align_tex_xfer_pitch = 1;
            gpu_dummy.limits.align_tex_xfer_offset = 1;

            /* pl_pass */
            gpu_dummy.limits.max_variable_comps = SIZE_MAX;
            gpu_dummy.limits.max_constants = SIZE_MAX;
            gpu_dummy.limits.max_pushc_size = SIZE_MAX;
            gpu_dummy.limits.max_dispatch[0] = UINT32_MAX;
            gpu_dummy.limits.max_dispatch[1] = UINT32_MAX;
            gpu_dummy.limits.max_dispatch[2] = UINT32_MAX;
            gpu_dummy.limits.fragment_queues = 0;
            gpu_dummy.limits.compute_queues = 0;

            gpu = pl_gpu_dummy_create(ctx, log, &gpu_dummy);
            if (!gpu)
            {
                throw std::runtime_error("pl_gpu_dummy_create failed!");
            }
        }

        LibPlaceboData::~LibPlaceboData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                // glDeleteTextures(1, &textures[i].id);
            }

            pl_gpu_dummy_destroy(&gpu);
            pl_log_destroy(&log);
        }
#endif // TLRENDER_LIBPLACEBO

        void Render::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<TextureCache>& textureCache)
        {
            IRender::_init(context);
            TLRENDER_P();

            p.textureCache = textureCache;
            if (!p.textureCache)
            {
                p.textureCache = std::make_shared<TextureCache>();
            }

            p.glyphTextureAtlas = vk::TextureAtlas::create(
                ctx, 1, 4096, image::PixelType::L_U8,
                timeline::ImageFilter::Linear);

            p.logTimer = std::chrono::steady_clock::now();
        }

        Render::Render() :
            _p(new Private)
        {
        }

        Render::~Render() {}

        std::shared_ptr<Render> Render::create(
            ctx, const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<TextureCache>& textureCache)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(context, textureCache);
            return out;
        }

        const std::shared_ptr<TextureCache>& Render::getTextureCache() const
        {
            return _p->textureCache;
        }

        void Render::begin(
            const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.timer = std::chrono::steady_clock::now();

            p.renderSize = renderSize;
            p.renderOptions = renderOptions;
            p.textureCache->setMax(renderOptions.textureCacheByteCount);

            // glEnable(GL_BLEND);
            // glBlendEquation(GL_FUNC_ADD);

            if (!p.shaders["rect"])
            {
                p.shaders["rect"] = vk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["mesh"])
            {
                p.shaders["mesh"] = vk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["colorMesh"])
            {
                p.shaders["colorMesh"] = vk::Shader::create(
                    ctx, colorMeshVertexSource(), colorMeshFragmentSource());
            }
            if (!p.shaders["text"])
            {
                p.shaders["text"] = vk::Shader::create(
                    ctx, vertexSource(), textFragmentSource());
            }
            if (!p.shaders["texture"])
            {
                p.shaders["texture"] = vk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource());
            }
            if (!p.shaders["image"])
            {
                p.shaders["image"] = vk::Shader::create(
                    ctx, vertexSource(), imageFragmentSource());
            }
            if (!p.shaders["wipe"])
            {
                p.shaders["wipe"] = vk::Shader::create(
                    ctx, vertexSource(), meshFragmentSource());
            }
            if (!p.shaders["overlay"])
            {
                p.shaders["overlay"] = vk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource());
            }
            if (!p.shaders["difference"])
            {
                p.shaders["difference"] = vk::Shader::create(
                    ctx, vertexSource(), differenceFragmentSource());
            }
            if (!p.shaders["dissolve"])
            {
                p.shaders["dissolve"] = vk::Shader::create(
                    ctx, vertexSource(), textureFragmentSource());
            }
            _displayShader();

            p.vbos["rect"] = vk::VBO::create(ctx, 2 * 3, vk::VBOType::Pos2_F32);
            p.vaos["rect"] = vk::VAO::create(
                ctx, p.vbos["rect"]->getType(), p.vbos["rect"]->getID());
            p.vbos["texture"] =
                vk::VBO::create(ctx, 2 * 3, vk::VBOType::Pos2_F32_UV_U16);
            p.vaos["texture"] = vk::VAO::create(
                ctx, p.vbos["texture"]->getType(), p.vbos["texture"]->getID());
            p.vbos["image"] =
                vk::VBO::create(ctx, 2 * 3, vk::VBOType::Pos2_F32_UV_U16);
            p.vaos["image"] = vk::VAO::create(
                ctx, p.vbos["image"]->getType(), p.vbos["image"]->getID());
            p.vbos["wipe"] = vk::VBO::create(ctx, 1 * 3, vk::VBOType::Pos2_F32);
            p.vaos["wipe"] = vk::VAO::create(
                ctx, p.vbos["wipe"]->getType(), p.vbos["wipe"]->getID());
            p.vbos["video"] =
                vk::VBO::create(ctx, 2 * 3, vk::VBOType::Pos2_F32_UV_U16);
            p.vaos["video"] = vk::VAO::create(
                ctx, p.vbos["video"]->getType(), p.vbos["video"]->getID());

            setViewport(math::Box2i(0, 0, renderSize.w, renderSize.h));
            if (renderOptions.clear)
            {
                clearViewport(renderOptions.clearColor);
            }
            setTransform(math::ortho(
                0.F, static_cast<float>(renderSize.w),
                static_cast<float>(renderSize.h), 0.F, -1.F, 1.F));
        }

        void Render::end()
        {
            TLRENDER_P();

            //! \bug Should these be reset periodically?
            // p.glyphIDs.clear();
            // p.vbos["mesh"].reset();
            // p.vaos["mesh"].reset();
            // p.vbos["text"].reset();
            // p.vaos["text"].reset();

            const auto now = std::chrono::steady_clock::now();
            const auto diff =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - p.timer);
            p.currentStats.time = diff.count();
            p.stats.push_back(p.currentStats);
            p.currentStats = Private::Stats();
            while (p.stats.size() > 60)
            {
                p.stats.pop_front();
            }

            const std::chrono::duration<float> logDiff = now - p.logTimer;
            if (logDiff.count() > 10.F)
            {
                p.logTimer = now;
                if (auto context = _context.lock())
                {
                    Private::Stats average;
                    const size_t size = p.stats.size();
                    if (size > 0)
                    {
                        for (const auto& i : p.stats)
                        {
                            average.time += i.time;
                            average.rects += i.rects;
                            average.meshes += i.meshes;
                            average.meshTriangles += i.meshTriangles;
                            average.text += i.text;
                            average.textTriangles += i.textTriangles;
                            average.textures += i.textures;
                            average.images += i.images;
                        }
                        average.time /= p.stats.size();
                        average.rects /= p.stats.size();
                        average.meshes /= p.stats.size();
                        average.meshTriangles /= p.stats.size();
                        average.text /= p.stats.size();
                        average.textTriangles /= p.stats.size();
                        average.textures /= p.stats.size();
                        average.images /= p.stats.size();
                    }

                    context->log(
                        string::Format("tl::timeline::GLRender {0}").arg(this),
                        string::Format("\n"
                                       "    Average render time: {0}ms\n"
                                       "    Average rectangle count: {1}\n"
                                       "    Average mesh count: {2}\n"
                                       "    Average mesh triangles: {3}\n"
                                       "    Average text count: {4}\n"
                                       "    Average text triangles: {5}\n"
                                       "    Average texture count: {6}\n"
                                       "    Average image count: {7}\n"
                                       "    Glyph texture atlas: {8}%\n"
                                       "    Glyph IDs: {9}")
                            .arg(average.time)
                            .arg(average.rects)
                            .arg(average.meshes)
                            .arg(average.meshTriangles)
                            .arg(average.text)
                            .arg(average.textTriangles)
                            .arg(average.textures)
                            .arg(average.images)
                            .arg(p.glyphTextureAtlas->getPercentageUsed())
                            .arg(p.glyphIDs.size()));
                }
            }
        }

        math::Size2i Render::getRenderSize() const
        {
            return _p->renderSize;
        }

        void Render::setRenderSize(const math::Size2i& value)
        {
            _p->renderSize = value;
        }

        math::Box2i Render::getViewport() const
        {
            return _p->viewport;
        }

        void Render::setViewport(const math::Box2i& value)
        {
            TLRENDER_P();
            p.viewport = value;
            // glViewport(
            //     value.x(), p.renderSize.h - value.h() - value.y(), value.w(),
            //     value.h());
        }

        void Render::clearViewport(const image::Color4f& value)
        {
            // glClearColor(value.r, value.g, value.b, value.a);
            // glClear(GL_COLOR_BUFFER_BIT);
        }

        bool Render::getClipRectEnabled() const
        {
            return _p->clipRectEnabled;
        }

        void Render::setClipRectEnabled(bool value)
        {
            TLRENDER_P();
            p.clipRectEnabled = value;
            if (p.clipRectEnabled)
            {
                // glEnable(GL_SCISSOR_TEST);
            }
            else
            {
                // glDisable(GL_SCISSOR_TEST);
            }
        }

        math::Box2i Render::getClipRect() const
        {
            return _p->clipRect;
        }

        void Render::setClipRect(const math::Box2i& value)
        {
            TLRENDER_P();
            p.clipRect = value;
            if (value.w() > 0 && value.h() > 0)
            {
                // glScissor(
                //     value.x(), p.renderSize.h - value.h() - value.y(),
                //     value.w(), value.h());
            }
        }

        math::Matrix4x4f Render::getTransform() const
        {
            return _p->transform;
        }

        void Render::setTransform(const math::Matrix4x4f& value)
        {
            TLRENDER_P();
            p.transform = value;
            for (auto i : p.shaders)
            {
                i.second->bind();
                i.second->setUniform("transform.mvp", value);
            }
        }

        namespace
        {
            void setTextureParameters(GLenum textureType, GLenum interpolation)
            {
                // glTexParameteri(
                //     textureType, GL_TEXTURE_MIN_FILTER, interpolation);
                // glTexParameteri(
                //     textureType, GL_TEXTURE_MAG_FILTER, interpolation);
                // glTexParameteri(
                //     textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                // glTexParameteri(
                //     textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // glTexParameteri(
                //     textureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }

#if defined(TLRENDER_OCIO)

            void setTextureParameters(
                GLenum textureType, OCIO::Interpolation interpolation)
            {
                setTextureParameters(
                    textureType, OCIO::INTERP_NEAREST == interpolation
                                     ? GL_NEAREST
                                     : GL_LINEAR);
            }

            void addGPUTextures(
                std::vector<tl::timeline_vk::OCIOTexture>& textures,
                const OCIO::GpuShaderDescRcPtr& shaderDesc)
            {
                // Create 3D textures.
                // glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                // glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                const unsigned num3DTextures = shaderDesc->getNum3DTextures();
                for (unsigned i = 0; i < num3DTextures; ++i)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    shaderDesc->get3DTexture(
                        i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName || !*textureName || !samplerName ||
                        !*samplerName || 0 == edgelen)
                    {
                        throw std::runtime_error(
                            "The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    shaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        throw std::runtime_error(
                            "The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    // glGenTextures(1, &textureId);
                    // glBindTexture(GL_TEXTURE_3D, textureId);
                    // setTextureParameters(GL_TEXTURE_3D, interpolation);
                    // glTexImage3D(
                    //     GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen,
                    //     edgelen, 0, GL_RGB, GL_FLOAT, values);
                    textures.push_back(OCIOTexture(
                        textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = shaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel =
                        OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::GpuShaderCreator::TextureDimensions dimensions =
                        OCIO::GpuShaderDesc::TEXTURE_1D;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    shaderDesc->getTexture(
                        i, textureName, samplerName, width, height, channel,
                        dimensions, interpolation);
                    if (!textureName || !*textureName || !samplerName ||
                        !*samplerName || width == 0)
                    {
                        throw std::runtime_error(
                            "The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    shaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        throw std::runtime_error(
                            "The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    // GLint internalformat = GL_RGB32F;
                    // GLenum format = GL_RGB;
                    // if (OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL ==
                    // channel)
                    // {
                    //     internalformat = GL_R32F;
                    //     format = GL_RED;
                    // }
                    // glGenTextures(1, &textureId);
                    switch (dimensions)
                    {
                    case OCIO::GpuShaderDesc::TEXTURE_1D:
                        // glBindTexture(GL_TEXTURE_1D, textureId);
                        // setTextureParameters(GL_TEXTURE_1D, interpolation);
                        // glTexImage1D(
                        //     GL_TEXTURE_1D, 0, internalformat, width, 0,
                        //     format, GL_FLOAT, values);
                        break;
                    case OCIO::GpuShaderDesc::TEXTURE_2D:
                        // glBindTexture(GL_TEXTURE_2D, textureId);
                        // setTextureParameters(GL_TEXTURE_2D, interpolation);
                        // glTexImage2D(
                        //     GL_TEXTURE_2D, 0, internalformat, width, height,
                        //     0, format, GL_FLOAT, values);
                        break;
                    }
                    textures.push_back(OCIOTexture(
                        textureId, textureName, samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
            void addGPUTextures(
                std::vector<tl::timeline_vk::OCIOTexture>& textures,
                const pl_shader_res* res)
            {
                // Create 3D textures.
                // glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                // glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                for (unsigned i = 0; i < res->num_descriptors; ++i)
                {
                    const pl_shader_desc* sd = &res->descriptors[i];
                    switch (sd->desc.type)
                    {
                    case PL_DESC_STORAGE_IMG:
                        throw "Unimplemented img";
                    case PL_DESC_SAMPLED_TEX:
                    {
                        pl_tex tex =
                            reinterpret_cast<pl_tex>(sd->binding.object);
                        pl_fmt fmt = tex->params.format;

                        int dims = pl_tex_params_dimension(tex->params);
                        assert(dims >= 1 && dims <= 3);

                        const char* textureName = sd->desc.name;
                        const char* samplerName = sd->desc.name;

                        // std::cerr << "fmt->type=" << fmt->type << std::endl;
                        // std::cerr << "fmt->num_components=" <<
                        // fmt->component_depth << std::endl; for (int i = 0; i
                        // < fmt->num_components; ++i)
                        // {
                        //     std::cerr << "fmt->component_depth[" << i << "]="
                        //     << fmt->component_depth[i] << std::endl;
                        // }
                        // std::cerr << "fmt->internal_size=" <<
                        // fmt->internal_size << std::endl;

                        // Defaults
                        unsigned textureId = 0;
                        // GLint internalFormat = GL_RGB32F;
                        // GLint type = GL_FLOAT;
                        // GLenum format = GL_RGB;
                        int size = fmt->internal_size / fmt->num_components;

                        // Map libplacebo format to Vulkan format
                        VkFormat imageFormat = to_vk_format(fmt);

                        // Texture dimensions
                        uint32_t width = tex->params.w;
                        uint32_t height = (dims >= 2) ? tex->params.h : 1;
                        uint32_t depth = (dims == 3) ? tex->params.d : 1;

                        assert(width > 0);
                        if (dims >= 2)
                            assert(height > 0);
                        if (dims == 3)
                            assert(depth > 0);

                        // Get texture data from libplacebo
                        const void* values = pl_tex_dummy_data(tex);
                        if (!values)
                        {
                            throw std::runtime_error(
                                "Could not read pl_tex_dummy_data");
                        }

                        // Determine image type
                        VkImageType imageType = (dims == 1) ? VK_IMAGE_TYPE_1D
                                                : (dims == 2)
                                                    ? VK_IMAGE_TYPE_2D
                                                    : VK_IMAGE_TYPE_3D;

                        // Create Vulkan image
                        VkImage image = createImage(
                            imageType, width, height, depth, imageFormat);

                        // Allocate and bind memory for the image
                        VkDeviceMemory imageMemory = allocateAndBindImageMemory(
                            ctx->gpu, ctx->device, image);

                        // Transition image layout to TRANSFER_DST_OPTIMAL
                        transitionImageLayout(
                            ctx->device, ctx->queue, ctx->command_pool, image,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                        // Upload texture data
                        uploadTextureData(
                            image, width, height, depth, imageFormat, size,
                            values);

                        // Transition image layout to SHADER_READ_ONLY_OPTIMAL
                        transitionImageLayout(
                            ctx->device, ctx->queue, ctx->command_pool, image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                        // Create image view
                        VkImageView imageView =
                            createImageView(image, imageFormat, imageType);

                        // Create sampler (equivalent to GL_LINEAR)
                        VkSampler sampler = createSampler();

                        Fl_Vk_Texture texture(
                            imageType, imageFormat, image, imageView, sampler,
                            imageMemory, samplerName, width, height, depth);
                        m_textures.push_back(texture);
                    }
                    }
                }
            }
#endif

            void Render::setOCIOOptions(const timeline::OCIOOptions& value)
            {
                TLRENDER_P();
                if (value == p.ocioOptions)
                    return;

#if defined(TLRENDER_OCIO)
                p.ocioData.reset();
#endif // TLRENDER_OCIO

                p.ocioOptions = value;

#if defined(TLRENDER_OCIO)
                if (p.ocioOptions.enabled)
                {
                    p.ocioData.reset(new OCIOData);

                    if (!p.ocioOptions.fileName.empty())
                    {
                        p.ocioData->config = OCIO::Config::CreateFromFile(
                            p.ocioOptions.fileName.c_str());
                    }
                    else
                    {
                        p.ocioData->config = OCIO::GetCurrentConfig();
                    }
                    if (!p.ocioData->config)
                    {
                        throw std::runtime_error(
                            "Cannot get OCIO configuration");
                    }

                    p.ocioData->transform =
                        OCIO::DisplayViewTransform::Create(Ctx, );
                    if (!p.ocioData->transform)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO transform");
                    }
                    if (!p.ocioOptions.input.empty())
                    {
                        OCIO::ConstColorSpaceRcPtr srcCS =
                            p.ocioData->config->getColorSpace(
                                p.ocioOptions.input.c_str());
                        OCIO::ConstColorSpaceRcPtr dstCS =
                            p.ocioData->config->getColorSpace(
                                OCIO::ROLE_SCENE_LINEAR);
                        p.ocioData->processor =
                            p.ocioData->config->getProcessor(
                                p.ocioData->config->getCurrentContext(), srcCS,
                                dstCS);
                        if (!p.ocioData->processor)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot get OCIO processor");
                        }

                        p.ocioData->gpuProcessor =
                            p.ocioData->processor->getOptimizedGPUProcessor(
                                OCIO::OPTIMIZATION_DEFAULT);
                        if (!p.ocioData->gpuProcessor)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot get OCIO GPU processor for ICS");
                        }
                        p.ocioData->icsDesc =
                            OCIO::GpuShaderDesc::CreateShaderDesc();
                        if (!p.ocioData->icsDesc)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot create OCIO ICS shader description");
                        }

                        p.ocioData->icsDesc->setLanguage(
                            OCIO::GPU_LANGUAGE_GLSL_4_0);
                        p.ocioData->icsDesc->setFunctionName("ocioICSFunc");
                        p.ocioData->icsDesc->setResourcePrefix(
                            "ocioICS"); // ocio?
                        p.ocioData->gpuProcessor->extractGpuShaderInfo(
                            p.ocioData->icsDesc);
                        try
                        {
                            addGPUTextures(
                                p.ocioData->textures, p.ocioData->icsDesc);
                        }
                        catch (const std::exception& e)
                        {
                            p.ocioData.reset();
                            throw e;
                        }
                    }
                    if (!p.ocioOptions.display.empty() &&
                        !p.ocioOptions.view.empty())
                    {
                        p.ocioData->transform->setSrc(OCIO::ROLE_SCENE_LINEAR);
                        p.ocioData->transform->setDisplay(
                            p.ocioOptions.display.c_str());
                        p.ocioData->transform->setView(
                            p.ocioOptions.view.c_str());

                        p.ocioData->lvp =
                            OCIO::LegacyViewingPipeline::Create(Ctx, );
                        if (!p.ocioData->lvp)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot create OCIO viewing pipeline");
                        }
                        p.ocioData->lvp->setDisplayViewTransform(
                            p.ocioData->transform);
                        p.ocioData->lvp->setLooksOverrideEnabled(true);
                        p.ocioData->lvp->setLooksOverride(
                            p.ocioOptions.look.c_str());

                        p.ocioData->processor = p.ocioData->lvp->getProcessor(
                            p.ocioData->config,
                            p.ocioData->config->getCurrentContext());
                        if (!p.ocioData->processor)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot get OCIO processor");
                        }
                        p.ocioData->gpuProcessor =
                            p.ocioData->processor->getOptimizedGPUProcessor(
                                OCIO::OPTIMIZATION_DEFAULT);
                        if (!p.ocioData->gpuProcessor)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot get OCIO GPU processor");
                        }
                        p.ocioData->shaderDesc =
                            OCIO::GpuShaderDesc::CreateShaderDesc();
                        if (!p.ocioData->shaderDesc)
                        {
                            p.ocioData.reset();
                            throw std::runtime_error(
                                "Cannot create OCIO shader description");
                        }
                        p.ocioData->shaderDesc->setLanguage(
                            OCIO::GPU_LANGUAGE_GLSL_4_0);
                        p.ocioData->shaderDesc->setFunctionName(
                            "ocioDisplayFunc");
                        p.ocioData->shaderDesc->setResourcePrefix("ocio");
                        p.ocioData->gpuProcessor->extractGpuShaderInfo(
                            p.ocioData->shaderDesc);
                        try
                        {
                            addGPUTextures(
                                p.ocioData->textures, p.ocioData->shaderDesc);
                        }
                        catch (const std::exception& e)
                        {
                            p.ocioData.reset();
                            throw e;
                        }
                    }
                }
#endif // TLRENDER_OCIO

                p.shaders["display"].reset();
                _displayShader();
            }

            void Render::setLUTOptions(const timeline::LUTOptions& value)
            {
                TLRENDER_P();
                if (value == p.lutOptions)
                    return;

#if defined(TLRENDER_OCIO)
                p.lutData.reset();
#endif // TLRENDER_OCIO

                p.lutOptions = value;

#if defined(TLRENDER_OCIO)
                if (p.lutOptions.enabled && !p.lutOptions.fileName.empty())
                {
                    p.lutData.reset(new OCIOLUTData);

                    p.lutData->config = OCIO::Config::CreateRaw();
                    if (!p.lutData->config)
                    {
                        throw std::runtime_error(
                            "Cannot create OCIO configuration");
                    }

                    p.lutData->transform = OCIO::FileTransform::Create(Ctx, );
                    if (!p.lutData->transform)
                    {
                        p.lutData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO transform");
                    }
                    p.lutData->transform->setSrc(p.lutOptions.fileName.c_str());
                    p.lutData->transform->validate();

                    p.lutData->processor =
                        p.lutData->config->getProcessor(p.lutData->transform);
                    if (!p.lutData->processor)
                    {
                        p.lutData.reset();
                        throw std::runtime_error("Cannot get OCIO processor");
                    }
                    p.lutData->gpuProcessor =
                        p.lutData->processor->getDefaultGPUProcessor();
                    if (!p.lutData->gpuProcessor)
                    {
                        p.lutData.reset();
                        throw std::runtime_error(
                            "Cannot get OCIO GPU processor");
                    }
                    p.lutData->shaderDesc =
                        OCIO::GpuShaderDesc::CreateShaderDesc();
                    if (!p.lutData->shaderDesc)
                    {
                        p.lutData.reset();
                        throw std::runtime_error(
                            "Cannot create OCIO shader description");
                    }
                    p.lutData->shaderDesc->setLanguage(
                        OCIO::GPU_LANGUAGE_GLSL_4_0);
                    p.lutData->shaderDesc->setFunctionName("lutFunc");
                    p.lutData->shaderDesc->setResourcePrefix("lut");
                    p.lutData->gpuProcessor->extractGpuShaderInfo(
                        p.lutData->shaderDesc);
                    try
                    {
                        addGPUTextures(
                            p.lutData->textures, p.lutData->shaderDesc);
                    }
                    catch (const std::exception& e)
                    {
                        p.ocioData.reset();
                        throw e;
                    }
                }
#endif // TLRENDER_OCIO

                p.shaders["display"].reset();
                _displayShader();
            }

            void Render::setHDROptions(const timeline::HDROptions& value)
            {
                TLRENDER_P();
                if (value == p.hdrOptions)
                    return;

                p.hdrOptions = value;

#if defined(TLRENDER_LIBPLACEBO)
                if (p.hdrOptions.tonemap)
                {
                    p.placeboData.reset(new LibPlaceboData);
                }
#endif // TLRENDER_LIBPLACEBO

                p.shaders["display"].reset();
                _displayShader();
            }

            void Render::_displayShader()
            {
                TLRENDER_P();

                if (!p.shaders["display"])
                {
                    std::string ocioICSDef;
                    std::string ocioICS;
                    std::string ocioDef;
                    std::string ocio;
                    std::string lutDef;
                    std::string lut;
                    std::string toneMapDef;
                    std::string toneMap;

#if defined(TLRENDER_OCIO)
                    if (p.ocioData && p.ocioData->icsDesc)
                    {
                        ocioICSDef = p.ocioData->icsDesc->getShaderText();
                        ocioICS = "outColor = ocioICSFunc(outColor);";
                    }
                    if (p.ocioData && p.ocioData->shaderDesc)
                    {
                        ocioDef = p.ocioData->shaderDesc->getShaderText();
                        ocio = "outColor = ocioDisplayFunc(outColor);";
                    }
                    if (p.lutData && p.lutData->shaderDesc)
                    {
                        lutDef = p.lutData->shaderDesc->getShaderText();
                        lut = "outColor = lutFunc(outColor);";
                    }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                    pl_shader_params shader_params;
                    memset(&shader_params, 0, sizeof(pl_shader_params));

                    shader_params.id = 1;
                    shader_params.gpu = p.placeboData->gpu;
                    shader_params.dynamic_constants = false;

                    pl_shader shader =
                        pl_shader_alloc(p.placeboData->log, &shader_params);
                    if (!shader)
                    {
                        throw std::runtime_error("pl_shader_alloc failed!");
                    }

                    pl_color_map_params cmap;
                    memset(&cmap, 0, sizeof(pl_color_map_params));

                    // defaults, generates LUTs if state is set.
                    cmap.gamut_mapping = &pl_gamut_map_perceptual;

                    // PL_GAMUT_MAP_CONSTANTS is defined in wrong order for C++
                    cmap.gamut_constants = {0};
                    cmap.gamut_constants.perceptual_deadzone = 0.3F;
                    cmap.gamut_constants.perceptual_strength = 0.8F;
                    cmap.gamut_constants.colorimetric_gamma = 1.80f;
                    cmap.gamut_constants.softclip_knee = 0.70f;
                    cmap.gamut_constants.softclip_desat = 0.35f;

                    cmap.tone_mapping_function = nullptr;

                    cmap.tone_constants = {0};
                    cmap.tone_constants.knee_adaptation = 0.4f;
                    cmap.tone_constants.knee_minimum = 0.1f;
                    cmap.tone_constants.knee_maximum = 0.8f;
                    cmap.tone_constants.knee_default = 0.4f;
                    cmap.tone_constants.knee_offset = 1.0f;

                    cmap.tone_constants.slope_tuning = 1.5f;
                    cmap.tone_constants.slope_offset = 0.2f;

                    cmap.tone_constants.spline_contrast = 0.5f;

                    cmap.tone_constants.reinhard_contrast = 0.5f;
                    cmap.tone_constants.linear_knee = 0.3f;

                    cmap.tone_constants.exposure = 1.0f;

                    cmap.contrast_smoothness = 3.5f;

                    const image::HDRData& data = p.hdrData;

                    pl_color_space src_colorspace;
                    memset(&src_colorspace, 0, sizeof(pl_color_space));

                    if (p.hasHDR)
                    {
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                        src_colorspace.transfer = PL_COLOR_TRC_PQ;
                        if (p.transferName == "bt_2100_hlg")
                        {
                            src_colorspace.transfer = PL_COLOR_TRC_HLG;
                        }
                        cmap.metadata = PL_HDR_METADATA_ANY;
                        pl_hdr_metadata& hdr = src_colorspace.hdr;
                        hdr.min_luma = data.displayMasteringLuminance.getMin();
                        hdr.max_luma = data.displayMasteringLuminance.getMax();
                        hdr.prim.red.x =
                            data.primaries[image::HDRPrimaries::Red][0];
                        hdr.prim.red.y =
                            data.primaries[image::HDRPrimaries::Red][1];
                        hdr.prim.green.x =
                            data.primaries[image::HDRPrimaries::Green][0];
                        hdr.prim.green.y =
                            data.primaries[image::HDRPrimaries::Green][1];
                        hdr.prim.blue.x =
                            data.primaries[image::HDRPrimaries::Blue][0];
                        hdr.prim.blue.y =
                            data.primaries[image::HDRPrimaries::Blue][1];
                        hdr.prim.white.x =
                            data.primaries[image::HDRPrimaries::White][0];
                        hdr.prim.white.y =
                            data.primaries[image::HDRPrimaries::White][1];
                        hdr.max_cll = data.maxCLL;
                        hdr.max_fall = data.maxFALL;
                        hdr.scene_max[0] = data.sceneMax[0];
                        hdr.scene_max[1] = data.sceneMax[1];
                        hdr.scene_max[2] = data.sceneMax[2];
                        hdr.scene_avg = data.sceneAvg;
                        hdr.ootf.target_luma = data.ootf.targetLuma;
                        hdr.ootf.knee_x = data.ootf.kneeX;
                        hdr.ootf.knee_y = data.ootf.kneeY;
                        hdr.ootf.num_anchors = data.ootf.numAnchors;
                        for (int i = 0; i < hdr.ootf.num_anchors; i++)
                            hdr.ootf.anchors[i] = data.ootf.anchors[i];
                    }
                    else
                    {
                        src_colorspace.primaries = PL_COLOR_PRIM_BT_709;

                        // SDR uses sRGB-like gamma
                        src_colorspace.transfer = PL_COLOR_TRC_SRGB;
                    }

                    pl_color_space_infer(&src_colorspace);

                    pl_color_space dst_colorspace;
                    memset(&dst_colorspace, 0, sizeof(pl_color_space));

                    if (0) // p.hdrMonitorFound)
                    {
                        dst_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
                        dst_colorspace.transfer = PL_COLOR_TRC_PQ;
                        if (m_color_space ==
                            VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
                        {
                            dst_colorspace.primaries = PL_COLOR_PRIM_DISPLAY_P3;
                            dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;
                        }
                        else if (m_color_space == VK_COLOR_SPACE_HDR10_HLG_EXT)
                        {
                            dst_colorspace.transfer = PL_COLOR_TRC_HLG;
                        }
                        else if (
                            m_color_space == VK_COLOR_SPACE_DOLBYVISION_EXT)
                        {
                            // \@todo:  How to handle this?
                            // PL_COLOR_TRC_DOLBYVISION does
                            //          not exist.
                            // dst_colorspace.transfer = ???
                            dst_colorspace.transfer = PL_COLOR_TRC_PQ;
                        }

                        // For SDR content on HDR monitor, enable tone mapping
                        // to fit SDR into HDR
                        if (!p.hasHDR)
                        {
                            cmap.tone_mapping_function = &pl_tone_map_st2094_40;
                            cmap.metadata = PL_HDR_METADATA_NONE; // Simplify
                        }
                        else
                        {
                            cmap.tone_mapping_function = &pl_tone_map_auto;
                        }
                    }
                    else
                    {
                        cmap.lut3d_size[0] = 48;
                        cmap.lut3d_size[1] = 32;
                        cmap.lut3d_size[2] = 256;
                        cmap.lut_size = 256;
                        cmap.visualize_rect.x0 = 0;
                        cmap.visualize_rect.y0 = 0;
                        cmap.visualize_rect.x1 = 1;
                        cmap.visualize_rect.y1 = 1;
                        cmap.contrast_smoothness = 3.5f;

                        dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
                        dst_colorspace.transfer = PL_COLOR_TRC_BT_1886;

                        if (p.hasHDR)
                            cmap.tone_mapping_function = &pl_tone_map_st2094_40;
                        else
                            cmap.tone_mapping_function = nullptr;
                    }

                    pl_color_space_infer(&dst_colorspace);

                    pl_color_map_args color_map_args;
                    memset(&color_map_args, 0, sizeof(pl_color_map_args));

                    color_map_args.src = src_colorspace;
                    color_map_args.dst = dst_colorspace;
                    color_map_args.prelinearized = false;

                    pl_shader_obj state = NULL;
                    color_map_args.state =
                        &state; // with NULL and tonemap_clip works

                    pl_shader_color_map_ex(shader, &cmap, &color_map_args);

                    const pl_shader_res* res = pl_shader_finalize(shader);
                    if (!res)
                    {
                        pl_shader_free(&shader);
                        throw std::runtime_error("pl_shader_finalize failed!");
                    }

                    std::stringstream s;

                    // std::cerr << "num_vertex_attribs=" <<
                    // res->num_vertex_attribs
                    //           << std::endl
                    //           << "num_descriptors="
                    //           << res->num_descriptors << std::endl
                    //           << "num_variables=" << res->num_variables
                    //           << std::endl
                    //           << "num_constants="
                    //           << res->num_constants << std::endl;

                    for (int i = 0; i < res->num_descriptors; i++)
                    {
                        const pl_shader_desc* sd = &res->descriptors[i];
                        const pl_desc* desc = &sd->desc;
                        switch (desc->type)
                        {
                        case PL_DESC_SAMPLED_TEX:
                        case PL_DESC_STORAGE_IMG:
                        {
                            static const char* types[] = {
                                "sampler1D",
                                "sampler2D",
                                "sampler3D",
                            };

                            pl_desc_binding binding = sd->binding;
                            pl_tex tex = (pl_tex)binding.object;
                            int dims = pl_tex_params_dimension(tex->params);
                            const char* type = types[dims - 1];

                            char prefix = ' ';
                            switch (tex->params.format->type)
                            {
                            case PL_FMT_UINT:
                                prefix = 'u';
                                break;
                            case PL_FMT_SINT:
                                prefix = 'i';
                                break;
                            case PL_FMT_FLOAT:
                            case PL_FMT_UNORM:
                            case PL_FMT_SNORM:
                            default:
                                break;
                            }

                            s << "layout(binding=" << (i + 1) << ") uniform "
                              << prefix << type << " " << desc->name << ";"
                              << std::endl;
                            break;
                        }
                        case PL_DESC_BUF_UNIFORM:
                            throw "buf uniform";
                            break;
                        case PL_DESC_BUF_STORAGE:
                            throw "buf storage";
                        case PL_DESC_BUF_TEXEL_UNIFORM:
                            throw "buf texel uniform";
                        case PL_DESC_BUF_TEXEL_STORAGE:
                            throw "buf texel storage";
                        case PL_DESC_INVALID:
                        case PL_DESC_TYPE_COUNT:
                            throw "invalid or count";
                            break;
                        }
                    }

                    s << "//" << std::endl
                      << "// Variables" << "//" << std::endl
                      << std::endl;
                    // \@todo: add uniform bindings
                    // s << "layout(binding = 2) uniform UBO {\n";
                    for (int i = 0; i < res->num_variables; ++i)
                    {
                        const struct pl_shader_var shader_var =
                            res->variables[i];
                        const struct pl_var var = shader_var.var;
                        std::string glsl_type = pl_var_glsl_type_name(var);
                        s << "const " << glsl_type << " " << var.name;
                        if (!shader_var.data)
                        {
                            s << ";" << std::endl;
                        }
                        else
                        {
                            int dim_v = var.dim_v;
                            int dim_m = var.dim_m;
                            switch (var.type)
                            {
                            case PL_VAR_SINT:
                            {
                                int* m = (int*)shader_var.data;
                                s << " = " << m[0] << ";" << std::endl;
                                break;
                            }
                            case PL_VAR_UINT:
                            {
                                unsigned* m = (unsigned*)shader_var.data;
                                s << " = " << m[0] << ";" << std::endl;
                                break;
                            }
                            case PL_VAR_FLOAT:
                            {
                                float* m = (float*)shader_var.data;
                                if (dim_m > 1 && dim_v > 1)
                                {
                                    s << " = " << glsl_type << "(";
                                    for (int c = 0; c < dim_v; ++c)
                                    {
                                        for (int r = 0; r < dim_m; ++r)
                                        {
                                            int index = c * dim_m + r;
                                            s << m[index];

                                            // Check if it's the last element
                                            if (!(r == dim_m - 1 &&
                                                  c == dim_v - 1))
                                            {
                                                s << ", ";
                                            }
                                        }
                                    }
                                    s << ");" << std::endl;
                                }
                                else if (dim_v > 1)
                                {
                                    s << " = " << glsl_type << "(";
                                    for (int c = 0; c < dim_v; ++c)
                                    {
                                        s << m[c];

                                        // Check if it's the last element
                                        if (!(c == dim_v - 1))
                                        {
                                            s << ", ";
                                        }
                                    }
                                    s << ");" << std::endl;
                                }
                                else
                                {
                                    s << " = " << m[0] << ";" << std::endl;
                                }
                                break;
                            }
                            default:
                                break;
                            }
                        }
                    }

                    s << std::endl
                      << "//" << std::endl
                      << "// Constants" << std::endl
                      << "//" << std::endl
                      << std::endl;
                    for (int i = 0; i < res->num_constants; ++i)
                    {
                        // s << "layout(constant_id=" << i << ") ";
                        const struct pl_shader_const constant =
                            res->constants[i];
                        switch (constant.type)
                        {
                        case PL_VAR_SINT:
                            s << "const int " << constant.name << " = "
                              << *(reinterpret_cast<const int*>(constant.data));
                            break;
                        case PL_VAR_UINT:
                            s << "const uint " << constant.name << " = "
                              << *(reinterpret_cast<const unsigned*>(
                                     constant.data));
                            break;
                        case PL_VAR_FLOAT:
                            s << "const float " << constant.name << " = "
                              << *(reinterpret_cast<const float*>(
                                     constant.data));
                            break;
                        default:
                            break;
                        }
                        s << ";" << std::endl;
                    }

                    s << res->glsl << std::endl;
                    toneMapDef = s.str();

                    try
                    {
                        addGPUTextures(res);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                        pl_shader_free(&shader);
                        return;
                    }
                    toneMap = "outColor = ";
                    toneMap += res->name;
                    toneMap += "(tmp);\n";

                    pl_shader_free(&shader);
#endif
                    const std::string source = displayFragmentSource(
                        ocioICSDef, ocioICS, ocioDef, ocio, lutDef, lut,
                        p.lutOptions.order, toneMapDef, toneMap);
                    if (auto context = _context.lock())
                    {
                        context->log(
                            "tl::vk::GLRender", "Creating display shader");
                    }
                    p.shaders["display"] =
                        vk::Shader::create(ctx, vertexSource(), source);
                }
                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform("transform.mvp", p.transform);
                size_t texturesOffset = 1;
#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                    {
                        p.shaders["display"]->setUniform(
                            p.ocioData->textures[i].sampler,
                            static_cast<int>(texturesOffset + i));
                    }
                    texturesOffset += p.ocioData->textures.size();
                }
                if (p.lutData)
                {
                    for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                    {
                        p.shaders["display"]->setUniform(
                            p.lutData->textures[i].sampler,
                            static_cast<int>(texturesOffset + i));
                    }
                    texturesOffset += p.lutData->textures.size();
                }
#endif // TLRENDER_OCIO
#if defined(TLRENDER_LIBPLACEBO)
                for (size_t i = 0; i < p.placeboData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.placeboData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.placeboData->textures.size();
#endif
            }
        } // namespace timeline_gl
    } // namespace tl
