// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include <tlDraw/Point.h>
#include <tlDraw/Polyline2D.h>
#include <tlDraw/Shape.h>

#include <tlCore/Box.h>
#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>
#include <tlCore/Mesh.h>

#include <tlTimeline/IRender.h>


#include "mrvNetwork/mrvMessage.h"

#include "mrvGL/mrvGLErrors.h"

namespace mrv
{
    using namespace tl;

    //! Draw a rectangle outline with a mesh.
    inline void drawRectOutline(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Box2i& rect, const image::Color4f& color, const float width)
    {
        geom::TriangleMesh2 mesh;

        // Add the outside vertices.
        // math::Box2i outside = rect.margin(width / 2);
        mesh.v.push_back(math::Vector2f(rect.min.x - width, rect.min.y - width));
        mesh.v.push_back(math::Vector2f(rect.max.x + width, rect.min.y - width));
        mesh.v.push_back(math::Vector2f(rect.max.x + width, rect.max.y + width));
        mesh.v.push_back(math::Vector2f(rect.min.x - width, rect.max.y + width));

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

    //! Draw a filled circle.
    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const image::Color4f& color, const bool soft = false);

    //! Create a mesh for drawing checkers.
    geom::TriangleMesh2
    checkers(const math::Box2i&, const math::Size2i& checkerSize);

    // Function to perform bilinear interpolation for resizing
    void resizeImage(
        GLubyte* targetPixels, GLubyte* srcPixels, const int srcWidth,
        const int srcHeight, const int targetWidth, const int targetHeight);
} // namespace mrv
