// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlCore/File.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace tl
{
    namespace file
    {
        bool isNetwork(const std::string& path)
        {
            static const std::string schemes[] = {"crypto", "ftp",       "http",
                                                  "https",  "httpproxy", "rtmp",
                                                  "rtp",    "tcp",       "tls"};

            for (const std::string& scheme : schemes)
            {
                if (path.find(scheme + ":") == 0)
                    return true;
            }
            return false;
        }

        bool isReadable(const std::string& fileName)
        {
#if defined(__cpp_lib_char8_t)
            // C++20: u8path is deprecated. We cast the string data to char8_t.
            fs::path p{reinterpret_cast<const char8_t*>(fileName.data())};
#else
            // C++17: u8path is the standard way to handle UTF-8 strings.
            fs::path p = fs::u8path(fileName);   // same in C++17
#endif
            
            // 2. Logic checks
            if (p.empty()) 
                return false;
    
            if (isNetwork(p.string()))
                return true;

            std::ifstream f(p);
            return f.is_open();
        }
    } // namespace file
} // namespace tl
