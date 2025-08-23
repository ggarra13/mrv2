// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/platform.H>
#include <FL/Fl_Vk_Utils.H>


#ifdef FLTK_USE_WAYLAND
#    include <wayland-client.h>
#endif

#include "mrvVk/mrvVkWindow.h"

namespace
{
    const char* kModule = "vk";
}

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
            m_validate = false;
            m_debugSync = false;
#ifndef NDEBUG
            // m_validate = true;
#endif
        }

        // m_depth (optionally) -> creates m_renderPass
        void VkWindow::prepare_render_pass() 
        {
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

            VkRenderPassCreateInfo rp_info = {};
            rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rp_info.pNext = NULL;
            rp_info.attachmentCount = (has_depth || has_stencil) ? 2: 1;
            rp_info.pAttachments = attachments;
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 0;
            rp_info.pDependencies = NULL;
                    
            VkResult result;
            result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
            VK_CHECK(result);
        }


        void VkWindow::show()
        {
            Fl_Vk_Window::show();

#ifdef FLTK_USE_WAYLAND
            // Not sure if this is needed
            if (fl_wl_display())
            {
                wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)), NULL);
            }
#endif
        }

    }

    
} // namespace mrv
