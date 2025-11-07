
#include "mrvString.h"

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <direct.h>
#    include <winsock2.h>
#    include <windows.h>
#    include <shellapi.h>
#endif

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
}

#endif
