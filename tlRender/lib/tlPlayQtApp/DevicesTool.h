// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Devices tool.
        class DevicesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            DevicesTool(App*, QWidget* parent = nullptr);

            virtual ~DevicesTool();

        private:
            TLRENDER_PRIVATE();
        };

        //! Devices tool dock widget.
        class DevicesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            DevicesDockWidget(DevicesTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
