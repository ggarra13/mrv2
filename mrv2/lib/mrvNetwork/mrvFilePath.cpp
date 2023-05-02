// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCypher.h"

#include <iostream>
#include "mrvFilePath.h"

namespace tl
{
    namespace file
    {
        void to_json(nlohmann::json& j, const Path& value)
        {
            const std::string& plainText = value.get();
            std::string encodedText = mrv::encode_string(plainText);
            j["path"] = encodedText;
        }

        void from_json(const nlohmann::json& j, Path& value)
        {
            std::string encodedText;
            j.at("path").get_to(encodedText);
            std::string plainText = mrv::decode_string(encodedText);
            value = Path(plainText);
        }
    } // namespace file
} // namespace tl
