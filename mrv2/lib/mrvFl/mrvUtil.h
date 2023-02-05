// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include <FL/Fl_SVG_Image.H>

namespace mrv
{

    Fl_SVG_Image* load_svg( const std::string& file );
    std::string readShaderSource( const std::string& filename );
  
}
