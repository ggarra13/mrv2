// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/DrawUtil.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace ui
    {
        geom::TriangleMesh2
        rect(const math::Box2i& box, int cornerRadius, size_t resolution)
        {
            geom::TriangleMesh2 out;

            const int x = box.x();
            const int y = box.y();
            const int w = box.w();
            const int h = box.h();

            if (0 == cornerRadius)
            {
                out.v.push_back(math::Vector2f(x, y));
                out.v.push_back(math::Vector2f(x + w, y));
                out.v.push_back(math::Vector2f(x + w, y + h));
                out.v.push_back(math::Vector2f(x, y + h));

                out.triangles.push_back(geom::Triangle2({1, 2, 3}));
                out.triangles.push_back(geom::Triangle2({3, 4, 1}));
            }
            else
            {
                const int r = cornerRadius;

                const std::vector<math::Vector2f> c = {
                    math::Vector2f(x + w - r, y + h - r),
                    math::Vector2f(x + r, y + h - r),
                    math::Vector2f(x + r, y + r),
                    math::Vector2f(x + w - r, y + r)};
                size_t i = 0;
                for (size_t j = 0; j < 4; ++j)
                {
                    out.v.push_back(c[j]);
                    for (size_t k = 0; k < resolution; ++k)
                    {
                        const float v = k / static_cast<float>(resolution - 1);
                        const float a =
                            math::lerp(v, j * 90.F, j * 90.F + 90.F);
                        const float cos = cosf(math::deg2rad(a));
                        const float sin = sinf(math::deg2rad(a));
                        out.v.push_back(
                            math::Vector2f(c[j].x + cos * r, c[j].y + sin * r));
                    }
                    for (size_t k = 0; k < resolution - 1; ++k)
                    {
                        out.triangles.push_back(
                            geom::Triangle2({i + 1, i + k + 2, i + k + 3}));
                    }
                    i += 1 + resolution;
                }

                i = 0;
                size_t j = resolution;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, j + 2}));
                out.triangles.push_back(geom::Triangle2({j + 1, j + 3, j + 2}));

                i += 1 + resolution;
                j += 1 + resolution;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, j + 2}));
                out.triangles.push_back(geom::Triangle2({j + 1, j + 3, j + 2}));

                i += 1 + resolution;
                j += 1 + resolution;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, j + 2}));
                out.triangles.push_back(geom::Triangle2({j + 1, j + 3, j + 2}));

                i += 1 + resolution;
                j += 1 + resolution;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, 2}));
                out.triangles.push_back(geom::Triangle2({2, 1, i + 1}));

                i = 0;
                j = 1 + resolution;
                size_t k = (1 + resolution) * 2;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, k + 1}));
                i = k;
                j = k + 1 + resolution;
                k = 0;
                out.triangles.push_back(geom::Triangle2({i + 1, j + 1, k + 1}));
            }

            return out;
        }

        geom::TriangleMesh2
        circle(const math::Vector2i& pos, int radius, size_t resolution)
        {
            geom::TriangleMesh2 out;

            const int inc = 360 / resolution;
            for (int i = 0; i < 360; i += inc)
            {
                const size_t size = out.v.size();
                out.v.push_back(math::Vector2f(pos.x, pos.y));
                out.v.push_back(math::Vector2f(
                    pos.x + cos(math::deg2rad(i)) * radius,
                    pos.y + sin(math::deg2rad(i)) * radius));
                const int d = std::min(i + inc, 360);
                out.v.push_back(math::Vector2f(
                    pos.x + cos(math::deg2rad(d)) * radius,
                    pos.y + sin(math::deg2rad(d)) * radius));
                out.triangles.push_back({size + 1, size + 2, size + 3});
            }

            return out;
        }

        geom::TriangleMesh2
        border(const math::Box2i& box, int width, int radius, size_t resolution)
        {
            geom::TriangleMesh2 out;

            const int x = box.x();
            const int y = box.y();
            const int w = box.w();
            const int h = box.h();

            if (0 == radius)
            {
                out.v.push_back(math::Vector2f(x, y));
                out.v.push_back(math::Vector2f(x + w, y));
                out.v.push_back(math::Vector2f(x + w, y + h));
                out.v.push_back(math::Vector2f(x, y + h));
                out.v.push_back(math::Vector2f(x + width, y + width));
                out.v.push_back(math::Vector2f(x + w - width, y + width));
                out.v.push_back(math::Vector2f(x + w - width, y + h - width));
                out.v.push_back(math::Vector2f(x + width, y + h - width));

                out.triangles.push_back(geom::Triangle2({1, 2, 5}));
                out.triangles.push_back(geom::Triangle2({2, 6, 5}));
                out.triangles.push_back(geom::Triangle2({2, 3, 6}));
                out.triangles.push_back(geom::Triangle2({3, 7, 6}));
                out.triangles.push_back(geom::Triangle2({3, 4, 7}));
                out.triangles.push_back(geom::Triangle2({4, 8, 7}));
                out.triangles.push_back(geom::Triangle2({4, 1, 8}));
                out.triangles.push_back(geom::Triangle2({1, 5, 8}));
            }
            else
            {
                const int r = radius;

                const std::vector<math::Vector2f> c = {
                    math::Vector2f(x + w - r, y + h - r),
                    math::Vector2f(x + r, y + h - r),
                    math::Vector2f(x + r, y + r),
                    math::Vector2f(x + w - r, y + r)};
                size_t i = 0;
                for (size_t j = 0; j < 4; ++j)
                {
                    for (size_t k = 0; k < resolution; ++k)
                    {
                        const float v = k / static_cast<float>(resolution - 1);
                        const float a =
                            math::lerp(v, j * 90.F, j * 90.F + 90.F);
                        const float cos = cosf(math::deg2rad(a));
                        const float sin = sinf(math::deg2rad(a));
                        out.v.push_back(
                            math::Vector2f(c[j].x + cos * r, c[j].y + sin * r));
                        out.v.push_back(math::Vector2f(
                            c[j].x + cos * (r - width),
                            c[j].y + sin * (r - width)));
                    }
                    for (size_t k = 0; k < resolution - 1; ++k)
                    {
                        out.triangles.push_back(
                            geom::Triangle2({i + 1, i + 3, i + 2}));
                        out.triangles.push_back(
                            geom::Triangle2({i + 3, i + 4, i + 2}));
                        i += 2;
                    }
                    i += 2;
                }

                i = resolution * 2 - 2;
                out.triangles.push_back(geom::Triangle2({i + 1, i + 3, i + 2}));
                out.triangles.push_back(geom::Triangle2({i + 3, i + 4, i + 2}));

                i = resolution * 4 - 2;
                out.triangles.push_back(geom::Triangle2({i + 1, i + 3, i + 2}));
                out.triangles.push_back(geom::Triangle2({i + 3, i + 4, i + 2}));

                i = resolution * 6 - 2;
                out.triangles.push_back(geom::Triangle2({i + 1, i + 3, i + 2}));
                out.triangles.push_back(geom::Triangle2({i + 3, i + 4, i + 2}));

                i = resolution * 8 - 2;
                out.triangles.push_back(geom::Triangle2({i + 1, 1, i + 2}));
                out.triangles.push_back(geom::Triangle2({1, 2, i + 2}));
            }

            return out;
        }

        geom::TriangleMesh2 shadow(
            const math::Box2i& box, int cornerRadius, const float alpha,
            size_t resolution)
        {
            geom::TriangleMesh2 out;

            const int x = box.x();
            const int y = box.y();
            const int w = box.w();
            const int h = box.h();
            const int r = cornerRadius;

            out.c.push_back(math::Vector4f(0.F, 0.F, 0.F, alpha));
            out.c.push_back(math::Vector4f(0.F, 0.F, 0.F, 0.F));

            const std::vector<math::Vector2f> c = {
                math::Vector2f(x + w - r, y + h - r),
                math::Vector2f(x + r, y + h - r), math::Vector2f(x + r, y + r),
                math::Vector2f(x + w - r, y + r)};
            size_t i = 0;
            for (size_t j = 0; j < 4; ++j)
            {
                out.v.push_back(c[j]);
                for (size_t k = 0; k < resolution; ++k)
                {
                    const float v = k / static_cast<float>(resolution - 1);
                    const float a = math::lerp(v, j * 90.F, j * 90.F + 90.F);
                    const float cos = cosf(math::deg2rad(a));
                    const float sin = sinf(math::deg2rad(a));
                    out.v.push_back(
                        math::Vector2f(c[j].x + cos * r, c[j].y + sin * r));
                }
                for (size_t k = 0; k < resolution - 1; ++k)
                {
                    out.triangles.push_back(geom::Triangle2(
                        {geom::Vertex2(i + 1, 0, 1),
                         geom::Vertex2(i + k + 2, 0, 2),
                         geom::Vertex2(i + k + 3, 0, 2)}));
                }
                i += 1 + resolution;
            }

            i = 0;
            size_t j = resolution;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(j + 1, 0, 2), geom::Vertex2(j + 3, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));

            i += 1 + resolution;
            j += 1 + resolution;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(j + 1, 0, 2), geom::Vertex2(j + 3, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));

            i += 1 + resolution;
            j += 1 + resolution;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(j + 1, 0, 2), geom::Vertex2(j + 3, 0, 2),
                 geom::Vertex2(j + 2, 0, 1)}));

            i += 1 + resolution;
            j += 1 + resolution;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 2),
                 geom::Vertex2(2, 0, 2)}));
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(2, 0, 2), geom::Vertex2(1, 0, 1),
                 geom::Vertex2(i + 1, 0, 1)}));

            i = 0;
            j = 1 + resolution;
            size_t k = (1 + resolution) * 2;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 1),
                 geom::Vertex2(k + 1, 0, 1)}));
            i = k;
            j = k + 1 + resolution;
            k = 0;
            out.triangles.push_back(geom::Triangle2(
                {geom::Vertex2(i + 1, 0, 1), geom::Vertex2(j + 1, 0, 1),
                 geom::Vertex2(k + 1, 0, 1)}));

            return out;
        }
    } // namespace ui
} // namespace tl
