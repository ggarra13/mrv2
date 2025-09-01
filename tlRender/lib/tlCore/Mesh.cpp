// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Mesh.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace geom
    {
        TriangleMesh2 box(const math::Box2i& box, bool flipV)
        {
            TriangleMesh2 out;

            const auto& min = box.min;
            const auto& max = box.max;
            out.v.push_back(math::Vector2f(min.x, min.y));
            out.v.push_back(math::Vector2f(max.x + 1, min.y));
            out.v.push_back(math::Vector2f(max.x + 1, max.y + 1));
            out.v.push_back(math::Vector2f(min.x, max.y + 1));
            out.t.push_back(math::Vector2f(0.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 0.F : 1.F));
            out.t.push_back(math::Vector2f(0.F, flipV ? 0.F : 1.F));

            Triangle2 triangle;
            triangle.v[0].v = 1;
            triangle.v[1].v = 2;
            triangle.v[2].v = 3;
            triangle.v[0].t = 1;
            triangle.v[1].t = 2;
            triangle.v[2].t = 3;
            out.triangles.push_back(triangle);
            triangle.v[0].v = 3;
            triangle.v[1].v = 4;
            triangle.v[2].v = 1;
            triangle.v[0].t = 3;
            triangle.v[1].t = 4;
            triangle.v[2].t = 1;
            out.triangles.push_back(triangle);

            return out;
        }

        TriangleMesh2 box(const math::Box2f& box, bool flipV)
        {
            TriangleMesh2 out;

            const auto& min = box.min;
            const auto& max = box.max;
            out.v.push_back(math::Vector2f(min.x, min.y));
            out.v.push_back(math::Vector2f(max.x, min.y));
            out.v.push_back(math::Vector2f(max.x, max.y));
            out.v.push_back(math::Vector2f(min.x, max.y));
            out.t.push_back(math::Vector2f(0.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 0.F : 1.F));
            out.t.push_back(math::Vector2f(0.F, flipV ? 0.F : 1.F));

            Triangle2 triangle;
            triangle.v[0].v = 1;
            triangle.v[1].v = 2;
            triangle.v[2].v = 3;
            triangle.v[0].t = 1;
            triangle.v[1].t = 2;
            triangle.v[2].t = 3;
            out.triangles.push_back(triangle);
            triangle.v[0].v = 3;
            triangle.v[1].v = 4;
            triangle.v[2].v = 1;
            triangle.v[0].t = 3;
            triangle.v[1].t = 4;
            triangle.v[2].t = 1;
            out.triangles.push_back(triangle);

            return out;
        }

        geom::TriangleMesh2 checkers(
            const math::Box2i& box, const math::Size2i& checkerSize)
        {
            geom::TriangleMesh2 out;

            // X points.
            std::vector<int> xs;
            int x = box.min.x;
            for (; x < box.max.x; x += checkerSize.w)
            {
                xs.push_back(x);
            }
            if (x >= box.max.x)
            {
                xs.push_back(box.max.x);
            }

            // Y points.
            std::vector<int> ys;
            int y = box.min.y;
            for (; y < box.max.y; y += checkerSize.h)
            {
                ys.push_back(y);
            }
            if (y >= box.max.y)
            {
                ys.push_back(box.max.y);
            }

            if (!xs.empty() && !ys.empty())
            {
                // 2D points.
                for (int j = 0; j < ys.size(); ++j)
                {
                    for (int i = 0; i < xs.size(); ++i)
                    {
                        out.v.push_back(math::Vector2f(xs[i], ys[j]));
                    }
                }

                // Triangles.
                for (int j = 0; j < ys.size() - 1; ++j)
                {
                    for (int i = 0; i < xs.size() - 1; ++i)
                    {
                        if ( (i + j) % 2 == 0 )
                        {
                            const int v0 = j * xs.size() + i + 1;
                            const int v1 = j * xs.size() + (i + 1) + 1;
                            const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                            const int v3 = (j + 1) * xs.size() + i + 1;
                            out.triangles.push_back(
                                {geom::Vertex2(v0, 0, 0),
                                 geom::Vertex2(v1, 0, 0),
                                 geom::Vertex2(v2, 0, 0)});
                            out.triangles.push_back(
                                {geom::Vertex2(v2, 0, 0),
                                 geom::Vertex2(v3, 0, 0),
                                 geom::Vertex2(v0, 0, 0)});
                        }
                    }
                }
            }

            return out;
        }
            
        geom::TriangleMesh2 checkers(
            const math::Box2i& box, const image::Color4f& color0,
            const image::Color4f& color1, const math::Size2i& checkerSize)
        {
            geom::TriangleMesh2 out;

            // X points.
            std::vector<int> xs;
            int x = box.min.x;
            for (; x < box.max.x; x += checkerSize.w)
            {
                xs.push_back(x);
            }
            if (x >= box.max.x)
            {
                xs.push_back(box.max.x);
            }

            // Y points.
            std::vector<int> ys;
            int y = box.min.y;
            for (; y < box.max.y; y += checkerSize.h)
            {
                ys.push_back(y);
            }
            if (y >= box.max.y)
            {
                ys.push_back(box.max.y);
            }

            if (!xs.empty() && !ys.empty())
            {
                // 2D points.
                for (int j = 0; j < ys.size(); ++j)
                {
                    for (int i = 0; i < xs.size(); ++i)
                    {
                        out.v.push_back(math::Vector2f(xs[i], ys[j]));
                    }
                }

                // Colors.
                out.c.push_back(
                    math::Vector4f(color0.r, color0.g, color0.b, color0.a));
                out.c.push_back(
                    math::Vector4f(color1.r, color1.g, color1.b, color1.a));

                // Triangles.
                for (int j = 0; j < ys.size() - 1; ++j)
                {
                    for (int i = 0; i < xs.size() - 1; ++i)
                    {
                        const int v0 = j * xs.size() + i + 1;
                        const int v1 = j * xs.size() + (i + 1) + 1;
                        const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                        const int v3 = (j + 1) * xs.size() + i + 1;
                        const int c = (j + i) % 2 + 1;
                        out.triangles.push_back(
                            {geom::Vertex2(v0, 0, c), geom::Vertex2(v1, 0, c),
                             geom::Vertex2(v2, 0, c)});
                        out.triangles.push_back(
                            {geom::Vertex2(v2, 0, c), geom::Vertex2(v3, 0, c),
                             geom::Vertex2(v0, 0, c)});
                    }
                }
            }

            return out;
        }

        TriangleMesh3
        sphere(float radius, size_t xResolution, size_t yResolution)
        {
            TriangleMesh3 out;

            //! \bug Use only a single vertex at each pole.
            for (size_t v = 0; v <= yResolution; ++v)
            {
                const float v1 =
                    static_cast<float>(v) / static_cast<float>(yResolution);

                for (size_t u = 0; u <= xResolution; ++u)
                {
                    const float u1 =
                        static_cast<float>(u) / static_cast<float>(xResolution);
                    const float x =
                        radius * sinf(v1 * math::pi) * cosf(u1 * math::pi2);
                    const float y = radius * cosf(v1 * math::pi);
                    const float z =
                        radius * sinf(v1 * math::pi) * sinf(u1 * math::pi2);
                    out.v.push_back(math::Vector3f(x, y, z));
                    out.t.push_back(math::Vector2f(u1, 1.F - v1));
                }
            }

            Triangle3 triangle;
            for (size_t v = 0; v < yResolution; ++v)
            {
                for (size_t u = 0; u < xResolution; ++u)
                {
                    const size_t i = u + v * (xResolution + 1);
                    const size_t j = u + (v + 1) * (xResolution + 1);
                    triangle.v[0].v = triangle.v[0].t = j + 1 + 1;
                    triangle.v[1].v = triangle.v[1].t = j + 1;
                    triangle.v[2].v = triangle.v[2].t = i + 1;
                    out.triangles.push_back(triangle);
                    triangle.v[0].v = triangle.v[0].t = i + 1;
                    triangle.v[1].v = triangle.v[1].t = i + 1 + 1;
                    triangle.v[2].v = triangle.v[2].t = j + 1 + 1;
                    out.triangles.push_back(triangle);
                }
            }

            return out;
        }
        
        geom::TriangleMesh2 scanlines(const math::Box2i& box)
        {
            geom::TriangleMesh2 out;
            
            const size_t lines = box.h() / 2;
            out.v.reserve(lines * 3 * 2);
            out.triangles.reserve(lines * 2);
            
            math::Vector2f pts[4];

            size_t idx = 1;
            for (int y = box.min.y; y < box.max.y; y += 2, idx += 6)
            {
                pts[0].x = 0;
                pts[0].y = y;
                pts[1].x = box.w();
                pts[1].y = y;
                pts[2].x = box.w();
                pts[2].y = y + 1;
                pts[3].x = 0;
                pts[3].y = y + 1;

                out.v.push_back(pts[0]);
                out.v.push_back(pts[1]);
                out.v.push_back(pts[2]);

                geom::Triangle2 tri;
                tri.v[0] = idx;
                tri.v[1] = idx + 1;
                tri.v[2] = idx + 2;
                out.triangles.push_back(tri);
                
                out.v.push_back(pts[0]);
                out.v.push_back(pts[2]);
                out.v.push_back(pts[3]);
                
                tri.v[0] = idx + 3;
                tri.v[1] = idx + 4;
                tri.v[2] = idx + 5;
                out.triangles.push_back(tri);
            }
            return out;
        }
        
        geom::TriangleMesh2 columns(const math::Box2i& box)
        {
            geom::TriangleMesh2 out;
            
            const size_t lines = box.w() / 2;
            out.v.reserve(lines * 3 * 2);
            out.triangles.reserve(lines * 2);
            
            math::Vector2f pts[4];

            size_t idx = 1;
            for (int x = box.min.x; x < box.max.x; x += 2, idx += 6)
            {
                pts[0].x = x + 1;
                pts[0].y = box.h();

                pts[1].x = x;
                pts[1].y = box.h();

                pts[2].x = x;
                pts[2].y = 0;
                    
                pts[3].x = x + 1;
                pts[3].y = 0;

                out.v.push_back(pts[0]);
                out.v.push_back(pts[1]);
                out.v.push_back(pts[2]);

                geom::Triangle2 tri;
                tri.v[0] = idx;
                tri.v[1] = idx + 1;
                tri.v[2] = idx + 2;
                out.triangles.push_back(tri);
                    
                out.v.push_back(pts[0]);
                out.v.push_back(pts[2]);
                out.v.push_back(pts[3]);
                
                tri.v[0] = idx + 3;
                tri.v[1] = idx + 4;
                tri.v[2] = idx + 5;
                out.triangles.push_back(tri);
            }
            return out;
        }
            
    } // namespace geom
} // namespace tl
