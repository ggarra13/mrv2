// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Original code is:
//
// Copyright © 2019 Marius Metzger (CrushedPixel)
//

#include "Polyline2D.h"

namespace tl
{
    namespace draw
    {
        using namespace Imath;

        void Polyline2D::filterPoints()
        {
            PointList filteredPoints;
            filteredPoints.push_back(points.front());

            for (size_t i = 1; i < points.size(); i++)
            {
                const Point& p0 = filteredPoints.back();
                const Point& p1 = points[i];
                const Point tmp = p0 - p1;
                const float length = tmp.length();
                if (length <= m_width)
                    continue;
                filteredPoints.push_back(p1);
            }
            points = filteredPoints;
        }

        void Polyline2D::create(
            const PointList& inPoints, JointStyle jointStyle,
            EndCapStyle endCapStyle, bool catmullRomSpline, bool allowOverlap)
        {

            // Filter the points
            points = inPoints;
            filterPoints();

            // operate on half the thickness to make our lives easier
            float thickness = m_width / 2;

            // create poly segments from the points
            std::vector<PolySegment<Point>> segments;
            double numPoints = static_cast<double>(points.size() - 1);
            for (size_t i = 0; i + 1 < points.size(); i++)
            {
                auto& point1 = points[i];
                auto& point2 = points[i + 1];

                if (point1 != point2)
                    segments.emplace_back(
                        LineSegment<Point>(point1, point2), thickness);
            }

            if (endCapStyle == EndCapStyle::JOINT)
            {
                // create a connecting segment from the last to the first
                // point

                auto& point1 = points[points.size() - 1];
                auto& point2 = points[0];

                if (point1 != point2)
                    segments.emplace_back(
                        LineSegment<Point>(point1, point2), thickness);
            }

            if (segments.empty())
            {
                const float w = thickness;
                Point center = points[0];

                if (!m_softEdges)
                {
                    // Splat, create a filled circle
                    Point top = center + Point(0, -w);
                    Point bottom = center + Point(0, w);

                    createTriangleFan(
                        center, center, top, bottom, 0.5, 0.0, false);
                    createTriangleFan(
                        center, center, top, bottom, 0.5, 0.0, true);
                }
                else
                {
                    // Splat, create a square with UVs
                    Point start1 = Point(center.x - w, center.y - w);
                    Point start2 = Point(center.x + w, center.y - w);
                    Point end1 = Point(center.x + w, center.y + w);
                    Point end2 = Point(center.x - w, center.y + w);

                    V2f uvStart1 = V2f(0, 0);
                    V2f uvStart2 = V2f(1, 0);
                    V2f uvEnd1 = V2f(1, 1);
                    V2f uvEnd2 = V2f(0, 1);

                    size_t n = m_vertices.size();

                    // emit vertices
                    m_vertices.emplace_back(start1);
                    m_vertices.emplace_back(start2);
                    m_vertices.emplace_back(end1);
                    m_vertices.emplace_back(end2);

                    // emit UVs
                    m_uvs.emplace_back(uvStart1);
                    m_uvs.emplace_back(uvStart2);
                    m_uvs.emplace_back(uvEnd1);
                    m_uvs.emplace_back(uvEnd2);

                    m_tris.emplace_back(IndexTriangle(n, n + 1, n + 2));
                    m_tris.emplace_back(IndexTriangle(n + 2, n, n + 3));
                }

                return;
            }

            Point nextStart1{0, 0};
            Point nextStart2{0, 0};
            Point start1{0, 0};
            Point start2{0, 0};
            Point end1{0, 0};
            Point end2{0, 0};

            // calculate the path's global start and end points
            auto& firstSegment = segments[0];
            auto& lastSegment = segments[segments.size() - 1];

            auto pathStart1 = firstSegment.edge1.a;
            auto pathStart2 = firstSegment.edge2.a;
            auto pathEnd1 = lastSegment.edge1.b;
            auto pathEnd2 = lastSegment.edge2.b;

            // handle different end cap styles
            if (endCapStyle == EndCapStyle::SQUARE)
            {
                // extend the start/end points by half the thickness
                pathStart1 =
                    pathStart1 - firstSegment.edge1.direction() * thickness;
                pathStart2 =
                    pathStart2 - firstSegment.edge2.direction() * thickness;
                pathEnd1 = pathEnd1 + lastSegment.edge1.direction() * thickness;
                pathEnd2 = pathEnd2 + lastSegment.edge2.direction() * thickness;
            }
            else if (endCapStyle == EndCapStyle::ROUND)
            {
                if (m_softEdges)
                {
                    // If soft shader, we draw 3 triangles with UVs at each cap.
                    createRoundSoftCap(firstSegment, false);
                    createRoundSoftCap(lastSegment, true);
                }
                else
                {
                    // if solid shader, w draw half circle at caps
                    createTriangleFan(
                        firstSegment.center.a, firstSegment.center.a,
                        firstSegment.edge1.a, firstSegment.edge2.a, 0.5, 0.0,
                        false);
                    createTriangleFan(
                        lastSegment.center.b, lastSegment.center.b,
                        lastSegment.edge1.b, lastSegment.edge2.b, 0.5, 0.0,
                        true);
                }
            }
            else if (endCapStyle == EndCapStyle::JOINT)
            {
                // join the last (connecting) segment and the first segment
                createJoint(
                    lastSegment, firstSegment, jointStyle, pathEnd1, pathEnd2,
                    pathStart1, pathStart2, allowOverlap);
            }

            // generate mesh data for path segments
            for (size_t i = 0; i < segments.size(); i++)
            {
                auto& segment = segments[i];

                // calculate start
                if (i == 0)
                {
                    // this is the first segment
                    start1 = pathStart1;
                    start2 = pathStart2;
                }

                if (i + 1 == segments.size())
                {
                    // this is the last segment
                    end1 = pathEnd1;
                    end2 = pathEnd2;
                }
                else
                {
                    createJoint(
                        segment, segments[i + 1], jointStyle, end1, end2,
                        nextStart1, nextStart2, allowOverlap);
                }

                size_t n = m_vertices.size();

                // emit vertices
                m_vertices.emplace_back(start1);
                m_vertices.emplace_back(start2);
                m_vertices.emplace_back(end1);
                m_vertices.emplace_back(end2);

                // emit UVs
                if (m_softEdges)
                {
                    m_uvs.emplace_back(V2f(0, 0.5));
                    m_uvs.emplace_back(V2f(1, 0.5));
                    m_uvs.emplace_back(V2f(0, 0.5));
                    m_uvs.emplace_back(V2f(1, 0.5));
                }

                // Create two triangles
                m_tris.emplace_back(IndexTriangle(n, n + 1, n + 2));
                m_tris.emplace_back(IndexTriangle(n + 2, n + 1, n + 3));

                start1 = nextStart1;
                start2 = nextStart2;
            }
        }

