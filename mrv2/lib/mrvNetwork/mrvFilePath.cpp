// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFilePath.h"

namespace tl
{
    namespace file
    {
        void to_json(nlohmann::json& j, const Path& value)
        {
            j["path"] = value.get();
        }

        void from_json(const nlohmann::json& j, Path& value)
        {
            std::string tmp;
            j.at("path").get_to(tmp);
            value = Path(tmp);
        }
    } // namespace file
} // namespace tl
