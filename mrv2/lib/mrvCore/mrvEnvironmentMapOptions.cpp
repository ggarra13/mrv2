// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvEnvironmentMapOptions.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const EnvironmentMapOptions& value)
    {
        j["type"] = value.type;
        j["horizontalAperture"] = value.horizontalAperture;
        j["verticalAperture"] = value.verticalAperture;
        j["focalLength"] = value.focalLength;
        j["rotateX"] = value.rotateX;
        j["rotateY"] = value.rotateY;
        j["subdivisionX"] = value.subdivisionX;
        j["subdivisionY"] = value.subdivisionY;
        j["spin"] = value.spin;
    }

    void from_json(const nlohmann::json& j, EnvironmentMapOptions& value)
    {
        j.at("type").get_to(value.type);
        j.at("horizontalAperture").get_to(value.horizontalAperture);
        j.at("verticalAperture").get_to(value.verticalAperture);
        j.at("focalLength").get_to(value.focalLength);
        j.at("rotateX").get_to(value.rotateX);
        j.at("rotateY").get_to(value.rotateY);
        j.at("subdivisionX").get_to(value.subdivisionX);
        j.at("subdivisionY").get_to(value.subdivisionY);
        j.at("spin").get_to(value.spin);
    }
} // namespace mrv
