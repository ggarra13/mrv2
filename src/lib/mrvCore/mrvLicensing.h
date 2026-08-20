// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <array>
#include <string>
#include <vector>

namespace mrv
{

    //! License Status.
    enum class License
    {
        kValid,
        kInvalid,
        kExpired,
    };

    //! License Type.
    enum class LicenseType
    {
        kDemo,
        kNodeLocked,
        kFloating,

        Count,
        First = kDemo
    };
    TLRENDER_ENUM(LicenseType);
    TLRENDER_ENUM_SERIALIZE(LicenseType);


    std::vector<std::string> get_machine_ids();

    bool release_license();
    License validate_license(std::string& expiration_date);
    License license_beat();

    /**
     * Request a temporary webrtc ticket for Pro/Pro+ plans.
     *
     * @return string with the query.
     */
    std::string request_webrtc_ticket();
}
