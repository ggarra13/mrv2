// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include "mrvCore/mrvString.h"

namespace mrv
{
    bool matches_chars(const char* src, const char* charlist)
    {
        const char* s = src;
        for (; *s != 0; ++s)
        {
            const char* d = charlist;
            bool found = false;
            for (; *d != 0; ++d)
            {
                if (*s == *d)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
        return true;
    }

    void split_string(
        stringArray& output, const std::string& str, const std::string& delim)
    {
        size_t offset = 0;
        size_t delimIndex = 0;

        delimIndex = str.find(delim, offset);

        output.clear();
        while (delimIndex != std::string::npos)
        {
            output.push_back(str.substr(offset, delimIndex - offset));
            offset += delimIndex - offset + delim.length();
            delimIndex = str.find(delim, offset);
        }

        output.push_back(str.substr(offset));
    }

    int64_t String::toInt() const
    {
        int64_t r = 0;
        sscanf(c_str(), "%" PRId64, &r);
        return r;
    }

    double String::toDouble() const
    {
        double r = 0;
        sscanf(c_str(), "%lg", &r);
        return r;
    }

} // namespace mrv
