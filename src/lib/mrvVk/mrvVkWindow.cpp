// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/platform.H>
#include <FL/Fl_Vk_Utils.H>

#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
#    include <wayland-client.h>
#  endif
#endif

#include "mrvVk/mrvVkWindow.h"

namespace mrv
{
    namespace vulkan
    {

        VkWindow::VkWindow(int X, int Y, int W, int H, const char* L) :
            Fl_Vk_Window(X, Y, W, H, L)
        {
            _init();
        }

        VkWindow::VkWindow(int W, int H, const char* L) :
            Fl_Vk_Window(W, H, L)
        {
            _init();
        }

        void VkWindow::_init()
        {
            m_debugSync = false;
        }

        // m_depth (optionally) -> creates m_renderPass
        void VkWindow::prepare_render_pass() 
        {
            if (m_renderPass != VK_NULL_HANDLE &&
                _oldRenderPassFormat == ctx.format)
            {
                return;
            }
            
            bool has_depth = mode() & FL_DEPTH;
            bool has_stencil = mode() & FL_STENCIL;

            VkAttachmentDescription attachments[2];
            attachments[0] = VkAttachmentDescription();
            attachments[0].format = format();
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Start undefined
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

            attachments[1] = VkAttachmentDescription();


            VkAttachmentReference color_reference = {};
            color_reference.attachment = 0;
            color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
            VkAttachmentReference depth_reference = {};
            depth_reference.attachment = 1;
            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.flags = 0;
            subpass.inputAttachmentCount = 0;
            subpass.pInputAttachments = NULL;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_reference;
            subpass.pResolveAttachments = NULL;

            if (has_depth || has_stencil)
            {
                attachments[1].format = m_depth.format;
                attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                if (has_stencil)
                {
                    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                }
                else
                {
                    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                }
                attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[1].initialLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments[1].finalLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
                subpass.pDepthStencilAttachment = &depth_reference;
                subpass.preserveAttachmentCount = 0;
                subpass.pPreserveAttachments = NULL;
            }

            VkSubpassDependency dependencies[2] = {};

            // External -> subpass: wait for color attachment output before writing
            dependencies[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass    = 0;
            dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = 0;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = 0;

            // Subpass -> external: ensure writes are done before presentation reads
            dependencies[1].srcSubpass    = 0;
            dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = 0;
            dependencies[1].dependencyFlags = 0;

            VkRenderPassCreateInfo rp_info = {};
            rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rp_info.pNext = NULL;
            rp_info.attachmentCount = (has_depth || has_stencil) ? 2: 1;
            rp_info.pAttachments = attachments;
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 2;
            rp_info.pDependencies = dependencies;
                    
            VkResult result;
            result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
            VK_CHECK(result);

            _oldRenderPassFormat = ctx.format;
        }


        void VkWindow::show()
        {
            Fl_Vk_Window::show();

#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
            // \@todo: Not sure if this is needed.  It was added to provide
            //         transparency on Wayland surfaces.
            if (fl_wl_display())
            {
                wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)), NULL);
            }
#  endif
#endif
        }

    }

    
} // namespace mrv
