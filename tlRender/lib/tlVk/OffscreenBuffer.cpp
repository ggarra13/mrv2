// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/OffscreenBuffer.h>

#include <tlVk/Vk.h>
#include <tlVk/Texture.h>
#include <tlVk/Util.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

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
                    colorFilters == other.colorFilters && depth == other.depth &&
                    stencil == other.stencil && sampling == other.sampling &&
                    clearColor == other.clearColor &&
                    clearDepth == other.clearDepth &&
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

            // PBO for reading from main FBO.
            uint32_t currentPBO = 0;
            std::vector<StagingBuffer> pboRing;

            // Vulkan handles
            VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
            VkFormat depthFormat = VK_FORMAT_UNDEFINED;

            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory imageMemory = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;

            VkImageLayout depthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImage depthImage = VK_NULL_HANDLE;
            VkDeviceMemory depthMemory = VK_NULL_HANDLE;
            VkImageView depthImageView = VK_NULL_HANDLE;

            VkRenderPass renderPass = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;

            VkFramebuffer framebufferCompositing = VK_NULL_HANDLE;

            VkSampler sampler = VK_NULL_HANDLE;

            VkViewport viewport = {};
            VkRect2D scissor = {};
        };

        void OffscreenBuffer::_init(
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            TLRENDER_P();

            p.size = size;
            p.options = options;
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

            if (p.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, p.sampler, nullptr);

            if (p.framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(device, p.framebuffer, nullptr);
            if (p.renderPass != VK_NULL_HANDLE)
                vkDestroyRenderPass(device, p.renderPass, nullptr);

            if (p.imageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.imageView, nullptr);
            if (p.image != VK_NULL_HANDLE)
                vkDestroyImage(device, p.image, nullptr);
            if (p.imageMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.imageMemory, nullptr);
            if (p.depthImageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, p.depthImageView, nullptr);
            if (p.depthImage != VK_NULL_HANDLE)
                vkDestroyImage(device, p.depthImage, nullptr);
            if (p.depthMemory != VK_NULL_HANDLE)
                vkFreeMemory(device, p.depthMemory, nullptr);

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
            p.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            p.depthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void OffscreenBuffer::initialize()
        {
            TLRENDER_P();

            createImage();
            createImageView();

            if (p.depthFormat != VK_FORMAT_UNDEFINED)
            {
                createDepthImage();
                createDepthImageView();
            }

            createRenderPass(p.options.clear,
                             p.options.clearDepth);
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

        bool OffscreenBuffer::hasDepth() const
        {
            return (_p->options.depth != OffscreenDepth::kNone);
        }

        bool OffscreenBuffer::hasStencil() const
        {
            return (_p->options.stencil != OffscreenStencil::kNone);
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
            return _p->imageLayout;
        }

        VkImageLayout OffscreenBuffer::getDepthLayout() const
        {
            return _p->depthLayout;
        }
        
        const std::string OffscreenBuffer::getImageLayoutName() const
        {
            return getLayoutName(_p->imageLayout);
        }

        const std::string OffscreenBuffer::getDepthLayoutName() const
        {
            return getLayoutName(_p->depthLayout);
        }

        VkImageView OffscreenBuffer::getImageView() const
        {
            return _p->imageView;
        }

        VkImage OffscreenBuffer::getImage() const
        {
            return _p->image;
        }

        VkFramebuffer OffscreenBuffer::getFramebuffer() const
        {
            return _p->framebuffer;
        }

        VkRenderPass OffscreenBuffer::getRenderPass() const
        {
            return _p->renderPass;
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

        void OffscreenBuffer::createImage()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkImageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = VK_IMAGE_TYPE_2D;
            info.extent.width = static_cast<uint32_t>(p.size.w);
            info.extent.height = static_cast<uint32_t>(p.size.h);
            info.extent.depth = 1;
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.format = p.colorFormat;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                         VK_IMAGE_USAGE_SAMPLED_BIT |
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            VkSampleCountFlagBits samples = getVulkanSamples(p.options.sampling);
            info.samples = samples;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(device, &info, nullptr, &p.image) != VK_SUCCESS)
                throw std::runtime_error("Failed to create offscreen image");

            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(device, p.image, &memReq);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &p.imageMemory) !=
                VK_SUCCESS)
                throw std::runtime_error(
                    "Failed to allocate offscreen image memory");

            vkBindImageMemory(device, p.image, p.imageMemory, 0);
        }

        void OffscreenBuffer::createImageView()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = p.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = p.colorFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr, &p.imageView) !=
                VK_SUCCESS)
                throw std::runtime_error("Failed to create image view");
        }

        void OffscreenBuffer::createRenderPass(bool clearColor,
                                               bool clearDepth)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = p.colorFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = clearColor ?
                                     VK_ATTACHMENT_LOAD_OP_CLEAR :
                                     VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkImageLayout initialLayout = clearColor ?
                                          VK_IMAGE_LAYOUT_UNDEFINED :
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.initialLayout = initialLayout;
            colorAttachment.finalLayout =
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorRef{};
            colorRef.attachment = 0;
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorRef;

            std::vector<VkAttachmentDescription> attachments;
            attachments.push_back(colorAttachment);

            VkAttachmentReference depthRef{};

            if (p.depthFormat != VK_FORMAT_UNDEFINED)
            {
                depthRef.attachment = 1;
                depthRef.layout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                subpass.pDepthStencilAttachment = &depthRef;

                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = p.depthFormat;
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = clearDepth ?
                                         VK_ATTACHMENT_LOAD_OP_CLEAR :
                                         VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.stencilStoreOp =
                    VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments.push_back(depthAttachment);
            }

            VkRenderPassCreateInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            rpInfo.pAttachments = attachments.data();
            rpInfo.subpassCount = 1;
            rpInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(
                    device, &rpInfo, nullptr, &p.renderPass) !=
                VK_SUCCESS)
                throw std::runtime_error(
                    "Failed to create compositing render pass");
        }


        void OffscreenBuffer::createFramebuffer()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            std::vector<VkImageView> attachments;
            attachments.push_back(p.imageView);
            if (p.depthFormat != VK_FORMAT_UNDEFINED)
                attachments.push_back(p.depthImageView);

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = p.renderPass;
            fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            fbInfo.pAttachments = attachments.data();
            fbInfo.width = p.size.w;
            fbInfo.height = p.size.h;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(device, &fbInfo, nullptr, &p.framebuffer) !=
                VK_SUCCESS)
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

        void OffscreenBuffer::transitionToShaderRead(VkCommandBuffer cmd)
        {
            TLRENDER_P();
            
            if (p.imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                return;
            }
            
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.image = p.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                nullptr, 1, &barrier);

            // Track layout
            p.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

        void OffscreenBuffer::beginRenderPass(
            VkCommandBuffer cmd, const std::string& name,
            VkSubpassContents contents
            )
        {
            TLRENDER_P();
            
            std::vector<VkClearValue> clearValues;
            VkClearValue colorClear = {};
            const image::Color4f& color = p.options.clearColor;
            colorClear.color = {{color.r, color.g, color.b, color.a}}; // Black clear
            clearValues.push_back(colorClear);

            if (p.depthFormat != VK_FORMAT_UNDEFINED)
            {
                VkClearValue depthClear = {};
                depthClear.depthStencil = {1.0f, 0};
                clearValues.push_back(depthClear);
            }

            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = p.renderPass;
            beginInfo.framebuffer = p.framebuffer;
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = {
                static_cast<uint32_t>(p.size.w),
                static_cast<uint32_t>(p.size.h)};
            beginInfo.clearValueCount =
                static_cast<uint32_t>(clearValues.size());
            beginInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(cmd, &beginInfo, contents);
        }

        void OffscreenBuffer::endRenderPass(VkCommandBuffer cmd)
        {
            vkCmdEndRenderPass(cmd);
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

            if (p.imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                return;
            }
            
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = p.imageLayout; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.image = p.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
                nullptr, 1, &barrier);

            // Track layout
            p.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        void OffscreenBuffer::transitionDepthToShaderRead(VkCommandBuffer cmd)
        {
            TLRENDER_P();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
            p.depthLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
                allocInfo.memoryTypeIndex =
                    findMemoryType(
                    memReq.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
            VkCommandPool commandPool = ctx.commandPool;
            VkQueue  queue  = ctx.queue;

            auto& pbo = p.pboRing[p.currentPBO];
            vkWaitForFences(device, 1, &pbo.fence, VK_TRUE, UINT64_MAX);
            vkResetFences(device, 1, &pbo.fence);

            // Transition image to TRANSFER_SRC
            transitionImageLayout(cmd, device, commandPool, queue,
                                  p.image,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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

            vkCmdCopyImageToBuffer(cmd, p.image,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   pbo.buffer, 1, &region);

            // Transition back if needed
            transitionImageLayout(cmd, device, commandPool, queue, p.image,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }

        void OffscreenBuffer::submitReadback(VkCommandBuffer cmd)
        {
            TLRENDER_P();
            
            VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;

            // Should we use another queue?  For faster feedback yes.
            VkQueue transferQueue = ctx.queue;
            
            vkQueueSubmit(transferQueue, 1, &submitInfo,
                          p.pboRing[p.currentPBO].fence);
            p.currentPBO = (p.currentPBO + 1) % NUM_PBO_BUFFERS;
        }

        void* OffscreenBuffer::getLatestReadPixels()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            
            int readIndex = (p.currentPBO + NUM_PBO_BUFFERS - 1) %
                            NUM_PBO_BUFFERS;
            auto& pbo = p.pboRing[readIndex];

            if (vkGetFenceStatus(device, pbo.fence) == VK_SUCCESS)
            {
                return pbo.mappedPtr; // valid data
            }
            return nullptr; // not ready yet
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
