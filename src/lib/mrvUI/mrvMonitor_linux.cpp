// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include <FL/platform.H>

#ifdef FLTK_USE_X11
#    include "mrvUI/mrvMonitor_x11.cpp"
#endif

#ifdef FLTK_USE_WAYLAND
#    include "mrvUI/mrvMonitor_wayland.cpp"
#endif

namespace mrv
{
    namespace monitor
    {
        // Get the monitor name given its FLTK screen index
        std::string getName(int monitorIndex)
        {
            std::string out;
#ifdef FLTK_USE_X11
            if (fl_x11_display())
                out = getX11Name(monitorIndex);
#endif
#ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
                out = getWaylandName(monitorIndex);
#endif
            return out;
        }
    } // namespace desktop
} // namespace mrv
