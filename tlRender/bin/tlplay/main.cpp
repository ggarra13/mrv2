// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/App.h>

#include <tlPlay/Init.h>

#include <iostream>

#if defined(_WINDOWS)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif // WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <tchar.h>
#endif // _WINDOWS

TLRENDER_MAIN()
{
    int r = 1;
    try
    {
        auto context = tl::system::Context::create();
        tl::play::init(context);
        auto app =
            tl::play_app::App::create(tl::app::convert(argc, argv), context);
        r = app->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}

#if defined(_WINDOWS)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    return wmain(__argc, __wargv);
}
#endif // _WINDOWS
