// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <mrvCore/mrvMesh.h>

namespace mrv
{

    /**
     * @brief Create an environment cube of triangle meshes.
     *
     * @param size size of cube (1.0 default)
     *
     * @return a triangle mesh.
     */
    geom::TriangleMesh3 createEnvCube(const float size, const bool flipped)
    {
        geom::TriangleMesh3 out;

        const float t6 = 1.0;
        const float t5 = 5.0 / 6.0;
        const float t4 = 4.0 / 6.0;
        const float t3 = 3.0 / 6.0;
        const float t2 = 2.0 / 6.0;
        const float t1 = 1.0 / 6.0;
        const float t0 = 0.0;

        // *RIGHT* Face (OK)
        out.v.push_back(math::Vector3f(-size, size, size));
        out.v.push_back(math::Vector3f(size, size, size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.v.push_back(math::Vector3f(-size, -size, size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(1.F, t2));
            out.t.push_back(math::Vector2f(0.F, t2));
            out.t.push_back(math::Vector2f(0.F, t1));
            out.t.push_back(math::Vector2f(1.F, t1));
        }
        else
        {
            out.t.push_back(math::Vector2f(1.F, t5));
            out.t.push_back(math::Vector2f(0.F, t5));
            out.t.push_back(math::Vector2f(0.F, t4));
            out.t.push_back(math::Vector2f(1.F, t4));
        }

        // *LEFT* Face (OK)
        out.v.push_back(math::Vector3f(-size, size, -size));
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.v.push_back(math::Vector3f(size, -size, -size));
        out.v.push_back(math::Vector3f(size, size, -size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(0.F, t1));
            out.t.push_back(math::Vector2f(0.F, t0));
            out.t.push_back(math::Vector2f(1.F, t0));
            out.t.push_back(math::Vector2f(1.F, t1));
        }
        else
        {
            out.t.push_back(math::Vector2f(0.F, t6));
            out.t.push_back(math::Vector2f(0.F, t5));
            out.t.push_back(math::Vector2f(1.F, t5));
            out.t.push_back(math::Vector2f(1.F, t6));
        }
        
        // *TOP* Face (OK)
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.v.push_back(math::Vector3f(-size, -size, size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.v.push_back(math::Vector3f(size, -size, -size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(1.0, t3));
            out.t.push_back(math::Vector2f(0.0, t3));
            out.t.push_back(math::Vector2f(0.0, t2));
            out.t.push_back(math::Vector2f(1.0, t2));
        }
        else
        {
            out.t.push_back(math::Vector2f(1.0, t3));
            out.t.push_back(math::Vector2f(0.0, t3));
            out.t.push_back(math::Vector2f(0.0, t2));
            out.t.push_back(math::Vector2f(1.0, t2));
        }
        
        // *BOTTOM* Face (OK)
        out.v.push_back(math::Vector3f(-size, size, -size));
        out.v.push_back(math::Vector3f(size, size, -size));
        out.v.push_back(math::Vector3f(size, size, size));
        out.v.push_back(math::Vector3f(-size, size, size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(1.0, t3));
            out.t.push_back(math::Vector2f(1.0, t4));
            out.t.push_back(math::Vector2f(0.0, t4));
            out.t.push_back(math::Vector2f(0.0, t3));
        }
        else
        {
            out.t.push_back(math::Vector2f(1.0, t3));
            out.t.push_back(math::Vector2f(1.0, t4));
            out.t.push_back(math::Vector2f(0.0, t4));
            out.t.push_back(math::Vector2f(0.0, t3));
        }
        
        // *BACK* Face (OK)
        out.v.push_back(math::Vector3f(size, size, -size));
        out.v.push_back(math::Vector3f(size, -size, -size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.v.push_back(math::Vector3f(size, size, size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(0.0, t5));
            out.t.push_back(math::Vector2f(0.0, t4));
            out.t.push_back(math::Vector2f(1.0, t4));
            out.t.push_back(math::Vector2f(1.0, t5));
        }
        else
        {
            out.t.push_back(math::Vector2f(0.0, t2));
            out.t.push_back(math::Vector2f(0.0, t1));
            out.t.push_back(math::Vector2f(1.0, t1));
            out.t.push_back(math::Vector2f(1.0, t2));
        }
        
        // *FRONT* Face (OK)
        out.v.push_back(math::Vector3f(-size, size, size));
        out.v.push_back(math::Vector3f(-size, -size, size));
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.v.push_back(math::Vector3f(-size, size, -size));

        if (flipped)
        {
            out.t.push_back(math::Vector2f(0.0, t6));
            out.t.push_back(math::Vector2f(0.0, t5));
            out.t.push_back(math::Vector2f(1.0, t5));
            out.t.push_back(math::Vector2f(1.0, t6));
        }
        else
        {
            out.t.push_back(math::Vector2f(0.0, t1));
            out.t.push_back(math::Vector2f(0.0, t0));
            out.t.push_back(math::Vector2f(1.0, t0));
            out.t.push_back(math::Vector2f(1.0, t1));
        }
        
        geom::Triangle3 triangle;
        size_t numVertices = out.v.size();
        size_t numQuads = numVertices / 4;
        size_t numTriangles = numQuads * 2;
        // std::cerr << "numVertices= " << numVertices << std::endl;
        // std::cerr << "numQuads= " << numQuads << std::endl;
        // std::cerr << "numTriangles= " << numTriangles << std::endl;
        size_t v = 0;
        for (size_t i = 0; i < numQuads; ++i, v += 4)
        {
            triangle.v[0].v = v + 1;
            triangle.v[1].v = v + 2;
            triangle.v[2].v = v + 3;

            triangle.v[0].t = v + 1;
            triangle.v[1].t = v + 2;
            triangle.v[2].t = v + 3;
            out.triangles.emplace_back(triangle);

            triangle.v[0].v = v + 3;
            triangle.v[1].v = v + 4;
            triangle.v[2].v = v + 1;

            triangle.v[0].t = v + 3;
            triangle.v[1].t = v + 4;
            triangle.v[2].t = v + 1;
            out.triangles.emplace_back(triangle);
        }

        return out;
    }

    geom::TriangleMesh2 createRoundedRect(const math::Box2f& bbox,
                                          float radius, int segments)
    {
        geom::TriangleMesh2 out;
    
        // Ensure radius doesn't exceed half the dimensions
        float maxR = std::min(bbox.w(), bbox.h()) * 0.5f;
        radius = std::clamp(radius, 0.0f, maxR);

        // If radius is 0, just return a standard rectangle mesh
        if (radius <= 0.0f) {
            // Simplified: push 4 corners and 2 triangles
            return out; 
        }

        // Helper to add vertices and triangles
        // We create a center vertex to fan out triangles for each corner
        size_t centerIdx = out.v.size();
    
        // Points for the 4 arc centers
        math::Vector2f centers[4] = {
            { bbox.max.x - radius, bbox.min.y + radius }, // Top Right
            { bbox.max.x - radius, bbox.max.y - radius }, // Bottom Right
            { bbox.min.x + radius, bbox.max.y - radius }, // Bottom Left
            { bbox.min.x + radius, bbox.min.y + radius }  // Top Left
        };

        float angles[4] = { 3.0f*M_PI/2.0f, 0.f, M_PI/2.0f, M_PI };

        // 1. Generate Vertices
        // Add a central vertex for a clean fill (or build as a strip)
        out.v.push_back(bbox.getCenter());

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j <= segments; ++j) {
                float theta = angles[i] + (j / (float)segments) * (M_PI / 2.0f);
                math::Vector2f p(
                    centers[i].x + radius * cos(theta),
                    centers[i].y + radius * sin(theta)
                    );
                out.v.push_back(p);
            }
        }

        // 3. Generate Triangles
        // Indices in tlRender are 1-based.
        // Index 1 is our center. The perimeter starts at Index 2.
        size_t numPerimeterVertex = out.v.size() - 1; 

        for (size_t i = 0; i < numPerimeterVertex; ++i) {
            geom::Triangle2 tri;
            tri.v[0].v = 1;                             // Center
            tri.v[1].v = i + 2;                         // Current perimeter point
            tri.v[2].v = (i + 1 < numPerimeterVertex) ? (i + 3) : 2; // Next point (wrap to start)

            out.triangles.push_back(tri);
        }
    
        return out;
    }

} // namespace mrv
