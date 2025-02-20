// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#ifdef _WIN32
#     define strcasecmp(a, b) _stricmp(a, b)
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
