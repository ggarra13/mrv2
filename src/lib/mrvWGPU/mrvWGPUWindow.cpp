// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvWGPU/mrvWGPUWindow.h"

namespace
{
    const char* kModule = "wgpu";
}

namespace mrv
{
    namespace webgpu
    {

        WGPUWindow::WGPUWindow(int X, int Y, int W, int H, const char* L) :
            Fl_WebGPU_Window(X, Y, W, H, L)
        {

        }

        WGPUWindow::WGPUWindow(int W, int H, const char* L) :
            Fl_WebGPU_Window(W, H, L)
        {
        }

        WGPUWindow::~WGPUWindow()
        {
        }

    } // namespace webgpu

} // namespace mrv
