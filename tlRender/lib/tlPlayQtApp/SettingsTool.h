// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! Settings tool.
        class SettingsTool : public IToolWidget
        {
            Q_OBJECT

        public:
            SettingsTool(App*, QWidget* parent = nullptr);
        };

        //! Settings tool dock widget.
        class SettingsDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            SettingsDockWidget(SettingsTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
