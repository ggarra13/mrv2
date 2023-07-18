// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/BBox.h>
#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>
#include <tlCore/Mesh.h>

#include <tlTimeline/IRender.h>

#include "mrvDraw/Point.h"
#include "mrvDraw/Polyline2D.h"
#include "mrvDraw/Shape.h"

#include "mrvNetwork/mrvMessage.h"

#include "mrvGL/mrvGLErrors.h"

namespace mrv
{
    using namespace tl;

    //! Draw a rectangle outline with a mesh.
    inline void drawRectOutline(
        const std::shared_ptr<timeline::IRender>& render,
        const math::BBox2i& rect, const imaging::Color4f& color,
        const int width)
    {
        geom::TriangleMesh2 mesh;

        // Add the outside vertices.
        // math::BBox2i outside = rect.margin(width / 2);
        mesh.v.push_back(math::Vector2f(rect.min.x - 2.0f, rect.min.y - 2.0f));
        mesh.v.push_back(math::Vector2f(rect.max.x + 2.0f, rect.min.y - 2.0f));
        mesh.v.push_back(math::Vector2f(rect.max.x + 2.0f, rect.max.y + 2.0f));
        mesh.v.push_back(math::Vector2f(rect.min.x - 2.0f, rect.max.y + 2.0f));

        // Add the inside vertices.
        mesh.v.push_back(math::Vector2f(rect.min.x, rect.min.y));
        mesh.v.push_back(math::Vector2f(rect.max.x, rect.min.y));
        mesh.v.push_back(math::Vector2f(rect.max.x, rect.max.y));
        mesh.v.push_back(math::Vector2f(rect.min.x, rect.max.y));

        // Add the triangles. Note that vertex indexes start at one,
        // zero is invalid.
        mesh.triangles.push_back(geom::Triangle2({1, 2, 5}));
        mesh.triangles.push_back(geom::Triangle2({2, 6, 5}));
        mesh.triangles.push_back(geom::Triangle2({2, 3, 6}));
        mesh.triangles.push_back(geom::Triangle2({3, 7, 6}));
        mesh.triangles.push_back(geom::Triangle2({3, 4, 8}));
        mesh.triangles.push_back(geom::Triangle2({8, 7, 3}));
        mesh.triangles.push_back(geom::Triangle2({4, 5, 8}));
        mesh.triangles.push_back(geom::Triangle2({4, 1, 5}));

        math::Vector2i pos;
        CHECK_GL;
        render->drawMesh(mesh, pos, color);
        CHECK_GL;
    }

    //! Draw a points in raster coordinates with glPointSize
    void drawPoints(
        const std::vector<math::Vector2f>& pts, const imaging::Color4f& color,
        const int size = 1);

    //! Draw a single line in raster coordinates with a mesh.
    void drawLine(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& start, const math::Vector2i& end,
        const imaging::Color4f& color, const int width);

    //! Draw a set of connected line segments.
    void drawLines(
        const std::shared_ptr<timeline::IRender>& render,
        const tl::draw::PointList& pts, const imaging::Color4f& color,
        const int width, const bool soft = false,
        const tl::draw::Polyline2D::JointStyle jointStyle =
            tl::draw::Polyline2D::JointStyle::MITER,
        const tl::draw::Polyline2D::EndCapStyle endStyle =
            tl::draw::Polyline2D::EndCapStyle::BUTT,
        const bool catmullRomSpline = false, const bool allowOverlap = false);

    //! Draw a circle.
    void drawCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius, const float width,
        const imaging::Color4f& color, const bool soft = false);

    //! Draw drawing cursor (two circles, one white, one black).
    void drawCursor(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const imaging::Color4f& color);

    //! Draw a filled circle.
    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const imaging::Color4f& color, const bool soft = false);

    //! Translate a nlohmann::json message to a tl::draw::Shape.
    std::shared_ptr< tl::draw::Shape > messageToShape(const Message&);

    //! Translate a tl::draw::Shape to a nlohmann::json message.
    Message shapeToMessage(const std::shared_ptr< tl::draw::Shape > shape);

} // namespace mrv
