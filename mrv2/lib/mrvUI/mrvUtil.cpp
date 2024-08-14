// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <errno.h> // errno
#include <cstring> // strerror

#include "mrvCore/mrvHome.h"

#include "mrvUI/mrvUtil.h"

#include "mrvFl/mrvIO.h"


namespace
{
    const char* kModule = "util";
}

namespace mrv
{
    Fl_SVG_Image* load_svg(const std::string& svg_name)
    {
        const std::string svg_root = iconpath();
        
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
    
    Fl_PNG_Image* load_png(const std::string& png_name)
    {
        const std::string png_root = iconpath();
        
        std::string file = png_root + png_name;
        Fl_PNG_Image* png = new Fl_PNG_Image(file.c_str());
        if (!png)
            return nullptr;

        switch (png->fail())
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
        return png;
    }
    
    Fl_XPM_Image* load_xpm(const std::string& xpm_name)
    {
        const std::string xpm_root = iconpath();
        
        std::string file = xpm_root + xpm_name;
        Fl_XPM_Image* xpm = new Fl_XPM_Image(file.c_str());
        if (!xpm)
            return nullptr;

        switch (xpm->fail())
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
        return xpm;
    }

} // namespace mrv
