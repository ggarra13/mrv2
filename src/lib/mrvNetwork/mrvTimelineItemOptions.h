// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvBackend.h"

#include <tlTimelineUI/IItem.h>

namespace tl
{
#ifdef OPENGL_BACKEND
    namespace timelineui
    {
        void to_json(nlohmann::json& j, const ItemOptions& value);

        void from_json(const nlohmann::json& j, ItemOptions& value);
        
        void to_json(nlohmann::json& j, const DisplayOptions& value);

        void from_json(const nlohmann::json& j, DisplayOptions& value);
    }; // namespace timelineui
#endif

#ifdef VULKAN_BACKEND
    namespace timelineui_vk
    {
        void to_json(nlohmann::json& j, const ItemOptions& value);

        void from_json(const nlohmann::json& j, ItemOptions& value);
        
        void to_json(nlohmann::json& j, const DisplayOptions& value);

        void from_json(const nlohmann::json& j, DisplayOptions& value);
    }; // namespace timelineui
#endif

} // namespace tl
