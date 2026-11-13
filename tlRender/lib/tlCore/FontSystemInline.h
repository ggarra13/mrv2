// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <functional>
#include <string>

namespace tl
{
    namespace image
    {
        inline FontInfo::FontInfo() {}

        inline FontInfo::FontInfo(const std::string& family, int size) :
            family(family),
            size(size)
        {
        }

        inline bool FontInfo::operator==(const FontInfo& other) const
        {
            return family == other.family && size == other.size;
        }

        inline bool FontInfo::operator!=(const FontInfo& other) const
        {
            return !(*this == other);
        }

        inline bool FontInfo::operator<(const FontInfo& other) const
        {
            return std::tie(family, size) < std::tie(other.family, other.size);
        }

        inline GlyphInfo::GlyphInfo() {}

        inline GlyphInfo::GlyphInfo(uint32_t code, const FontInfo& fontInfo) :
            code(code),
            fontInfo(fontInfo)
        {
        }

        inline bool GlyphInfo::operator<(const GlyphInfo& other) const
        {
            return std::tie(code, fontInfo) <
                   std::tie(other.code, other.fontInfo);
        }
    } // namespace image
} // namespace tl

namespace std
{
    template <>
    struct hash<tl::image::FontInfo>
    {
        std::size_t operator()(const tl::image::FontInfo& value) const noexcept
        {
            std::size_t out = std::hash<std::string>{}(value.family);
            out ^= std::hash<int>{}(value.size) + 0x9e3779b97f4a7c15ULL +
                   (out << 6) + (out >> 2);
            return out;
        }
    };

    template <>
    struct hash<tl::image::GlyphInfo>
    {
        std::size_t operator()(const tl::image::GlyphInfo& value) const noexcept
        {
            std::size_t out = std::hash<uint32_t>{}(value.code);
            out ^= std::hash<tl::image::FontInfo>{}(value.fontInfo) +
                   0x9e3779b97f4a7c15ULL + (out << 6) + (out >> 2);
            return out;
        }
    };
}
