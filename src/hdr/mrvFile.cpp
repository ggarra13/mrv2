// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>

#include "mrvFile.h"
#include "mrvHome.h"

#include <regex>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;


#ifdef _WIN32
#    include <process.h>
#else
#    include <unistd.h> // For access()
#endif

namespace mrv
{
    namespace file
    {
        std::string normalizePath(const std::string& path)
        {
            std::string normalized = path;
            std::replace(normalized.begin(), normalized.end(), '\\', '/');
            return normalized;
        }
        

        bool exists(const fs::path& filePath)
        {
            return fs::exists(filePath);
        }
        
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