        void Polyline2D::createJoint(
            const PolySegment<Point>& segment1,
            const PolySegment<Point>& segment2, JointStyle jointStyle,
            Point& end1, Point& end2, Point& nextStart1, Point& nextStart2,
            const bool allowOverlap)
        {
            // calculate the angle between the two line segments
            auto dir1 = segment1.center.direction();
            auto dir2 = segment2.center.direction();

            auto angle = dir1.angle(dir2);

            // wrap the angle around the 180° mark if it exceeds 90°
            // for minimum angle detection
            auto wrappedAngle = angle;
            if (wrappedAngle > math::pi / 2)
            {
                wrappedAngle = math::pi - wrappedAngle;
            }

            if (jointStyle == JointStyle::MITER && wrappedAngle < miterMinAngle)
            {
                // the minimum angle for mitered joints wasn't exceeded.
                // to avoid the intersection point being extremely far out,
                // thus producing an enormous joint like a rasta on 4/20,
                // we render the joint beveled instead.
                jointStyle = JointStyle::BEVEL;

                // jointStyle = JointStyle::ROUND;  // ok, but uvs on next
                // segment don't match
            }

            if (jointStyle == JointStyle::MITER)
            {
                // calculate each edge's intersection point
                // with the next segment's central line
                Point* sec1 = nullptr;
                Point* uv1 = nullptr;
                Point* sec2 = nullptr;
                Point* uv2 = nullptr;
                LineSegment<Point>::intersection(
                    segment1.edge1, segment2.edge1, sec1, true);
                LineSegment<Point>::intersection(
                    segment1.edge2, segment2.edge2, sec2, true);

                end1 = sec1 ? *sec1 : segment1.edge1.b;
                end2 = sec2 ? *sec2 : segment1.edge2.b;

                delete sec1;
                delete sec2;

                nextStart1 = end1;
                nextStart2 = end2;
            }
            else
            {
                // joint style is either BEVEL or ROUND

                // find out which are the inner edges for this joint
                auto x1 = dir1.x;
                auto x2 = dir2.x;
                auto y1 = dir1.y;
                auto y2 = dir2.y;

                auto clockwise = x1 * y2 - x2 * y1 < 0;

                const LineSegment<Point>*inner1, *inner2, *outer1, *outer2;

                // as the normal vector is rotated counter-clockwise,
                // the first edge lies to the left
                // from the central line's perspective,
                // and the second one to the right.
                if (clockwise)
                {
                    outer1 = &segment1.edge1;
                    outer2 = &segment2.edge1;
                    inner1 = &segment1.edge2;
                    inner2 = &segment2.edge2;
                }
                else
                {
                    outer1 = &segment1.edge2;
                    outer2 = &segment2.edge2;
                    inner1 = &segment1.edge1;
                    inner2 = &segment2.edge1;
                }

                // calculate the intersection point of the inner edges
                Point* innerSecOpt = nullptr;

                LineSegment<Point>::intersection(
                    *inner1, *inner2, innerSecOpt, allowOverlap);

                // for parallel lines, simply
                // connect them directly
                auto innerSec = innerSecOpt ? *innerSecOpt : inner1->b;

                // if there's no inner intersection, flip
                // the next start position for near-180° turns
                Point innerStart;
                if (innerSecOpt)
                {
                    innerStart = innerSec;
                }
                else if (angle > math::pi / 2)
                {
                    innerStart = outer1->b;
                }
                else
                {
                    innerStart = inner1->b;
                }

                if (clockwise)
                {
                    end1 = outer1->b;
                    end2 = innerSec;

                    nextStart1 = outer2->a;
                    nextStart2 = innerStart;
                }
                else
                {
                    end1 = innerSec;
                    end2 = outer1->b;

                    nextStart1 = innerStart;
                    nextStart2 = outer2->a;
                }

                // connect the intersection points according to the joint
                // style

                if (jointStyle == JointStyle::BEVEL)
                {
                    // simply connect the intersection points
                    size_t n = m_vertices.size();
                    m_vertices.emplace_back(outer1->b);
                    m_vertices.emplace_back(outer2->a);
                    m_vertices.emplace_back(innerSec);

                    if (m_softEdges)
                    {
                        V2f tmp(0.0f, 0.5f);
                        m_uvs.emplace_back(tmp);
                        m_uvs.emplace_back(tmp);
                        tmp.x = 1.0f;
                        m_uvs.emplace_back(tmp);
                    }

                    m_tris.emplace_back(IndexTriangle(n, n + 1, n + 2));
                }
                else if (jointStyle == JointStyle::ROUND)
                {
                    if (!innerSecOpt && m_softEdges)
                    {
                        if (angle > math::pi / 2)
                        {
                            createRoundSoftCap(segment1, true);
                            createRoundSoftCap(segment2, false);
                            nextStart1 = segment2.edge1.a;
                            nextStart2 = segment2.edge2.a;
                        }
                    }
                    else
                    {
                        // draw a semicircle between the ends of the outer
                        // edges, centered at the actual point
                        // with half the line thickness as the radius
                        createTriangleFan(
                            innerSec, segment1.center.b, outer1->b, outer2->a,
                            1.0, 0.0, clockwise);
                    }
                }
                else
                {
                    assert(false);
                }

                delete innerSecOpt;
            }
        }

