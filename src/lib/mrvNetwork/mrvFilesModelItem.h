// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <mrvApp/mrvFilesModel.h>

namespace mrv
{
    void to_json(nlohmann::json& j, const FilesModelItem& value);

    void from_json(const nlohmann::json& j, FilesModelItem& value);
} // namespace mrv
