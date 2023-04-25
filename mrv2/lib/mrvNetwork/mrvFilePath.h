// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlCore/Path.h>

namespace tl
{
    namespace file
    {
        void to_json(nlohmann::json& j, const Path& value);

        void from_json(const nlohmann::json& j, Path& value);
    } // namespace file
} // namespace tl
