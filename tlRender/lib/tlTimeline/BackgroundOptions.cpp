// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/BackgroundOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Background, "Transparent", "Solid", "Checkers", "Gradient");
        TLRENDER_ENUM_SERIALIZE_IMPL(Background);

        void to_json(nlohmann::json& json, const BackgroundOptions& in)
        {
            json = nlohmann::json{
                {"type", in.type},
                {"color0", in.color0},
                {"color1", in.color1},
                {"checkersSize", in.checkersSize},
            };
        }

        void from_json(const nlohmann::json& json, BackgroundOptions& out)
        {
            json.at("type").get_to(out.type);
            json.at("color0").get_to(out.color0);
            json.at("color1").get_to(out.color1);
            json.at("checkersSize").get_to(out.checkersSize);
        }
    } // namespace timeline
} // namespace tl
