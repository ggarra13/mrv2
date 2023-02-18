// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/BBox.h>
#include <tlCore/Color.h>

namespace mrv
{
    namespace area
    {
        using namespace tl;

        struct Channels
        {
            imaging::Color4f max;
            imaging::Color4f min;
            imaging::Color4f diff;
            imaging::Color4f mean;

            friend std::ostream& operator<<(std::ostream&, const Channels&);
        };

        struct Info
        {
            math::BBox2i box;
            Channels rgba;
            Channels hsv;

            friend std::ostream& operator<<(std::ostream&, const Info&);
        };

        inline std::ostream& operator<<(std::ostream& o, const Channels& b)
        {
            return o << "max:  " << b.max << std::endl
                     << "min:  " << b.min << std::endl
                     << "diff: " << b.diff << std::endl
                     << "mean: " << b.mean << std::endl;
        }

        inline std::ostream& operator<<(std::ostream& o, const Info& b)
        {
            return o << "BOX: " << b.box << std::endl
                     << "RGBA: " << b.rgba << std::endl
                     << "HSVL: " << b.hsv << std::endl;
        }
    } // namespace area
} // namespace mrv
