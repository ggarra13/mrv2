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
        float focalLength = 90.F;
        float rotateX =   0.F;
        float rotateY = -90.F;

        bool operator==( const EnvironmentMapOptions& b ) const;
        bool operator!=( const EnvironmentMapOptions& b ) const;
    };
}

#include "mrvEnvironmentMapOptionsInline.h"
