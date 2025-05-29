// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"

#include <tlQt/ContextObject.h>

#include <QApplication>

namespace tl
{
    namespace examples
    {
        //! Example Qt widgets application.
        namespace widgets_qtwidget
        {
            class App : public QApplication
            {
                Q_OBJECT

            public:
                App(int& argc, char** argv,
                    const std::shared_ptr<system::Context>&);

            private:
                QScopedPointer<qt::ContextObject> _contextObject;
                QScopedPointer<MainWindow> _mainWindow;
            };
        } // namespace widgets_qtwidget
    } // namespace examples
} // namespace tl
