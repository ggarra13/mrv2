// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    namespace desktop
    {
        bool X11();
        bool Wayland();
        bool XWayland();
        bool Windows();
        bool macOS();

        std::string monitorName(int monitorIndex);
    } // namespace desktop
} // namespace mrv
