// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>
#include <tlGL/Shader.h>

#include "mrvGL/mrvGLErrors.h"
#include "mrvGL/mrvGLShaders.h"
#include "mrvGL/mrvGLLines.h"

namespace tl
{
    namespace timeline_gl
    {
        extern std::string vertexSource();
    } // namespace timeline_gl

} // namespace tl

namespace mrv
{

    namespace opengl
    {

        using namespace tl;

        struct Lines::Private
        {
            std::shared_ptr<gl::Shader> softShader = nullptr;
            std::shared_ptr<gl::Shader> hardShader = nullptr;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
        };

        Lines::Lines() :
            _p(new Private)
        {
        }

        Lines::~Lines() {}

        void Lines::drawLines(
            const std::shared_ptr<timeline::IRender>& render,
            const draw::PointList& pts, const image::Color4f& color,
            const float width, const bool soft,
            const draw::Polyline2D::JointStyle jointStyle,
            const draw::Polyline2D::EndCapStyle endStyle,
            const bool catmullRomSpline, const bool allowOverlap)
        {
            TLRENDER_P();

            if (!p.softShader)
            {
                try
                {
                    const std::string& vertexSource =
                        tl::timeline_gl::vertexSource();
                    p.softShader = gl::Shader::create(
                        vertexSource, mrv::softFragmentSource());
                    p.hardShader = gl::Shader::create(
                        vertexSource, mrv::hardFragmentSource());
                }
                catch (const std::exception& e)
                {
                    throw e;
                }
            }

            using namespace mrv::draw;

            Polyline2D path;
            path.setWidth(width);
            path.setSoftEdges(soft);
            path.create(
                pts, jointStyle, endStyle, catmullRomSpline, allowOverlap);

            const PointList& draw = path.getVertices();
            const Polyline2D::UVList& uvs = path.getUVs();
            const Polyline2D::TriangleList& triangles = path.getTriangles();

            geom::TriangleMesh2 mesh;

            size_t numVertices = draw.size();
            size_t numTriangles = triangles.size();
            size_t numUVs = uvs.size();

            // Verify data in debug mode
            assert(numTriangles > 0);
            assert(numVertices > 0);
            assert(numUVs == 0 || numUVs == numVertices);

            mesh.triangles.reserve(numTriangles);

            tl::gl::VBOType vboType = gl::VBOType::Pos2_F32;
            geom::Triangle2 triangle;
            if (numUVs > 0)
            {
                vboType = gl::VBOType::Pos2_F32_UV_U16;
                for (size_t i = 0; i < numTriangles; ++i)
                {
                    Polyline2D::IndexTriangle t = triangles[i];
                    t += Polyline2D::IndexTriangle(1, 1, 1);
                    triangle.v[0].v = t[0];
                    triangle.v[1].v = t[1];
                    triangle.v[2].v = t[2];
                    triangle.v[0].t = t[0];
                    triangle.v[1].t = t[1];
                    triangle.v[2].t = t[2];
                    mesh.triangles.push_back(triangle);
                }
                mesh.t.reserve(numUVs);
                for (size_t i = 0; i < numUVs; ++i)
                    mesh.t.push_back(math::Vector2f(uvs[i].x, uvs[i].y));
            }
            else
            {
                for (size_t i = 0; i < numTriangles; ++i)
                {
                    Polyline2D::IndexTriangle t = triangles[i];
                    t += Polyline2D::IndexTriangle(1, 1, 1);
                    triangle.v[0].v = t[0];
                    triangle.v[1].v = t[1];
                    triangle.v[2].v = t[2];
                    mesh.triangles.push_back(triangle);
                }
            }

            mesh.v.reserve(numVertices);
            for (size_t i = 0; i < numVertices; ++i)
                mesh.v.push_back(math::Vector2f(draw[i].x, draw[i].y));

            const math::Matrix4x4f& mvp = render->getTransform();
            CHECK_GL;
            if (soft)
            {
                p.softShader->bind();
                CHECK_GL;
                p.softShader->setUniform("transform.mvp", mvp);
                CHECK_GL;
                p.softShader->setUniform("color", color);
                CHECK_GL;
            }
            else
            {
                p.hardShader->bind();
                CHECK_GL;
                p.hardShader->setUniform("transform.mvp", mvp);
                CHECK_GL;
                p.hardShader->setUniform("color", color);
                CHECK_GL;
            }

            if (!p.vbo || (p.vbo && (p.vbo->getSize() != numTriangles * 3 ||
                                     p.vbo->getType() != vboType)))
            {
                p.vbo = gl::VBO::create(numTriangles * 3, vboType);
                CHECK_GL;
                p.vao.reset();
                CHECK_GL;
            }

            if (p.vbo)
            {
                p.vbo->copy(convert(mesh, vboType));
                CHECK_GL;
            }

            if (!p.vao && p.vbo)
            {
                p.vao = gl::VAO::create(p.vbo->getType(), p.vbo->getID());
                CHECK_GL;
            }

            if (p.vao && p.vbo)
            {
                p.vao->bind();
                CHECK_GL;
                p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                CHECK_GL;

                // #ifndef NDEBUG
                //             if (!soft)
                //             {
                //                 p.wireShader->bind();
                //                 CHECK_GL;
                //                 p.wireShader->setUniform("transform.mvp",
                //                 mvp); CHECK_GL;
                //                 p.wireShader->setUniform("color",
                //                 image::Color4f(1, 0, 0, 1)); CHECK_GL;
                //                 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //                 CHECK_GL;
                //                 p.vao->bind();
                //                 CHECK_GL;
                //                 p.vao->draw(GL_TRIANGLES, 0,
                //                 p.vbo->getSize()); CHECK_GL;
                //                 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                //                 CHECK_GL;
                //             }
                // #endif
            }
        }

