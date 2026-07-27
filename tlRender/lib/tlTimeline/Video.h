// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/Transition.h>

#include <tlCore/Box.h>
#include <tlCore/Image.h>
#include <tlCore/Time.h>

#include <optional>

namespace tl
{
    namespace timeline
    {
        //! Video layer.
        struct VideoLayer
        {
            std::shared_ptr<image::Image> image;
            ImageOptions imageOptions;

            std::shared_ptr<image::Image> imageB;
            ImageOptions imageOptionsB;

            //! The box the image occupies within the timeline canvas, when the
            //! clip provides OTIO spatial coordinates (see the
            //! "available_image_bounds" media reference property). Clips that
            //! share the same box are displayed at the same size and position
            //! regardless of their image resolution.
            //!
            //! This has been converted from the OTIO coordinate system: scaled
            //! from unit-less coordinates to pixels, flipped from Y-up
            //! Y-down, and translated so the canvas starts at the origin.
            std::optional<math::Box2f>   bounds;

            //! The canvas box for "imageB", which comes from the neighbouring
            //! clip during a transition and may be placed differently.
            std::optional<math::Box2f>   boundsB;

            Transition transition = Transition::kNone;
            float transitionValue = 0.F;

            bool operator==(const VideoLayer&) const;
            bool operator!=(const VideoLayer&) const;
        };

        //! Video data.
        struct VideoFrame
        {
            image::Size size;

            //! The size of the canvas shared by the whole timeline, when any
            //! clip provides OTIO spatial coordinates. The layer boxes are
            //! positioned within it. Empty otherwise, which lays the frame
            //! out from the image sizes instead.
            math::Size2i canvasSize;

            otime::RationalTime time = time::invalidTime;
            std::vector<VideoLayer> layers;

            bool operator==(const VideoFrame&) const;
            bool operator!=(const VideoFrame&) const;
        };

        //! Compare the time values of video data.
        bool isTimeEqual(const VideoFrame&, const VideoFrame&);
    } // namespace timeline
} // namespace tl

#include <tlTimeline/VideoInline.h>
