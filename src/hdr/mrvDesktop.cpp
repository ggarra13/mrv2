// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvOS.h"
#include "mrvString.h"

#include "mrvUI/mrvMonitor.h"

#undef Status
#undef None
#include "mrvFl/mrvIO.h"

#include <FL/Fl.H>
#include <FL/platform.H>

#include <string>

namespace
{
    const char* kModule = "desktop";
}

namespace mrv
{

    namespace desktop
    {
        bool X11()
        {
            bool out = false;

            fl_open_display();
#ifdef __linux__
#    ifdef FLTK_USE_X11
            if (fl_x11_display())
            {
                out = true;
            }
#    endif
#endif
            return out;
        }

        bool Wayland()
        {
            bool out = false;

            fl_open_display();
#ifdef __linux__
#    ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
            {
                out = true;
            }
#    endif
#endif
            return out;
        }

        bool XWayland()
        {
            bool out = false;

            fl_open_display();
#ifdef __linux__
#    ifdef FLTK_USE_WAYLAND
#        ifdef FLTK_USE_X11
            if (fl_x11_display())
            {
                const char* session = fl_getenv("XDG_SESSION_TYPE");
                if (session && strcmp(session, "wayland") == 0)
                {
                    const char* backend = fl_getenv("FLTK_BACKEND");
                    if (backend && strcmp(backend, "x11") == 0)
                    {
                        out = true;
                    }
                }
            }
#        endif
#    endif
#endif
            return out;
        }

        bool Windows()
        {
            bool out = false;
#ifdef _WIN32
            out = true;
#endif
            return out;
        }

        bool macOS()
        {
            bool out = false;
#ifdef __APPLE__
            out = true;
#endif
            return out;
        }
        

    } // namespace desktop
} // namespace mrv
