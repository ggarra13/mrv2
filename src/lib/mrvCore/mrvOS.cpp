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


#ifdef _WIN32
        int exec_command(const std::string& command)
        {
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;

            // Initialize the structures
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Convert command to a mutable char buffer for CreateProcessA
            std::vector<char> cmd(command.begin(), command.end());
            cmd.push_back('\0');

            // Create the process
            // The CREATE_NO_WINDOW flag is key to preventing a console from popping up
            if (!CreateProcessA(
                    NULL,           // No module name (use command line)
                    cmd.data(),     // Command line
                    NULL,           // Process handle not inheritable
                    NULL,           // Thread handle not inheritable
                    FALSE,          // Set handle inheritance to FALSE
                    CREATE_NO_WINDOW, // Don't create a console window
                    NULL,           // Use parent's environment block
                    NULL,           // Use parent's starting directory
                    &si,            // Pointer to STARTUPINFOA structure
                    &pi             // Pointer to PROCESS_INFORMATION structure
                    )) {
                throw std::runtime_error("CreateProcess failed!");
            }

            // Wait until the child process exits.
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code.
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Clean up handles.
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return static_cast<int>(exitCode);
        }
        
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
        int exec_command(const std::string& command)
        {
            return ::system(command.c_str());
        }
        
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

        // When no session is provided, pass all the arguments the user
        // used to call the execuutable.  We use this routine to restart
        // mrv2 with all its parameters so that the LANGUAGE env. variable
        // takes effect.
        //
        // When a session is provided, just pass that as a parameter
        //
        int execv(const std::string& exe, const std::string& session)
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

            // // Allocate newArgv with exact size
            // size_t argCount = wArgs.size() + 1; // +1 for nullptr
            // newArgv = new LPWSTR[argCount];
            // for (size_t i = 0; i < wArgs.size(); ++i)
            // {
            //     if (wcschr(wArgs[i].c_str(), L' ') != nullptr)
            //     {
            //         wArgs[i] = L"\"" + wArgs[i] + L"\""; // Quote if spaces
            //     }
            //     newArgv[i] = const_cast<LPWSTR>(wArgs[i].c_str()); // Safe: wArgs lives until execv
            // }
            // newArgv[argCount - 1] = nullptr;
            // // Call _wexecv
            // LPWSTR cmd = wExe;  // command must be unquoted
            // unsetenv(L"MRV2_ROOT"); // Remove MRV2_ROOT from environment
            // int result = _wexecv(wExe, newArgv);
            // if (result == -1)
            // {
            //     LOG_ERROR("'_wexecv' failed with errno: " + std::to_string(errno));
            //     std::wcerr << L"Command: " << cmd;
            //     for (int i = 1; i < argc; ++i)
            //     {
            //         std::wcerr << L" " << newArgv[i];
            //     }
            //     std::wcerr << std::endl;
            // }
            //
            // // Cleanup
            // delete[] newArgv;
            // if (argv) LocalFree(argv);
            //
            // return result == -1 ? EXIT_FAILURE : 0;


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
                &si,                    // STARTUPINFO
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

        const std::string getGPUVendor()
        {
            std::string out = _("Unknown");

#ifdef OPENGL_BACKEND
            tl::gl::initGLAD();

            // Get OpenGL information
            const char* vendorString = (char*)glGetString(GL_VENDOR);
            if (vendorString)
                out = vendorString;
#endif
            out = "GPU: " + out;
            return out;
        }

        const std::string
        getWaylandCompositorVersion(const std::string& compositor)
        {
            static std::string compositor_version;
            if (!compositor_version.empty())
                return compositor_version;
            
            std::string version_command;
            
            // Choose the command based on the compositor name
            if (compositor == "gnome-shell")
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
                compositor_version = out;
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
            if (desktop == "ubuntu-wayland" || desktop == "ubuntu" ||
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
            else if (desktop == "wayfire")
            {
                return "wayfire";
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
            static std::string out;

            if (!out.empty())
                return out;
            
            out = _("\tDesktop Environment: ");

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
