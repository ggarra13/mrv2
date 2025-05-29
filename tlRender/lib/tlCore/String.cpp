// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/String.h>

#include <algorithm>
#include <codecvt>
#include <locale>

namespace tl
{
    namespace string
    {
        std::vector<std::string>
        split(const std::string& s, char delimeter, SplitOptions options)
        {
            std::vector<std::string> out;
            bool word = false;
            std::size_t wordStart = 0;
            std::size_t i = 0;
            for (; i < s.size(); ++i)
            {
                if (s[i] != delimeter)
                {
                    if (!word)
                    {
                        word = true;
                        wordStart = i;
                    }
                }
                else
                {
                    if (word)
                    {
                        word = false;
                        out.push_back(s.substr(wordStart, i - wordStart));
                    }
                    if (SplitOptions::KeepEmpty == options && i > 0 &&
                        s[i - 1] == delimeter)
                    {
                        out.push_back(std::string());
                    }
                }
            }
            if (word)
            {
                out.push_back(s.substr(wordStart, i - wordStart));
            }
            return out;
        }

        std::vector<std::string> split(
            const std::string& s, const std::vector<char>& delimeters,
            SplitOptions options)
        {
            std::vector<std::string> out;
            bool word = false;
            std::size_t wordStart = 0;
            std::size_t i = 0;
            for (; i < s.size(); ++i)
            {
                if (std::find(delimeters.begin(), delimeters.end(), s[i]) ==
                    delimeters.end())
                {
                    if (!word)
                    {
                        word = true;
                        wordStart = i;
                    }
                }
                else
                {
                    if (word)
                    {
                        word = false;
                        out.push_back(s.substr(wordStart, i - wordStart));
                    }
                    if (SplitOptions::KeepEmpty == options && i > 0 &&
                        std::find(
                            delimeters.begin(), delimeters.end(), s[i - 1]) !=
                            delimeters.end())
                    {
                        out.push_back(std::string());
                    }
                }
            }
            if (word)
            {
                out.push_back(s.substr(wordStart, i - wordStart));
            }
            return out;
        }

        std::string join(const std::vector<std::string>& values, char delimeter)
        {
            std::string out;
            const std::size_t size = values.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                out += values[i];
                if (i < size - 1)
                {
                    out += delimeter;
                }
            }
            return out;
        }

