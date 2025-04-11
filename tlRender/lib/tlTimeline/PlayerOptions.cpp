// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(Playback, "Stop", "Forward", "Reverse");
        TLRENDER_ENUM_SERIALIZE_IMPL(Playback);

        TLRENDER_ENUM_IMPL(TimerMode, "System", "Audio");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimerMode);
    } // namespace timeline
} // namespace tl
