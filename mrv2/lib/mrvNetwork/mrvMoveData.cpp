// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <nlohmann/json.hpp>

#include "mrvNetwork/mrvMoveData.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const MoveData& value)
    {
        j["fromTrack"] = value.fromTrack;
        j["fromIndex"] = value.fromIndex;
        j["trackIndex"] = value.toTrack;
        j["toIndex"] = value.toIndex;
    }

    void from_json(const nlohmann::json& j, MoveData& value)
    {
        j["fromTrack"].get_to(value.fromTrack);
        j["fromIndex"].get_to(value.fromIndex);
        j["toTrace"].get_to(value.toTrack);
        j["toIndex"].get_to(value.toIndex);
    }
} // namespace mrv
