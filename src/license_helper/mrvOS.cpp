
#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <direct.h>
#    include <winsock2.h>
#    include <windows.h>
#    include <shellapi.h>
#endif

#include <string>
#include <vector>
#include <stdexcept>


#ifdef _WIN32

namespace mrv
{
    namespace os
    {
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
    }
}

#endif
