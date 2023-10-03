#pragma once

#include "mrvDraw/Annotation.h"

#include "mrvNetwork/mrvMessage.h"

namespace mrv
{
    namespace draw
    {
        //! Translate a nlohmann::json message to a draw::Shape.
        std::shared_ptr< Shape > messageToShape(const Message&);

        //! Translate a tl::draw::Shape to a nlohmann::json message.
        Message shapeToMessage(const std::shared_ptr< Shape > shape);

        void to_json(nlohmann::json& json, const Annotation& value);

        void from_json(const nlohmann::json& json, Annotation& value);
    } // namespace draw

} // namespace mrv
