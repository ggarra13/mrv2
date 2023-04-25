// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvCompareOptions.h"

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
            j.at("mode").get_to(value.mode);
            j.at("wipeCenter").get_to(value.wipeCenter);
            j.at("wipeRotation").get_to(value.wipeRotation);
            j.at("overlay").get_to(value.overlay);
        }
    } // namespace timeline
} // namespace tl
