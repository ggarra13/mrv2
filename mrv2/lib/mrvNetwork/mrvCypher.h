// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    std::string encode_string(const std::string& text);
    std::string decode_string(const std::string& encodedText);
} // namespace mrv
