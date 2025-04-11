// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play_qt
    {
        //! Messages tool.
        class MessagesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            MessagesTool(App*, QWidget* parent = nullptr);

            virtual ~MessagesTool();

        private:
            TLRENDER_PRIVATE();
        };

        //! Messages tool dock widget.
        class MessagesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            MessagesDockWidget(MessagesTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
