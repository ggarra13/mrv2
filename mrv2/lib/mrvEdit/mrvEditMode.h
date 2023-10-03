// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlCore/Util.h>

namespace mrv
{
    enum class EditMode {
        kNone,
        kTimeline,
        kSaved,
        kFull,

        Count,
        First = kNone
    };

    TLRENDER_ENUM(EditMode);
    TLRENDER_ENUM_SERIALIZE(EditMode);

} // namespace mrv
