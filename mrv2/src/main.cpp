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
/* static int mbcs2utf(const char *s, int l, char *dst, unsigned dstlen) */
static int mbcs2utf(const char* s, int l, char* dst)
{
    static wchar_t* mbwbuf;
    unsigned dstlen = 0;
    if (!s)
        return 0;
    dstlen = (l * 6) + 6;
    mbwbuf = (wchar_t*)malloc(dstlen * sizeof(wchar_t));
    l = (int)mbstowcs(mbwbuf, s, l);
    /* l = fl_unicode2utf(mbwbuf, l, dst); */
    l = fl_utf8fromwc(dst, dstlen, mbwbuf, l);
    dst[l] = 0;
    free(mbwbuf);
    return l;
}

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int rc, i;
    char** ar;

    ar = (char**)malloc(sizeof(char*) * (__argc + 1));
    i = 0;
    while (i < __argc)
    {
        int l;
        unsigned dstlen;
        if (__wargv)
        {
            for (l = 0; __wargv[i] && __wargv[i][l]; l++)
            {
            }; /* is this just wstrlen??? */
            dstlen = (l * 5) + 1;
            ar[i] = (char*)malloc(dstlen);
            /*    ar[i][fl_unicode2utf(__wargv[i], l, ar[i])] = 0; */
            dstlen = fl_utf8fromwc(ar[i], dstlen, __wargv[i], l);
            ar[i][dstlen] = 0;
        }
        else
        {
            for (l = 0; __argv[i] && __argv[i][l]; l++)
            {
            };
            dstlen = (l * 5) + 1;
            ar[i] = (char*)malloc(dstlen);
            /*      ar[i][mbcs2utf(__argv[i], l, ar[i], dstlen)] = 0; */
            ar[i][mbcs2utf(__argv[i], l, ar[i])] = 0;
        }
        i++;
    }
    ar[__argc] = 0;
    /* Run the standard main entry point function... */
    rc = main(__argc, ar);

    return rc;
}

#endif
