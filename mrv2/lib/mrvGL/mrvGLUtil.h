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

namespace mrv
{
    using namespace tl;

    inline void drawRectOutline(
        const std::shared_ptr<timeline::IRender>& render,
        const math::BBox2i& rect, const imaging::Color4f& color,
        const int width, const math::Matrix4x4f& mvp)
    {
        geom::TriangleMesh2 mesh;

        // Add the outside vertices.
        math::BBox2i outside = rect.margin(width / 2);
        mesh.v.push_back(math::Vector2f(outside.min.x, outside.min.y));
        mesh.v.push_back(math::Vector2f(outside.max.x, outside.min.y));
        mesh.v.push_back(math::Vector2f(outside.max.x, outside.max.y));
        mesh.v.push_back(math::Vector2f(outside.min.x, outside.max.y));

        // Add the inside vertices.
        math::BBox2i inside = rect.margin(-width / 2);
        mesh.v.push_back(math::Vector2f(inside.min.x, inside.min.y));
        mesh.v.push_back(math::Vector2f(inside.max.x, inside.min.y));
        mesh.v.push_back(math::Vector2f(inside.max.x, inside.max.y));
        mesh.v.push_back(math::Vector2f(inside.min.x, inside.max.y));

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

        render->setTransform(mvp);
        math::Vector2i pos;
        render->drawMesh(mesh, pos, color);
    }

    void drawLines(
        const std::shared_ptr<timeline::IRender>& render,
        const tl::draw::PointList& pts, const imaging::Color4f& color,
        const int width,
        const tl::draw::Polyline2D::JointStyle jointStyle =
            tl::draw::Polyline2D::JointStyle::MITER,
        const tl::draw::Polyline2D::EndCapStyle endStyle =
            tl::draw::Polyline2D::EndCapStyle::BUTT,
        const bool allowOverlap = false);

    void drawCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float perimeter, const float width,
        const imaging::Color4f& color);

    void drawCursor(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float perimeter,
        const imaging::Color4f& color);

    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float perimeter,
        const imaging::Color4f& color);

    std::shared_ptr< tl::draw::Shape > messageToShape(const Message&);
    Message shapeToMessage(const std::shared_ptr< tl::draw::Shape > shape);

} // namespace mrv
