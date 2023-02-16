// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_SVG_Image.H>

#include <string>

namespace mrv {
Fl_SVG_Image *load_svg(const std::string &file);
}
