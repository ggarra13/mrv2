// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

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

    
    std::string get_machine_id();

    bool release_license();
    License validate_license(std::string& expiration_date);
    License license_beat();
}
