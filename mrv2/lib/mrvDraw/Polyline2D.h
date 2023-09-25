// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Original code is:
//
// Copyright © 2019 Marius Metzger (CrushedPixel)
//

#pragma once

#include <vector>
#include <iterator>
#include <cassert>

#include <tlCore/Math.h>

#include "Point.h"
#include "LineSegment.h"

namespace mrv
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
            bool m_softEdges = false;
            float m_width = 2.0F;

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

            // After create is called, return the computed vertices
            const PointList& getVertices() const { return m_vertices; }

            // After create is called, return the computed UVs
            const UVList& getUVs() const { return m_uvs; }

            // After create is called, return the computed triangle indices
            const TriangleList& getTriangles() const { return m_tris; }

            //! Whether soft edges are used (shader)
            void setSoftEdges(bool t) { m_softEdges = t; }

            //! Set the width of the polyline
            void setWidth(float w) { m_width = w; }

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
                const PointList& points,
                JointStyle jointStyle = JointStyle::MITER,
                EndCapStyle endCapStyle = EndCapStyle::BUTT,
                bool catmullRomSplines = false, bool allowOverlap = false);

        protected:
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

            void filterPoints();

            void createJoint(
                const PolySegment<Point>& segment1,
                const PolySegment<Point>& segment2, JointStyle jointStyle,
                Point& end1, Point& end2, Point& nextStart1, Point& nextStart2,
                const bool allowOverlap);

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
                Point connectTo, Point origin, Point start, Point end,
                float uvConnectTo, float uvEdge, bool clockwise);

            void createRoundSoftCap(
                const PolySegment<Point>& segment, const bool start = true);
        };

    } // namespace draw

} // namespace mrv
