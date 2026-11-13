// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

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

        inline bool GlyphInfo::operator==(const GlyphInfo& other) const
        {
            return code == other.code && fontInfo == other.fontInfo;
        }

        inline bool GlyphInfo::operator!=(const GlyphInfo& other) const
        {
            return !(*this == other);
        }

        inline bool GlyphInfo::operator<(const GlyphInfo& other) const
        {
            return std::tie(code, fontInfo) <
                   std::tie(other.code, other.fontInfo);
        }
    } // namespace image
} // namespace tl
