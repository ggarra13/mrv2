// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Vk_Window.H>

#include <tlCore/Mesh.h>

#include <tlVk/Mesh.h>
#include <tlVk/Util.h>
#include <tlVk/Shader.h>

// #include "mrvGL/mrvGLErrors.h"
#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkLines.h"


namespace tl
{
    namespace timeline_vk
    {
        extern std::string vertexSource();
    } // namespace timeline_vk

} // namespace tl

namespace mrv
{

    namespace vulkan
    {

        using namespace tl;

        struct Lines::Private
        {
            std::shared_ptr<vk::Shader> softShader = nullptr;
            std::shared_ptr<vk::Shader> hardShader = nullptr;
            std::shared_ptr<vk::VBO> vbo;
            std::shared_ptr<vk::VAO> vao;
        };

        Lines::Lines() :
            _p(new Private)
        {
        }

        Lines::~Lines() {}

        void Lines::drawLines(
            Fl_Vk_Context& ctx,
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
                        tl::timeline_vk::vertexSource();
                    // p.softShader = vk::Shader::create(
                    //     vertexSource, mrv::softFragmentSource());
                    // p.hardShader = vk::Shader::create(
                    //     vertexSource, mrv::hardFragmentSource());
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

            tl::vk::VBOType vboType = vk::VBOType::Pos2_F32;
            geom::Triangle2 triangle;
            if (numUVs > 0)
            {
                vboType = vk::VBOType::Pos2_F32_UV_U16;
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
            
            if (soft)
            {
                p.softShader->bind();
                
                p.softShader->setUniform("transform.mvp", mvp);
                
                p.softShader->setUniform("color", color);
                
            }
            else
            {
                p.hardShader->bind();
                
                p.hardShader->setUniform("transform.mvp", mvp);
                
                p.hardShader->setUniform("color", color);
                
            }

            if (!p.vbo || (p.vbo && (p.vbo->getSize() != numTriangles * 3 ||
                                     p.vbo->getType() != vboType)))
            {
                p.vbo = vk::VBO::create(numTriangles * 3, vboType);
                
                p.vao.reset();
                
            }

            if (p.vbo)
            {
                p.vbo->copy(convert(mesh, vboType));
                
            }

            if (!p.vao && p.vbo)
            {
                p.vao = vk::VAO::create(ctx, p.vbo->getType(), p.vbo->getID());
                
            }

            if (p.vao && p.vbo)
            {
                p.vao->bind();
                
                // p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                

                // #ifndef NDEBUG
                //             if (!soft)
                //             {
                //                 p.wireShader->bind();
                //                 
                //                 p.wireShader->setUniform("transform.mvp",
                //                 mvp); 
                //                 p.wireShader->setUniform("color",
                //                 image::Color4f(1, 0, 0, 1)); 
                //                 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //                 
                //                 p.vao->bind();
                //                 
                //                 p.vao->draw(GL_TRIANGLES, 0,
                //                 p.vbo->getSize()); 
                //                 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                //                 
                //             }
                // #endif
            }
        }

        void Lines::drawLine(
            Fl_Vk_Context& ctx,
            const std::shared_ptr<timeline::IRender>& render,
            const math::Vector2i& start, const math::Vector2i& end,
            const image::Color4f& color, const float width)
        {
            using namespace mrv::draw;

            std::vector< Point > line;
            line.push_back(Point(start.x, start.y));
            line.push_back(Point(end.x, end.y));

            drawLines(
                ctx, render, line, color, width, false,
                Polyline2D::JointStyle::MITER, Polyline2D::EndCapStyle::BUTT,
                false);
        }

        void Lines::drawPoints(
            Fl_Vk_Context& ctx,
            const std::vector<math::Vector2f>& pnts,
            const image::Color4f& color, const int size)
        {
            TLRENDER_P();

            const size_t numPoints = pnts.size();
            const tl::vk::VBOType vboType = vk::VBOType::Pos2_F32;
            if (!p.vbo || (p.vbo && p.vbo->getSize() != numPoints))
            {
                p.vbo = vk::VBO::create(numPoints, vboType);
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
                p.vao = vk::VAO::create(ctx, p.vbo->getType(), p.vbo->getID());
            }

            if (p.vao && p.vbo)
            {
                p.vao->bind();
                // glPointSize(size);
                // p.vao->draw(GL_POINTS, 0, p.vbo->getSize());
            }
        }

        void Lines::drawCircle(
            Fl_Vk_Context& ctx,
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
                ctx, render, verts, color, width, soft,
                draw::Polyline2D::JointStyle::ROUND,
                draw::Polyline2D::EndCapStyle::JOINT);
        }

        void Lines::drawCursor(
            Fl_Vk_Context& ctx,
            const std::shared_ptr<timeline::IRender>& render,
            const math::Vector2f& center, const float radius,
            const image::Color4f& color)
        {
            float lineWidth = 2.0;
            if (radius <= 2.0)
                lineWidth = 1.0f;
            drawCircle(ctx, render, center, radius, lineWidth, color, false);
            image::Color4f black(0.F, 0.F, 0.F, 1.F);
            if (radius > 2.0F)
                drawCircle(ctx, render, center, radius - 2.0F, 2.0, black, false);
        }

    } // namespace opengl
} // namespace mrv
