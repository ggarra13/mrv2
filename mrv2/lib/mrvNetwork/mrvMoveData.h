// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

namespace mrv
{
    //! Move data for mrv2 Network connections.
    struct MoveData
    {
        int fromTrack = 0;
        int fromIndex = 0;
        int toTrack = 0;
        int toIndex = 0;
    };

    void to_json(nlohmann::json& j, const MoveData& value);

    void from_json(const nlohmann::json& j, MoveData& value);
} // namespace mrv
