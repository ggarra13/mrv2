// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlPlay/ViewportModel.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! Background color widget.
        class BackgroundWidget : public QWidget
        {
            Q_OBJECT

        public:
            BackgroundWidget(App*, QWidget* parent = nullptr);

            virtual ~BackgroundWidget();

        private:
            void _optionsUpdate(const timeline::BackgroundOptions&);

            TLRENDER_PRIVATE();
        };

        //! View tool.
        class ViewTool : public IToolWidget
        {
            Q_OBJECT

        public:
            ViewTool(App*, QWidget* parent = nullptr);

            virtual ~ViewTool();

        private:
            TLRENDER_PRIVATE();
        };

        //! View tool dock widget.
        class ViewDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            ViewDockWidget(ViewTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
