// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvI8N.h"

#include "mrvVk/mrvVkJson.h"
#include "mrvVk/mrvVkShape.h"

namespace mrv
{
    using namespace tl::draw;
    
#if defined(VULKAN_BACKEND)
    std::shared_ptr< Shape > messageToShape(const nlohmann::json& json)
    {
        std::string type = json["type"];
        if (type == "DrawPath")
        {
            auto shape = std::make_shared< VKPathShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "ErasePath")
        {
            auto shape = std::make_shared< VKErasePathShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Arrow")
        {
            auto shape = std::make_shared< VKArrowShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Circle")
        {
            auto shape = std::make_shared< VKCircleShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "FilledCircle")
        {
            auto shape = std::make_shared< VKFilledCircleShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Polygon")
        {
            auto shape = std::make_shared< VKPolygonShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "FilledPolygon")
        {
            auto shape = std::make_shared< VKFilledPolygonShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "Rectangle")
        {
            auto shape = std::make_shared< VKRectangleShape >();
            json.get_to(*shape.get());
            return shape;
        }
        else if (type == "FilledRectangle")
        {
            auto shape = std::make_shared< VKFilledRectangleShape >();
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
        //     auto shape = std::make_shared< VKTextShape >(fontSystem);
        //     json.get_to( *shape.get() );
        //     value.shapes.push_back(shape);
        // }
#ifdef USE_OPENVK2
        else if (type == "VK2Text")
        {
            auto shape = std::make_shared< VK2TextShape >();
            json.get_to(*shape.get());
            return shape;
        }
#endif
        std::string err = _("Could not convert message to shape: ");
        err += type;
        throw std::runtime_error(type);
    }

    nlohmann::json shapeToMessage(const std::shared_ptr< Shape > shape)
    {
        nlohmann::json msg;
        auto ptr = shape.get();

        if (dynamic_cast< VKFilledRectangleShape* >(ptr))
        {
            VKFilledRectangleShape* p =
                reinterpret_cast< VKFilledRectangleShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKRectangleShape* >(ptr))
        {
            VKRectangleShape* p =
                reinterpret_cast< VKRectangleShape* >(ptr);
            msg = *p;
        }
#ifdef USE_OPENVK2
        else if (dynamic_cast< VK2TextShape* >(ptr))
        {
            VK2TextShape* p = reinterpret_cast< VK2TextShape* >(ptr);
            msg = *p;
        }
#endif
        else if (dynamic_cast< VKFilledCircleShape* >(ptr))
        {
            VKFilledCircleShape* p =
                reinterpret_cast< VKFilledCircleShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKCircleShape* >(ptr))
        {
            VKCircleShape* p = reinterpret_cast< VKCircleShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKFilledPolygonShape* >(ptr))
        {
            VKFilledPolygonShape* p =
                reinterpret_cast< VKFilledPolygonShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKPolygonShape* >(ptr))
        {
            VKPolygonShape* p = reinterpret_cast< VKPolygonShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKArrowShape* >(ptr))
        {
            VKArrowShape* p = reinterpret_cast< VKArrowShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKTextShape* >(ptr))
        {
            VKTextShape* p = reinterpret_cast< VKTextShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKErasePathShape* >(ptr))
        {
            VKErasePathShape* p = dynamic_cast< VKErasePathShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< VKPathShape* >(ptr))
        {
            VKPathShape* p = dynamic_cast< VKPathShape* >(ptr);
            msg = *p;
        }
        else if (dynamic_cast< draw::NoteShape* >(ptr))
        {
            draw::NoteShape* p = dynamic_cast< draw::NoteShape* >(ptr);
            msg = *p;
        }
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
            if (json.contains("time"))
            {
                json.at("time").get_to(value.time);
            }
            else
            {
                int64_t frame;
                json.at("frame").get_to(frame);
                value.time = otime::RationalTime(frame, 24.0);
            }
            const nlohmann::json& shapes = json["shapes"];
            for (auto& shape : shapes)
            {
                value.shapes.push_back(mrv::messageToShape(shape));
            }
        }
#endif // VULKAN_BACKEND
        
    } // namespace draw
} // namespace tl

