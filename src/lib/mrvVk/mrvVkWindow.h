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

            void show() FL_OVERRIDE;

            void valid(int x) {};
            int valid() const { return 1; }

            void prepare() FL_OVERRIDE { prepare_render_pass(); };
            void destroy_resources() FL_OVERRIDE {};
        
#ifdef __APPLE__
        protected:
            void set_window_transparency(double alpha);
#endif
            void prepare_render_pass();

            void _init();
        };

    } // namespace vulkan
    
} // namespace mrv
