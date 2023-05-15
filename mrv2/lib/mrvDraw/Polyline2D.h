// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Original code is:
//
// Copyright © 2019 Marius Metzger (CrushedPixel)
//
// Smoothing code is:
//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved.

#pragma once

#include <vector>
#include <iterator>
#include <cassert>

#include <tlCore/Math.h>

#include "Point.h"
#include "LineSegment.h"

namespace tl
{
    namespace draw
    {
        using namespace Imath;

        class Polyline2D
        {
        public:
            typedef std::vector<V2f> UVList;
            typedef Vec3<size_t> IndexTriangle;
            typedef std::vector<IndexTriangle> TriangleList;

            PointList points;

            PointList m_vertices;
            UVList m_uvs;
            TriangleList m_tris;

        public:
            enum class JointStyle {
                /**
                 * Corners are drawn with sharp joints.
                 * If the joint's outer angle is too large,
                 * the joint is drawn as beveled instead,
                 * to avoid the miter extending too far out.
                 */
                MITER,
                /**
                 * Corners are flattened.
                 */
                BEVEL,
                /**
                 * Corners are rounded off.
                 */
                ROUND
            };

            enum class EndCapStyle {
                /**
                 * Path ends are drawn flat,
                 * and don't exceed the actual end point.
                 */
                BUTT,
                /**
                 * Path ends are drawn flat,
                 * but extended beyond the end point
                 * by half the line thickness.
                 */
                SQUARE,
                /**
                 * Path ends are rounded off.
                 */
                ROUND,
                /**
                 * Path ends are connected according to the JointStyle.
                 * When using this EndCapStyle, don't specify the common
                 * start/end point twice, as Polyline2D connects the first and
                 * last input point itself.
                 */
                JOINT
            };

            const PointList& getVertices() const { return m_vertices; }
            const UVList& getUVs() const { return m_uvs; }
            const TriangleList& getTriangles() const { return m_tris; }

            /**
             * Creates a vector of vertices describing a solid path through the
             * input points.
             * @param thickness The path's thickness.
             * @param jointStyle The path's joint style.
             * @param endCapStyle The path's end cap style.
             * @param allowOverlap Whether to allow overlapping vertices.
             *                                         This yields better
             * results when dealing with paths whose points have a distance
             * smaller than the thickness, but may introduce overlapping
             * vertices, which is undesirable when rendering transparent paths.
             * @return The vertices describing the path.
             * @tparam Point The vector type to use for the vertices.
             *              Must have public non-const float fields "x" and "y".
             *              Must have a two-args constructor taking x and y
             * values. See mrv::Point for a type that satisfies these
             * requirements.
             * @tparam InputCollection The collection type of the input points.
             *                         Must contain elements of type Point.
             *                         Must expose size() and operator[]
             * functions.
             */
            void create(
                const PointList& points, float thickness,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool doSmooth = false, bool allowOverlap = false)
            {
                create(
                    std::back_inserter(m_vertices), std::back_inserter(m_uvs),
                    points, thickness, jointStyle, endCapStyle, doSmooth,
                    allowOverlap);
            }

            void filterPoints(const float thickness)
            {
                PointList filteredPoints;
                filteredPoints.push_back(points.front());

                for (size_t i = 1; i < points.size(); i++)
                {
                    const Point& p0 = filteredPoints.back();
                    const Point& p1 = points[i];
                    const Point tmp = p0 - p1;
                    const float length = tmp.length();
                    if (length <= thickness)
                        continue;
                    filteredPoints.push_back(p1);
                }
                points = filteredPoints;
            }

