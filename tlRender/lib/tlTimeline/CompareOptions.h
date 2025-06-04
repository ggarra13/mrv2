// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Video.h>

namespace tl
{
    namespace timeline
    {
        //! Comparison mode.
        enum class CompareMode {
            A,
            B,
            Wipe,
            Overlay,
            Difference,
            Horizontal,
            Vertical,
            Tile,

            Count,
            First = A
        };
        TLRENDER_ENUM(CompareMode);
        TLRENDER_ENUM_SERIALIZE(CompareMode);

        //! Comparison time mode.
        enum class CompareTimeMode {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        TLRENDER_ENUM(CompareTimeMode);
        TLRENDER_ENUM_SERIALIZE(CompareTimeMode);

        //! Comparison options.
        struct CompareOptions
        {
            CompareMode mode = CompareMode::A;
            math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
            float wipeRotation = 0.F;
            float overlay = .5F;

            bool operator==(const CompareOptions&) const;
            bool operator!=(const CompareOptions&) const;
        };

        //! Get the boxes for the given compare mode and sizes.
        std::vector<math::Box2i>
        getBoxes(CompareMode, const std::vector<image::Size>&);

        //! Get the boxes for the given compare mode and video data.
        std::vector<math::Box2i>
        getBoxes(CompareMode, const std::vector<VideoData>&);

        //! Get the render size for the given compare mode and sizes.
        math::Size2i
        getRenderSize(CompareMode, const std::vector<image::Size>&);

        //! Get the render size for the given compare mode and video data.
        math::Size2i getRenderSize(CompareMode, const std::vector<VideoData>&);

        //! Get a compare time.
        otime::RationalTime getCompareTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& compareTimeRange, CompareTimeMode);
    } // namespace timeline
} // namespace tl

#include <tlTimeline/CompareOptionsInline.h>
