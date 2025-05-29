// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

namespace tl
{
    namespace ui
    {
        //! Orientation.
        enum class Orientation { Horizontal, Vertical };

        //! Layout stretch.
        enum class Stretch { Fixed, Expanding };

        //! Horizontal alignment.
        enum class HAlign { Left, Center, Right };

        //! Vertical alignment.
        enum class VAlign { Top, Center, Bottom };

        //! Updates.
        enum Update { kNone = 0, Size = 1, Draw = 2 };
    } // namespace ui
} // namespace tl
