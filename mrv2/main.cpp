// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Util.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::gl::init(context);
        auto app = mrv::App::create(argc, argv, context);
        r = app->run();
        std::cerr << "exit " << r << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
