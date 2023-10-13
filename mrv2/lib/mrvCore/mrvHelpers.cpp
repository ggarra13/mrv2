// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef __linux__
#    include <stdlib.h>
#endif

#ifdef _WIN32
#    include <windows.h>
#    include <filesystem>
namespace fs = std::filesystem;
#endif

#include <iostream>

#include "mrvCore/mrvHelpers.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "helpers";
}

#ifdef __linux__
namespace
{
    int pipe_command(const char* buf)
    {
        FILE* cmdOutput = popen(buf, "r");
        if (cmdOutput)
        {
            char outputBuffer[128];
            while (fgets(outputBuffer, sizeof(outputBuffer), cmdOutput) !=
                   nullptr)
            {
                // Print the output or log it for debugging
            }
            pclose(cmdOutput);
            return 0;
        }
        return 1;
    }

} // namespace
#endif

namespace mrv
{

#ifdef __linux__
    int nautilus_file_manager(const std::string& file)
    {
        char buf[4096];
        const std::string uri = "file://localhost" + file;

        // Construct the D-Bus command to show the file in Nautilus.
        snprintf(
            buf, 4096,
            "dbus-send --session --print-reply "
            "--dest=org.freedesktop.FileManager1 "
            "--type=method_call /org/freedesktop/FileManager1 "
            "org.freedesktop.FileManager1.ShowItems array:string:\"%s\" "
            "string:\"\"",
            uri.c_str());

        return pipe_command(buf);
    }
#endif

#ifdef _WIN32
    int explorer_file_manager(const std::string& file)
    {
        fs::path path(file);
        const fs::path back = path.make_preferred();
        const auto native_path = back.string();
        std::string buf = "explorer /select,\"" + native_path + "\"";

        // CreateProcess parameters
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        // Create the process
        if (CreateProcess(
                NULL, const_cast<char*>(buf.c_str()), NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            // Wait for the process to complete (optional)
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Close process and thread handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return 0;
        }
        else
        {
            // Handle the error if needed
            return 1;
        }
    }
#endif

    namespace file_manager
    {

#ifndef __APPLE__
        int show_uri(const std::string& file)
        {
#    ifdef __linux__
            return nautilus_file_manager(file);
#    elif _WIN32
            return explorer_file_manager(file);
#    endif
        }
#endif

    } // namespace file_manager

} // namespace mrv
