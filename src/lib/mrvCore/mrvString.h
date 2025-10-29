// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <tlCore/String.h>

namespace mrv
{
    namespace string
    {
        using namespace tl::string;

        inline
        bool ends_with(const std::string& value, const std::string& ending)
        {
            if (ending.size() > value.size()) return false;
            return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
        }
        
        /**
         * Split a string on a char delimiter
         *
         * @param s     original string.
         * @param delim char delimiter.
         *
         * @return std::vector< std::string > to return.
         */
        inline std::vector< std::string >
        split(const std::string& s, char delim)
        {
            std::vector<std::string> out;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim))
            {
                out.push_back(item);
            }
            return out;
        }

        /**
         * Strip leading whitespace (' ', '\t', '\n' and '\r') from a string
         *
         * @param input string
         *
         * @return stripped string
         */
        inline std::string stripLeadingWhitespace(const std::string& input)
        {
            size_t start = 0;
            while (start < input.length() &&
                   (std::isspace(input[start]) || input[start] == '\n' ||
                    input[start] == '\r' || input[start] == '\t'))
            {
                start++;
            }

            return input.substr(start);
        }

        /**
         * Strip leading whitespace (' ', '\t', '\n' and '\r') from a string
         *
         * @param input string
         *
         * @return stripped string
         */
        inline std::string stripTrailingWhitespace(const std::string& input)
        {
            size_t endPos = input.find_last_not_of(" \r\n\t");

            if (endPos != std::string::npos)
            {
                return input.substr(0, endPos + 1);
            }
            else
            {
                // The string is entirely whitespace
                return "";
            }
        }

        inline std::string stripWhitespace(const std::string& s)
        {
            std::string out;
            out = stripLeadingWhitespace(s);
            out = stripTrailingWhitespace(out);
            return out;
        }

        inline std::string commentSlash(const std::string& s)
        {
            std::string out;
            for (const auto& c : s)
            {
                if (c == '/')
                {
                    out.push_back('\\');
                }
                out.push_back(c);
            }
            return out;
        }

        /**
         * Return a string with all the 'match' characters commented out with
         * with backslashes.
         */
        std::string
        commentCharacter(const std::string& input, const char match = '/');

        
        /**
         * @brief Class used to mimic Qt's string function so that it converts
         *       to integers or doubles.
         *
         */
        class String : public std::string
        {
        public:
            String() :
                std::string() {};
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

        struct CaseInsensitiveCompare
        {
            inline bool
            operator()(const std::string& a, const std::string& b) const
            {
                return string::toLower(a) < string::toLower(b);
            }
        };

    } // namespace string

} // namespace mrv
