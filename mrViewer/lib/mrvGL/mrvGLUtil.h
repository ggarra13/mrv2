// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvDraw/Point.h>
#include <mrvDraw/Polyline2D.h>

#include <tlCore/BBox.h>
#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>
#include <tlCore/Mesh.h>

#include <tlTimeline/IRender.h>

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
        mesh.v.push_back(math::Vector2f( outside.min.x, outside.min.y));
        mesh.v.push_back(math::Vector2f( outside.max.x, outside.min.y));
        mesh.v.push_back(math::Vector2f( outside.max.x, outside.max.y));
        mesh.v.push_back(math::Vector2f( outside.min.x, outside.max.y));

        // Add the inside vertices.
        math::BBox2i inside = rect.margin(-width / 2);
        mesh.v.push_back(math::Vector2f( inside.min.x, inside.min.y));
        mesh.v.push_back(math::Vector2f( inside.max.x, inside.min.y));
        mesh.v.push_back(math::Vector2f( inside.max.x, inside.max.y));
        mesh.v.push_back(math::Vector2f( inside.min.x, inside.max.y));

        // Add the triangles. Note that vertex indexes start at one,
        // zero is invalid.
        mesh.triangles.push_back(geom::Triangle2({ 1, 2, 5 }));
        mesh.triangles.push_back(geom::Triangle2({ 2, 6, 5 }));
        mesh.triangles.push_back(geom::Triangle2({ 2, 3, 6 }));
        mesh.triangles.push_back(geom::Triangle2({ 3, 7, 6 }));
        mesh.triangles.push_back(geom::Triangle2({ 3, 4, 8 }));
        mesh.triangles.push_back(geom::Triangle2({ 8, 7, 3 }));
        mesh.triangles.push_back(geom::Triangle2({ 4, 5, 8 }));
        mesh.triangles.push_back(geom::Triangle2({ 4, 1, 5 }));

        render->setMatrix(mvp);
        render->drawMesh(mesh, color);
    }



    inline void drawLines(
        const std::shared_ptr<timeline::IRender>& render,
        const tl::draw::PointList& pts, const imaging::Color4f& color,
        const int width, const math::Matrix4x4f& mvp,
        const tl::draw::Polyline2D::JointStyle jointStyle =
        tl::draw::Polyline2D::JointStyle::MITER,
        const tl::draw::Polyline2D::EndCapStyle endStyle =
        tl::draw::Polyline2D::EndCapStyle::BUTT,
        const bool allowOverlap = false)
        {
            using namespace tl::draw;
            const PointList& draw =
               Polyline2D::create( pts, width, jointStyle,
                                   endStyle, allowOverlap );

            geom::TriangleMesh2 mesh;
            size_t numVertices = draw.size();
            mesh.triangles.reserve( numVertices / 3 );

            geom::Triangle2 triangle;
            for ( size_t v = 0; v < numVertices; v += 3 )
            {
                triangle.v[0].v = v+1;
                triangle.v[1].v = v+2;
                triangle.v[2].v = v+3;
                mesh.triangles.emplace_back(triangle);
            }

            mesh.v.reserve( numVertices );
            for ( size_t i = 0; i < numVertices; ++i )
                mesh.v.emplace_back( math::Vector2f( draw[i].x, draw[i].y ) );

            render->setMatrix(mvp);
            render->drawMesh( mesh, color );
        }

    inline void drawCursor(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float perimeter,
        const float width,
        const imaging::Color4f& color,
        const math::Matrix4x4f& mvp )
    {
        const int triangleAmount = 40;
        const double twoPi = math::pi * 2.0;
        const float radius = perimeter / 2.0;

        tl::draw::PointList verts;
        verts.reserve( triangleAmount+1 );
        for ( int i = 0; i < triangleAmount; ++i )
        {
            tl::draw::Point pt(
                center.x + (radius * cos( i* twoPi / triangleAmount )),
                center.y + (radius * sin( i* twoPi / triangleAmount )) );
            verts.push_back( pt );
        }

        drawLines( render, verts, color, width, mvp,
                   tl::draw::Polyline2D::JointStyle::MITER,
                   tl::draw::Polyline2D::EndCapStyle::JOINT );

    }
}
