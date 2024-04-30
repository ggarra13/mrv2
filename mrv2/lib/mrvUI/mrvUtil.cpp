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
    const char* kModule = "util";
}

namespace mrv
{
    Fl_SVG_Image* load_svg(const std::string& svg_name)
    {
        std::string svg_root = rootpath();
        svg_root += "/icons/";

        std::string file = svg_root + svg_name;
        Fl_SVG_Image* svg = new Fl_SVG_Image(file.c_str());
        if (!svg)
            return nullptr;

        switch (svg->fail())
        {
        case Fl_Image::ERR_FILE_ACCESS:
            // File couldn't load? show path + os error to user
            LOG_ERROR(file << ": " << strerror(errno));
            return nullptr;
        case Fl_Image::ERR_FORMAT:
            // Parsing error
            LOG_ERROR(file << ": couldn't decode image");
            return nullptr;
        }
        return svg;
    }

    bool          runningUnderXWayland()
    {
        bool out = false;
#ifdef __linux__
#    ifdef FLTK_USE_X11
        if (fl_x11_display())
        {
            const char* backend = fl_getenv("FLTK_BACKEND");
            if (backend && strcmp(backend, "x11") == 0)
            {
                out = true;
            }
        }
#    endif
#endif
        return out;
    }
} // namespace mrv
