// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool ImageFilters::operator==(const ImageFilters& other) const
        {
            return minify == other.minify && magnify == other.magnify;
        }

        inline bool ImageFilters::operator!=(const ImageFilters& other) const
        {
            return !(*this == other);
        }

        inline bool ImageOptions::operator==(const ImageOptions& other) const
        {
            return videoLevels == other.videoLevels &&
                   alphaBlend == other.alphaBlend &&
                   imageFilters == other.imageFilters && cache == other.cache;
        }

        inline bool ImageOptions::operator!=(const ImageOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
