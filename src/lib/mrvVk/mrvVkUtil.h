// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include <tlCore/Box.h>
#include <tlCore/Matrix.h>
#include <tlCore/Vector.h>
#include <tlCore/Mesh.h>

#include <tlTimelineVk/Render.h>

#include "tlDraw/Point.h"
#include "tlDraw/Polyline2D.h"
#include "tlDraw/Shape.h"

#include "mrvNetwork/mrvMessage.h"

namespace mrv
{
    using namespace tl;

    namespace util
    {
        //! Draw a rectangle outline with a mesh.
        inline void drawRectOutline(
            const std::shared_ptr<timeline_vlk::Render>& render,
            const std::string& pipelineName,
            const math::Box2i& rect, const
            image::Color4f& color, const int width,
            VkRenderPass renderPass = VK_NULL_HANDLE)
    {
        geom::TriangleMesh2 mesh;

        // Add the outside vertices.
        // math::Box2i outside = rect.margin(width / 2);
        mesh.v.push_back(math::Vector2f(rect.min.x - 2.0f, rect.min.y - 2.0f));
        mesh.v.push_back(math::Vector2f(rect.max.x + 2.0f, rect.min.y - 2.0f));
        mesh.v.push_back(math::Vector2f(rect.max.x + 2.0f, rect.max.y + 2.0f));
        mesh.v.push_back(math::Vector2f(rect.min.x - 2.0f, rect.max.y + 2.0f));

        // Add the inside vertices.
        mesh.v.push_back(math::Vector2f(rect.min.x, rect.min.y));
        mesh.v.push_back(math::Vector2f(rect.max.x, rect.min.y));
        mesh.v.push_back(math::Vector2f(rect.max.x, rect.max.y));
        mesh.v.push_back(math::Vector2f(rect.min.x, rect.max.y));

        // Add the triangles. Note that vertex indexes start at one,
        // zero is invalid.
        mesh.triangles.push_back(geom::Triangle2({1, 2, 5}));
        mesh.triangles.push_back(geom::Triangle2({2, 6, 5}));
        mesh.triangles.push_back(geom::Triangle2({2, 3, 6}));
        mesh.triangles.push_back(geom::Triangle2({3, 7, 6}));
        mesh.triangles.push_back(geom::Triangle2({3, 4, 8}));
        mesh.triangles.push_back(geom::Triangle2({8, 7, 3}));
        mesh.triangles.push_back(geom::Triangle2({4, 5, 8}));
        mesh.triangles.push_back(geom::Triangle2({4, 1, 5}));

        math::Vector2i pos;

        if (renderPass)
        {
            render->drawMesh(pipelineName, "mesh", "mesh", renderPass,
                             mesh, pos, color, false);
        }
        else
        {
            render->drawMesh(pipelineName, "mesh", "mesh", "mesh",
                             mesh, pos, color, false);
        }
        
    }

    //! Draw a filled circle.
    void drawFilledCircle(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::string& pipelineName,
        const math::Vector2i& center, const float radius,
        const image::Color4f& color, const bool soft = false);

    }  // namespace util
    
} // namespace mrv
