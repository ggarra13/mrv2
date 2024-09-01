// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlTimeline/LUTOptions.h>

namespace tl
{
    namespace timeline
    {
        void to_json(nlohmann::json& j, const LUTOptions& value);

        void from_json(const nlohmann::json& j, LUTOptions& value);
    } // namespace timeline
} // namespace tl