        /**
         * Creates a partial circle between two points.
         * The points must be equally far away from the origin.
         * @param vertices The vector to add vertices to.
         * @param connectTo The position to connect the triangles to.
         * @param origin The circle's origin.
         * @param start The circle's starting point.
         * @param end The circle's ending point.
         * @param clockwise Whether the circle's rotation is clockwise.
         */
        void Polyline2D::createTriangleFan(
            Point connectTo, Point origin, Point start, Point end,
            float uvConnectTo, float uvEdge, bool clockwise)
        {
            auto point1 = start - origin;
            auto point2 = end - origin;

            // calculate the angle between the two points
            auto angle1 = atan2(point1.y, point1.x);
            auto angle2 = atan2(point2.y, point2.x);

            // ensure the outer angle is calculated
            if (clockwise)
            {
                if (angle2 > angle1)
                {
                    angle2 = angle2 - 2 * math::pi;
                }
            }
            else
            {
                if (angle1 > angle2)
                {
                    angle1 = angle1 - 2 * math::pi;
                }
            }

            auto jointAngle = angle2 - angle1;

            // calculate the amount of triangles to use for the joint
            auto numTriangles = std::max(
                1, (int)std::floor(std::abs(jointAngle) / roundMinAngle));

            // calculate the angle of each triangle
            auto triAngle = jointAngle / numTriangles;

            size_t connectIndex = m_vertices.size();

            m_vertices.emplace_back(connectTo);

            if (m_softEdges)
                m_uvs.emplace_back(V2f(uvConnectTo, 0.5f));

            Point startPoint = start;
            Point endPoint;
            for (int t = 0; t < numTriangles; t++)
            {
                if (t + 1 == numTriangles)
                {
                    // it's the last triangle - ensure it perfectly
                    // connects to the next line
                    endPoint = end;
                }
                else
                {
                    auto rot = (t + 1) * triAngle;

                    // rotate the original point around the origin
                    endPoint.x =
                        std::cos(rot) * point1.x - std::sin(rot) * point1.y;
                    endPoint.y =
                        std::sin(rot) * point1.x + std::cos(rot) * point1.y;

                    // re-add the rotation origin to the target point
                    endPoint = endPoint + origin;
                }

                // emit the triangle
                size_t n = m_vertices.size();

                m_vertices.emplace_back(startPoint);
                m_vertices.emplace_back(endPoint);

                if (m_softEdges)
                {
                    m_uvs.emplace_back(V2f(uvEdge, 0.5f));
                    m_uvs.emplace_back(V2f(uvEdge, 0.5f));
                }

                m_tris.emplace_back(IndexTriangle(connectIndex, n, n + 1));

                startPoint = endPoint;
            }
        }

