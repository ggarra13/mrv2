// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include <nlohmann/json.hpp>

#include <tlCore/Util.h>

namespace mrv
{

    typedef nlohmann::json Message;

    // Local-only metadata tag, added when a message arrives over
    // WebRTC so downstream code (CommandInterpreter, syncMedia) knows
    // which mesh peer sent it. Never present on outgoing/TCP messages.
    // Leading underscore + prefix to avoid any collision with real
    // protocol keys ("command", "value", etc.).
    constexpr const char* kLocalPeerIdKey = "_mrvLocalPeerId";

} // namespace mrv
