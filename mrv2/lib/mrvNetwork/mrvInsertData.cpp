// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <nlohmann/json.hpp>

#include "mrvNetwork/mrvInsertData.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const InsertData& value)
    {
        j["oldIndex"] = value.oldIndex;
        j["oldTrackIndex"] = value.oldTrackIndex;
        j["trackIndex"] = value.trackIndex;
        j["insertIndex"] = value.insertIndex;
    }

    void from_json(const nlohmann::json& j, InsertData& value)
    {
        j["oldIndex"].get_to(value.oldIndex);
        j["oldTrackIndex"].get_to(value.oldTrackIndex);
        j["trackIndex"].get_to(value.trackIndex);
        j["insertIndex"].get_to(value.insertIndex);
    }
} // namespace mrv
