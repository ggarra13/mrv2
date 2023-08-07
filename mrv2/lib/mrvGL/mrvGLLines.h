
#pragma once

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Color.h>

#include <tlTimeline/IRender.h>

#include "mrvDraw/Polyline2D.h"

namespace tl
{
    namespace gl
    {
        //! OpenGL Lines renderer.
        class Lines
        {
        public:
            Lines();
            ~Lines();

            //! Draw a points in raster coordinates with glPointSize
            void drawPoints(
                const std::vector<math::Vector2f>& pts,
                const image::Color4f& color, const int size = 1);

            //! Draw a single line in raster coordinates with a mesh.
            void drawLine(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2i& start, const math::Vector2i& end,
                const image::Color4f& color, const int width);

            //! Draw a set of connected line segments.
            void drawLines(
                const std::shared_ptr<timeline::IRender>& render,
                const tl::draw::PointList& pts, const image::Color4f& color,
                const int width, const bool soft = false,
                const tl::draw::Polyline2D::JointStyle jointStyle =
                    tl::draw::Polyline2D::JointStyle::MITER,
                const tl::draw::Polyline2D::EndCapStyle endStyle =
                    tl::draw::Polyline2D::EndCapStyle::BUTT,
                const bool catmullRomSpline = false,
                const bool allowOverlap = false);

            //! Draw a circle.
            void drawCircle(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2i& center, const float radius,
                const float width, const image::Color4f& color,
                const bool soft = false);

            //! Draw drawing cursor (two circles, one white, one black).
            void drawCursor(
                const std::shared_ptr<timeline::IRender>& render,
                const math::Vector2i& center, const float radius,
                const image::Color4f& color);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace gl
} // namespace tl
