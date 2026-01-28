// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvString.h"

#include <FL/fl_utf8.h>

namespace mrv
{
    namespace string
    {
        std::wstring convert_utf8_to_utf16(const std::string& utf8_command)
        {
            if (utf8_command.empty())
            {
                return L"";
            }

            // 1. Determine the required size of the UTF-16 buffer.
            // The 'to' argument is passed as NULL to just calculate the length.
            // The length returned is the number of wide characters (wchar_t), 
            // including the null terminator.
            int wide_len = fl_utf8toUtf16(
                utf8_command.c_str(), 
                static_cast<unsigned>(utf8_command.length()),
                nullptr, 
                0);

            if (wide_len <= 0) {
                // Handle conversion error or empty string case
                return L""; 
            }

            // 2. Allocate the buffer and perform the conversion.
            // We use a std::vector<wchar_t> to manage the memory.
            std::vector<unsigned short> wide_buffer(wide_len + 1);

            fl_utf8toUtf16(
                utf8_command.c_str(), 
                static_cast<unsigned>(utf8_command.length()),
                wide_buffer.data(),
                wide_len + 1 // The size of the output buffer in bytes
                );

            // 3. Construct the std::wstring from the buffer (excluding the null terminator).
            return std::wstring(reinterpret_cast<const wchar_t*>(wide_buffer.data()), wide_len);
        }
        
    } // namespace string
} // namespace mrv
