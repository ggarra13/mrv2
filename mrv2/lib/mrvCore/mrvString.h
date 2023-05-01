// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <set>

typedef std::vector< std::string > stringArray;
typedef std::set< std::string > stringSet;

namespace mrv
{
    bool matches_chars(const char* src, const char* charlist);

    void split_string(
        stringArray& output, const std::string& str, const std::string& delim);

    /**
     * Split a string on a char delimiter, returning a vector of std::string.
     *
     * @param elems std::vector< std::string > to return.
     * @param s     original string.
     * @param delim char delimiter.
     */
    inline void split(stringArray& elems, const std::string& s, char delim)
    {
        std::stringstream ss(s);
        std::string item;
        elems.clear();
        while (std::getline(ss, item, delim))
        {
            elems.push_back(item);
        }
    }

    /**
     * @brief Class used to mimic Qt's string function so that it converts
     *       to integers or doubles.
     *
     */
    class String : public std::string
    {
    public:
        String() :
            std::string(){};
        String(const std::string& s) :
            std::string(s)
        {
        }
        String(const char* s) :
            std::string(s)
        {
        }

        int64_t toInt() const;
        double toDouble() const;
    };

} // namespace mrv
