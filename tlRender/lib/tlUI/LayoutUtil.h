// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetOptions.h>

#include <tlCore/Box.h>

namespace tl
{
    namespace ui
    {
        //! Align within the given box.
        math::Box2i align(
            const math::Box2i& box, const math::Size2i& sizeHint,
            Stretch hStretch, Stretch vStretch, HAlign hAlign, VAlign vAlign);

        //! Get a format string for the given number.
        std::string format(int);

        //! Get a format string for the given number.
        std::string format(float, int precision);
    } // namespace ui
} // namespace tl
