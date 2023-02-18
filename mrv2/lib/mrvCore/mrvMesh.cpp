// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <mrvCore/mrvMesh.h>

namespace mrv
{

    geom::TriangleMesh3 createEnvCube(float size)
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
        out.v.push_back(math::Vector3f(-size, -size, size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.v.push_back(math::Vector3f(size, size, size));
        out.v.push_back(math::Vector3f(-size, size, size));

        out.t.push_back(math::Vector2f(1.F, t5));
        out.t.push_back(math::Vector2f(0.F, t5));
        out.t.push_back(math::Vector2f(0.F, t4));
        out.t.push_back(math::Vector2f(1.F, t4));

        // *LEFT* Face (OK)
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.v.push_back(math::Vector3f(-size, size, -size));
        out.v.push_back(math::Vector3f(size, size, -size));
        out.v.push_back(math::Vector3f(size, -size, -size));
        out.t.push_back(math::Vector2f(0.F, t6));
        out.t.push_back(math::Vector2f(0.F, t5));
        out.t.push_back(math::Vector2f(1.F, t5));
        out.t.push_back(math::Vector2f(1.F, t6));

        // *BOTTOM* Face (OK)
        out.v.push_back(math::Vector3f(-size, size, -size));
        out.v.push_back(math::Vector3f(-size, size, size));
        out.v.push_back(math::Vector3f(size, size, size));
        out.v.push_back(math::Vector3f(size, size, -size));
        out.t.push_back(math::Vector2f(1.0, t3));
        out.t.push_back(math::Vector2f(0.0, t3));
        out.t.push_back(math::Vector2f(0.0, t2));
        out.t.push_back(math::Vector2f(1.0, t2));

        // *TOP* Face (OK)
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.v.push_back(math::Vector3f(size, -size, -size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.v.push_back(math::Vector3f(-size, -size, size));
        out.t.push_back(math::Vector2f(1.0, t3));
        out.t.push_back(math::Vector2f(1.0, t4));
        out.t.push_back(math::Vector2f(0.0, t4));
        out.t.push_back(math::Vector2f(0.0, t3));

        // *FRONT* Face (OK)
        out.v.push_back(math::Vector3f(size, -size, -size));
        out.v.push_back(math::Vector3f(size, size, -size));
        out.v.push_back(math::Vector3f(size, size, size));
        out.v.push_back(math::Vector3f(size, -size, size));
        out.t.push_back(math::Vector2f(0.0, t2));
        out.t.push_back(math::Vector2f(0.0, t1));
        out.t.push_back(math::Vector2f(1.0, t1));
        out.t.push_back(math::Vector2f(1.0, t2));

        // *BACK* Face (OK)
        out.v.push_back(math::Vector3f(-size, -size, size));
        out.v.push_back(math::Vector3f(-size, size, size));
        out.v.push_back(math::Vector3f(-size, size, -size));
        out.v.push_back(math::Vector3f(-size, -size, -size));
        out.t.push_back(math::Vector2f(0.0, t1));
        out.t.push_back(math::Vector2f(0.0, t0));
        out.t.push_back(math::Vector2f(1.0, t0));
        out.t.push_back(math::Vector2f(1.0, t1));

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
} // namespace mrv
