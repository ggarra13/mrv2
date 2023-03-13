// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <pybind11/embed.h>
namespace py = pybind11;

#include "tlGL/Util.h"

#include "mrvPy/Cmds.h"

#include "mrvApp/App.h"

PYBIND11_EMBEDDED_MODULE(mrv2, m)
{
    mrv2_enums(m);
    mrv2_vectors(m);
    mrv2_otio(m);
    mrv2_filepath(m);
    mrv2_fileitem(m);
    mrv2_timeline(m);
    mrv2_filesmodel(m);
    mrv2_commands(m);
}

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
#ifdef MRV2_PYBIND11
        // start the interpreter and keep it alive
        py::scoped_interpreter guard{};
#endif
        auto context = tl::system::Context::create();
        tl::gl::init(context);
        mrv::App app(argc, argv, context);
        if (0 == app.getExit())
        {
            r = app.run();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
