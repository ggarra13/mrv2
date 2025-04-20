// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/OffscreenBuffer.h>

#include <tlVk/Vk.h>
#include <tlVk/Texture.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace vk
    {
        TLRENDER_ENUM_IMPL(OffscreenDepth, "None", "16", "24", "32");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenDepth);

        TLRENDER_ENUM_IMPL(OffscreenStencil, "None", "8");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenStencil);

        TLRENDER_ENUM_IMPL(OffscreenSampling, "None", "2", "4", "8", "16");
        TLRENDER_ENUM_SERIALIZE_IMPL(OffscreenSampling);

        namespace
        {
            enum class Error { ColorTexture, RenderBuffer, Create, Init };

            std::string getErrorLabel(Error error)
            {
                std::string out;
                switch (error)
                {
                case Error::ColorTexture:
                    out = "Cannot create color texture";
                    break;
                case Error::RenderBuffer:
                    out = "Cannot create render buffer";
                    break;
                case Error::Create:
                    out = "Cannot create frame buffer";
                    break;
                case Error::Init:
                    out = "Cannot initialize frame buffer";
                    break;
                default:
                    break;
                }
                return out;
            }

            VkFormat getBufferInternalFormat(
                OffscreenDepth depth, OffscreenStencil stencil)
            {
                VkFormat out = VK_FORMAT_D32_SFLOAT_S8_UINT;
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
            return colorType == other.colorType &&
                   colorFilters == other.colorFilters && depth == other.depth &&
                   stencil == other.stencil && sampling == other.sampling;
        }

        bool OffscreenBufferOptions::operator!=(
            const OffscreenBufferOptions& other) const
        {
            return !(*this == other);
        }

        struct OffscreenBuffer::Private
        {
            math::Size2i size;
            OffscreenBufferOptions options;

            // Vulkan handles
            VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
            VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory imageMemory = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            
            VkImage depthImage = VK_NULL_HANDLE;
            VkDeviceMemory depthMemory = VK_NULL_HANDLE;
            VkImageView depthImageView = VK_NULL_HANDLE;

            VkRenderPass renderPass = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
        };

        void OffscreenBuffer::_init(
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            TLRENDER_P();
            p.size = size;
            p.options = options;
            p.colorFormat = getTextureInternalFormat(p.options.colorType);
            p.depthFormat = getBufferInternalFormat(p.options.depth,
                                                    p.options.stencil);
            
            initialize();

            // Get maximum texture resolution for gfx card
            // GLint glMaxTexDim;
            // glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);

            // if (p.size.w > glMaxTexDim)
            //     p.size.w = glMaxTexDim;
            // if (p.size.h > glMaxTexDim)
            //     p.size.h = glMaxTexDim;


            // Create the color texture.
            if (p.options.colorType != image::PixelType::kNone)
            {
                // glGenTextures(1, &p.colorID);
                // if (!p.colorID)
                // {
                //     throw std::runtime_error(
                //         getErrorLabel(Error::ColorTexture));
                // }
                // glBindTexture(target, p.colorID);
                switch (p.options.sampling)
                {
#if defined(TLRENDER_API_GL_4_1)
                case OffscreenSampling::_2:
                case OffscreenSampling::_4:
                case OffscreenSampling::_8:
                case OffscreenSampling::_16:
                    // glTexImage2DMultisample(
                    //     target, static_cast<GLsizei>(samples),
                    //     getTextureInternalFormat(p.options.colorType),
                    //     p.size.w, p.size.h, false);
                    break;
#endif // TLRENDER_API_GL_4_1
                default:
                    // glTexParameteri(
                    //     target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    // glTexParameteri(
                    //     target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    // glTexParameteri(
                    //     target, GL_TEXTURE_MIN_FILTER,
                    //     getTextureFilter(p.options.colorFilters.minify));
                    // glTexParameteri(
                    //     target, GL_TEXTURE_MAG_FILTER,
                    //     getTextureFilter(p.options.colorFilters.magnify));
                    // glTexImage2D(
                    //     target, 0,
                    //     getTextureInternalFormat(p.options.colorType),
                    //     p.size.w, p.size.h, 0,
                    //     getTextureFormat(p.options.colorType),
                    //     getTextureType(p.options.colorType), 0);
                    break;
                }
            }
        }

        OffscreenBuffer::OffscreenBuffer(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        OffscreenBuffer::~OffscreenBuffer()
        {
            TLRENDER_P();

            cleanup();
        }

        void OffscreenBuffer::cleanup()
        {
            TLRENDER_P();
            
            VkDevice device = ctx.device;
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
        }
        
        void OffscreenBuffer::initialize()
        {
            createImage();
            createImageView();
            createDepthImage();
            createDepthImageView();
            createRenderPass();
            createFramebuffer();
        }

        void OffscreenBuffer::createDepthImage()
        {
            TLRENDER_P();
            
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = {
                static_cast<uint32_t>(p.size.w),
                static_cast<uint32_t>(p.size.h), 1
            };
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = p.depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(ctx.device, &imageInfo, nullptr,
                              &p.depthImage) != VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image");

            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(ctx.device, p.depthImage, &memReq);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(ctx.device, &allocInfo, nullptr,
                                 &p.depthMemory) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate depth memory");

            vkBindImageMemory(ctx.device, p.depthImage, p.depthMemory, 0);
        }

        void OffscreenBuffer::createDepthImageView() {
            TLRENDER_P();
            
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = p.depthImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = p.depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            
            if (vkCreateImageView(ctx.device, &viewInfo, nullptr,
                                  &p.depthImageView) != VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image view");
        }
        
        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            Fl_Vk_Context& context,
            const math::Size2i& size, const OffscreenBufferOptions& options)
        {
            auto out = std::shared_ptr<OffscreenBuffer>(new OffscreenBuffer(context));
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

        const OffscreenBufferOptions& OffscreenBuffer::getOptions() const
        {
            return _p->options;
        }
        
        VkImageView      OffscreenBuffer::getImageView() const
        {
            return _p->imageView;
        }
        
        VkImage          OffscreenBuffer::getImage() const
        {
            return _p->image;
        }

        VkFramebuffer    OffscreenBuffer::getFramebuffer() const
        {
            return _p->framebuffer;
        }
        
        VkRenderPass     OffscreenBuffer::getRenderPass() const
        {
            return _p->renderPass;
        }
        
        VkExtent2D OffscreenBuffer::getExtent() const
        {
            return { static_cast<uint32_t>(_p->size.w),
                     static_cast<uint32_t>(_p->size.h) };
        }
        
        void OffscreenBuffer::createImage()
        {
            TLRENDER_P();
            
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
                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(ctx.device, &info, nullptr, &p.image) !=
                VK_SUCCESS)
                throw std::runtime_error("Failed to create offscreen image");

            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(ctx.device, p.image, &memReq);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(ctx.device, &allocInfo, nullptr,
                                 &p.imageMemory) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate offscreen image memory");

            vkBindImageMemory(ctx.device, p.image, p.imageMemory, 0);
        }

        void OffscreenBuffer::createImageView()
        {
            TLRENDER_P();
            
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = p.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = p.colorFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(ctx.device, &viewInfo, nullptr,
                                  &p.imageView)
                != VK_SUCCESS)
                throw std::runtime_error("Failed to create image view");
        }

        void OffscreenBuffer::createRenderPass()
        {
            TLRENDER_P();
            
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = p.colorFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = p.depthFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorRef{};
            colorRef.attachment = 0;
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthRef{};
            depthRef.attachment = 1;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorRef;
            subpass.pDepthStencilAttachment = &depthRef;

            std::array<VkAttachmentDescription, 2> attachments = {
                colorAttachment,
                depthAttachment
            };

            VkRenderPassCreateInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            rpInfo.pAttachments = attachments.data();
            rpInfo.subpassCount = 1;
            rpInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(ctx.device, &rpInfo, nullptr, &p.renderPass)
                != VK_SUCCESS)
                throw std::runtime_error("Failed to create render pass");
        }

        void OffscreenBuffer::createFramebuffer()
        {
            TLRENDER_P();
            
            std::array<VkImageView, 2> attachments = {
                p.imageView,
                p.depthImageView
            };
    
            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = p.renderPass;
            fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            fbInfo.pAttachments = attachments.data();
            fbInfo.width = p.size.w;
            fbInfo.height = p.size.h;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(ctx.device, &fbInfo, nullptr,
                                    &p.framebuffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer");
        }
        
        uint32_t OffscreenBuffer::findMemoryType(uint32_t typeFilter,
                                                 VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(ctx.gpu, &memProps);

            for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) &&
                    (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("Failed to find suitable memory type");
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

        struct OffscreenBufferBinding::Private
        {
            std::shared_ptr<OffscreenBuffer> buffer;
            uint32_t previous = 0;
        };

        OffscreenBufferBinding::OffscreenBufferBinding(
            const std::shared_ptr<OffscreenBuffer>& buffer) :
            _p(new Private)
        {
            TLRENDER_P();
            p.buffer = buffer;
            // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &p.previous);
            // p.buffer->bind();
        }

        OffscreenBufferBinding::~OffscreenBufferBinding()
        {
            // glBindFramebuffer(GL_FRAMEBUFFER, _p->previous);
        }
    } // namespace vk


    /* Usage:
       
       vk::OffscreenBuffer fbo(ctx, size, OffscreenOptions);
       
       // Clear red + clear depth/stencil
       VkClearValue clearValues[2];
       clearValues[0].color = { 1.f, 0.f, 0.f, 1.f };
       clearValues[1].depthStencil = { 1.0f, 0 };

       VkRenderPassBeginInfo rpBegin{};
       rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
       rpBegin.renderPass = fbo.getRenderPass();
       rpBegin.framebuffer = fbo.getFramebuffer();
       rpBegin.renderArea.extent = fbo.getExtent();
       rpBegin.clearValueCount = 2;
       rpBegin.pClearValues = clearValues;

       vkCmdBeginRenderPass(cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
       // Draw commands here...
       vkCmdEndRenderPass(cmdBuffer);

    */

} // namespace tl
