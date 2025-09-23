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

        //! Execute a command without bringing up a console on Windows.
        int exec_command_no_block(const std::string& command);
        
        //! Execute a command without bringing up a console on Windows.
        int exec_command(const std::string& command);

        //! Execute a command through pipes and capture stdout/sterr.
        //! Returns the exit value of the command.
        int exec_command(const std::string& command, std::string& output,
                         std::string& errors);

        //! Re-run the executable with its parameters or an optional session
        //! file.
        int execv(const std::string& exe = "", const std::string& session = "");

        //! Return the name of the wayland compositor.
        const std::string getWaylandCompositor(const std::string& desktop);

        //! Return the name of the GPU Vendor.
        const std::string getGPUVendor();

        //! Return the name of the Kernel (OS).
        const std::string getKernel();

        //! Return the name of the desktop.
        const std::string getDesktop();

        //! Return the version of the OS.
        const std::string getVersion();

        //! Returns true if running on the terminal (ie. TERM is set).
        bool runningInTerminal();
    } // namespace os

} // namespace mrv
