// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            CompareMode, "A", "B", "Wipe", "Overlay", "Difference",
            "Horizontal", "Vertical", "Tile");
        TLRENDER_ENUM_SERIALIZE_IMPL(CompareMode);

        TLRENDER_ENUM_IMPL(CompareTimeMode, "Relative", "Absolute");
        TLRENDER_ENUM_SERIALIZE_IMPL(CompareTimeMode);

        std::vector<math::Box2i>
        getBoxes(CompareMode mode, const std::vector<image::Size>& sizes)
        {
            std::vector<math::Box2i> out;
            const size_t count = sizes.size();
            switch (mode)
            {
            case CompareMode::Horizontal:
            {
                image::Size size;
                if (count > 0)
                {
                    size = sizes[0];
                }
                if (count > 0)
                {
                    out.push_back(math::Box2i(
                        0, 0, size.w * size.pixelAspectRatio, size.h));
                }
                if (count > 1)
                {
                    out.push_back(math::Box2i(
                        size.w * size.pixelAspectRatio, 0,
                        size.w * size.pixelAspectRatio, size.h));
                }
                break;
            }
            case CompareMode::Vertical:
            {
                image::Size size;
                if (count > 0)
                {
                    size = sizes[0];
                }
                if (count > 0)
                {
                    out.push_back(math::Box2i(
                        0, 0, size.w * size.pixelAspectRatio, size.h));
                }
                if (count > 1)
                {
                    out.push_back(math::Box2i(
                        0, size.h, size.w * size.pixelAspectRatio, size.h));
                }
                break;
            }
            case CompareMode::Tile:
                if (count > 0)
                {
                    image::Size tileSize;
                    for (const auto& i : sizes)
                    {
                        tileSize = std::max(tileSize, i);
                    }

                    int columns = 0;
                    int rows = 0;
                    switch (count)
                    {
                    case 1:
                        columns = 1;
                        rows = 1;
                        break;
                    case 2:
                        columns = 1;
                        rows = 2;
                        break;
                    default:
                    {
                        const float sqrt = std::sqrt(count);
                        columns = std::ceil(sqrt);
                        const std::div_t d = std::div(count, columns);
                        rows = d.quot + (d.rem > 0 ? 1 : 0);
                        break;
                    }
                    }

                    int i = 0;
                    for (int r = 0, y = 0; r < rows; ++r)
                    {
                        for (int c = 0, x = 0; c < columns; ++c, ++i)
                        {
                            if (i < count)
                            {
                                const auto& s = sizes[i];
                                const math::Box2i box(
                                    x, y,
                                    tileSize.w * tileSize.pixelAspectRatio,
                                    tileSize.h);
                                out.push_back(box);
                            }
                            x += tileSize.w * tileSize.pixelAspectRatio;
                        }
                        y += tileSize.h;
                    }
                }
                break;
            default:
                for (size_t i = 0; i < std::min(count, static_cast<size_t>(2));
                     ++i)
                {
                    out.push_back(math::Box2i(
                        0, 0, sizes[0].w * sizes[0].pixelAspectRatio,
                        sizes[0].h));
                }
                break;
            }
            return out;
        }

        std::vector<math::Box2i>
        getBoxes(CompareMode mode, const std::vector<VideoData>& videoData)
        {
            std::vector<image::Size> sizes;
            for (const auto& i : videoData)
            {
                sizes.push_back(i.size);
            }
            return getBoxes(mode, sizes);
        }

        math::Size2i
        getRenderSize(CompareMode mode, const std::vector<image::Size>& sizes)
        {
            math::Size2i out;
            math::Box2i box;
            const auto boxes = getBoxes(mode, sizes);
            if (!boxes.empty())
            {
                box = boxes[0];
                for (size_t i = 1; i < boxes.size(); ++i)
                {
                    box.expand(boxes[i]);
                }
                out.w = box.w();
                out.h = box.h();
            }
            return out;
        }

        math::Size2i
        getRenderSize(CompareMode mode, const std::vector<VideoData>& videoData)
        {
            std::vector<image::Size> sizes;
            for (const auto& i : videoData)
            {
                sizes.push_back(i.size);
            }
            return getRenderSize(mode, sizes);
        }

        otime::RationalTime getCompareTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& compareTimeRange, CompareTimeMode mode)
        {
            otime::RationalTime out;
            switch (mode)
            {
            case CompareTimeMode::Relative:
            {
                const otime::RationalTime relativeTime =
                    sourceTime - sourceTimeRange.start_time();
                const otime::RationalTime relativeTimeRescaled =
                    relativeTime.rescaled_to(compareTimeRange.duration().rate())
                        .floor();
                out = compareTimeRange.start_time() + relativeTimeRescaled;
                break;
            }
            case CompareTimeMode::Absolute:
                out = sourceTime.rescaled_to(compareTimeRange.duration().rate())
                          .floor();
                break;
            default:
                break;
            }
            return out;
        }
    } // namespace timeline
} // namespace tl
