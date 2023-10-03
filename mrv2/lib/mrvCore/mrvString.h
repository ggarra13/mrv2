// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace mrv
{
    /**
     * Given a source string, see if it matches all characters in charlist
     *
     * @param src source string to see if there are matches
     * @param charlist list of characters to match
     *
     * @return true if all characters in src are in charlist.
     */
    bool matches_chars(const char* src, const char* charlist);

    /**
     * Split string on a delimiter and return a std::vector< std::string >
     *
     * @param output returned std::vector< std::string > array.
     * @param str    original string to split.
     * @param delim  delimiter to use for splitting the string.
     */
    void split_string(
        std::vector<std::string>& output, const std::string& str,
        const std::string& delim);

    /**
     * Split a string on a char delimiter, returning a vector of std::string.
     *
     * @param elems std::vector< std::string > to return.
     * @param s     original string.
     * @param delim char delimiter.
     */
    inline void
    split(std::vector<std::string>& elems, const std::string& s, char delim)
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
