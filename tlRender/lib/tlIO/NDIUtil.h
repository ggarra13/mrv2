// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <cstddef>

#include <tlDevice/NDI/NDI.h>
#include <tlDevice/OutputData.h>

#include <tlCore/Image.h>

#include <string>

namespace tl
{
    namespace ndi
    {
        //! Convert FourCC to a string.
        std::string FourCCString(const NDIlib_FourCC_video_type_e type);

        inline std::ostream&
        operator<<(std::ostream& o, const NDIlib_FourCC_video_type_e& fourcc)
        {
            // Extract each byte using bitwise AND and right shift operators
            char ch0 =
                (fourcc & 0xFF); // Extract the first byte (least significant)
            char ch1 =
                ((fourcc >> 8) &
                 0xFF); // Shift right by 8 bits, then extract the second byte
            char ch2 =
                ((fourcc >> 16) &
                 0xFF); // Shift right by 16 bits, then extract the third byte
            char ch3 =
                ((fourcc >> 24) &
                 0xFF); // Shift right by 24 bits, then extract the fourth byte
            o << ch0 << ch1 << ch2 << ch3;
            return o;
        }

    } // namespace ndi
} // namespace tl