            void sampleCentripetalCatmull(
                const Point p0, const Point p1, const Point p2, const Point p3,
                size_t n, PointList& newPoints)
            {
                // using centripetal catmull interpolation
                // http://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline

                float t0, t1, t2, t3;
                Point a1, a2, a3, b1, b2;

                // compute ts
                t0 = 0;
                t1 = pow(
                    (float)((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y)),
                    (float)0.25);
                t2 =
                    t1 +
                    pow((float)((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y)),
                        (float)0.25);
                t3 =
                    t2 +
                    pow((float)((p3.x - p2.x) * (p3.x - p2.x) + (p3.y - p2.y) * (p3.y - p2.y)),
                        (float)0.25);

                // sample between p1 and p2
                for (size_t q = 1; q < n; q++)
                {
                    float t = t1 + (t2 - t1) * (float)q / (float)n;

                    // compute as
                    a1 = p0 * (t1 - t) / (t1 - t0) + p1 * (t - t0) / (t1 - t0);
                    a2 = p1 * (t2 - t) / (t2 - t1) + p2 * (t - t1) / (t2 - t1);
                    a3 = p2 * (t3 - t) / (t3 - t2) + p3 * (t - t2) / (t3 - t2);

                    // compute bs
                    b1 = a1 * (t2 - t) / (t2 - t0) + a2 * (t - t0) / (t2 - t0);
                    b2 = a2 * (t3 - t) / (t3 - t1) + a3 * (t - t1) / (t3 - t1);

                    // compute point
                    Point p =
                        b1 * (t2 - t) / (t2 - t1) + b2 * (t - t1) / (t2 - t1);
                    newPoints.push_back(p);
                }
            }

            void smoothPoints(const float width)
            {
                if (points.size() < 3)
                {
                    return;
                }

                PointList newPoints;
                float distmult = 0.1;
                newPoints.push_back(points.front());
                for (size_t i = 1; i < points.size(); i++)
                {
                    const Point& p3 = points[i];
                    const Point& p0 = points[i - 1];
                    const Point d = p3 - p0;
                    const float dist = d.length();
                    const int n = size_t(dist / (width * distmult));

                    const float w = 0.25;

                    if (n > 1)
                    {
                        Point t0, t1;
                        float scale = 1.0F;

                        if (i == 1)
                        {
                            sampleCentripetalCatmull(
                                p0 + (p0 - p3), p0, p3, points[i + 1], n,
                                newPoints);
                        }
                        else if (i == points.size() - 1)
                        {
                            sampleCentripetalCatmull(
                                points[i - 2], p0, p3, p3 + (p3 - p0), n,
                                newPoints);
                        }
                        else
                        {
                            sampleCentripetalCatmull(
                                points[i - 2], p0, p3, points[i + 1], n,
                                newPoints);
                        }
                        newPoints.push_back(p3);
                    }
                    else
                    {
                        newPoints.push_back(points[i]);
                    }
                }

                points = newPoints;
            }

            void create(
                std::back_insert_iterator<PointList> vertices,
                std::back_insert_iterator<UVList> uvs,
                const PointList& inPoints, float thickness,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool doSmooth = false, bool allowOverlap = false)
            {
                // operate on half the thickness to make our lives easier
                thickness /= 2;

                // Filter the points
                points = inPoints;
                filterPoints(thickness);

                if (doSmooth && endCapStyle != EndCapStyle::JOINT)
                {
                    smoothPoints(1.0);
                }

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
                    // Splat, create a filled circle
                    const float w = thickness;
                    Point center = points[0];

                    Point top = center + Point(0, -w);
                    Point bottom = center + Point(0, w);

                    createTriangleFan(
                        vertices, uvs, center, center, top, bottom, 0.5, 0.0,
                        false);
                    createTriangleFan(
                        vertices, uvs, center, center, top, bottom, 0.5, 0.0,
                        true);

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
                    pathEnd1 =
                        pathEnd1 + lastSegment.edge1.direction() * thickness;
                    pathEnd2 =
                        pathEnd2 + lastSegment.edge2.direction() * thickness;
                }
                else if (endCapStyle == EndCapStyle::ROUND)
                {
                    // draw half circle end caps
                    createTriangleFan(
                        vertices, uvs, firstSegment.center.a,
                        firstSegment.center.a, firstSegment.edge1.a,
                        firstSegment.edge2.a, 0.5, 0.0, false);
                    createTriangleFan(
                        vertices, uvs, lastSegment.center.b,
                        lastSegment.center.b, lastSegment.edge1.b,
                        lastSegment.edge2.b, 0.5, 0.0, true);
                }
                else if (endCapStyle == EndCapStyle::JOINT)
                {
                    // join the last (connecting) segment and the first segment
                    createJoint(
                        vertices, uvs, lastSegment, firstSegment, jointStyle,
                        pathEnd1, pathEnd2, pathStart1, pathStart2,
                        allowOverlap);
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
                            vertices, uvs, segment, segments[i + 1], jointStyle,
                            end1, end2, nextStart1, nextStart2, allowOverlap);
                    }

