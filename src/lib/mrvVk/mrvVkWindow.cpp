// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/platform.H>


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

    VkWindow::VkWindow(int X, int Y, int W, int H, const char* L) :
        Fl_Vk_Window(X, Y, W, H, L)
    {
    }

    VkWindow::VkWindow(int W, int H, const char* L) :
        Fl_Vk_Window(W, H, L)
    {
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

} // namespace mrv
