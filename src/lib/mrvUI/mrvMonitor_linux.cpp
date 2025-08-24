// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <map>

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
        // We cache the names as getting them from X11 can be slow
        std::map<int, std::string> names;
        
        // Get the monitor name given its FLTK screen index
        std::string getName(int monitorIndex, int numMonitors)
        {
            std::string out;

            if (names.find(monitorIndex) != names.end() &&
                !names[monitorIndex].empty())
            {
                return names[monitorIndex];
            }
            
#ifdef FLTK_USE_X11
            if (fl_x11_display())
                out = getX11Name(monitorIndex);
#endif
#ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
                out = getWaylandName(monitorIndex, numMonitors);
#endif

            names[monitorIndex] = out;
            
            return out;
        }
        
        bool is_hdr_active(int screen, const bool silent)
        {
#ifdef FLTK_USE_X11
            if (fl_x11_display())
                return true;
#endif
#ifdef FLTK_USE_WAYLAND
            // if (fl_wl_display())
            //     return WaylandIsHdrActive(screen, true);
            return true;
#endif
        }
    } // namespace desktop
} // namespace mrv
