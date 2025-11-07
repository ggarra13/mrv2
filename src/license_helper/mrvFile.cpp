// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFile.h"

#include <fstream>

namespace mrv
{
    namespace file
    {
        bool isReadable(const fs::path& p)
        {
            const std::string& filePath = p.u8string();
            if (filePath.empty())
                return false;

            std::ifstream f(filePath);
            if (f.is_open())
            {
                f.close();
                return true;
            }

            return false;
        }

    } // namespace file

} // namespace mrv
