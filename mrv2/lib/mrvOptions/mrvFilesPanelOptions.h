// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

namespace mrv
{
    /**
     * Struct holding the definitions for Environment Map Options.
     *
     */
    struct FilesPanelOptions
    {
        bool filterEDL = false;

        bool operator==(const FilesPanelOptions& b) const;
        bool operator!=(const FilesPanelOptions& b) const;
    };

    void to_json(nlohmann::json& j, const FilesPanelOptions& value);

    void from_json(const nlohmann::json& j, FilesPanelOptions& value);

} // namespace mrv

#include "mrvOptions/mrvFilesPanelOptionsInline.h"
