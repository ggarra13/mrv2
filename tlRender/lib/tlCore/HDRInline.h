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

        inline std::ostream& operator<<(std::ostream& s, const HDRBezier& o)
        {
            s << "targetLuma=" << o.targetLuma << std::endl
              << "kneeX=" << o.kneeX << std::endl
              << "kneeY=" << o.kneeY << std::endl
              << "numAnchors=" << (int)o.numAnchors << std::endl;
            for (int i = 0; i < o.numAnchors; ++i)
            {
                s << "\t" << o.anchors[i] << std::endl;
            }
            return s;
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
                isDolbyVision == other.isDolbyVision &&
                avgPQY == other.avgPQY;
        }

        inline bool HDRData::operator!=(const HDRData& other) const
        {
            return !(other == *this);
        }
        
        inline bool isHDR(const HDRData& o)
        {
            return (o.eotf != image::EOTF_BT709 &&
                    o.eotf != image::EOTF_BT601);
        }
        
        inline bool isHDRPlus(const HDRData& o)
        {
            return (o.eotf == image::EOTF_BT2100_PQ &&
                    o.sceneAvg != 0.F);
        }
        
        inline bool isHDRDolbyVision(const HDRData& o)
        {
            return (o.eotf == image::EOTF_BT2020 && o.isDolbyVision);
        }
        
        inline std::ostream& operator<<(std::ostream& s, const HDRData& o)
        {
            s << "eotf=" << (int)o.eotf << std::endl
              << "red    primaries=" << o.primaries[0].x << ", "
              << o.primaries[0].y << std::endl
              << "green  primaries=" << o.primaries[1].x << ", "
              << o.primaries[1].y << std::endl
              << "blue  primaries=" << o.primaries[2].x << ", "
              << o.primaries[2].y << std::endl
              << "white primaries=" << o.primaries[3].x << ", "
              << o.primaries[3].y << std::endl
              << "display luminance=" << o.displayMasteringLuminance.getMin()
              << " to " << o.displayMasteringLuminance.getMax() << std::endl
              << "maxCLL =" << o.maxCLL << std::endl
              << "maxFALL=" << o.maxFALL;
            if (o.sceneMax[0] > 0.F)
            {
                s << std::endl
                  << "sceneMax=" << o.sceneMax[0] << ", "
                  << o.sceneMax[1] << ", "
                  << o.sceneMax[2] << std::endl
                  << "sceneAvg=" << o.sceneAvg << std::endl
                  << o.ootf; // print bezier info
            }
            return s;
        }
    } // namespace image
} // namespace tl
