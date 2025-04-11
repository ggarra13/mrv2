// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Image.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace image
    {
        //! Get font data.
        std::vector<uint8_t> getFontData(const std::string&);

        //! Font information.
        struct FontInfo
        {
            FontInfo();
            FontInfo(const std::string& family, int size);

            std::string family = "NotoSans-Regular";
            int size = 12;

            bool operator==(const FontInfo&) const;
            bool operator!=(const FontInfo&) const;
            bool operator<(const FontInfo&) const;
        };

        //! Font metrics.
        struct FontMetrics
        {
            int ascender = 0;
            int descender = 0;
            int lineHeight = 0;
        };

        //! Font glyph information.
        struct GlyphInfo
        {
            GlyphInfo();
            GlyphInfo(uint32_t code, const FontInfo&);

            uint32_t code = 0;
            FontInfo fontInfo;

            bool operator==(const GlyphInfo&) const;
            bool operator!=(const GlyphInfo&) const;
            bool operator<(const GlyphInfo&) const;
        };

        //! Font glyph.
        struct Glyph
        {
            GlyphInfo info;
            std::shared_ptr<image::Image> image;
            math::Vector2i offset;
            int advance = 0;
            int32_t lsbDelta = 0;
            int32_t rsbDelta = 0;
        };

        //! Font system.
        //!
        //! \todo Add text elide functionality.
        //! \todo Add support for gamma correction?
        //! -
        //! https://www.freetype.org/freetype2/docs/text-rendering-general.html
        class FontSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(FontSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            FontSystem();

        public:
            virtual ~FontSystem();

            //! Create a new system.
            static std::shared_ptr<FontSystem>
            create(const std::shared_ptr<system::Context>&);

            //! Add a font.
            void addFont(const std::string& name, const uint8_t*, size_t);

            //! \name Information
            ///@{

            //! Get the glyph cache size.
            size_t getGlyphCacheSize() const;

            //! Get the percentage of the glyph cache in use.
            float getGlyphCachePercentage() const;

            ///@}

            //! \name Measure
            ///@{

            //! Get font metrics.
            FontMetrics getMetrics(const FontInfo&);

            //! Get the size of text.
            math::Size2i
            getSize(const std::string&, const FontInfo&, int maxLineWidth = 0);

            //! Get the character boxes.
            std::vector<math::Box2i>
            getBox(const std::string&, const FontInfo&, int maxLineWidth = 0);

            ///@}

            //! \name Glyphs
            ///@{

            //! Get glyphs.
            std::vector<std::shared_ptr<Glyph> >
            getGlyphs(const std::string&, const FontInfo&);

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace image
} // namespace tl

#include <tlCore/FontSystemInline.h>
