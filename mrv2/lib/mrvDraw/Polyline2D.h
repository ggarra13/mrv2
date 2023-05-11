// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Original code is:
//
// Copyright © 2019 Marius Metzger (CrushedPixel)

#pragma once

#include <vector>
#include <iterator>
#include <cassert>

#include <tlCore/Math.h>

#include "LineSegment.h"

namespace tl
{
    namespace draw
    {

        class Polyline2D
        {
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

            /**
             * Creates a vector of vertices describing a solid path through the
             * input points.
             * @param points The points of the path.
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
            template <typename Point>
            static void create(
                std::vector<Point>& vertices, std::vector<Point>& uvs,
                const std::vector<Point>& points, float thickness,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool allowOverlap = false)
            {
                create<Point, std::vector<Point>>(
                    vertices, uvs, points, thickness, jointStyle, endCapStyle,
                    allowOverlap);
            }

            template <typename Point, typename InputCollection>
            static void create(
                std::vector<Point>& vertices, std::vector<Point>& uvs,
                const InputCollection& points, float thickness,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool allowOverlap = false)
            {
                create<Point, InputCollection>(
                    std::back_inserter(vertices), std::back_inserter(uvs),
                    points, thickness, jointStyle, endCapStyle, allowOverlap);
            }

            template < typename Point, typename InputCollection>
            static void filterPoints(
                InputCollection& filteredPoints, const InputCollection& points,
                const float thickness)
            {
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
            }

            template <
                typename Point, typename InputCollection,
                typename OutputIterator>
            static void create(
                OutputIterator vertices, OutputIterator uvs,
                const InputCollection& inPoints, float thickness,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool allowOverlap = false)
            {
                // operate on half the thickness to make our lives easier
                thickness /= 2;

                // Filter the points
                InputCollection points;
                filterPoints<Point, InputCollection>(
                    points, inPoints, thickness);

                // create poly segments from the points
                std::vector<PolySegment<Point>> segments;
                double numPoints = static_cast<double>(points.size() - 1);
                for (size_t i = 0; i + 1 < points.size(); i++)
                {
                    auto& point1 = points[i];
                    auto& point2 = points[i + 1];
                    Point uv1(1, i / numPoints);
                    Point uv2(1, (i + 1) / numPoints);

                    if (point1 != point2)
                        segments.emplace_back(
                            LineSegment<Point>(point1, point2, uv1, uv2),
                            thickness);
                }

                if (endCapStyle == EndCapStyle::JOINT)
                {
                    // create a connecting segment from the last to the first
                    // point

                    auto& point1 = points[points.size() - 1];
                    auto& point2 = points[0];
                    Point uv1(1, 1);
                    Point uv2(1, 0);

                    if (point1 != point2)
                        segments.emplace_back(
                            LineSegment<Point>(point1, point2, uv1, uv2),
                            thickness);
                }

                if (segments.empty())
                {
                    // Splat, create a circle
                    const float w = thickness;
                    Point center = points[0];

                    Point top = center + Point(0, -w);
                    Point bottom = center + Point(0, w);

                    createTriangleFan(
                        vertices, uvs, center, center, top, bottom,
                        Point(0.5, 0), Point(1, 1), Point(1, 1), Point(1, 1),
                        false);
                    createTriangleFan(
                        vertices, uvs, center, center, top, bottom,
                        Point(0.5, 0), Point(1, 1), Point(1, 1), Point(1, 1),
                        true);

                    return;
                }

                Point nextStart1{0, 0};
                Point nextStart2{0, 0};
                Point start1{0, 0};
                Point start2{0, 0};
                Point end1{0, 0};
                Point end2{0, 0};

                Point uvNextStart1{1, 0};
                Point uvNextStart2{1, 0};
                Point uvStart1{1, 0};
                Point uvStart2{1, 0};
                Point uvEnd1{1, 0};
                Point uvEnd2{1, 0};

                // calculate the path's global start and end points
                auto& firstSegment = segments[0];
                auto& lastSegment = segments[segments.size() - 1];

                auto pathStart1 = firstSegment.edge1.a;
                auto pathStart2 = firstSegment.edge2.a;
                auto pathEnd1 = lastSegment.edge1.b;
                auto pathEnd2 = lastSegment.edge2.b;

                auto uvPathStart1 = firstSegment.edge1.aUV;
                auto uvPathStart2 = firstSegment.edge2.aUV;
                auto uvPathEnd1 = lastSegment.edge1.bUV;
                auto uvPathEnd2 = lastSegment.edge2.bUV;

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
                    firstSegment.center.aUV.x = 0.5;
                    lastSegment.center.bUV.x = 0.5;
                    createTriangleFan(
                        vertices, uvs, firstSegment.center.a,
                        firstSegment.center.a, firstSegment.edge1.a,
                        firstSegment.edge2.a, firstSegment.center.aUV,
                        firstSegment.center.aUV, firstSegment.edge1.aUV,
                        firstSegment.edge2.aUV, false);
                    createTriangleFan(
                        vertices, uvs, lastSegment.center.b,
                        lastSegment.center.b, lastSegment.edge1.b,
                        lastSegment.edge2.b, lastSegment.center.bUV,
                        lastSegment.center.bUV, lastSegment.edge1.bUV,
                        lastSegment.edge2.bUV, true);
                }
                else if (endCapStyle == EndCapStyle::JOINT)
                {
                    // join the last (connecting) segment and the first segment
                    createJoint(
                        vertices, uvs, lastSegment, firstSegment, jointStyle,
                        pathEnd1, pathEnd2, uvPathEnd1, uvPathEnd2, pathStart1,
                        pathStart2, uvPathStart1, uvPathStart2, allowOverlap);
                }

                // generate mesh data for path segments
                Point uv;
                for (size_t i = 0; i < segments.size(); i++)
                {
                    auto& segment = segments[i];

                    // calculate start
                    if (i == 0)
                    {
                        // this is the first segment
                        start1 = pathStart1;
                        start2 = pathStart2;

                        uvStart1 = uvPathStart1;
                        uvStart2 = uvPathStart2;
                    }

                    if (i + 1 == segments.size())
                    {
                        // this is the last segment
                        end1 = pathEnd1;
                        end2 = pathEnd2;

                        // this is the last segment
                        uvEnd1 = uvPathEnd1;
                        uvEnd2 = uvPathEnd2;
                    }
                    else
                    {
                        createJoint(
                            vertices, uvs, segment, segments[i + 1], jointStyle,
                            end1, end2, uvEnd1, uvEnd2, nextStart1, nextStart2,
                            uvNextStart1, uvNextStart2, allowOverlap);
                    }

                    // emit vertices
                    *vertices++ = start1;
                    *vertices++ = start2;
                    *vertices++ = end1;

                    *vertices++ = end1;
                    *vertices++ = start2;
                    *vertices++ = end2;

                    // emit UVs
                    uvStart1.x = 0;
                    uvStart2.x = 1;
                    uvEnd1.x = 0;
                    uvEnd2.x = 1;

                    *uvs++ = uvStart1;
                    *uvs++ = uvStart2;
                    *uvs++ = uvEnd1;

                    *uvs++ = uvEnd1;
                    *uvs++ = uvStart2;
                    *uvs++ = uvEnd2;

                    start1 = nextStart1;
                    start2 = nextStart2;

                    uvStart1 = uvNextStart1;
                    uvStart2 = uvNextStart2;
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
                    edge1.aUV = center.aUV;
                    edge1.bUV = center.bUV;
                    edge2.aUV = center.aUV;
                    edge2.bUV = center.bUV;
                    this->center.aUV[0] = 0.5;
                    this->center.bUV[0] = 0.5;
                }

                LineSegment<Point> center, edge1, edge2;
            };

            template <typename Point, typename OutputIterator>
            static void createJoint(
                OutputIterator vertices, OutputIterator uvs,
                const PolySegment<Point>& segment1,
                const PolySegment<Point>& segment2, JointStyle jointStyle,
                Point& end1, Point& end2, Point& uvEnd1, Point& uvEnd2,
                Point& nextStart1, Point& nextStart2, Point& uvNextStart1,
                Point& uvNextStart2, bool allowOverlap)
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
                        segment1.edge1, segment2.edge1, sec1, uv1, true);
                    LineSegment<Point>::intersection(
                        segment1.edge2, segment2.edge2, sec2, uv2, true);

                    end1 = sec1 ? *sec1 : segment1.edge1.b;
                    end2 = sec2 ? *sec2 : segment1.edge2.b;

                    uvEnd1 = uv1 ? *uv1 : segment1.edge1.bUV;
                    uvEnd2 = uv2 ? *uv2 : segment1.edge2.bUV;

                    delete sec1;
                    delete sec2;
                    delete uv1;
                    delete uv2;

                    nextStart1 = end1;
                    nextStart2 = end2;

                    uvNextStart1 = uvEnd1;
                    uvNextStart2 = uvEnd2;
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
                    Point* uvInnerSecOpt = nullptr;
                    LineSegment<Point>::intersection(
                        *inner1, *inner2, innerSecOpt, uvInnerSecOpt,
                        allowOverlap);

                    auto innerSec = innerSecOpt ? *innerSecOpt
                                                // for parallel lines, simply
                                                // connect them directly
                                                : inner1->b;
                    auto uvInnerSec = uvInnerSecOpt
                                          ? *uvInnerSecOpt
                                          // for parallel lines, simply
                                          // connect them directly
                                          : inner1->bUV;
                    delete innerSecOpt;
                    delete uvInnerSecOpt;

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
                        *vertices++ = outer1->b;
                        *vertices++ = outer2->a;
                        *vertices++ = innerSec;

                        Point tmp(-1, outer1->bUV.y);
                        *uvs++ = tmp;
                        tmp.y = outer2->aUV.y;
                        *uvs++ = tmp;
                        *uvs++ = uvInnerSec;
                    }
                    else if (jointStyle == JointStyle::ROUND)
                    {
                        // draw a circle between the ends of the outer edges,
                        // centered at the actual point
                        // with half the line thickness as the radius
                        uvInnerSec.x = 1.0;
                        createTriangleFan(
                            vertices, uvs, innerSec, segment1.center.b,
                            outer1->b, outer2->a, uvInnerSec,
                            segment1.center.bUV, outer1->bUV, outer2->aUV,
                            clockwise);
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
            template <typename Point, typename OutputIterator>
            static void createTriangleFan(
                OutputIterator vertices, OutputIterator uvs, Point connectTo,
                Point origin, Point start, Point end, Point uvConnectTo,
                Point uvOrigin, Point uvStart, Point uvEnd, bool clockwise)
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
                Point uvStartPoint = uvStart;
                Point endPoint, uvEndPoint = uvEnd;
                for (int t = 0; t < numTriangles; t++)
                {
                    if (t + 1 == numTriangles)
                    {
                        // it's the last triangle - ensure it perfectly
                        // connects to the next line
                        endPoint = end;
                        uvEndPoint = uvEnd;
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

                    uvStartPoint.x = 0;
                    uvEndPoint.x = 0;
                    *uvs++ = uvStartPoint;
                    *uvs++ = uvEndPoint;
                    *uvs++ = uvConnectTo;

                    startPoint = endPoint;
                    uvStartPoint = uvEndPoint;
                }
            }
        };

    } // namespace draw

} // namespace tl
