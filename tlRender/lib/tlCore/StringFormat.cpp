// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/StringFormat.h>

#include <tlCore/Error.h>

#include <iomanip>
#include <map>
#include <regex>
#include <sstream>

namespace tl
{
    namespace string
    {
        namespace
        {
            struct Match
            {
                Match() {}

                Match(std::ptrdiff_t pos, std::ptrdiff_t length) :
                    pos(pos),
                    length(length)
                {
                }

                std::ptrdiff_t pos = 0;
                std::ptrdiff_t length = 0;
            };

        } // namespace

        Format::Format(const std::string& value) :
            _text(value)
        {
        }

        Format& Format::arg(const std::string& value)
        {
            try
            {
                std::string subject = _text;
                std::regex r("\\{([0-9]+)\\}");
                std::smatch m;
                std::map<int, Match> matches;
                std::ptrdiff_t currentPos = 0;
                while (std::regex_search(subject, m, r))
                {
                    if (2 == m.size())
                    {
                        const int index = std::stoi(m[1]);
                        const auto i = matches.find(index);
                        if (i == matches.end())
                        {
                            const std::ptrdiff_t pos = m.position(0);
                            const std::ptrdiff_t len = m.length(0);
                            matches[std::stoi(m[1])] =
                                Match(currentPos + pos, len);
                            currentPos += pos + len;
                        }
                        else
                        {
                            throw std::invalid_argument("Duplicate argument");
                        }
                    }
                    else
                    {
                        throw error::ParseError();
                    }
                    subject = m.suffix().str();
                }
                if (matches.size() > 0)
                {
                    _text.replace(
                        matches.begin()->second.pos,
                        matches.begin()->second.length, value);
                }
                else
                {
                    throw std::invalid_argument("Argument not found");
                }
            }
            catch (const std::exception& e)
            {
                _error = e.what();
            }
            return *this;
        }

        Format& Format::arg(int value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        Format& Format::arg(int8_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width)
               << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(uint8_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width)
               << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(int16_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width)
               << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(uint16_t value, int width, char pad)
        {
            std::stringstream ss;
            ss << std::setfill(pad) << std::setw(width)
               << static_cast<int>(value);
            return arg(ss.str());
        }

        Format& Format::arg(float value, int precision, int width, char pad)
        {
            std::stringstream ss;
            if (precision >= 0)
            {
                ss.precision(precision);
                ss << std::fixed;
            }
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        Format& Format::arg(double value, int precision, int width, char pad)
        {
            std::stringstream ss;
            if (precision >= 0)
            {
                ss.precision(precision);
                ss << std::fixed;
            }
            ss << std::setfill(pad) << std::setw(width) << value;
            return arg(ss.str());
        }

        bool Format::hasError() const
        {
            return _error.size() > 0;
        }

        const std::string& Format::getError() const
        {
            return _error;
        }

        Format::operator std::string() const
        {
            return _text;
        }
    } // namespace string
} // namespace tl
