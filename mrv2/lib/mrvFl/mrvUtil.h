// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <memory>

#include <FL/Fl_SVG_Image.H>

#include <tlCore/Context.h>

namespace mrv
{
    void init(const std::shared_ptr<tl::system::Context>& context);
    Fl_SVG_Image* load_svg(const std::string& file);
} // namespace mrv
