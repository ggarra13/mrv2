// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
            bool found    = false;
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
        size_t offset     = 0;
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
