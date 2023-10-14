// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <tlCore/StringFormat.h>

#include <tlGL/Shader.h>
#include <tlGL/Mesh.h>
#include <tlTimeline/GLRenderPrivate.h>
#include <tlGL/Util.h>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLShaders.h"
#include "mrvGL/mrvGLUtil.h"

namespace
{
    const char* kModule = "glutil";
}

namespace mrv
{
    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const image::Color4f& color, const bool soft)
    {
        geom::TriangleMesh2 mesh;
        const int CIRCLE_SEGMENTS = 32;
        const double twoPi = math::pi * 2.0;
        math::Vector2f v(center.x, center.y);
        mesh.v.push_back(v);
        for (int i = 0; i < CIRCLE_SEGMENTS; ++i)
        {
            v.x = center.x + radius * std::cos(twoPi * i / CIRCLE_SEGMENTS);
            v.y = center.y + radius * std::sin(twoPi * i / CIRCLE_SEGMENTS);
            mesh.v.push_back(v);
        }

        geom::Triangle2 triangle;
        unsigned numTriangles = CIRCLE_SEGMENTS;
        int i;
        for (i = 1; i < numTriangles; ++i)
        {
            triangle.v[0] = 1;
            triangle.v[1] = i + 1;
            triangle.v[2] = i + 2;
            mesh.triangles.emplace_back(triangle);
        }

        triangle.v[0] = 1;
        triangle.v[1] = 2;
        triangle.v[2] = i + 1;
        mesh.triangles.emplace_back(triangle);

        math::Vector2i pos;
        render->drawMesh(mesh, pos, color);
    }

    geom::TriangleMesh2
    checkers(const math::Box2i& box, const math::Size2i& checkerSize)
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
            for (int j = 0; j < ys.size() - 1; j += 2)
            {
                for (int i = 0; i < xs.size() - 1; i += 2)
                {
                    const int v0 = j * xs.size() + i + 1;
                    const int v1 = j * xs.size() + (i + 1) + 1;
                    const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                    const int v3 = (j + 1) * xs.size() + i + 1;
                    out.triangles.push_back(
                        {geom::Vertex2(v0, 0, 0), geom::Vertex2(v1, 0, 0),
                         geom::Vertex2(v2, 0, 0)});
                    out.triangles.push_back(
                        {geom::Vertex2(v2, 0, 0), geom::Vertex2(v3, 0, 0),
                         geom::Vertex2(v0, 0, 0)});
                }
            }
            for (int j = 1; j < ys.size() - 1; j += 2)
            {
                for (int i = 1; i < xs.size() - 1; i += 2)
                {
                    const int v0 = j * xs.size() + i + 1;
                    const int v1 = j * xs.size() + (i + 1) + 1;
                    const int v2 = (j + 1) * xs.size() + (i + 1) + 1;
                    const int v3 = (j + 1) * xs.size() + i + 1;
                    out.triangles.push_back(
                        {geom::Vertex2(v0, 0, 0), geom::Vertex2(v1, 0, 0),
                         geom::Vertex2(v2, 0, 0)});
                    out.triangles.push_back(
                        {geom::Vertex2(v2, 0, 0), geom::Vertex2(v3, 0, 0),
                         geom::Vertex2(v0, 0, 0)});
                }
            }
        }

        return out;
    }

} // namespace mrv
