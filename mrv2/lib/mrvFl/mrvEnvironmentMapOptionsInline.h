// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.


namespace mrv
{
    inline bool EnvironmentMapOptions::operator==(
        const EnvironmentMapOptions& b ) const
    {
        return ( type == b.type &&
                 horizontalAperture == b.horizontalAperture &&
                 verticalAperture == b.verticalAperture &&
                 focalLength == b.focalLength &&
                 rotateX == b.rotateX &&
                 rotateY == b.rotateY ) ;
    }

    inline bool EnvironmentMapOptions::operator!=(
        const EnvironmentMapOptions& b ) const
    {
        return !(*this == b);
    }
}
