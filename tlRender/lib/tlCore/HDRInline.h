// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace image
    {
        inline bool HDRBezier::operator==(const HDRBezier& other) const
        {
            bool anchorsEqual = true;
            for (uint8_t i = 0; i < numAnchors; ++i)
            {
                if (anchors[i] != other.anchors[i])
                {
                    anchorsEqual = false;
                    break;
                }
            }

            return targetLuma == other.targetLuma && kneeX == other.kneeX &&
                   kneeY == other.kneeY && numAnchors == other.numAnchors &&
                   anchorsEqual;
        }

        inline bool HDRBezier::operator!=(const HDRBezier& other) const
        {
            return !(other == *this);
        }

        inline bool HDRData::operator==(const HDRData& other) const
        {
            return eotf == other.eotf && primaries == other.primaries &&
                   displayMasteringLuminance ==
                       other.displayMasteringLuminance &&
                   maxCLL == other.maxCLL && maxFALL == other.maxFALL &&
                   sceneMax[0] == other.sceneMax[0] &&
                   sceneMax[1] == other.sceneMax[1] &&
                   sceneMax[2] == other.sceneMax[2] && ootf == other.ootf &&
                   sceneAvg == other.sceneAvg && maxPQY == other.maxPQY &&
                   avgPQY == other.avgPQY;
        }

        inline bool HDRData::operator!=(const HDRData& other) const
        {
            return !(other == *this);
        }
    } // namespace image
} // namespace tl
