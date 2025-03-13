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
#include <vector>

#include <tlCore/OS.h>
#include <tlCore/String.h>

#include <FL/Fl.H>

#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvString.h"
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
        #include <string>
#include <vector>
#include <stdexcept>
#include <array>
#include <memory>

#ifdef _WIN32
        int exec_command(const std::string& command,
                         std::string& std_out,
                         std::string& std_err)
        {
            SECURITY_ATTRIBUTES saAttr;
            HANDLE hStdOutRead, hStdOutWrite;
            HANDLE hStdErrRead, hStdErrWrite;

            // Initialize output variables
            std_out.clear();
            std_err.clear();

            // Set up security attributes
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
            saAttr.bInheritHandle = TRUE;
            saAttr.lpSecurityDescriptor = NULL;

            // Create pipes for stdout and stderr
            if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0) ||
                !CreatePipe(&hStdErrRead, &hStdErrWrite, &saAttr, 0))
            {
                throw std::runtime_error("CreatePipe failed!");
            }

            // Ensure the read handles are not inherited
            SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
            SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);

            // Configure STARTUPINFO
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(STARTUPINFO));
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            si.cb = sizeof(STARTUPINFO);
            si.hStdOutput = hStdOutWrite;
            si.hStdError = hStdErrWrite;
            si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE; // Prevent console window from appearing

            // Convert command to mutable char buffer
            std::vector<char> cmd(command.begin(), command.end());
            cmd.push_back(0);

            // Create the process
            if (!CreateProcess(NULL, cmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                CloseHandle(hStdOutRead);
                CloseHandle(hStdOutWrite);
                CloseHandle(hStdErrRead);
                CloseHandle(hStdErrWrite);
                throw std::runtime_error("CreateProcess failed!");
            }

            // Close write ends in parent
            CloseHandle(hStdOutWrite);
            CloseHandle(hStdErrWrite);

            // Read stdout and stderr
            char buffer[256];
            DWORD bytesRead;

            while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                std_out += buffer;
            }

            while (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                std_err += buffer;
            }

            // Close handles
            CloseHandle(hStdOutRead);
            CloseHandle(hStdErrRead);

            // Wait for process to exit and get exit code
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Cleanup
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return static_cast<int>(exitCode);
        }

#else

        int exec_command(const std::string& command,
                         std::string& std_out, std::string& std_err)
        {
            int stdout_pipe[2], stderr_pipe[2];
            std_out.clear();
            std_err.clear();

            if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0)
            {
                throw std::runtime_error("pipe() failed!");
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                throw std::runtime_error("fork() failed!");
            }
            else if (pid == 0) // Child process
            {
                close(stdout_pipe[0]); // Close read end of stdout pipe
                close(stderr_pipe[0]); // Close read end of stderr pipe

                dup2(stdout_pipe[1], STDOUT_FILENO); // Redirect stdout to pipe
                dup2(stderr_pipe[1], STDERR_FILENO); // Redirect stderr to pipe

                close(stdout_pipe[1]);
                close(stderr_pipe[1]);

                execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
                _exit(127); // Only reached if execl fails
            }

            // Parent process
            close(stdout_pipe[1]); // Close write end
            close(stderr_pipe[1]); // Close write end

            char buffer[256];
            ssize_t bytesRead;

            while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytesRead] = '\0';
                std_out += buffer;
            }

            while ((bytesRead = read(stderr_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytesRead] = '\0';
                std_err += buffer;
            }

            close(stdout_pipe[0]);
            close(stderr_pipe[0]);

            int status;
            waitpid(pid, &status, 0);

            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        }

#endif

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

        const std::string getGPUVendor()
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

        const std::string
        getWaylandCompositorVersion(const std::string& compositor)
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
                int ret;
                std::string err;
                std::string out;
                ret = os::exec_command(version_command, out, err);
                return out;
            }
            catch (const std::exception& e)
            {
                return std::string("Error executing command: ") + e.what();
            }
        }

        const std::string getWaylandCompositor(const std::string& desktop_env)
        {
            const std::string& desktop = tl::string::toLower(desktop_env);

            // Check against common Wayland compositor names
            if (desktop == "ubuntu-wayland" ||
                desktop.substr(0, 5) == "gnome" || desktop == "mutter")
            {
                return "gnome-shell";
            }
            else if (
                desktop.substr(0, 4) == "kwin" || desktop == "kde" ||
                desktop == "plasma")
            {
                return "kwin";
            }
            else if (desktop == "weston")
            {
                return "weston";
            }
            else if (desktop == "sway")
            {
                return "sway";
            }
            else
            {
                return desktop; // If no match is found
            }
        }

        const std::string getDesktop()
        {
            std::string out = _("\tDesktop Environment: ");

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

                // Remove any newlines or spaces at end (needed on Rocky Linux)
                out = string::stripTrailingWhitespace(out);
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

        const std::string getKernel()
        {
            int ret;
            std::string out;
            std::string err;
#ifdef _WIN32
            ret = exec_command("cmd /C ver", out, err);
#else
            ret = exec_command("uname -r", out, err);
#endif
            out = _("\tKernel Info: ") + string::stripWhitespace(out);
            return out;
        }

        const std::string getVersion()
        {
            static std::string os_version;
            if (!os_version.empty())
                return os_version;
            tl::os::SystemInfo info = tl::os::getSystemInfo();
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
#elif _WIN32
            int ret;
            std::string out, err;
            ret = exec_command("powershell -Command  \"(Get-CimInstance "
                               "Win32_OperatingSystem).Caption\"", out, err);
            os_version = string::stripWhitespace(out);
#else
            os_version = info.name;
#endif
            os_version = _("\tDistribution: ") + os_version;
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
