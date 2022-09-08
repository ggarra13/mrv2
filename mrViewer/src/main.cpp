// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <iostream>

#include "tlGL/Util.h"

#include "mrvPlayApp/App.h"

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::gl::init(context);
        auto app = mrv::App::create(argc, argv, context);
        r = app->run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
