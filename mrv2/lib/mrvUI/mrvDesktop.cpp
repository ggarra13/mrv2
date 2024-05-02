// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <errno.h> // errno
#include <cstring> // strerror

#include "mrvCore/mrvHome.h"

#include "mrvUI/mrvUtil.h"

#include "mrvFl/mrvIO.h"

#include <FL/Fl.H>
#include <FL/platform.H>

namespace
{
    const char* kModule = "desktop";
}

namespace mrv
{

    namespace desktop
    {
        bool XWayland()
        {
            bool out = false;
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
        
    }  // desktop
} // namespace mrv
