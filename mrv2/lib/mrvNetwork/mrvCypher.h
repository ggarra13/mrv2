
#pragma once

#include <string>

namespace mrv
{
    std::string encode_string(const std::string& text);
    std::string decode_string(const std::string& encodedText);
} // namespace mrv
