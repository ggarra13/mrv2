// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
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
        };

        
        struct Info
        {
            math::BBox2i     box;
            Channels         rgba;
            Channels         hsv;
        };
    }
}
