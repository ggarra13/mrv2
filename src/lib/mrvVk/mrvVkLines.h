// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvDraw/Polyline2D.h"

#include <tlTimelineVk/Render.h>

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Color.h>

namespace mrv
{
    namespace vulkan
    {
        using namespace tl;

        //! Vulkan Lines renderer.
        class Lines
        {
        public:
            Lines();
            ~Lines();

            //! Draw a points in raster coordinates with glPointSize
            void drawPoints(
                VkCommandBuffer& cmd, const uint32_t frameIndex,
                Fl_Vk_Context& ctx,
                const std::vector<math::Vector2f>& pts,
                const image::Color4f& color, const int size = 1);

            //! Draw a single line in raster coordinates with a mesh.
            void drawLine(
                Fl_Vk_Context& ctx,
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2i& start, const math::Vector2i& end,
                const image::Color4f& color, const float width);

            //! Draw a set of connected line segments.
            void drawLines(
                Fl_Vk_Context& ctx,
                const std::shared_ptr<timeline_vlk::Render>& render,
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
                Fl_Vk_Context& ctx,
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2f& center, const float radius,
                const float width, const image::Color4f& color,
                const bool soft = false);

            //! Draw drawing cursor (two circles, one white, one black).
            void drawCursor(
                Fl_Vk_Context& ctx,
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2f& center, const float radius,
                const image::Color4f& color);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace opengl
} // namespace mrv
