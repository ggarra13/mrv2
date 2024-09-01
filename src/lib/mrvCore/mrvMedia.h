// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{

    struct Media
    {
        static std::string ocio_8bits_ics;
        static std::string ocio_16bits_ics;
        static std::string ocio_32bits_ics;
        static std::string ocio_half_ics;
        static std::string ocio_float_ics;

        // @todo:
        static std::string default_subtitle_font;
        static std::string default_subtitle_encoding;
    };

} // namespace mrv
