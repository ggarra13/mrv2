// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#ifdef _WIN32

#    if !(defined vsnprintf)
#        define vsnprintf _vsnprintf
#    endif

#    if !(defined putenv)
#        define putenv(x) _putenv(x)
#    endif

#    undef printf
#    undef max
#    undef min
#    undef stricmp

#    if !(defined strcasecmp)
#        ifdef _WIN32
#            define strcasecmp(a, b) _stricmp(a, b)
#        else
#            define strcasecmp(a, b) stricmp(a, b)
#        endif
#    endif

#    if !(defined strtok_r)
#        define strtok_r(a, b, c) strtok(a, b)
#    endif

#    if !(defined snprintf)
#        define snprintf _snprintf
#    endif

#    if !(defined access)
#        define access _access
#    endif

#    undef itoa
#    define itoa(x, a, b) _itoa(x, a, b)

#    undef getcwd
#    define getcwd _getcwd

#    undef chdir
#    define chdir _chdir

#    undef popen
#    define popen _popen

#    undef pclose
#    define pclose _pclose

#endif // _WIN32

namespace mrv
{
    namespace os
    {
        std::string exec_command(const std::string& command);
        
        int execv(const std::string& exe = "",
                  const std::string& session = "");
        
        const std::string getWaylandCompositor(const std::string& desktop);
        
        const std::string getGPUVendor();

        const std::string getKernel();
        
        const std::string getDesktop();

        const std::string getVersion();

        bool runningInTerminal();
    } // namespace os

} // namespace mrv
