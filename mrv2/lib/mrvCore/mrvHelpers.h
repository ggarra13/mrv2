// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    //! Given a filename, open it in Nautilus (Linux), Explorer (Windows)
    //! or Finder (macOS).
    int file_manager_show_uri(const std::string& path);
} // namespace mrv
