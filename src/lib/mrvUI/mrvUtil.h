// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_XPM_Image.H>

#define MRV2_LOAD_SVG(name) mrv::load_svg(name ## _svg, name ## _svg_size)

namespace mrv
{
    Fl_SVG_Image* load_svg(const std::string& file);
    Fl_SVG_Image* load_svg(const unsigned char* data,
                           const size_t  length,
                           const char* file = nullptr);
    Fl_PNG_Image* load_png(const std::string& file);
    Fl_XPM_Image* load_xpm(const std::string& file);
} // namespace mrv
