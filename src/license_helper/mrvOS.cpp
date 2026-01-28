// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <direct.h>
#    include <winsock2.h>
#    include <windows.h>
#    include <shellapi.h>
#else
#    include <cstdio>
#    include <sys/wait.h>
#    include <unistd.h>
#    include <fcntl.h>
#endif

#include <algorithm>
#include <array>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <tlCore/OS.h>
#include <tlCore/String.h>

#include <FL/Fl.H>

#include "mrvApp/mrvApp.h"

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvString.h"
#include "mrvCore/mrvOS.h"

#include "mrvFl/mrvIO.h"

#include "mrvUI/mrvDesktop.h"

#ifdef OPENGL_BACKEND
#    include <tlGL/Init.h>
#    include <FL/gl.h>
#endif

#ifdef VULKAN_BACKEND
#    include "mrvWidgets/mrvVersion.h"
#endif

#include <string>
#include <vector>
#include <stdexcept>
#include <array>
#include <memory>

namespace
{
    const char* kModule = "os";
}

namespace mrv
{
    namespace os
    {

        std::string sgetenv(const char* const n)
        {
            if (fl_getenv(n))
                return fl_getenv(n);
            else
                return std::string();
        }

        // When no session is provided, pass all the arguments the user
        // used to call the execuutable.  We use this routine to restart
        // mrv2 with all its parameters so that the LANGUAGE env. variable
        // takes effect.
        //
        // When a session is provided, just pass that as a parameter
        //
        int execv(const std::string& exe, const std::string& session,
                  const bool wait)
        {
#ifdef _WIN32
            LPWSTR* argv = nullptr;
            LPWSTR* newArgv = nullptr;
            int argc = 0;
            wchar_t wExe[MAX_PATH];
            std::vector<std::wstring> wArgs; // To store converted args safely

            // Get the executable path
            if (exe.empty())
            {
                // Get the full command line and parse it
                LPWSTR lpCmdLine = GetCommandLineW();
                argv = CommandLineToArgvW(lpCmdLine, &argc);
                if (!argv || argc < 1)
                {
                    LOG_ERROR("Failed to parse command line");
                    return EXIT_FAILURE;
                }

                // Use the resolved executable path
                DWORD len = GetModuleFileNameW(NULL, wExe, MAX_PATH);
                if (len == 0 || len >= MAX_PATH)
                {
                    LOG_ERROR("GetModuleFileNameW failed");
                    LocalFree(argv);
                    return EXIT_FAILURE;
                }

                // Store original arguments (skip argv[0])
                wArgs.push_back(wExe);
                if (session.empty())
                {
                    for (int i = 1; i < argc; ++i)
                        wArgs.push_back(argv[i]);
                }
            }
            else
            {
                // Convert provided exe to wchar_t
                int len = MultiByteToWideChar(CP_UTF8, 0, exe.c_str(), -1, wExe, MAX_PATH);
                if (len <= 0)
                {
                    LOG_ERROR("Failed to convert exe to wide string");
                    return EXIT_FAILURE;
                }
                wArgs.push_back(wExe);
            }

            // Add session if provided
            if (!session.empty())
            {
                wchar_t wSession[MAX_PATH];
                int len = MultiByteToWideChar(CP_UTF8, 0, session.c_str(), -1, wSession, MAX_PATH);
                if (len <= 0)
                {
                    LOG_ERROR("Failed to convert session to wide string");
                    if (argv) LocalFree(argv);
                    return EXIT_FAILURE;
                }
                if (wArgs.size() > 1) wArgs.resize(1); // Clear extra args
                wArgs.push_back(wSession);
            }

            // Build the command line string for CreateProcess
            std::wstring cmdLine;
            for (size_t i = 0; i < wArgs.size(); ++i)
            {
                if (wcschr(wArgs[i].c_str(), L' ') != nullptr)
                {
                    cmdLine += L"\"" + wArgs[i] + L"\"";
                }
                else
                {
                    cmdLine += wArgs[i];
                }
                if (i < wArgs.size() - 1) cmdLine += L" ";
            }

            // Setup STARTUPINFOW to inherit handles
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            // Create the process
            BOOL success = CreateProcessW(
                wExe,                   // Executable path
                &cmdLine[0],            // Command line (must be writable)
                NULL,                   // Process handle not inheritable
                NULL,                   // Thread handle not inheritable
                TRUE,                   // Inherit handles
                0,                      // Creation flags
                NULL,                   // Use parent's environment
                NULL,                   // Use parent's current directory
                &si,                    // STARTUPINFOW
                &pi                     // PROCESS_INFORMATION
                );

            if (!success)
            {
                DWORD error = GetLastError();
                LOG_ERROR("CreateProcessW failed with error: " +
                          std::to_string(error));
                std::wcerr << L"Command: " << cmdLine << std::endl;
                if (argv) LocalFree(argv);
                return EXIT_FAILURE;
            }

            // Wait for the process to complete (optional, remove if
            // you don't want to wait)
            if (wait)
                WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Cleanup
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            if (argv) LocalFree(argv);

            exit(exitCode);
#else
            std::string run;
            if (exe.empty())
            {
                run = mrv::rootpath() + "/bin/mrv2";
            }
            else
            {
                unsetenv("MRV2_ROOT");
                run = exe;
            }

            const char* const newArgv[] = {run.c_str(), session.c_str(), NULL};
            int ret = ::execv(run.c_str(), const_cast<char**>(newArgv));
            if (ret == -1)
            {
                perror("execv failed");
            }
            return ret;
#endif
        }
    } // namespace os

} // namespace mrv
