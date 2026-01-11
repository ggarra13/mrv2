// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvOptions/mrvCompareOptions.h"

namespace tl
{
    namespace timeline
    {

        void to_json(nlohmann::json& j, const CompareOptions& value)
        {
            nlohmann::json wipeCenter(value.wipeCenter);
            j["mode"] = value.mode;
            j["wipeCenter"] = wipeCenter;
            j["wipeRotation"] = value.wipeRotation;
            j["overlay"] = value.overlay;
        }

        void from_json(const nlohmann::json& j, CompareOptions& value)
        {

            if (j["mode"].type() == nlohmann::json::value_t::string)
            {
                j.at("mode").get_to(value.mode);
            }
            else
            {
                int v;
                j.at("mode").get_to(v);
                value.mode = static_cast<timeline::CompareMode>(v);
            }
            j.at("wipeCenter").get_to(value.wipeCenter);
            j.at("wipeRotation").get_to(value.wipeRotation);
            j.at("overlay").get_to(value.overlay);
        }
    } // namespace timeline
} // namespace tl
