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

#include "mrvCore/mrvFileManager.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvOS.h"


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
        const fs::path path(file);
        const fs::path back = path.make_preferred();
        const auto native_path = back.u8string();
        const std::string cmd = "explorer /select,\"" + native_path + "\"";
        return os::exec_command(cmd);
    }
#endif

    namespace file_manager
    {

#ifndef __APPLE__
        int show_uri(const std::string& file)
        {
#    ifdef __linux__
            return nautilus_file_manager(file);
#    elif defined(_WIN32)
            return explorer_file_manager(file);
#    endif
        }
#endif

    } // namespace file_manager

} // namespace mrv
