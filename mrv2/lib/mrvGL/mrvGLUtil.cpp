// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

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

} // namespace mrv