        void Lines::drawLine(
            const std::shared_ptr<timeline::IRender>& render,
            const math::Vector2i& start, const math::Vector2i& end,
            const image::Color4f& color, const float width)
        {
            using namespace mrv::draw;

            std::vector< Point > line;
            line.push_back(Point(start.x, start.y));
            line.push_back(Point(end.x, end.y));

            drawLines(
                render, line, color, width, false,
                Polyline2D::JointStyle::MITER, Polyline2D::EndCapStyle::BUTT,
                false);
        }

        void Lines::drawPoints(
            const std::vector<math::Vector2f>& pnts,
            const image::Color4f& color, const int size)
        {
            TLRENDER_P();

            const size_t numPoints = pnts.size();
            const tl::gl::VBOType vboType = gl::VBOType::Pos2_F32;
            if (!p.vbo || (p.vbo && p.vbo->getSize() != numPoints))
            {
                p.vbo = gl::VBO::create(numPoints, vboType);
                p.vao.reset();
            }
            if (p.vbo)
            {
                std::vector<uint8_t> pts;
                pts.resize(numPoints * 2 * sizeof(float));
                memcpy(
                    pts.data(), pnts.data(), pnts.size() * 2 * sizeof(float));
                p.vbo->copy(pts);
            }

            if (!p.vao && p.vbo)
            {
                p.vao = gl::VAO::create(p.vbo->getType(), p.vbo->getID());
            }

            if (p.vao && p.vbo)
            {
                p.vao->bind();
                glPointSize(size);
                p.vao->draw(GL_POINTS, 0, p.vbo->getSize());
            }
        }

        void Lines::drawCircle(
            const std::shared_ptr<timeline::IRender>& render,
            const math::Vector2f& center, const float radius, const float width,
            const image::Color4f& color, const bool soft)
        {
            const int triangleAmount = 30;
            const double twoPi = math::pi * 2.0;

            draw::PointList verts;
            verts.reserve(triangleAmount);
            for (int i = 0; i < triangleAmount; ++i)
            {
                draw::Point pt(
                    center.x + (radius * cos(i * twoPi / triangleAmount)),
                    center.y + (radius * sin(i * twoPi / triangleAmount)));
                verts.push_back(pt);
            }

            drawLines(
                render, verts, color, width, soft,
                draw::Polyline2D::JointStyle::ROUND,
                draw::Polyline2D::EndCapStyle::JOINT);
        }

        void Lines::drawCursor(
            const std::shared_ptr<timeline::IRender>& render,
            const math::Vector2f& center, const float radius,
            const image::Color4f& color)
        {
            float lineWidth = 2.0;
            if (radius <= 2.0)
                lineWidth = 1.0f;
            drawCircle(render, center, radius, lineWidth, color, false);
            image::Color4f black(0.F, 0.F, 0.F, 1.F);
            if (radius > 2.0F)
                drawCircle(render, center, radius - 2.0F, 2.0, black, false);
        }

    } // namespace opengl
} // namespace mrv
