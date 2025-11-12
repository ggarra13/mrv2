// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/Edit.h>

namespace mrv
{
    enum class ActionMode {
        kScrub,
        kSelection,
        kDraw,
        kErase,
        kPolygon,
        kCircle,
        kRectangle,
        kArrow,
        kText,
        kVoice,
        kLink,
        kRotate,
        kFilledCircle,
        kFilledRectangle,
        kFilledPolygon,
    };

}
