// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include <tlTimelineVk/Render.h>

#include <tlDraw/Polyline2D.h>

#include <tlCore/Util.h>
#include <tlCore/Matrix.h>
#include <tlCore/Color.h>

namespace mrv
{
    namespace vulkan
    {
        using namespace tl;

        //! Vulkan Lines renderer 
        class Lines
        {
        public:
            Lines(Fl_Vk_Context&, VkRenderPass);
            ~Lines();

            //! Draw a single line in raster coordinates with a mesh.
            void drawLine(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2i& start, const math::Vector2i& end,
                const image::Color4f& color, const float width);

            //! Draw a mesh.
            void drawMesh(const std::shared_ptr<timeline_vlk::Render>& render,
                          const geom::TriangleMesh2& mesh,
                          const image::Color4f& color,
                          const bool soft = false,
                          const std::string& pipelineName = "annotation");
            
            //! Draw a set of connected line segments.
            void drawLines(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const draw::PointList& pts, const image::Color4f& color,
                const float width, const bool soft = false,
                const draw::Polyline2D::JointStyle jointStyle =
                    draw::Polyline2D::JointStyle::MITER,
                const draw::Polyline2D::EndCapStyle endStyle =
                    draw::Polyline2D::EndCapStyle::BUTT,
                const bool catmullRomSpline = false,
                const bool allowOverlap = false,
                const std::string& pipelineName = "annotation");

            //! Draw a circle.
            void drawCircle(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2f& center, const float radius,
                const float width, const image::Color4f& color,
                const bool soft = false);
            
            //! Draw a circle.
            void drawFilledCircle(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2f& center, const float radius,
                const image::Color4f& color, const unsigned sides = 20);

            //! Draw drawing cursor (two circles, one white, one black).
            void drawCursor(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const math::Vector2f& center, const float radius,
                const image::Color4f& color);

            inline VkRenderPass getRenderPass() { return m_renderPass; }
            
        private:
            Fl_Vk_Context& ctx;
            VkRenderPass m_renderPass;
            
            TLRENDER_PRIVATE();
        };
    } // namespace opengl
} // namespace mrv
