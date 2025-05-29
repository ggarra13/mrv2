// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>

#include <QMainWindow>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            class MainWindow : public QMainWindow
            {
                Q_OBJECT

            public:
                MainWindow(const std::shared_ptr<system::Context>&);
            };
        } // namespace widgets_qtwidget
    } // namespace examples
} // namespace tl
