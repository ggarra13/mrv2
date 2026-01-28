// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace mrv
{
    namespace string
    {
        /** 
         * Convert a utf8 string to a utf16 string as used in Windows APIs
         * 
         * @param command as utf8 string
         * 
         * @return Same command as utf16 string
         */
        std::wstring convert_utf8_to_utf16(const std::string& utf8_command);

    } // namespace string

} // namespace mrv
