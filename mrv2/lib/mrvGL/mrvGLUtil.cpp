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
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLUtil.h"

namespace
{
    const char* kModule = "glutil";
}

namespace mrv
{

    std::shared_ptr< draw::Shape > messageToShape(const Message& json)
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
        else if (type == "Note")
        {
            auto shape = std::make_shared< draw::NoteShape >();
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

    Message shapeToMessage(const std::shared_ptr< draw::Shape > shape)
    {
        Message msg;
        auto ptr = shape.get();

        if (dynamic_cast< GLRectangleShape* >(ptr))
        {
            GLRectangleShape* p = reinterpret_cast< GLRectangleShape* >(ptr);
            msg = *p;
        }
#ifdef USE_OPENGL2
        else if (dynamic_cast< GL2TextShape* >(ptr))
        {
            GL2TextShape* p = reinterpret_cast< GL2TextShape* >(ptr);
            msg = *p;
        }
#endif
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
        else if (dynamic_cast< draw::NoteShape* >(ptr))
        {
            draw::NoteShape* p = dynamic_cast< draw::NoteShape* >(ptr);
            msg = *p;
        }
        return msg;
    }

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
