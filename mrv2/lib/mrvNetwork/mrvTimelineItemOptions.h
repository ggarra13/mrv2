
#pragma once

#include <tlTimelineUI/IItem.h>

namespace tl
{
    namespace timelineui
    {
        void to_json(nlohmann::json& j, const ItemOptions& value);

        void from_json(const nlohmann::json& j, ItemOptions& value);
    }; // namespace timelineui

} // namespace tl
