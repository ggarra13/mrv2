// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <tlCore/StringFormat.h>

#include <tlGlad/gl.h>

#include <tlGL/Shader.h>
#include <tlGL/Mesh.h>
#include <tlGL/RenderPrivate.h>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

#include "mrvGL/mrvGLShaders.h"
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLUtil.h"

namespace
{
    const char* kModule = "glutil";
}

namespace mrv
{
    namespace
    {
        std::shared_ptr<tl::gl::Shader> softShader = nullptr;
        std::shared_ptr<tl::gl::Shader> hardShader = nullptr;
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;
    } // namespace

    std::shared_ptr< tl::draw::Shape > messageToShape(const Message& json)
    {
        std::string type = json["type"];
        if (type == "DrawPath")
        {
            auto shape = std::make_shared< GLPathShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "ErasePath")
        {
            auto shape = std::make_shared< GLErasePathShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Arrow")
        {
            auto shape = std::make_shared< GLArrowShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Circle")
        {
            auto shape = std::make_shared< GLCircleShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Rectangle")
        {
            auto shape = std::make_shared< GLRectangleShape >();
            json.get_to(*shape.get());
            return shape;
        }
        // else if ( type == "Text" )
        // {
        //     auto shape = std::make_shared< GLTextShape >(fontSystem);
        //     json.get_to( *shape.get() );
        //     value.shapes.push_back(shape);
        // }
#ifdef USE_OPENGL2
        else if (type == "GL2Text")
        {
            auto shape = std::make_shared< GL2TextShape >();
            json.get_to(*shape.get());
            return shape;
        }
#endif
        std::string err = _("Could not convert message to shape: ");
        err += type;
        throw std::runtime_error(type);
    }

    Message shapeToMessage(const std::shared_ptr< tl::draw::Shape > shape)
    {
        Message msg;
        auto ptr = shape.get();

#ifdef USE_OPENGL2
        if (dynamic_cast< GL2TextShape* >(ptr))
        {
            GL2TextShape* p = reinterpret_cast< GL2TextShape* >(ptr);
            msg = *p;
        }
#endif
        else if (dynamic_cast< GLRectangleShape* >(ptr))
        {
            GLRectangleShape* p = reinterpret_cast< GLRectangleShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< GLCircleShape* >(ptr))
        {
            GLCircleShape* p = reinterpret_cast< GLCircleShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< GLArrowShape* >(ptr))
        {
            GLArrowShape* p = reinterpret_cast< GLArrowShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< GLTextShape* >(ptr))
        {
            GLTextShape* p = reinterpret_cast< GLTextShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< GLErasePathShape* >(ptr))
        {
            GLErasePathShape* p = dynamic_cast< GLErasePathShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< GLPathShape* >(ptr))
        {
            GLPathShape* p = dynamic_cast< GLPathShape* >(ptr);
            msg = *p;
        }
        return msg;
    }

    void drawCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius, const float width,
        const imaging::Color4f& color, const bool soft)
    {
        const int triangleAmount = 40;
        const double twoPi = math::pi * 2.0;

        tl::draw::PointList verts;
        verts.reserve(triangleAmount);
        for (int i = 0; i < triangleAmount; ++i)
        {
            tl::draw::Point pt(
                center.x + (radius * cos(i * twoPi / triangleAmount)),
                center.y + (radius * sin(i * twoPi / triangleAmount)));
            verts.push_back(pt);
        }

        drawLines(
            render, verts, color, width, soft,
            tl::draw::Polyline2D::JointStyle::MITER,
            tl::draw::Polyline2D::EndCapStyle::JOINT);
    }

    void drawCursor(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const imaging::Color4f& color)
    {
#if 1
        drawFilledCircle(render, center, radius, color, false);
#else
        drawCircle(render, center, radius, 2.0, color, false);
        imaging::Color4f black(0.F, 0.F, 0.F, 1.F);
        if (radius > 2.0F)
            drawCircle(render, center, radius - 2.0F, 2.0, black, false);
#endif
    }

    void drawFilledCircle(
        const std::shared_ptr<timeline::IRender>& render,
        const math::Vector2i& center, const float radius,
        const imaging::Color4f& color, const bool soft)
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

    void drawLines(
        const std::shared_ptr<timeline::IRender>& render,
        const tl::draw::PointList& pts, const imaging::Color4f& color,
        const int width, const bool soft,
        const tl::draw::Polyline2D::JointStyle jointStyle,
        const tl::draw::Polyline2D::EndCapStyle endStyle, const bool doSmooth,
        const bool allowOverlap)
    {

        if (!softShader)
        {
            try
            {
                const std::string& vertexSource = tl::gl::vertexSource();
                softShader =
                    gl::Shader::create(vertexSource, softFragmentSource());
                hardShader =
                    gl::Shader::create(vertexSource, hardFragmentSource());
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }

        using namespace tl::draw;
        PointList draw;
        PointList uvs;

        Polyline2D::create(
            draw, uvs, pts, width, jointStyle, endStyle, doSmooth,
            allowOverlap);

        geom::TriangleMesh2 mesh;
        size_t numVertices = draw.size();

        mesh.triangles.reserve(numVertices / 3);

        geom::Triangle2 triangle;
        for (size_t v = 0; v < numVertices; v += 3)
        {
            triangle.v[0].v = v + 1;
            triangle.v[1].v = v + 2;
            triangle.v[2].v = v + 3;
            triangle.v[0].t = v + 1;
            triangle.v[1].t = v + 2;
            triangle.v[2].t = v + 3;
            mesh.triangles.emplace_back(triangle);
        }

        mesh.v.reserve(numVertices);
        for (size_t i = 0; i < numVertices; ++i)
            mesh.v.emplace_back(math::Vector2f(draw[i].x, draw[i].y));

        size_t numUVs = uvs.size();
        assert(numUVs == numVertices);

        mesh.t.reserve(numUVs);
        for (size_t i = 0; i < numUVs; ++i)
            mesh.t.emplace_back(math::Vector2f(uvs[i].x, uvs[i].y));

#ifndef NDEBUG
        std::cerr << "numTriangles=" << mesh.triangles.size() << std::endl;
        for (size_t i = 0; i < numUVs; ++i)
        {
            std::cerr << i << ")\t" << draw[i].x << "\t" << draw[i].y
                      << "\tt=" << uvs[i].x << std::endl;
        }
        std::cerr
            << "------------------------------------------------------------"
            << std::endl;
#endif

        const math::Matrix4x4f& mvp = render->getTransform();
        if (soft)
        {
            softShader->bind();
            softShader->setUniform("transform.mvp", mvp);
            softShader->setUniform("color", color);
        }
        else
        {
            hardShader->bind();
            hardShader->setUniform("transform.mvp", mvp);
            hardShader->setUniform("color", color);
        }

        if (!vbo || (vbo && vbo->getSize() != numVertices))
        {
            vbo = gl::VBO::create(numVertices, gl::VBOType::Pos2_F32_UV_U16);
            vao.reset();
        }
        if (vbo)
        {
            vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
        }

        if (!vao && vbo)
        {
            vao = gl::VAO::create(vbo->getType(), vbo->getID());
        }

        if (vao && vbo)
        {
            vao->bind();
            vao->draw(GL_TRIANGLES, 0, vbo->getSize());
        }
    }

} // namespace mrv
