// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvLUTOptions.h"

namespace tl
{
    namespace timeline
    {
        void to_json(nlohmann::json& j, const LUTOptions& value)
        {
            j["fileName"] = value.fileName;
            j["order"] = value.order;
        }

        void from_json(const nlohmann::json& j, LUTOptions& value)
        {
            j.at("fileName").get_to(value.fileName);
            j.at("order").get_to(value.order);
        }

    } // namespace timeline
} // namespace tl
