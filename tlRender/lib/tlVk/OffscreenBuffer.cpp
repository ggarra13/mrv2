// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlVk/OffscreenBuffer.h>

#include <tlVk/Vk.h>
#include <tlVk/Texture.h>
#include <tlVk/Util.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <FL/vk_enum_string_helper.h>
#include <FL/Fl_Vk_Utils.H>

#include <array>
#include <sstream>
#include <iostream>

namespace tl
{
    namespace vlk
    {
        TLRENDER_ENUM_IMPL(OffscreenDepth, "None", "16", "24", "32");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenDepth);

        TLRENDER_ENUM_IMPL(OffscreenStencil, "None", "8");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenStencil);

        TLRENDER_ENUM_IMPL(OffscreenSampling, "None", "2", "4", "8", "16");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenSampling);

        namespace
        {

            VkSampleCountFlagBits getVulkanSamples(OffscreenSampling sampling)
            {
                VkSampleCountFlagBits out = VK_SAMPLE_COUNT_1_BIT;
                switch (sampling)
                {
                case OffscreenSampling::_2:
                    out = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case OffscreenSampling::_4:
                    out = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case OffscreenSampling::_8:
                    out = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case OffscreenSampling::_16:
                    out = VK_SAMPLE_COUNT_16_BIT;
                    break;
                default:
                    break;
                }
                return out;
            }

            VkFormat getBufferInternalFormat(
                OffscreenDepth depth, OffscreenStencil stencil)
            {
                VkFormat out = VK_FORMAT_UNDEFINED;
                switch (depth)
                {
                case OffscreenDepth::kNone:
                    switch (stencil)
                    {
                    case OffscreenStencil::_8:
                        out = VK_FORMAT_S8_UINT;
                        break;
                    default:
                        break;
                    }
                    break;
                case OffscreenDepth::_16:
                    out = VK_FORMAT_D16_UNORM;
                    switch (stencil)
                    {
                    case OffscreenStencil::_8:
                        out = VK_FORMAT_D16_UNORM_S8_UINT;
                        break;
                    default:
                        break;
                    }
                    break;
                case OffscreenDepth::_24:
                    switch (stencil)
                    {
                    case OffscreenStencil::kNone:
                        // No pure D24 depth format in Vulkan, fallback
                        out = VK_FORMAT_D24_UNORM_S8_UINT;
                        break;
                    case OffscreenStencil::_8:
                        out = VK_FORMAT_D24_UNORM_S8_UINT;
                        break;
                    default:
                        break;
                    }
                    break;
                case OffscreenDepth::_32:
                    switch (stencil)
                    {
                    case OffscreenStencil::kNone:
                        out = VK_FORMAT_D32_SFLOAT;
                        break;
                    case OffscreenStencil::_8:
                        out = VK_FORMAT_D32_SFLOAT_S8_UINT;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                return out;
            }
        } // namespace

        bool OffscreenBufferOptions::operator==(
            const OffscreenBufferOptions& other) const
        {
            return (colorType == other.colorType &&
                    colorFilters == other.colorFilters &&
                    depth == other.depth &&
                    stencil == other.stencil && sampling == other.sampling &&
                    clearColor == other.clearColor &&
                    clearDepth == other.clearDepth &&
                    storeDepth == other.storeDepth &&
                    pbo == other.pbo);
        }

        bool OffscreenBufferOptions::operator!=(
            const OffscreenBufferOptions& other) const
        {
            return !(*this == other);
        }

        static constexpr int NUM_PBO_BUFFERS = 3;

        struct StagingBuffer {
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory;
            void* mappedPtr = nullptr;
            VkFence fence = VK_NULL_HANDLE;
        };
        
        struct OffscreenBuffer::Private
        {
            uint32_t frameIndex = 0;

            math::Size2i size;
            OffscreenBufferOptions options;

            // Ring indices
            int writeIndex = 0;  // where we'll submit the next copy
            int readIndex  = 0;  // where we'll next wait+read
            
            std::vector<StagingBuffer> pboRing;

            // Vulkan handles
            VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
            VkFormat depthFormat = VK_FORMAT_UNDEFINED;

            // Multisampled color render target (only created when sampling > 1)
            VkImage         msColorImage       = VK_NULL_HANDLE;
            VkDeviceMemory  msColorMemory      = VK_NULL_HANDLE;
            VkImageView     msColorImageView   = VK_NULL_HANDLE;
            VkImageLayout   msColorImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            // Resolved single-sample color image (public API image/view/layout)
            VkImage         resolveImage       = VK_NULL_HANDLE;
            VkDeviceMemory  resolveMemory      = VK_NULL_HANDLE;
            VkImageView     resolveImageView   = VK_NULL_HANDLE;
            VkImageLayout   resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkImageLayout   depthLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImage         depthImage         = VK_NULL_HANDLE;
            VkDeviceMemory  depthMemory        = VK_NULL_HANDLE;
            VkImageView     depthImageView     = VK_NULL_HANDLE;
            
            VkRenderPass clearRenderPass = VK_NULL_HANDLE;
            VkRenderPass loadRenderPass = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;

            VkSampler sampler = VK_NULL_HANDLE;

            VkViewport viewport = {};
            VkRect2D scissor = {};

            bool inRenderPass = false;
        };

        void OffscreenBuffer::_init(
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            TLRENDER_P();
            
            OffscreenBufferOptions offscreenBufferOptions = options;
            switch(offscreenBufferOptions.colorType)
            {
            case image::PixelType::RGB_U8:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                break;
            case image::PixelType::RGB_U16:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U16;
                break;
            case image::PixelType::RGB_F16:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F16;
                break;
            case image::PixelType::RGB_F32:
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                break;
            default:
                break;
            };

            p.size = size;
            p.options = offscreenBufferOptions;
            p.colorFormat = getTextureFormat(p.options.colorType);
            p.depthFormat =
                getBufferInternalFormat(p.options.depth, p.options.stencil);

            // Get maximum texture resolution for gfx card
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(ctx.gpu, &props);
            uint32_t maxTextureSize = props.limits.maxImageDimension2D;

            if (p.size.w > maxTextureSize)
                p.size.w = maxTextureSize;
            if (p.size.h > maxTextureSize)
                p.size.h = maxTextureSize;

            initialize();
        }

        OffscreenBuffer::OffscreenBuffer(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        OffscreenBuffer::~OffscreenBuffer()
        {
            cleanup();
        }

        void OffscreenBuffer::cleanup()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            {
                std::lock_guard<std::mutex> lock(ctx.queue_mutex());
                vkDeviceWaitIdle(device);
            }
            
            if (p.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, p.sampler, nullptr);

            // Multisampled color
            if (p.msColorImage != VK_NULL_HANDLE)
                vkDestroyImage(device, p.msColorImage, nullptr);
            if (p.msColorMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.msColorMemory, nullptr);
            if (p.msColorImageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.msColorImageView, nullptr);

             if (p.framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(device, p.framebuffer, nullptr);
            if (p.clearRenderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(device, p.clearRenderPass, nullptr);
            if (p.loadRenderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(device, p.loadRenderPass, nullptr);

           // Resolved color (always present)
            if (p.resolveImage != VK_NULL_HANDLE)
                vkDestroyImage(device, p.resolveImage, nullptr);
            if (p.resolveMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.resolveMemory, nullptr);
            if (p.resolveImageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.resolveImageView, nullptr);
            
            // Depth
            if (p.depthImageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.depthImageView, nullptr);
            if (p.depthImage != VK_NULL_HANDLE)
                vkDestroyImage(device, p.depthImage, nullptr);
            if (p.depthMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.depthMemory, nullptr);

            // PBO ring of buffers
            for (auto& pbo : p.pboRing)
            {
                if (pbo.buffer != VK_NULL_HANDLE)
                    vkDestroyBuffer(device, pbo.buffer, nullptr);
                if (pbo.fence != VK_NULL_HANDLE)
                    vkDestroyFence(device, pbo.fence, nullptr);
                if (pbo.memory != VK_NULL_HANDLE)
                    vkFreeMemory(device, pbo.memory, nullptr);
            }

            // Reset layouts
            p.msColorImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            p.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            p.depthLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void OffscreenBuffer::initialize()
        {
            TLRENDER_P();

            createColorImages();
            createImageViews();

            if (hasDepth() || hasStencil())
            {
                createDepthImage();
                createDepthImageView();
            }

            // Render passes
            createClearRenderPass();
            createLoadRenderPass();
            
            createFramebuffer();
            if (p.options.pbo)
            {
                createStagingBuffers();
            }
            createSampler();
        }

        void OffscreenBuffer::createSampler()
        {
            TLRENDER_P();

            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter =
                getTextureFilter(p.options.colorFilters.magnify);
            samplerInfo.minFilter =
                getTextureFilter(p.options.colorFilters.minify);
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            if (vkCreateSampler(
                    ctx.device, &samplerInfo, nullptr, &p.sampler) !=
                VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to create sampler for offscreen buffer.");
            }
        }

        void OffscreenBuffer::createDepthImage()
        {
            TLRENDER_P();

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = {
                static_cast<uint32_t>(p.size.w),
                static_cast<uint32_t>(p.size.h), 1};
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = p.depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            
            VkSampleCountFlagBits samples = getVulkanSamples(p.options.sampling);
            imageInfo.samples = samples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(ctx.device, &imageInfo, nullptr, &p.depthImage) !=
                VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image");

            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(ctx.device, p.depthImage, &memReq);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(
                    ctx.device, &allocInfo, nullptr, &p.depthMemory) !=
                VK_SUCCESS)
                throw std::runtime_error("Failed to allocate depth memory");

            vkBindImageMemory(ctx.device, p.depthImage, p.depthMemory, 0);
        }

        void OffscreenBuffer::createDepthImageView()
        {
            TLRENDER_P();

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = p.depthImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = p.depthFormat;
            viewInfo.subresourceRange.aspectMask = 0;
            if (hasDepth())
                viewInfo.subresourceRange.aspectMask |=
                    VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencil())
                viewInfo.subresourceRange.aspectMask |=
                    VK_IMAGE_ASPECT_STENCIL_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(
                    ctx.device, &viewInfo, nullptr, &p.depthImageView) !=
                VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image view");
        }

        void OffscreenBuffer::createColorImages()
        {
            TLRENDER_P();
            VkDevice device = ctx.device;

            VkSampleCountFlagBits samples = getSampleCount();
            bool multisampled = (samples != VK_SAMPLE_COUNT_1_BIT);

            // --------------------------------------------------
            // 1. Resolve image (always single-sampled)
            // --------------------------------------------------
            {
                VkImageCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                info.imageType = VK_IMAGE_TYPE_2D;
                info.extent = { static_cast<uint32_t>(p.size.w), static_cast<uint32_t>(p.size.h), 1 };
                info.mipLevels = 1;
                info.arrayLayers = 1;
                info.format = p.colorFormat;
                info.tiling = VK_IMAGE_TILING_OPTIMAL;
                info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                             VK_IMAGE_USAGE_SAMPLED_BIT |
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                info.samples = VK_SAMPLE_COUNT_1_BIT;
                info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                if (vkCreateImage(device, &info, nullptr, &p.resolveImage) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create resolve image");

                VkMemoryRequirements memReq;
                vkGetImageMemoryRequirements(device, p.resolveImage, &memReq);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReq.size;
                allocInfo.memoryTypeIndex = findMemoryType(
                    memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                if (vkAllocateMemory(device, &allocInfo, nullptr, &p.resolveMemory) != VK_SUCCESS)
                    throw std::runtime_error("Failed to allocate resolve image memory");

                vkBindImageMemory(device, p.resolveImage, p.resolveMemory, 0);
            }

            // --------------------------------------------------
            // 2. Multisampled color image (only when needed)
            // --------------------------------------------------
            if (multisampled)
            {
                VkImageCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                info.imageType = VK_IMAGE_TYPE_2D;
                info.extent = { static_cast<uint32_t>(p.size.w), static_cast<uint32_t>(p.size.h), 1 };
                info.mipLevels = 1;
                info.arrayLayers = 1;
                info.format = p.colorFormat;
                info.tiling = VK_IMAGE_TILING_OPTIMAL;
                info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // internal only
                info.samples = samples;
                info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                if (vkCreateImage(device, &info, nullptr, &p.msColorImage) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create multisampled color image");

                VkMemoryRequirements memReq;
                vkGetImageMemoryRequirements(device, p.msColorImage, &memReq);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReq.size;
                allocInfo.memoryTypeIndex = findMemoryType(
                    memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                if (vkAllocateMemory(device, &allocInfo, nullptr, &p.msColorMemory) != VK_SUCCESS)
                    throw std::runtime_error("Failed to allocate multisampled color memory");

                vkBindImageMemory(device, p.msColorImage, p.msColorMemory, 0);
            }
        }
        
        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            Fl_Vk_Context& context, const math::Size2i& size,
            const OffscreenBufferOptions& options)
        {
            auto out =
                std::shared_ptr<OffscreenBuffer>(new OffscreenBuffer(context));
            out->_init(size, options);
            return out;
        }

        const math::Size2i& OffscreenBuffer::getSize() const
        {
            return _p->size;
        }

        int OffscreenBuffer::getWidth() const
        {
            return _p->size.w;
        }

        int OffscreenBuffer::getHeight() const
        {
            return _p->size.h;
        }

        int OffscreenBuffer::getChannelCount() const
        {
            return image::getChannelCount(_p->options.colorType);
        }

        bool OffscreenBuffer::hasDepth() const
        {
            return (_p->options.depth != OffscreenDepth::kNone);
        }

        bool OffscreenBuffer::hasStencil() const
        {
            return (_p->options.stencil != OffscreenStencil::kNone);
        }

        VkSampleCountFlagBits OffscreenBuffer::getSampleCount() const
        {
            return getVulkanSamples(_p->options.sampling);
        }

        const OffscreenBufferOptions& OffscreenBuffer::getOptions() const
        {
            return _p->options;
        }

        VkFormat OffscreenBuffer::getFormat() const
        {
            return _p->colorFormat;
        }
        
        VkFormat OffscreenBuffer::getDepthFormat() const
        {
            return _p->depthFormat;
        }
        
        VkImageLayout OffscreenBuffer::getImageLayout() const
        {
            return _p->resolveImageLayout;
        }
        
        void OffscreenBuffer::setImageLayout(VkImageLayout value)
        {
            _p->resolveImageLayout = value;
        }

        VkImageLayout OffscreenBuffer::getDepthLayout() const
        {
            return _p->depthLayout;
        }
        
        void OffscreenBuffer::setDepthLayout(VkImageLayout value)
        {
            _p->depthLayout = value;
        }
        
        const std::string OffscreenBuffer::getImageLayoutName() const
        {
            return getLayoutName(_p->resolveImageLayout);
        }

        const std::string OffscreenBuffer::getDepthLayoutName() const
        {
            return getLayoutName(_p->depthLayout);
        }

        VkImageView OffscreenBuffer::getImageView() const
        {
            return _p->resolveImageView;
        }

        VkImageView OffscreenBuffer::getDepthImageView() const
        {
            return _p->depthImageView;
        }

        VkImage OffscreenBuffer::getImage() const
        {
            return _p->resolveImage;
        }

        VkImage OffscreenBuffer::getDepthImage() const
        {
            return _p->depthImage;
        }

        VkFramebuffer OffscreenBuffer::getFramebuffer() const
        {
            return _p->framebuffer;
        }

        VkRenderPass OffscreenBuffer::getClearRenderPass() const
        {
            return _p->clearRenderPass;
        }

        VkRenderPass OffscreenBuffer::getLoadRenderPass() const
        {
            return _p->loadRenderPass;
        }

        VkExtent2D OffscreenBuffer::getExtent() const
        {
            return {
                static_cast<uint32_t>(_p->size.w),
                static_cast<uint32_t>(_p->size.h)};
        }

        VkSampler OffscreenBuffer::getSampler() const
        {
            return _p->sampler;
        }

        VkRect2D OffscreenBuffer::getScissor() const
        {
            return _p->scissor;
        }

        VkViewport OffscreenBuffer::getViewport() const
        {
            return _p->viewport;
        }

        void OffscreenBuffer::createImageViews()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);

            // MS color view (only when multisampled)
            if (multisampled)
            {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = p.msColorImage;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = p.colorFormat;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device, &viewInfo, nullptr, &p.msColorImageView) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create MS color image view");
            }

            // Resolve view (always present - this is what the public API returns)
            {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = p.resolveImage;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = p.colorFormat;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device, &viewInfo, nullptr, &p.resolveImageView) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create resolve image view");
            }
        }

        //
        // This function clears all buffers
        //
        void OffscreenBuffer::createClearRenderPass()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            
            VkSampleCountFlagBits samples = getSampleCount();
            bool multisampled = (samples != VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkAttachmentDescription> attachments;

            // MS color (or single-sample if not MS)
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = p.colorFormat;
            colorAttachment.samples = multisampled ? samples : VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = multisampled ?
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE :
                                      VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.push_back(colorAttachment);

            // Resolve attachment
            VkAttachmentDescription resolveAttachment{};
            if (multisampled)
            {
                resolveAttachment.format = p.colorFormat;
                resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachments.push_back(resolveAttachment);
            }
            else
            {
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            }

            // Depth/stencil
            if (hasDepth() || hasStencil())
            {
                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = p.depthFormat;
                depthAttachment.samples = multisampled ? samples : VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.storeOp = p.options.storeDepth ?
                                          VK_ATTACHMENT_STORE_OP_STORE :
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments.push_back(depthAttachment);
            }

            // Subpass
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;

            VkAttachmentReference colorRef{};
            colorRef.attachment = 0;
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            subpass.pColorAttachments = &colorRef;

            VkAttachmentReference resolveRef{};
            if (multisampled)
            {
                resolveRef.attachment = 1;
                resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                subpass.pResolveAttachments = &resolveRef;
            }

            VkAttachmentReference depthRef{};
            if (hasDepth() || hasStencil())
            {
                depthRef.attachment = multisampled ? 2 : 1;
                depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpass.pDepthStencilAttachment = &depthRef;
            }

            VkRenderPassCreateInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            rpInfo.pAttachments = attachments.data();
            rpInfo.subpassCount = 1;
            rpInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(device, &rpInfo, nullptr, &p.clearRenderPass) != VK_SUCCESS)
                throw std::runtime_error("Failed to create clear render pass");
        }


        //
        // This function creates render pass does not clear the color and
        // depth buffers but DOES clear the stencil buffer.
        //
        void OffscreenBuffer::createLoadRenderPass()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkSampleCountFlagBits samples = getSampleCount();
            bool multisampled = (samples != VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkAttachmentDescription> attachments;

            // MS color
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = p.colorFormat;
            colorAttachment.samples = multisampled ? samples : VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // previous MS data is not preserved
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.push_back(colorAttachment);

            // Resolve
            VkAttachmentDescription resolveAttachment{};
            if (multisampled)
            {
                resolveAttachment.format = p.colorFormat;
                resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;   // try to keep previous resolved pixels
                resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachments.push_back(resolveAttachment);
            }
            else
            {
                // Non-MS case – load previous color
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[0] = colorAttachment; // update the one we just pushed
            }

            // Depth/stencil (same as original intent)
            if (hasDepth() || hasStencil())
            {
                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = p.depthFormat;
                depthAttachment.samples = multisampled ? samples : VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachment.storeOp = p.options.storeDepth ?
                                          VK_ATTACHMENT_STORE_OP_STORE :
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments.push_back(depthAttachment);
            }

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;

            VkAttachmentReference colorRef{};
            colorRef.attachment = 0;
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            subpass.pColorAttachments = &colorRef;

            VkAttachmentReference resolveRef{};
            if (multisampled)
            {
                resolveRef.attachment = 1;
                resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                subpass.pResolveAttachments = &resolveRef;
            }

            VkAttachmentReference depthRef{};
            if (hasDepth() || hasStencil())
            {
                depthRef.attachment = multisampled ? 2 : (multisampled ? 2 : 1);
                depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpass.pDepthStencilAttachment = &depthRef;
            }

            VkRenderPassCreateInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            rpInfo.pAttachments = attachments.data();
            rpInfo.subpassCount = 1;
            rpInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(device, &rpInfo, nullptr, &p.loadRenderPass) != VK_SUCCESS)
                throw std::runtime_error("Failed to create load render pass");
        }

        void OffscreenBuffer::createFramebuffer()
        {
            TLRENDER_P();
            VkDevice device = ctx.device;

            std::vector<VkImageView> attachments;
            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);

            if (multisampled)
            {
                attachments.push_back(p.msColorImageView);
                attachments.push_back(p.resolveImageView);
            }
            else
            {
                attachments.push_back(p.resolveImageView);
            }

            if (hasDepth() || hasStencil())
                attachments.push_back(p.depthImageView);

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = p.clearRenderPass;
            fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            fbInfo.pAttachments = attachments.data();
            fbInfo.width = p.size.w;
            fbInfo.height = p.size.h;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbInfo, nullptr, &p.framebuffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer");
        }

        uint32_t OffscreenBuffer::findMemoryType(
            uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(ctx.gpu, &memProps);

            for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) &&
                    (memProps.memoryTypes[i].propertyFlags & properties) ==
                        properties)
                {
                    return i;
                }
            }
            throw std::runtime_error("Failed to find suitable memory type");
        }

        void OffscreenBuffer::setupViewportAndScissor()
        {
            TLRENDER_P();

            p.viewport = {};
            p.viewport.x = 0.0f;
            // Set the y origin to the bottom of the framebuffer
            p.viewport.y = static_cast<float>(p.size.h);
            p.viewport.width = static_cast<float>(p.size.w);
            // Use a negative height to flip the y-axis
            p.viewport.height = -static_cast<float>(p.size.h);
            p.viewport.minDepth = 0.0f;
            p.viewport.maxDepth = 1.0f;

            p.scissor = {};
            p.scissor.offset = {0, 0};
            p.scissor.extent = {
                static_cast<uint32_t>(p.size.w),
                static_cast<uint32_t>(p.size.h)};
        }

        void OffscreenBuffer::setupScissor(const math::Box2i& value)
        {
            TLRENDER_P();
            
            if (value.w() > 0 && value.h() > 0)
            {
                p.scissor.offset = { value.x(), value.y() };
                p.scissor.extent = {
                    static_cast<uint32_t>(value.w()),
                    static_cast<uint32_t>(value.h())
                };
            }
        }
        
        void OffscreenBuffer::beginLoadRenderPass(VkCommandBuffer cmd,
                                                  VkSubpassContents contents)
        {
            TLRENDER_P();
            
            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkClearValue> clearValues;

            // Color attachment (index 0) — never cleared in load pass
            clearValues.push_back({});   // dummy (ignored)

            if (multisampled)
            {
                // Resolve attachment (index 1) — LOAD, so ignored for clear
                clearValues.push_back({});   // dummy
            }

            if (hasDepth() || hasStencil())
            {
                // Depth/stencil (index 2 when MS, index 1 when not) — stencil is cleared
                VkClearValue depthClear = {};
                depthClear.depthStencil = {1.0f, 0};
                clearValues.push_back(depthClear);
            }

            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = p.loadRenderPass;
            beginInfo.framebuffer = p.framebuffer;
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = getExtent();
            beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            beginInfo.pClearValues = clearValues.data();
            
            vkCmdBeginRenderPass(cmd, &beginInfo, contents);
            
            setupViewportAndScissor(cmd);
        }

        void OffscreenBuffer::beginClearRenderPass(VkCommandBuffer cmd,
                                                   VkSubpassContents contents)
        {
            TLRENDER_P();
            
            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkClearValue> clearValues;

            // Color clear (always for attachment 0)
            VkClearValue colorClear = {};
            const image::Color4f& color = p.options.clearColor;
            colorClear.color = {{color.r, color.g, color.b, color.a}};
            clearValues.push_back(colorClear);

            if (multisampled)
            {
                // Resolve attachment (index 1) uses DONT_CARE → value ignored, but slot required
                clearValues.push_back({});  // dummy
            }

            if (hasDepth() || hasStencil())
            {
                // Depth/stencil clear (attachment index 2 when MS, or 1 when not)
                VkClearValue depthClear = {};
                depthClear.depthStencil = {1.0f, 0};   // depth=1.0, stencil=0
                clearValues.push_back(depthClear);
            }

            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = p.clearRenderPass;
            beginInfo.framebuffer = p.framebuffer;
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = getExtent();
            beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            beginInfo.pClearValues = clearValues.data();
            
            vkCmdBeginRenderPass(cmd, &beginInfo, contents);
            
            setupViewportAndScissor(cmd);
        }

        void OffscreenBuffer::setupViewportAndScissor(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            setupViewportAndScissor();

            vkCmdSetViewport(cmd, 0, 1, &p.viewport);
            vkCmdSetScissor(cmd, 0, 1, &p.scissor);
        }

        void OffscreenBuffer::transitionToColorAttachment(VkCommandBuffer cmd)
        {
            TLRENDER_P();
            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);

            // MS color (if present)
            if (multisampled && p.msColorImageLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                transitionImageLayout(cmd, p.msColorImage, p.msColorImageLayout,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                p.msColorImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            // Resolve color (always)
            if (p.resolveImageLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                transitionImageLayout(cmd, p.resolveImage, p.resolveImageLayout,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                p.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
        
        void OffscreenBuffer::transitionToShaderRead(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            if (p.resolveImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                return;

            transitionImageLayout(cmd, p.resolveImage, p.resolveImageLayout,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            p.resolveImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        void OffscreenBuffer::endRenderPass(VkCommandBuffer cmd)
        {
            TLRENDER_P();
            vkCmdEndRenderPass(cmd);

            bool multisampled = (getSampleCount() != VK_SAMPLE_COUNT_1_BIT);
            if (multisampled)
                p.msColorImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            p.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if (hasDepth() || hasStencil())
                p.depthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            std::cerr << __FUNCTION__ << " now in VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL" << std::endl;
        }

        void OffscreenBuffer::transitionDepthToStencilAttachment(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            if (p.depthLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            {
                std::cerr << __FUNCTION__ << " already in VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL" << std::endl;
                return;
            }
            
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = p.depthLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = p.depthLayout == VK_IMAGE_LAYOUT_UNDEFINED ?
                                    0 :
                                    VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            barrier.image = p.depthImage;

            barrier.subresourceRange.aspectMask = 0;
            if (hasDepth())
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencil())
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;


            VkPipelineStageFlags initialPipeline = p.depthLayout == VK_IMAGE_LAYOUT_UNDEFINED ?
                                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT :
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            
            vkCmdPipelineBarrier(
                cmd,
                initialPipeline,  // or TOP_OF_PIPE_BIT if oldLayout was UNDEFINED
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            // Track layout
            p.depthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            
            std::cerr << __FUNCTION__ << " now in VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL" << std::endl;
        }

        void OffscreenBuffer::transitionDepthToShaderRead(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            if (p.depthLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
            {
                std::cerr << __FUNCTION__ << " already in VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL" << std::endl;
                return;
            }

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.image = p.depthImage;
            barrier.subresourceRange.aspectMask = 0;
            if (hasDepth())
                barrier.subresourceRange.aspectMask |=
                    VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencil())
                barrier.subresourceRange.aspectMask |=
                    VK_IMAGE_ASPECT_STENCIL_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                nullptr, 1, &barrier);

            // Track layout
            p.depthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            
            std::cerr << __FUNCTION__ << " now in VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL" << std::endl;
        }
        
        void OffscreenBuffer::createStagingBuffers()
        {
            TLRENDER_P();

            // Calculate bufferSize
            image::Info info(p.size.w, p.size.h, p.options.colorType);
            VkDeviceSize bufferSize = image::getDataByteCount(info);
            
            VkDevice device = ctx.device;

            // Create Staging buffers
            p.pboRing.resize(NUM_PBO_BUFFERS);
            for (int i = 0; i < NUM_PBO_BUFFERS; ++i)
            {
                auto& pbo = p.pboRing[i];

                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = bufferSize;
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                vkCreateBuffer(device, &bufferInfo, nullptr, &pbo.buffer);

                VkMemoryRequirements memReq;
                vkGetBufferMemoryRequirements(device, pbo.buffer, &memReq);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReq.size;


                //
                // Important:  we must use VK_MEMORY_PROPERTY_HOST_CACHED_BIT
                //             for fast readbacks and invalidate the memory
                //             before accessing the pointer.
                //
                allocInfo.memoryTypeIndex =
                    findMemoryType(
                    memReq.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

                vkAllocateMemory(device, &allocInfo, nullptr, &pbo.memory);
                vkBindBufferMemory(device, pbo.buffer, pbo.memory, 0);

                vkMapMemory(
                    device, pbo.memory, 0, bufferSize, 0, &pbo.mappedPtr);

                VkFenceCreateInfo fenceInfo{
                    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                fenceInfo.flags =
                    VK_FENCE_CREATE_SIGNALED_BIT; // allow reuse on first frame
                vkCreateFence(device, &fenceInfo, nullptr, &pbo.fence);
            }
        }
        
        void OffscreenBuffer::readPixels(VkCommandBuffer cmd,
                                         int32_t x, int32_t y,
                                         uint32_t w, uint32_t h)
        {
            TLRENDER_P();
            
            VkDevice device = ctx.device;

            auto& pbo = p.pboRing[p.writeIndex];
            VkResult result = vkWaitForFences(device, 1, &pbo.fence, VK_TRUE,
                                              UINT64_MAX); 
            if (result != VK_SUCCESS) {
                fprintf(stderr, "OffscreenBuffer: vkWaitForFences failed: %s\n", string_VkResult(result));
                return;
            }
            vkResetFences(device, 1, &pbo.fence);

            // Transition image to TRANSFER_SRC
            transitionImageLayout(cmd, p.resolveImage,
                                  p.resolveImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            
            // Setup copy region
            VkBufferImageCopy region{};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {x, y, 0};

            if (w == 0) w = p.size.w;
            if (h == 0) h = p.size.h;
            
            region.imageExtent = {w, h, 1};

            vkCmdCopyImageToBuffer(cmd, p.resolveImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   pbo.buffer, 1, &region);

            // Transition back if needed
            transitionImageLayout(cmd, p.resolveImage,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            p.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        void OffscreenBuffer::submitReadback(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            VkResult result;
            VkDevice device = ctx.device;
        
            VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;

            {
                std::lock_guard<std::mutex> lock(ctx.queue_mutex());
                result = vkQueueSubmit(ctx.queue(), 1, &submitInfo,
                                       p.pboRing[p.writeIndex].fence);
            }
            
            if (result != VK_SUCCESS)
            {
                fprintf(stderr,
                        "OffscreenBuffer::vkQueueSubmit failed: %s\n",
                        string_VkResult(result));
                return;
            }

            p.writeIndex = (p.writeIndex + 1) % NUM_PBO_BUFFERS;
        }

        

        // In OffscreenBuffer.cpp — add implementations:
        void OffscreenBuffer::readPixelsInline(VkCommandBuffer cmd,
                                               int32_t x, int32_t y,
                                               uint32_t w, uint32_t h)
        {
            TLRENDER_P();

            // No fence management here — completion is tracked externally
            // via FLTK's frame fence (see usd_window::flush()).
            // The ring still protects against overwriting a slot too quickly
            // because flush() waits synchronously each frame.
            auto& pbo = p.pboRing[p.writeIndex];

            VkBufferImageCopy region{};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {x, y, 0};
            if (w == 0) w = p.size.w;
            if (h == 0) h = p.size.h;
            region.imageExtent = {w, h, 1};

            transitionImageLayout(cmd, p.resolveImage,
                                  p.resolveImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            vkCmdCopyImageToBuffer(cmd, p.resolveImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   pbo.buffer, 1, &region);

            // Transition back so the image is ready as a shader source next
            transitionImageLayout(cmd, p.resolveImage,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            p.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            // Advance ring — the data will be in the slot we just recorded into
            p.writeIndex = (p.writeIndex + 1) % NUM_PBO_BUFFERS;
        }
        
        void* OffscreenBuffer::getInlineReadbackPtr()
        {
            TLRENDER_P();
            VkDevice device = ctx.device;

            // The last-written slot is one behind the current writeIndex
            int slot = (p.writeIndex - 1 + NUM_PBO_BUFFERS) % NUM_PBO_BUFFERS;
            auto& pbo = p.pboRing[slot];

            // Invalidate host cache so we see the GPU's writes
            VkMappedMemoryRange range{};
            range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = pbo.memory;
            range.offset = 0;
            range.size   = VK_WHOLE_SIZE;
            vkInvalidateMappedMemoryRanges(device, 1, &range);

            return pbo.mappedPtr;
        }
        
        VkResult OffscreenBuffer::getLatestReadPixels(void*& imageData)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            auto& pbo = p.pboRing[p.readIndex];
            
            // Check fence result without waiting indefinitely
            VkResult result = vkGetFenceStatus(device, pbo.fence); 

            if (result == VK_SUCCESS) 
            {
                VkMappedMemoryRange memoryRange = {};
                memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                memoryRange.memory = pbo.memory;
                memoryRange.offset = 0; 
                memoryRange.size = VK_WHOLE_SIZE;
                vkInvalidateMappedMemoryRanges(device, 1, &memoryRange);
    
                imageData = pbo.mappedPtr;

                p.readIndex = (p.readIndex + 1) % NUM_PBO_BUFFERS; 

                return VK_SUCCESS;
            }
            else
            {
                return result; 
            }
        }

        
        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>& offscreenBuffer,
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            bool out = false;
            out |= size.isValid() && !offscreenBuffer;
            out |= size.isValid() && offscreenBuffer &&
                   offscreenBuffer->getSize() != size;
            out |= offscreenBuffer && offscreenBuffer->getOptions() != options;
            return out;
        }
    } // namespace vlk
} // namespace tl
