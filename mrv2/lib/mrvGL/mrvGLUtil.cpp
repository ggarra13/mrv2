// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlGlad/gl.h>

#include <tlGL/Shader.h>
#include <tlGL/Mesh.h>

#include "mrvCore/mrvI8N.h"

#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLUtil.h"

namespace mrv
{
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

    namespace
    {
        const std::string vertexSource =
            "#version 330 core\n"
            "\n"
            "layout (location = 0) in vec3 position;\n"
            "\n"
            "uniform float pointSize;"
            "\n"
            "void main()\n"
            "{\n"
            "gl_Position = vec4(position.xyz, 1.0);\n"
            "gl_PointSize = pointSize;\n"
            "}\n"
            "\n";

        const std::string fragmentSource = "#version 330 core\n"
                                           "in  vec4 fColor;\n"
                                           "out vec4 color;\n"
                                           "void main()\n"
                                           "{\n"
                                           "    color = vec4(1,1,1,1);\n"
                                           "}";

        std::shared_ptr<tl::gl::Shader> shader = nullptr;
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;
    } // namespace

    void drawCursor(
        const math::Vector2i& center, const float size,
        const imaging::Color4f& color)
    {
        if (!shader)
        {
            shader = gl::Shader::create(vertexSource, fragmentSource);
        }
        shader->bind();

        if (!vbo)
        {
            vbo = gl::VBO::create(1, gl::VBOType::Pos2_F32);
        }

        if (vbo)
        {
            geom::TriangleMesh2 mesh;
            const int CIRCLE_SEGMENTS = 32;
            math::Vector2f v;
            mesh.v.push_back(v);
            for (int i = 0; i < CIRCLE_SEGMENTS; i++)
            {
                v.x = std::cos(2.0f * M_PI * i / CIRCLE_SEGMENTS);
                v.y = std::sin(2.0f * M_PI * i / CIRCLE_SEGMENTS);
                mesh.v.push_back(v);
            }

            geom::Triangle2 triangle;
            for (int i = 0; i < CIRCLE_SEGMENTS / 3; ++i)
            {
                triangle.v[0] = i + 1;
                triangle.v[1] = i + 2;
                triangle.v[2] = i + 3;
                mesh.triangles.emplace_back(triangle);
            }
            vbo->copy(convert(mesh, gl::VBOType::Pos2_F32));
        }

        if (!vao && vbo)
        {
            vao = gl::VAO::create(gl::VBOType::Pos2_F32, vbo->getID());
        }

        if (vao && vbo)
        {
            vao->bind();
            vao->draw(GL_LINES, 0, vbo->getSize());
        }
    }

} // namespace mrv
