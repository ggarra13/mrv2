// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Box.h>
#include <tlCore/Color.h>

#include <array>
#include <vector>

namespace tl
{
    //! Geometry
    namespace geom
    {
        //! Two-dimensional vertex.
        struct Vertex2
        {
            Vertex2();
            Vertex2(size_t v, size_t t = 0, size_t c = 0);

            size_t v;
            size_t t;
            size_t c;
        };

        //! Three-dimensional vertex.
        struct Vertex3
        {
            Vertex3();
            Vertex3(size_t v, size_t t = 0, size_t n = 0, size_t c = 0);

            size_t v;
            size_t t;
            size_t n;
            size_t c;
        };

        //! Two-dimensional triangle.
        struct Triangle2
        {
            std::array<Vertex2, 3> v;
        };

        //! Three-dimensional triangle.
        struct Triangle3
        {
            std::array<Vertex3, 3> v;
        };

        //! Two-dimensional triangle mesh.
        struct TriangleMesh2
        {
            std::vector<math::Vector2f> v;
            std::vector<math::Vector4f> c;
            std::vector<math::Vector2f> t;
            std::vector<Triangle2> triangles;
        };

        //! Three-dimensional triangle mesh.
        struct TriangleMesh3
        {
            std::vector<math::Vector3f> v;
            std::vector<math::Vector4f> c;
            std::vector<math::Vector2f> t;
            std::vector<math::Vector3f> n;
            std::vector<Triangle3> triangles;
        };

        //! Create a two-dimensional box mesh.
        TriangleMesh2 box(const math::Box2i&, bool flipV = false);

        //! Create a two-dimensional box mesh.
        TriangleMesh2 box(const math::Box2f&, bool flipV = false);

        //! Create a mesh for drawing checkers.
        geom::TriangleMesh2 checkers(
            const math::Box2i&, const image::Color4f& color0,
            const image::Color4f& color1, const math::Size2i& checkerSize);

        //! Edge function.
        float edge(
            const math::Vector2f& p, const math::Vector2f& v0,
            const math::Vector2f& v1);

        //! Scanlines function
        geom::TriangleMesh2 scanlines(const int start,
                                      const math::Size2i& size);
        
        //! Columns function
        geom::TriangleMesh2 columns(const int start, const math::Size2i& size);
        
        //! Create a sphere triangle mesh.
        TriangleMesh3
        sphere(float radius, size_t xResolution, size_t yResolution);
    } // namespace geom
} // namespace tl

#include <tlCore/MeshInline.h>
