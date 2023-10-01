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

} // namespace mrv
