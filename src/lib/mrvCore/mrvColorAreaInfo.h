// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Box.h>
#include <tlCore/Color.h>

namespace mrv
{
    namespace area
    {
        using namespace tl;

        struct Channels
        {
            image::Color4f max;
            image::Color4f min;
            image::Color4f diff;
            image::Color4f mean;

            friend std::ostream& operator<<(std::ostream&, const Channels&);
        };

        struct Info
        {
            math::Box2i box;
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
