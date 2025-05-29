// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlUI/FileBrowser.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            void App::_init(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                ui_app::App::_init(
                    argv, context, "widgets", "Example widgets application.");
                if (_exit != 0)
                {
                    return;
                }

                auto fileBrowserSystem =
                    context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(false);

                auto mainWindow = MainWindow::create(_context);
                addWindow(mainWindow);
                mainWindow->show();
            }

            App::App() {}

            App::~App() {}

            std::shared_ptr<App> App::create(
                const std::vector<std::string>& argv,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argv, context);
                return out;
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
