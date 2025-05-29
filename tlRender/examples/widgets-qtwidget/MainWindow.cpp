// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "FloatEditSlider.h"
#include "IntEditSlider.h"

#include <QTabWidget>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            MainWindow::MainWindow(
                const std::shared_ptr<system::Context>& context)
            {
                auto tabWidget = new QTabWidget;
                tabWidget->addTab(new FloatEditSlider, "FloatEditSlider");
                tabWidget->addTab(new IntEditSlider, "IntEditSlider");
                setCentralWidget(tabWidget);
            }
        } // namespace widgets_qtwidget
    } // namespace examples
} // namespace tl
