// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Vk_Window.H>
#undef None
#undef Status

#include "mrvVk/mrvEnums.h"

namespace mrv
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

        void valid(int) {};

#ifdef __APPLE__
    protected:
        void set_window_transparency(double alpha);
#endif
    };
} // namespace mrv
