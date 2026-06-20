// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <errno.h> // errno
#include <cstring> // strerror

#include "mrvCore/mrvHome.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvUtil.h"

#include "mrvFLTK/mrvCallbacks.h"

#include "mrvFl/mrvIO.h"

#include "mrvApp/mrvGlobals.h"


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

    Fl_SVG_Image* load_svg(const unsigned char* svg_data,
                           const size_t length,
                           const char* svg_name)
    {
        Fl_SVG_Image* svg = new Fl_SVG_Image(svg_name, svg_data, length);
        if (!svg)
            return nullptr;

        switch (svg->fail())
        {
        case Fl_Image::ERR_FILE_ACCESS:
            // File couldn't load? show path + os error to user
            LOG_ERROR("Inline SVG: " << strerror(errno));
            return nullptr;
        case Fl_Image::ERR_FORMAT:
            // Parsing error
            LOG_ERROR("Inline SVG: couldn't decode image");
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

    bool feature_needs_solo_or_later()
    {
        if (!mrv::app::soporta_annotations)
        {
            std::string question = _("This feature needs a Solo donation or "
                                      "later.\n\n"
                                     "Do you want to upgrade?");
            int ok = mrv::fl_choice(question.c_str(),
                                    _("No"), _("Yes"), nullptr,
                                    nullptr);
            if (ok)
            {
                unlock_features_cb(nullptr, nullptr);
            }
            return false;
        }
        return true;
    }

    bool feature_needs_standard_or_later()
    {
        if (!mrv::app::soporta_python)
        {
            std::string question = _("This feature needs a Standard donation "
                                     "or later.\n\n"
                                     "Do you want to upgrade?");
            int ok = mrv::fl_choice(question.c_str(),
                                    _("No"), _("Yes"), nullptr,
                                    nullptr);
            if (ok)
            {
                unlock_features_cb(nullptr, nullptr);
            }
            return false;
        }
        return true;
    }

    bool feature_needs_edit_or_later()
    {
        if (!mrv::app::soporta_editing)
        {
            std::string question = _("This feature needs an Edit donation "
                                     "or later.\n\n"
                                     "Do you want to upgrade?");
            int ok = mrv::fl_choice(question.c_str(),
                                    _("No"), _("Yes"), nullptr,
                                    nullptr);
            if (ok)
            {
                unlock_features_cb(nullptr, nullptr);
            }
            return false;
        }
        return true;
    }

    bool feature_needs_pro_or_later()
    {
        if (!mrv::app::soporta_voice)
        {
            std::string question = _("This feature needs a Pro donation "
                                     "or later.\n\n"
                                     "Do you want to upgrade?");
            int ok = mrv::fl_choice(question.c_str(),
                                    _("No"), _("Yes"), nullptr,
                                    nullptr);
            if (ok)
            {
                unlock_features_cb(nullptr, nullptr);
            }
            return false;
        }
        return true;
    }

} // namespace mrv
