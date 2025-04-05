// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef _WIN32
#    include <windows.h>
#    include <stdio.h>
#    include <stdlib.h>
#    include <shellapi.h>
#endif

#include <string>
#include <iostream>

#include <FL/fl_utf8.h>

#include "mrvHDRApp.h"

int main(int argc, char* argv[])
{
    int r = 1;
    try
    {
        auto context = tl::system::Context::create();
        mrv::HDRApp app(argc, argv, context);
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

#if defined(_WIN32) && defined(_MSC_VER)

#    include <stdio.h>

#    include <FL/fl_utf8.h>
#    include <FL/fl_string_functions.h>

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    int i;
    int argc;
    char** argv;

    /*
     * If we are compiling in debug mode, open a console window so
     * we can see any printf's, etc...
     *
     * While we can detect if the program was run from the command-line -
     * look at the CMDLINE environment variable, it will be "WIN" for
     * programs started from the GUI - the shell seems to run all Windows
     * applications in the background anyways...
     */

    /* Convert the command line arguments to UTF-8 */
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

    /* Allocate an array of 'argc + 1' string pointers */
    argv = (char**)malloc((argc + 1) * sizeof(char*));

    /* Convert the command line arguments to UTF-8 */
    for (i = 0; i < argc; i++)
    {
        /* find the required size of the buffer */
        int u8size = WideCharToMultiByte(
            CP_UTF8,     /* CodePage */
            0,           /* dwFlags */
            wideArgv[i], /* lpWideCharStr */
            -1,          /* cchWideChar */
            NULL,        /* lpMultiByteStr */
            0,           /* cbMultiByte */
            NULL,        /* lpDefaultChar */
            NULL);       /* lpUsedDefaultChar */
        if (u8size > 0)
        {
            char* strbuf = (char*)malloc(u8size);
            int ret = WideCharToMultiByte(
                CP_UTF8,     /* CodePage */
                0,           /* dwFlags */
                wideArgv[i], /* lpWideCharStr */
                -1,          /* cchWideChar */
                strbuf,      /* lpMultiByteStr */
                u8size,      /* cbMultiByte */
                NULL,        /* lpDefaultChar */
                NULL);       /* lpUsedDefaultChar */
            if (ret)
            {
                argv[i] = strbuf;
            }
            else
            {
                argv[i] = _strdup("");
                free(strbuf);
                fprintf(stderr, "Failed to convert arg %d\n", i);
            }
        }
        else
        {
            argv[i] = _strdup("");
        }
    }
    argv[argc] = NULL; /* required by C standard at end of list */

    /* Free the wide character string array */
    LocalFree(wideArgv);

    /* Call the program's entry point main() */
    rc = main(argc, argv);

    /* Cleanup allocated memory for argv */
    for (int i = 0; i < argc; ++i)
    {
        free((void*)argv[i]);
    }
    free((void*)argv);

    return rc;
}

#endif
