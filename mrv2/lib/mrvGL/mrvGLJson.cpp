
#include "mrvCore/mrvI8N.h"

#include "mrvGL/mrvGLJson.h"
#include "mrvGL/mrvGLShape.h"

namespace mrv
{
    namespace draw
    {
        std::shared_ptr< Shape > messageToShape(const Message& json)
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

        Message shapeToMessage(const std::shared_ptr< Shape > shape)
        {
            Message msg;
            auto ptr = shape.get();

            if (dynamic_cast< GLRectangleShape* >(ptr))
            {
                GLRectangleShape* p =
                    reinterpret_cast< GLRectangleShape* >(ptr);
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

        void to_json(nlohmann::json& json, const Annotation& value)
        {
            nlohmann::json shapes;
            for (const auto& shape : value.shapes)
            {
                shapes.push_back(shapeToMessage(shape));
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
                value.shapes.push_back(messageToShape(shape));
            }
        }
    } // namespace draw
} // namespace mrv
