// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvFilesPanelOptions.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const FilesPanelOptions& value)
    {
        j["filterEDL"] = value.filterEDL;
    }

    void from_json(const nlohmann::json& j, FilesPanelOptions& value)
    {
        j.at("filterEDL").get_to(value.filterEDL);
    }
} // namespace mrv
