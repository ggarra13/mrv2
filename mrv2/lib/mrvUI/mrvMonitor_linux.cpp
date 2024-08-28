

#include <FL/platform.H>

#ifdef FLTK_USE_X11
#    include "mrvUI/mrvMonitor_x11.cpp"
#endif

#ifdef FLTK_USE_WAYLAND
#    include "mrvUI/mrvMonitor_wayland.cpp"
#endif

namespace mrv
{
    namespace desktop
    {
        // Get the monitor name given its FLTK screen index
        std::string getMonitorName(int monitorIndex)
        {
            std::string out;
#ifdef FLTK_USE_X11
            if (fl_x11_display())
                out = getX11MonitorName(monitorIndex);
#endif
#ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
                out = getWaylandMonitorName(monitorIndex);
#endif
            return out;
        }
    } // namespace desktop
} // namespace mrv
