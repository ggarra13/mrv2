// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvStereo3DOptions.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const Stereo3DOptions& value)
    {
        j["type"] = value.type;
        j["eyeSeparation"] = value.eyeSeparation;
    }

    void from_json(const nlohmann::json& j, Stereo3DOptions& value)
    {
        j.at("type").get_to(value.type);
        j.at("eyeSeparation").get_to(value.eyeSeparation);
    }
} // namespace mrv
