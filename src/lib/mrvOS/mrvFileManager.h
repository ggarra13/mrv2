// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    namespace file_manager
    {
        //! Given a filename, open it in Nautilus (Linux), Explorer (Windows)
        //! or Finder (macOS).
        int show_uri(const std::string& path);
    } // namespace file_manager
} // namespace mrv
