// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <iostream>

#include "tlGL/Util.h"

#include "mrvApp/App.h"

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::gl::init(context);
        mrv::App app(argc, argv, context);
        if (0 == app.getExit())
        {
            r = app.run();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
