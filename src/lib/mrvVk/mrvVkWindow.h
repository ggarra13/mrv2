// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Vk_Window.H>

#include "mrvOptions/mrvEnums.h"

namespace mrv
{
    namespace vulkan
    {

        //
        // This class implements coomon Vulkan functionality
        //
        class VkWindow : public Fl_Vk_Window
        {

        public:
            VkWindow(int X, int Y, int W, int H, const char* L = 0);
            VkWindow(int W, int H, const char* L = 0);

            Fl_Vk_Context& getContext() { return ctx; } 
            
            void show() FL_OVERRIDE;

            void refresh() {};

            void valid(int x) {};
            int valid() const { return 1; }
        
#ifdef __APPLE__
        protected:
            void set_window_transparency(double alpha);
#endif
            //! Main swapchain render pass (common to all Vulkan windows).
            void prepare_render_pass(); 

            void _init();

            VkFormat _oldRenderPassFormat = VK_FORMAT_UNDEFINED;
        };

    } // namespace vulkan
    
} // namespace mrv
