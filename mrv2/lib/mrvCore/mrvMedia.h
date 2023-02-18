// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{

    struct Media
    {

        static int colorspace_override; //!< Override YUV Hint always with this

        static double default_fps; //!< Default FPS when not selected
        static std::string ocio_8bits_ics;
        static std::string ocio_16bits_ics;
        static std::string ocio_32bits_ics;
        static std::string ocio_float_ics;

        static double thumbnail_percent;

        static std::string default_subtitle_font;
        static std::string default_subtitle_encoding;
        static bool _aces_metadata;
        static bool _ocio_color_space;
        static bool _all_layers;
        static bool _8bit_cache;
        static bool _cache_active;
        static bool _preload_cache;
        static int _cache_scale;
        static bool _initialize;
    };

} // namespace mrv
