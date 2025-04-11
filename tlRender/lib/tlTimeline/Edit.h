// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Copy the given timeline.
        otio::SerializableObject::Retainer<otio::Timeline>
        copy(const otio::SerializableObject::Retainer<otio::Timeline>&);

        //! Move items data.
        struct MoveData
        {
            int fromTrack = 0;
            int fromIndex = 0;
            int fromOtioIndex = 0;
            int toTrack = 0;
            int toIndex = 0;
            int toOtioIndex = 0;
        };

        //! Move items in the timeline.
        otio::SerializableObject::Retainer<otio::Timeline> move(
            const otio::SerializableObject::Retainer<otio::Timeline>&,
            const std::vector<MoveData>&);
    } // namespace timeline
} // namespace tl
