// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Color.h>

#include <tlTimeline/IRender.h>

#include <tlDraw/Polyline2D.h>

namespace mrv
{
    namespace opengl
    {
        using namespace tl;

        //! OpenGL Lines renderer.
        class Lines
        {
        public:
            Lines();
            ~Lines();

            //! Draw a single line in raster coordinates with a mesh.
            void drawLine(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2i& start, const math::Vector2i& end,
                const image::Color4f& color, const float width);

            //! Draw a set of connected line segments.
            void drawLines(
                const std::shared_ptr<timeline::IRender>& render,
                const draw::PointList& pts, const image::Color4f& color,
                const float width, const bool soft = false,
                const draw::Polyline2D::JointStyle jointStyle =
                    draw::Polyline2D::JointStyle::MITER,
                const draw::Polyline2D::EndCapStyle endStyle =
                    draw::Polyline2D::EndCapStyle::BUTT,
                const bool catmullRomSpline = false,
                const bool allowOverlap = false);

            //! Draw a circle.
            void drawCircle(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2f& center, const float radius,
                const float width, const image::Color4f& color,
                const bool soft = false);

            //! Draw drawing cursor (two circles, one white, one black).
            void drawCursor(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2f& center, const float radius,
                const image::Color4f& color);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace opengl
} // namespace mrv
