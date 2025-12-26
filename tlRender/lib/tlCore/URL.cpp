// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/URL.h>

#include <iomanip>
#include <regex>
#include <sstream>

namespace tl
{
    namespace url
    {
        std::string scheme(const std::string& url)
        {
            const std::regex rx("^([A-Za-z0-9+-\\.]+://)");
            const auto rxi = std::sregex_iterator(url.begin(), url.end(), rx);
            return rxi != std::sregex_iterator() ? rxi->str() : std::string();
        }

        std::string encode(const std::string& url)
        {
            // Don't encode these characters.
            const std::vector<char> chars =
            {
                '-', '.', '_', '~', ':', '/', '?',  '#',
                '[', ']', '@', '!', '$', '&', '\'', '(',
                ')', '*', '+', ',', ';', '=', '\\'
            };

            // Copy characters to the result, encoding if necessary.
            std::stringstream ss;
            ss.fill('0');
            ss << std::hex;
            for (auto i = url.begin(), end = url.end(); i != end; ++i)
            {
                const auto j = std::find(chars.begin(), chars.end(), *i);
                if (std::isalnum(*i) || j != chars.end())
                {
                    ss << *i;
                }
                else
                {
                    ss << '%' << std::setw(2) << int(*i);
                }
            }
            return ss.str();
        }

        std::string decode(const std::string& url)
        {
            std::string out;

            // Find all percent encodings.
            size_t pos = 0;
            const std::regex rx("(%[0-9A-Fa-f][0-9A-Fa-f])");
            for (auto i = std::sregex_iterator(url.begin(), url.end(), rx);
                i != std::sregex_iterator();
                ++i)
            {
                // Copy parts without any encodings.
                if (pos != static_cast<size_t>(i->position()))
                {
                    out.append(url.substr(pos, i->position() - pos));
                    pos = i->position();
                }

                // Convert the encoding and append it.
                std::stringstream ss;
                ss << std::hex << i->str().substr(1);
                unsigned int j = 0;
                ss >> j;
                out.push_back(char(j));
                pos += i->str().size();
            }

            // Copy the remainder without any encodings.
            if (!url.empty() && pos != url.size() - 1)
            {
                out.append(url.substr(pos, url.size() - pos));
            }

            return out;
        }
    }
}
