// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <nlohmann/json.hpp>

#include <tlTimeline/CompareOptions.h>

namespace tl
{
    namespace timeline
    {
        void to_json(nlohmann::json& j, const CompareOptions& value);

        void from_json(const nlohmann::json& j, CompareOptions& value);
    } // namespace timeline
} // namespace tl
