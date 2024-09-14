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
#    include <unistd.h>
#endif

#include <algorithm>
#include <array>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#include <tlCore/OS.h>
#include <tlCore/String.h>

#include <FL/Fl.H>

#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvOS.h"

#include "mrvFl/mrvIO.h"

#include "mrvUI/mrvDesktop.h"

#ifdef TLRENDER_GL
#    include <tlGL/Init.h>
#    include <FL/gl.h>
#endif

namespace
{
    const char* kModule = "os";
}

namespace
{
    

    // Function to execute a shell command and capture the output
    std::string exec_command(const std::string& command)
    {
        std::string out;

        std::array<char, 128> buffer;

        // Open a pipe to the command
        std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

        // Read the output from the command
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            out += buffer.data();
        }

        return out;
    }
    
} // namespace

namespace mrv
{
    namespace os
    {

        int execv(const std::string& exe, const std::string& session)
        {

#ifdef _WIN32
            int argc = 2;
            LPWSTR* argv = nullptr;
            LPWSTR* newArgv = nullptr;
            std::wstring wExe;
            std::wstring wSession;

            if (exe.empty())
            {
                // Get the full command line string
                LPWSTR lpCmdLine = GetCommandLineW();

                // Parse the command line string into an array of arguments
                argv = CommandLineToArgvW(lpCmdLine, &argc);

                if (argv == nullptr)
                {
                    wprintf(L"Failed to parse command line\n");
                    return EXIT_FAILURE;
                }

                // Allocate new array
                argc = argc + 2;

                newArgv = new LPWSTR[argc];
                for (int i = 0; i < argc; ++i)
                    newArgv[i] = nullptr;

                for (int i = 0; i < argc - 1; ++i)
                    newArgv[i] = argv[i];
            }
            else
            {
                unsetenv("MRV2_ROOT");
                wExe = std::wstring(exe.begin(), exe.end());

                // Allocate new array
                argc = 3;
                newArgv = new LPWSTR[argc];
                newArgv[0] = const_cast<LPWSTR>(wExe.c_str());
                newArgv[1] = nullptr;
                newArgv[2] = nullptr;
            }

            if (!session.empty())
            {
                wSession = std::wstring(session.begin(), session.end());
                newArgv[1] = const_cast<LPWSTR>(wSession.c_str());
            }

            // Enclose argv[0] in double quotes if it contains spaces
            LPWSTR cmd = newArgv[0];
            bool* allocated = new bool[argc];
            for (int i = 0; i < argc; i++)
            {
                allocated[i] = false;
                const LPWSTR arg = newArgv[i];
                if (arg == nullptr)
                    continue;

                if (wcschr(arg, L' ') != NULL)
                {
                    // 2 for quotes, 1 for null terminator
                    size_t len = wcslen(arg) + 3;
                    LPWSTR quoted_arg = (LPWSTR)malloc(len * sizeof(wchar_t));
                    if (quoted_arg == NULL)
                    {
                        wprintf(
                            L"Failed to allocate memory for command line\n");
                        return EXIT_FAILURE;
                    }
                    swprintf_s(quoted_arg, len, L"\"%s\"", arg);

                    // Free the memory used by the unquoted argument
                    newArgv[i] = quoted_arg;
                    allocated[i] = true;
                }
            }

            // Call _wexecv
            int result;
            result = _wexecv(cmd, newArgv);

            // Free the array of arguments
            for (int i = 0; i < argc; i++)
            {
                if (allocated[i])
                    free(newArgv[i]);
                newArgv[i] = nullptr;
            }
            delete[] newArgv;
            delete[] allocated;

            if (argv)
            {
                for (int i = 0; i < argc; i++)
                {
                    free(argv[i]);
                    argv[i] = nullptr;
                }
                LocalFree(argv);
            }
            if (result == -1)
            {
                perror("_wexecv");
                return EXIT_FAILURE;
            }

            exit(EXIT_SUCCESS);
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
                LOG_ERROR("execv failed " << run << " " << session);
                perror("execv failed");
            }
            exit(ret);
#endif
            return -1;
        }