                    // emit vertices
                    *vertices++ = start1;
                    *vertices++ = start2;
                    *vertices++ = end1;

                    *vertices++ = end1;
                    *vertices++ = start2;
                    *vertices++ = end2;

                    // emit UVs
                    *uvs++ = V2f(0, 0.5);
                    *uvs++ = V2f(1, 0.5);
                    *uvs++ = V2f(0, 0.5);

                    *uvs++ = V2f(0, 0.5);
                    *uvs++ = V2f(1, 0.5);
                    *uvs++ = V2f(1, 0.5);

                    start1 = nextStart1;
                    start2 = nextStart2;
                }
            }

        private:
            /**
             * The threshold for mitered joints.
             * If the joint's angle is smaller than this angle,
             * the joint will be drawn beveled instead.
             */
            static constexpr float miterMinAngle = 0.349066; // ~20 degrees

            /**
             * The minimum angle of a round joint's triangles.
             */
            static constexpr float roundMinAngle = 0.174533; // ~10 degrees

            template <typename Point> struct PolySegment
            {
                PolySegment(const LineSegment<Point>& center, float thickness) :
                    center(center),
                    // calculate the segment's outer edges by offsetting
                    // the central line by the normal vector
                    // multiplied with the thickness

                    // center + center.normal() * thickness
                    edge1(center + center.normal() * thickness),
                    edge2(center - center.normal() * thickness)
                {
                }

                LineSegment<Point> center, edge1, edge2;
            };

            void createJoint(
                std::back_insert_iterator<PointList> vertices,
                std::back_insert_iterator<UVList> uvs,
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

                if (jointStyle == JointStyle::MITER &&
                    wrappedAngle < miterMinAngle)
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

                    delete innerSecOpt;

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
                        *vertices++ = outer1->b;
                        *vertices++ = outer2->a;
                        *vertices++ = innerSec;

                        V2f tmp(0.5f, 0.5f);
                        *uvs++ = tmp;
                        *uvs++ = tmp;
                        tmp.x = 1.0f;
                        *uvs++ = tmp;
                    }
                    else if (jointStyle == JointStyle::ROUND)
                    {
                        // draw a semicircle between the ends of the outer
                        // edges, centered at the actual point
                        // with half the line thickness as the radius
                        if (!innerSecOpt)
                        {
                            const Point& connectTo =
                                (outer2->a + outer1->b) / 2;
                            const Point& origin = segment2.center.a;
                            createTriangleFan(
                                vertices, uvs, connectTo, origin, outer1->b,
                                outer2->a, 0.5, 0.0, clockwise);
                        }
                        else
                        {
                            createTriangleFan(
                                vertices, uvs, innerSec, segment1.center.b,
                                outer1->b, outer2->a, 1.0, 0.0, clockwise);
                        }
                    }
                    else
                    {
                        assert(false);
                    }
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
            void createTriangleFan(
                std::back_insert_iterator<PointList> vertices,
                std::back_insert_iterator<UVList> uvs, Point connectTo,
                Point origin, Point start, Point end, float uvConnectTo,
                float uvEdge, bool clockwise)
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
                    *vertices++ = startPoint;
                    *vertices++ = endPoint;
                    *vertices++ = connectTo;

                    *uvs++ = V2f(uvEdge, 0.5f);
                    *uvs++ = V2f(uvEdge, 0.5f);
                    *uvs++ = V2f(uvConnectTo, 0.5f);

                    startPoint = endPoint;
                }
            }
        };

    } // namespace draw

} // namespace tl
