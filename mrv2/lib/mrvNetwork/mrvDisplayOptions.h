// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <nlohmann/json.hpp>

#include <tlTimeline/DisplayOptions.h>

namespace tl
{
    namespace timeline
    {
        void to_json(nlohmann::json& j, const Color& value);

        void from_json(const nlohmann::json& j, Color& value);

        void to_json(nlohmann::json& j, const Levels& value);

        void from_json(const nlohmann::json& j, Levels& value);

        void to_json(nlohmann::json& j, const EXRDisplay& value);

        void from_json(const nlohmann::json& j, EXRDisplay& value);

        void to_json(nlohmann::json& j, const DisplayOptions& value);

        void from_json(const nlohmann::json& j, DisplayOptions& value);
    } // namespace timeline
} // namespace tl
