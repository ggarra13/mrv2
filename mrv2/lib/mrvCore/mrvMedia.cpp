// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvMedia.h"

namespace mrv {



int Media::colorspace_override; //!< Override YUV Hint always with this

double      Media::default_fps = 24.f;

std::string Media::ocio_8bits_ics;
std::string Media::ocio_16bits_ics;
std::string Media::ocio_32bits_ics;
std::string Media::ocio_float_ics;

double Media::thumbnail_percent = 0.0f;

std::string Media::default_subtitle_font = "Arial";
std::string Media::default_subtitle_encoding = "utf-8";
bool Media::_aces_metadata = false;
bool Media::_ocio_color_space = false;
bool Media::_all_layers = false;
bool Media::_cache_active = true;
bool Media::_preload_cache = true;
bool Media::_8bit_cache = false;
int  Media::_cache_scale = 0;

}
