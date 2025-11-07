// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace mrv
{
    namespace file
    {
        
        /**
         * Return true if the file exists and is readable
         *
         * @param p std::filesystem path (or std::string).
         *
         * @return true if it exists and is readable, false if not.
         */
        bool isReadable(const fs::path& path);
        
    } // namespace file

} // namespace mrv
