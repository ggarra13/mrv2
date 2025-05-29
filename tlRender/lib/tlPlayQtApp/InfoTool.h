// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace io
    {
        struct Info;
    }

    namespace play_qt
    {
        class App;

        //! Information tool.
        class InfoTool : public IToolWidget
        {
            Q_OBJECT

        public:
            InfoTool(App*, QWidget* parent = nullptr);

            virtual ~InfoTool();

            void setInfo(const io::Info&);

        private:
            TLRENDER_PRIVATE();
        };

        //! Information tool dock widget.
        class InfoDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            InfoDockWidget(InfoTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
