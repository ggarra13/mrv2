// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.




// #include "mrvGL/mrvGLErrors.h"
#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkLines.h"

#include <tlVk/Mesh.h>
#include <tlVk/Util.h>
#include <tlVk/Shader.h>

#include <tlDraw/Polyline2D.h>

#include <tlCore/Matrix.h>
#include <tlCore/Mesh.h>
#include <tlCore/Vector.h>

#include <FL/Fl_Vk_Window.H>


namespace tl
{
    namespace timeline_vlk
    {
        extern std::string vertex2Source();
    } // namespace timeline_vlk

} // namespace tl

namespace mrv
{

    namespace vulkan
    {

        using namespace tl;

        struct Lines::Private
        {
            std::shared_ptr<vlk::Shader> softShader;
            std::shared_ptr<vlk::Shader> hardShader;
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;
        };

        Lines::Lines(Fl_Vk_Context& vulkan_ctx, VkRenderPass renderPass) :
            _p(new Private),
            ctx(vulkan_ctx),
            m_renderPass(renderPass)
        {
            TLRENDER_P();

            if (!p.softShader)
            {
                try
                {
                    const std::string& vertexSource =
                        tl::timeline_vlk::vertex2Source();
                    p.softShader = vlk::Shader::create(ctx,
                                                       vertexSource,
                                                       softFragmentSource());
                    math::Matrix4x4f mvp;
                    image::Color4f color(1.F, 1.F, 1.F);
                    p.softShader->createUniform("transform.mvp", mvp,
                                                vlk::kShaderVertex);

                    p.softShader->addPush("color", color, vlk::kShaderFragment);
                    p.hardShader = vlk::Shader::create(ctx,
                                                       vertexSource,
                                                       hardFragmentSource());
                    p.hardShader->createUniform("transform.mvp", mvp,
                                                vlk::kShaderVertex);

                    p.hardShader->addPush("color", color, vlk::kShaderFragment);
                }
                catch (const std::exception& e)
                {
                    throw e;
                }
            }

        }

        Lines::~Lines() {}

        void Lines::drawLines(
            const std::shared_ptr<timeline_vlk::Render>& render,
            const draw::PointList& pts, const image::Color4f& color,
            const float width, const bool soft,
            const draw::Polyline2D::JointStyle jointStyle,
            const draw::Polyline2D::EndCapStyle endStyle,
            const bool catmullRomSpline, const bool allowOverlap)
        {
            TLRENDER_P();
            using namespace tl::draw;

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

            tl::vlk::VBOType vboType = vlk::VBOType::Pos2_F32;
            geom::Triangle2 triangle;
            if (numUVs > 0)
            {
                vboType = vlk::VBOType::Pos2_F32_UV_U16;
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
            
            render->drawMesh("annotation", "rect", "mesh", renderPass(),
                             mesh, math::Vector2i(0, 0), color, true);
        }

        void Lines::drawLine(
            const std::shared_ptr<timeline_vlk::Render>& render,
            const math::Vector2i& start, const math::Vector2i& end,
            const image::Color4f& color, const float width)
        {
            using namespace tl::draw;

            std::vector< Point > line;
            line.push_back(Point(start.x, start.y));
            line.push_back(Point(end.x, end.y));

            drawLines(
                render, line, color, width, false,
                Polyline2D::JointStyle::MITER, Polyline2D::EndCapStyle::BUTT,
                false);
        }

        void Lines::drawPoints(
            const std::shared_ptr<timeline_vlk::Render>& render,
            const std::vector<math::Vector2f>& pnts,
            const image::Color4f& color, const int size)
        {
            TLRENDER_P();

            const size_t numPoints = pnts.size();
            const tl::vlk::VBOType vboType = vlk::VBOType::Pos2_F32;
            if (!p.vbo || (p.vbo && p.vbo->getSize() != numPoints))
            {
                p.vbo = vlk::VBO::create(numPoints, vboType);
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
                p.vao = vlk::VAO::create(ctx);
            }

            if (p.vao && p.vbo)
            {
                // p.vao->bind(frameIndex);
                // glPointSize(size);
                // p.vao->draw(GL_POINTS, 0, p.vbo->getSize());
            }
        }

        void Lines::drawCircle(
            const std::shared_ptr<timeline_vlk::Render>& render,
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
            const std::shared_ptr<timeline_vlk::Render>& render,
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

    } // namespace vulkan
} // namespace mrv
