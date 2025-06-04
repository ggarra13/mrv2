// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "player-qtquick.h"

#include <tlQtQuick/Init.h>

#include <tlCore/Context.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 1;
    try
    {
        auto context = tl::system::Context::create();
        tl::qtquick::init(
            tl::qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile, context);
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        tl::examples::player_qtquick::App app(argc, argv, context);
        if (0 == app.getExit())
        {
            r = app.exec();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
