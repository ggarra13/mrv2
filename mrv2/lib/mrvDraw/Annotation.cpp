// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>

#ifdef TLRENDER_GL
#    include "mrvGL/mrvGLUtil.h"
#endif

#include "Annotation.h"

namespace tl
{
    namespace draw
    {
        using namespace mrv;

        Annotation::Annotation(
            const otime::RationalTime& inTime, const bool inAllFrames)
        {
            time = inTime;
            allFrames = inAllFrames;
        }

        Annotation::~Annotation() {}

        bool Annotation::empty() const
        {
            return shapes.empty();
        }

        void Annotation::push_back(const std::shared_ptr< Shape >& shape)
        {
            shapes.push_back(shape);
            undo_shapes.clear();
        }

        void Annotation::remove(const std::shared_ptr< Shape >& shape)
        {
            auto it = std::find(shapes.begin(), shapes.end(), shape);
            if (it != shapes.end())
            {
                shapes.erase(it);
            }
        }

        std::shared_ptr< Shape > Annotation::lastShape() const
        {
            if (shapes.empty())
                return nullptr;
            return shapes.back();
        }

        void Annotation::undo()
        {
            if (shapes.empty())
                return;

            const auto& shape = shapes.back();
            undo_shapes.push_back(shape);
            shapes.pop_back();
        }

        void Annotation::redo()
        {
            if (undo_shapes.empty())
                return;

            const auto& shape = undo_shapes.back();
            shapes.push_back(shape);
            undo_shapes.pop_back();
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

        std::shared_ptr< Annotation >
        messageToAnnotation(const mrv::Message& json)
        {
            auto annotation = std::make_shared< Annotation >();
            json.get_to(*annotation.get());
            return annotation;
        }

    } // namespace draw
} // namespace tl
