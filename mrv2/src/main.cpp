// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef _WIN32
#    include <windows.h>
#    include <stdio.h>
#    include <stdlib.h>
#endif

#include <iostream>

#include <FL/fl_utf8.h>

#ifdef MRV2_PYBIND11
#    include <pybind11/embed.h>
namespace py = pybind11;

#    include "mrvPy/Cmds.h"
#endif

#include "mrvFl/mrvInit.h"

#include "tlGL/Init.h"

#include "mrvApp/App.h"

#ifdef MRV2_PYBIND11
PYBIND11_EMBEDDED_MODULE(mrv2, m)
{
    mrv2_enums(m);
    mrv2_vectors(m);
    mrv2_otio(m);
    mrv2_filepath(m);
    mrv2_fileitem(m);
    mrv2_filesmodel(m);
    mrv2_image(m);
    mrv2_media(m);
    mrv2_settings(m);
    mrv2_timeline(m);
    mrv2_playlist(m);
    mrv2_io(m);
    mrv2_annotations(m);
#    ifdef TLRENDER_USD
    mrv2_usd(m);
#    endif
    mrv2_session(m);
    mrv2_commands(m);
    mrv2_python_plugins(m);
    mrv2_python_redirect(m);
}
#endif

int main(int argc, char* argv[])
{
    int r = 1;
#ifdef MRV2_PYBIND11
    // start the interpreter and keep it alive
    py::scoped_interpreter guard{};
#endif
    try
    {
        auto context = tl::system::Context::create();
        mrv::init(context);
        mrv::App app(argc, argv, context);
        r = app.getExit();
        if (0 == r)
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

#ifdef _WIN32

#    include <iostream>
#    include <locale>
#    include <codecvt>

// Function to convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& ws)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(ws);
}

int WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Convert the command line arguments to UTF-8
    int argc;
    char** argv;

    // Get the command line as a wide string
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Convert each wide string argument to UTF-8
    argv = new char*[argc];
    for (int i = 0; i < argc; ++i)
    {
        argv[i] = strdup(wstring_to_utf8(wideArgv[i]).c_str());
    }

    // Free the wide string array
    LocalFree(wideArgv);

    int ret = main(argc, argv);

    // Cleanup allocated memory for argv
    for (int i = 0; i < argc; ++i)
    {
        free(argv[i]);
    }
    delete[] argv;

    return ret;
}

#endif
