// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include "mrvFl/mrvIO.h"

#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkUtil.h"

#include "mrvCore/mrvI8N.h"

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/Shader.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/StringFormat.h>

namespace
{
    const char* kModule = "vkutil";
}

namespace mrv
{
    namespace util
    {
    
        void drawFilledCircle(
            const std::shared_ptr<timeline_vlk::Render>& render,
            const std::string& pipelineName,
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

            bool enableBlending = false;
            if (color.a < 0.99F)
                enableBlending = true;
        
            const math::Vector2i pos;
            render->drawMesh(pipelineName, "rect", "rect", "mesh",
                             mesh, pos, color, enableBlending);
        }


    } // namespace util
    
} // namespace mrv
