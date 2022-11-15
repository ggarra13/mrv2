#pragma once

#include "mrvPoint.h"
#include "mrvPolyline2D.h"

namespace mrv
{
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

        render->drawMesh(mesh, color, mvp);
    }



    inline void drawLines(
        const std::shared_ptr<timeline::IRender>& render,
        const PointList& pts, const imaging::Color4f& color,
        const int width, const math::Matrix4x4f& mvp,
        const Polyline2D::JointStyle jointStyle = Polyline2D::JointStyle::MITER,
        const Polyline2D::EndCapStyle endStyle = Polyline2D::EndCapStyle::BUTT,
        const bool allowOverlap = false)
        {
            const PointList& draw = Polyline2D::create( pts, width, jointStyle,
                                                        endStyle,
                                                        allowOverlap );
                    
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

            render->drawMesh( mesh, color, mvp );
        }
    
}
