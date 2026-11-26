// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvBackend.h"
#include "mrvOS/mrvI8N.h"

#include "mrvGL/mrvGLJson.h"
#include "mrvWGPU/mrvWGPUShape.h"

namespace mrv
{
    using namespace tl::draw;

    std::shared_ptr< Shape > messageToShape(const nlohmann::json& json)
    {
        // \@wmrv2: annotations
        std::string type = json["type"];
        // if (type == "DrawPath")
        // {
        //     auto shape = std::make_shared< WGPUPathShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "ErasePath")
        // {
        //     auto shape = std::make_shared< WGPUErasePathShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Arrow")
        // {
        //     auto shape = std::make_shared< WGPUArrowShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Circle")
        // {
        //     auto shape = std::make_shared< WGPUCircleShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "FilledCircle")
        // {
        //     auto shape = std::make_shared< WGPUFilledCircleShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Polygon")
        // {
        //     auto shape = std::make_shared< WGPUPolygonShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "FilledPolygon")
        // {
        //     auto shape = std::make_shared< WGPUFilledPolygonShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Rectangle")
        // {
        //     auto shape = std::make_shared< WGPURectangleShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "FilledRectangle")
        // {
        //     auto shape = std::make_shared< WGPUFilledRectangleShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Note")
        // {
        //     auto shape = std::make_shared< draw::NoteShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if (type == "Link")
        // {
        //     auto shape = std::make_shared< WGPULinkShape >();
        //     json.get_to(*shape.get());
        //     return shape;
        // }
        // else if ( type == "Text" )
        // {
        //     auto shape = std::make_shared< WGPUTextShape >(fontSystem);
        //     json.get_to( *shape.get() );
        //     value.shapes.push_back(shape);
        // }
        std::string err = _("Could not convert message to shape: ");
        err += type;
        throw std::runtime_error(type);
    }

    nlohmann::json shapeToMessage(const std::shared_ptr< Shape > shape)
    {
        nlohmann::json msg;
        auto ptr = shape.get();

//         if (dynamic_cast< WGPULinkShape* >(ptr))
//         {
//             WGPULinkShape* p = reinterpret_cast< WGPULinkShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUFilledRectangleShape* >(ptr))
//         {
//             WGPUFilledRectangleShape* p =
//                 reinterpret_cast< WGPUFilledRectangleShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPURectangleShape* >(ptr))
//         {
//             WGPURectangleShape* p =
//                 reinterpret_cast< WGPURectangleShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUFilledCircleShape* >(ptr))
//         {
//             WGPUFilledCircleShape* p =
//                 reinterpret_cast< WGPUFilledCircleShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUCircleShape* >(ptr))
//         {
//             WGPUCircleShape* p = reinterpret_cast< WGPUCircleShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUFilledPolygonShape* >(ptr))
//         {
//             WGPUFilledPolygonShape* p =
//                 reinterpret_cast< WGPUFilledPolygonShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUPolygonShape* >(ptr))
//         {
//             WGPUPolygonShape* p = reinterpret_cast< WGPUPolygonShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUArrowShape* >(ptr))
//         {
//             WGPUArrowShape* p = reinterpret_cast< WGPUArrowShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUTextShape* >(ptr))
//         {
//             WGPUTextShape* p = reinterpret_cast< WGPUTextShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUErasePathShape* >(ptr))
//         {
//             WGPUErasePathShape* p = dynamic_cast< WGPUErasePathShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< WGPUPathShape* >(ptr))
//         {
//             WGPUPathShape* p = dynamic_cast< WGPUPathShape* >(ptr);
//             msg = *p;
//         }
//         else if (dynamic_cast< draw::NoteShape* >(ptr))
//         {
//             draw::NoteShape* p = dynamic_cast< draw::NoteShape* >(ptr);
//             msg = *p;
//         }
        return msg;
    }
}

namespace tl
{
    namespace draw
    {
        void to_json(nlohmann::json& json, const Annotation& value)
        {
            nlohmann::json shapes;
            for (const auto& shape : value.shapes)
            {
                shapes.push_back(mrv::shapeToMessage(shape));
            }
            json = nlohmann::json{
                {"all_frames", value.allFrames},
                {"time", value.time},
                {"shapes", shapes},
            };
        }

        void from_json(const nlohmann::json& json, Annotation& value)
        {
            json.at("all_frames").get_to(value.allFrames);
            json.at("time").get_to(value.time);
            const nlohmann::json& shapes = json["shapes"];
            for (auto& shape : shapes)
            {
                value.shapes.push_back(mrv::messageToShape(shape));
            }
        }

    } // namespace draw
} // namespace tl