        //      +--+--+      + Polysegment points
        //      |     |      * Extended calculated points
        //      |     |
        //      +--+--+
        //      | / \ |
        //      |/   \|
        //      *-----*
        void Polyline2D::createRoundSoftCap(
            const PolySegment<Point>& segment, const bool start)
        {
            auto left = segment.edge1.direction() * m_width * 0.5;
            auto right = segment.edge2.direction() * m_width * 0.5;

            Point center, edge1, edge2;
            if (!start)
            {
                left *= -1;
                right *= -1;

                edge1 = segment.edge1.a;
                edge2 = segment.edge2.a;
                center = segment.center.a;
            }
            else
            {
                edge1 = segment.edge1.b;
                edge2 = segment.edge2.b;
                center = segment.center.b;
            }

            left += edge1;
            right += edge2;

            V2f uvCenter = V2f(0.5F, 0.5F);
            V2f uvLeft = V2f(0.0F, 0.0F);
            V2f uvRight = V2f(1.0F, 0.0F);
            V2f uvEdge1 = V2f(0.0F, 0.5F);
            V2f uvEdge2 = V2f(1.0F, 0.5F);

            size_t n = m_vertices.size();

            m_vertices.emplace_back(center); // 0
            m_vertices.emplace_back(edge1);  // 1
            m_vertices.emplace_back(left);   // 2
            m_vertices.emplace_back(edge2);  // 3
            m_vertices.emplace_back(right);  // 4

            if (m_softEdges)
            {
                m_uvs.emplace_back(uvCenter);
                m_uvs.emplace_back(uvEdge1);
                m_uvs.emplace_back(uvLeft);
                m_uvs.emplace_back(uvEdge2);
                m_uvs.emplace_back(uvRight);
            }

            m_tris.emplace_back(IndexTriangle(n, n + 1, n + 2));
            m_tris.emplace_back(IndexTriangle(n, n + 2, n + 4));
            m_tris.emplace_back(IndexTriangle(n, n + 4, n + 3));
        }

    } // namespace draw

} // namespace tl
