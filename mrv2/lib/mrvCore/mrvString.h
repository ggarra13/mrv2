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

    } // namespace string

} // namespace mrv
