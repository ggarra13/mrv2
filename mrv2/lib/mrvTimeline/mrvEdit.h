// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentime/rationalTime.h>

#include <opentimelineio/composition.h>
#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timelineui
    {
        // Slice an item.
        //
        // | A | B | -> |A|A| B |
        //   ^
        otio::SerializableObject::Retainer<otio::Timeline> slice(
            const otio::Timeline*, const otio::Item*,
            const otime::RationalTime&);

        // Remove an item, replacing it with a gap.
        //
        // | A | B | -> | G | B |
        //   ^
        otio::SerializableObject::Retainer<otio::Timeline>
        remove(const otio::Timeline*, const otio::Item*);
    } // namespace timelineui
} // namespace tl