        std::string join(
            const std::vector<std::string>& values,
            const std::string& delimeter)
        {
            std::string out;
            const std::size_t size = values.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                out += values[i];
                if (i < size - 1)
                {
                    out += delimeter;
                }
            }
            return out;
        }

        std::string toUpper(const std::string& value)
        {
            std::string out;
            for (auto i : value)
            {
                out.push_back(std::toupper(i));
            }
            return out;
        }

        std::string toLower(const std::string& value)
        {
            std::string out;
            for (auto i : value)
            {
                out.push_back(std::tolower(i));
            }
            return out;
        }

        void removeTrailingNewlines(std::string& value)
        {
            size_t size = value.size();
            while (size && ('\n' == value[size - 1] || '\r' == value[size - 1]))
            {
                value.pop_back();
                size = value.size();
            }
        }

        std::string removeTrailingNewlines(const std::string& value)
        {
            std::string out = value;
            removeTrailingNewlines(out);
            return out;
        }

        std::string elide(const std::string& value, size_t max)
        {
            std::string out = value.substr(0, max);
            if (out.size() < value.size())
            {
                out.push_back('.');
                out.push_back('.');
                out.push_back('.');
            }
            return out;
        }

        bool
        compare(const std::string& a, const std::string& b, Compare compare)
        {
            bool out = false;
            switch (compare)
            {
            case Compare::CaseSensitive:
                out = a == b;
                break;
            case Compare::CaseInsensitive:
                out = toLower(a) == toLower(b);
                break;
            default:
                break;
            }
            return out;
        }

        bool contains(
            const std::string& input, const std::string& substr,
            Compare compare)
        {
            size_t i = std::string::npos;
            switch (compare)
            {
            case Compare::CaseSensitive:
                i = input.find(substr);
                break;
            case Compare::CaseInsensitive:
                i = toLower(input).find(toLower(substr));
                break;
            default:
                break;
            }
            return i != std::string::npos;
        }

        std::string getLabel(bool value)
        {
            return value ? "True" : "False";
        }

        void fromString(const char* s, size_t size, int& out)
        {
            out = 0;

            // Find the sign.
            bool negativeSign = false;
            if ('-' == s[0])
            {
                negativeSign = true;
                ++s;
                --size;
            }
            else if ('+' == s[0])
            {
                ++s;
                --size;
            }

            // Find the end.
            size_t end = 0;
            for (; end < size && s[end]; ++end)
                ;

            // Add up the digits.
            int tens = 1;
            for (int i = int(end) - 1; i >= 0; --i, tens *= 10)
            {
                out += (s[i] - 48) * tens;
            }

            // Apply the sign.
            if (negativeSign)
            {
                out = -out;
            }
        }

        void fromString(const char* s, size_t size, int64_t& out)
        {
            out = 0;

            // Find the sign.
            bool negativeSign = false;
            if ('-' == s[0])
            {
                negativeSign = true;
                ++s;
                --size;
            }
            else if ('+' == s[0])
            {
                ++s;
                --size;
            }

            // Find the end.
            size_t end = 0;
            for (; end < size && s[end]; ++end)
                ;

            // Add up the digits.
            int64_t tens = 1;
            for (int i = int(end) - 1; i >= 0; --i, tens *= 10)
            {
                out += (static_cast<int64_t>(s[i] - 48)) * tens;
            }

            // Apply the sign.
            if (negativeSign)
            {
                out = -out;
            }
        }

        void fromString(const char* s, size_t size, size_t& out)
        {
            out = 0;

            // Add up the digits.
            size_t tens = 1;
            for (int i = int(size) - 1; i >= 0; --i, tens *= 10)
            {
                out += (static_cast<size_t>(s[i] - 48)) * tens;
            }
        }

        void fromString(const char* s, size_t size, float& out)
        {
            out = 0.F;

            // Find the sign.
            int isize = int(size);
            bool negativeSign = false;
            if ('-' == s[0])
            {
                negativeSign = true;
                ++s;
                --isize;
            }
            else if ('+' == s[0])
            {
                ++s;
                --isize;
            }

            // Find the engineering notation.
            int e = 0;
            for (int j = isize - 1; j >= 0; --j)
            {
                if ('e' == s[j] || 'E' == s[j])
                {
                    if (j < isize - 1)
                    {
                        fromString(
                            s + j + 1,
                            isize - static_cast<size_t>(j) -
                                static_cast<size_t>(1),
                            e);
                    }
                    isize = j;
                    break;
                }
            }

            // Find the decimal point.
            int decimalPoint = -1;
            for (int j = isize - 1; j >= 0; --j)
            {
                if ('.' == s[j])
                {
                    decimalPoint = j;
                    break;
                }
            }

            // Add up the digits.
            float tens = 1.F;
            for (int j = (decimalPoint != -1 ? decimalPoint : isize) - 1;
                 j >= 0; --j, tens *= 10.F)
            {
                out += (s[j] - 48) * tens;
            }

            // Add up the decimal digits.
            if (decimalPoint != -1)
            {
                tens = .1F;
                for (int j = decimalPoint + 1; j < isize; ++j, tens /= 10.F)
                {
                    out += (s[j] - 48) * tens;
                }
            }

            // Apply the engineering notation.
            if (e != 0)
            {
                tens = e < 0 ? .1F : 10.F;
                if (e < 0)
                    e = -e;
                for (int j = 0; j < e; ++j)
                {
                    out *= tens;
                }
            }

            // Apply the sign.
            if (negativeSign)
            {
                out = -out;
            }
        }

        std::wstring toWide(const std::string& value)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.from_bytes(value);
        }

        std::string fromWide(const std::wstring& value)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.to_bytes(value);
        }

        std::string escape(const std::string& value)
        {
            std::string out;
            for (const auto i : value)
            {
                if ('\\' == i)
                {
                    out.push_back('\\');
                }
                out.push_back(i);
            }
            return out;
        }

        std::string unescape(const std::string& value)
        {
            std::string out;
            const size_t size = value.size();
            for (size_t i = 0; i < size; ++i)
            {
                out.push_back(value[i]);
                if (i < size - 1 && '\\' == value[i] && '\\' == value[i + 1])
                {
                    ++i;
                }
            }
            return out;
        }
    } // namespace string
} // namespace tl
