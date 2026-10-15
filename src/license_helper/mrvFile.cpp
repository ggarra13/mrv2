// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFile.h"

#include <fstream>

namespace mrv
{
    namespace file
    {
        std::string fromFileSystem(const std::filesystem::path& value)
        {
            // Under C++17 u8string() is a std::string and this is a copy;
            // under C++20 it is a std::u8string and the iterators convert
            // the characters.
            const auto u8 = value.u8string();
            return std::string(u8.begin(), u8.end());
        }

        bool isReadable(const fs::path& p)
        {
            const std::string& filePath = fromFileSystem(p);
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
