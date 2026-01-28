// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#ifdef _WIN32
#    define strcasecmp(a, b) _stricmp(a, b)
#endif // _WIN32

namespace mrv
{
    namespace os
    {
        //! Return an environment variable's content in UTF-8 or empty string.
        std::string sgetenv(const char* const n);

        //! Execute a command through pipes and capture stdout/sterr.
        //! Returns the exit value of the command.
        int exec_command(const std::string& command, std::string& output,
                         std::string& errors);

        //! Re-run the executable with its parameters or an optional session
        //! file.
        int execv(const std::string& exe = "", const std::string& session = "",
                  const bool wait = true);

    } // namespace os

} // namespace mrv
