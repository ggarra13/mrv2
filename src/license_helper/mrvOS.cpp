
#include "mrvString.h"

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <direct.h>
#    include <winsock2.h>
#    include <windows.h>
#    include <shellapi.h>
#endif

#include <FL/fl_utf8.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <stdexcept>


#ifdef _WIN32

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

        
        // Helper function to read a pipe and append data to a string.
        // This will be run in a separate thread.
        static void ReadPipeThread(HANDLE hPipe, std::string& dest)
        {
            char buffer[256];
            DWORD bytesRead;
            dest.clear();

            // ReadFile will return FALSE (or 0 bytes read) when the
            // write-end of the pipe is closed by the child process.
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead,
                            NULL) && bytesRead > 0)
            {
                // Use append(buffer, bytesRead) instead of += buffer
                // as buffer is not null-terminated at bytesRead.
                // This also correctly handles null characters in the output.
                dest.append(buffer, bytesRead);
            }
        }
        
        int exec_command(const std::string& utf8_command,
                         std::string& std_out,
                         std::string& std_err)
        {
            SECURITY_ATTRIBUTES saAttr;
            HANDLE hStdOutRead, hStdOutWrite;
            HANDLE hStdErrRead, hStdErrWrite;

            // Convert UTF-8 std::string to UTF-16 std::wstring
            std::wstring utf16_command = string::convert_utf8_to_utf16(utf8_command);
            
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
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(STARTUPINFOW));
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            si.cb = sizeof(STARTUPINFOW);
            si.hStdOutput = hStdOutWrite;
            si.hStdError = hStdErrWrite;
            si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE; // Prevent console window from appearing

            // Convert command to mutable char buffer
            std::vector<wchar_t> cmd(utf16_command.begin(),
                                     utf16_command.end());
            cmd.push_back(0);

            // Create the process
            if (!CreateProcessW(NULL, cmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                DWORD error = GetLastError();
                std::cerr
                    << "exec_command: CreateProcessW failed with error: " +
                    std::to_string(error) << std::endl;
                
                CloseHandle(hStdOutRead);
                CloseHandle(hStdOutWrite);
                CloseHandle(hStdErrRead);
                CloseHandle(hStdErrWrite);
                
                std::string err = "Failed for " + utf8_command;
                throw std::runtime_error(err);
            }

            // Close write ends in parent
            CloseHandle(hStdOutWrite);
            CloseHandle(hStdErrWrite);

            // Create local strings for threads to write to.
            // This avoids race conditions on std_out and std_err.
            std::string outThreadData;
            std::string errThreadData;

            // Create two threads to read from stdout and stderr concurrently
            std::thread outThread(ReadPipeThread, hStdOutRead,
                                  std::ref(outThreadData));
            std::thread errThread(ReadPipeThread, hStdErrRead,
                                  std::ref(errThreadData));

            // Wait for process to exit and get exit code
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Get the exit code
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            // Wait for both reader threads to complete.
            // They will have already exited or will exit shortly after
            // the process terminates.
            outThread.join();
            errThread.join();

            std_out = outThreadData;
            std_err = errThreadData;
            
            // Close handles
            CloseHandle(hStdOutRead);
            CloseHandle(hStdErrRead);
            
            // Cleanup
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return static_cast<int>(exitCode);
        }
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
                    std::cerr << "Failed to parse command line" << std::endl;
                    return EXIT_FAILURE;
                }

                // Use the resolved executable path
                DWORD len = GetModuleFileNameW(NULL, wExe, MAX_PATH);
                if (len == 0 || len >= MAX_PATH)
                {
                    std::cerr << "GetModuleFileNameW failed" << std::endl;
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
                    std::cerr << "Failed to convert exe to wide string"
                              << std::endl;
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
                    std::cerr << "Failed to convert session to wide string" << std::endl;
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

}

#endif
