// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv
{   
    struct EnvironmentMapOptions
    {
        enum Type
        {
            kNone,
            kSpherical,
            kCubic,
        };

        Type  type = kNone;
        float horizontalAperture = 24.0F;
        float verticalAperture = 0.F;
        float focalLength =  45.F;
        float rotateX =   0.F;
        float rotateY =   0.F;
        unsigned subdivisionX = 36;
        unsigned subdivisionY = 36;

        bool operator==( const EnvironmentMapOptions& b ) const;
        bool operator!=( const EnvironmentMapOptions& b ) const;
    };
}

#include "mrvEnvironmentMapOptionsInline.h"
