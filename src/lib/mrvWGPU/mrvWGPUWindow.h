// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_WebGPU_Context.H>
#include <FL/Fl_WebGPU_Window.H>

#include "mrvOptions/mrvEnums.h"

namespace mrv
{
    namespace webgpu
    {
        //
        // This class implements common Webgpu functionality
        //
        class WGPUWindow : public Fl_WebGPU_Window
        {
        public:
            WGPUWindow(int X, int Y, int W, int H, const char* L = 0);
            WGPUWindow(int W, int H, const char* L = 0);
            ~WGPUWindow();


            void headless(bool) {};
            bool headless() { return false; };

            void refresh() {};

            void render_offscreen() {};
        };

    }
} // namespace mrv
