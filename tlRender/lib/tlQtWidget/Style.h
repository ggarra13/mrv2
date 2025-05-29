// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <QPalette>
#include <QString>

namespace tl
{
    namespace qtwidget
    {
        //! Get a dark style color palette.
        QPalette darkStyle();

        //! Get a custom style sheet.
        QString styleSheet();
    } // namespace qtwidget
} // namespace tl