        std::string getGPUVendor()
        {
            std::string out = _("Unknown");

            tl::gl::initGLAD();

            // Get OpenGL information
            const char* vendorString = (char*)glGetString(GL_VENDOR);
            if (vendorString)
                out = vendorString;

            out = "GPU: " + out;
            return out;
        }

        std::string getWaylandCompositorVersion(const std::string& compositor)
        {
            std::string version_command;

            // Choose the command based on the compositor name
            if (compositor == "mutter")
            {
                version_command = "gnome-shell --version";
            }
            else if (compositor == "kwin")
            {
                version_command = "kwin_wayland --version";
            }
            else if (compositor == "weston")
            {
                version_command = "weston --version";
            }
            else if (compositor == "sway")
            {
                version_command = "sway --version";
            }
            else
            {
                return "";
            }

            try
            {
                // Execute the command and capture the output
                std::string version_output = exec_command(version_command);
                return version_output;
            }
            catch (const std::exception& e)
            {
                return std::string("Error executing command: ") + e.what();
            }
        }

        std::string getWaylandCompositor(const std::string& desktop_env)
        {
            const std::string& desktop = tl::string::toLower(desktop_env);

            // Check against common Wayland compositor names
            if (desktop == "ubuntu-wayland" ||
                desktop.substr(0, 5) == "gnome" || desktop == "mutter" ||
                desktop == "gnome-wayland")
            {
                return "mutter";
            }
            else if (
                desktop == "kwin" || desktop == "kde" || desktop == "plasma")
            {
                return "kwin";
            }
            else if (desktop == "weston")
            {
                return "weston";
            }
            else if (desktop == "sway")
            {
                return "sway"; // If using Sway (a Wayland compositor based on
                               // i3)
            }
            else
            {
                return "unknown"; // If no match is found
            }
        }

        std::string getDesktop()
        {
            std::string out = _("Desktop: ");

#ifdef __linux__
            if (desktop::XWayland())
            {
                out += "XWayland";
            }
            else
            {
                const char* env = fl_getenv("XDG_SESSION_DESKTOP");
                if (env && strlen(env) > 0)
                {
                    out += env;
                }
                else
                {
                    env = fl_getenv("DESKTOP_SESSION");
                    if (env && strlen(env) > 0)
                    {
                        out += env;
                    }
                    else
                    {
                        env = "";
                    }
                }

                if (env && strlen(env) > 0)
                {
                    std::string compositor = getWaylandCompositor(env);
                    out += " ";
                    out += getWaylandCompositorVersion(compositor);
                }
            }
#elif _WIN32
            out += "Windows (GDI+)";
#elif __APPLE__
            out += "macOS (Cocoa)";
#else
            out += _("Unknown");
#endif

            return out;
        }

        std::string getVersion()
        {
            tl::os::SystemInfo info = tl::os::getSystemInfo();
            std::string os_version;
#ifdef __linux__
            std::ifstream os_release("/etc/os-release");
            std::string line;

            if (os_release.is_open())
            {
                while (std::getline(os_release, line))
                {
                    if (line.find("PRETTY_NAME=") != std::string::npos)
                    {
                        os_version = line.substr(line.find("=") + 1);
                        // Remove quotes from the version string
                        os_version.erase(
                            std::remove(
                                os_version.begin(), os_version.end(), '"'),
                            os_version.end());
                        break;
                    }
                }
                os_release.close();
            }
            if (os_version.empty())
                os_version = info.name;
#else
            os_version = info.name;
#endif

            // Output also ocmpile type
            std::string compile = "Debug Compile";
#ifdef NDEBUG
#    ifdef MRV2_RelWithDebInfo
            compile = "RelWithDebInfo";
#    else
            compile = "Release";
#    endif
#endif
            os_version = _("Running on: ") + os_version + " " + compile;
            return os_version;
        }

        bool runningInTerminal()
        {
#ifdef _WIN32
            char* term = fl_getenv("TERM");
            if (!term)
                return false;
            return true;
#else
            return (isatty(fileno(stdout)));
#endif
        }
    } // namespace os

} // namespace mrv
