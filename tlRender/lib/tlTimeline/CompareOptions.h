// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/Video.h>

#include <tlCore/Box.h>

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
            Multiply,
            Add,
            Horizontal,
            Vertical,
            Tile,
            Butterfly,

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
            float differenceGain = 1.F;
            bool  fitToA = true;

            bool operator==(const CompareOptions&) const;
            bool operator!=(const CompareOptions&) const;
        };

        //! Get the bounds for the given compare mode.
        std::vector<math::Box2i> getBounds(
            const CompareOptions&,
            const AspectRatioOptions&,
            const std::vector<image::Info>&);

        //! Get the boxes for the given compare mode.
        std::vector<math::Box2i> getBoxes(
            const CompareOptions&,
            const AspectRatioOptions&,
            const std::vector<image::Info>&);

        //! Get the boxes for the given compare mode.
        std::vector<math::Box2i> getBoxes(
            const CompareOptions&,
            const AspectRatioOptions&,
            const std::vector<VideoFrame>&);

        //! Get the boxes for the given compare mode.
        std::vector<math::Box2i> getBoxes(
            const CompareOptions&,
            const std::vector<DisplayOptions>&,
            const std::vector<VideoFrame>&);

        //! Get the boxes for the given compare mode.
        std::vector<math::Box2i> getBoxes(
            const CompareMode,
            const std::vector<DisplayOptions>&,
            const std::vector<VideoFrame>&);

        //! Get the render size for the given compare mode.
        math::Size2i getRenderSize(
            const CompareOptions&,
            const AspectRatioOptions&,
            const std::vector<image::Info>&);

        //! Get the render size for the given compare mode.
        math::Size2i getRenderSize(
            const CompareOptions&,
            const AspectRatioOptions&,
            const std::vector<VideoFrame>&);

        //! Get the render size for the given compare mode.
        math::Size2i getRenderSize(
            const CompareOptions&,
            const std::vector<DisplayOptions>&,
            const std::vector<VideoFrame>&);

        //! Get a compare time.
        otime::RationalTime getCompareTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& compareTimeRange, CompareTimeMode);
    } // namespace timeline
} // namespace tl

#include <tlTimeline/CompareOptionsInline.h>
