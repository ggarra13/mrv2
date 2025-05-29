// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FontSystemTest.h>

#include <tlCore/Assert.h>
#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        FontSystemTest::FontSystemTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FontSystemTest", context)
        {
        }

        std::shared_ptr<FontSystemTest>
        FontSystemTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FontSystemTest>(new FontSystemTest(context));
        }

        void FontSystemTest::run()
        {
            for (const auto& font :
                 {"NotoMono-Regular", "NotoSans-Regular", "NotoSans-Bold"})
            {
                auto data = getFontData(font);
                TLRENDER_ASSERT(!data.empty());
            }
            {
                const FontInfo fi;
                TLRENDER_ASSERT("NotoSans-Regular" == fi.family);
                TLRENDER_ASSERT(12 == fi.size);
            }
            {
                const FontInfo fi("NotoMono-Regular", 14);
                TLRENDER_ASSERT("NotoMono-Regular" == fi.family);
                TLRENDER_ASSERT(14 == fi.size);
            }
            {
                FontInfo a;
                FontInfo b;
                TLRENDER_ASSERT(a == b);
            }
            {
                FontInfo a("NotoMono-Regular", 14);
                FontInfo b;
                TLRENDER_ASSERT(a < b);
            }
            {
                const GlyphInfo gi;
                TLRENDER_ASSERT(0 == gi.code);
                TLRENDER_ASSERT(FontInfo() == gi.fontInfo);
            }
            {
                const FontInfo fi("NotoMono-Regular", 14);
                const GlyphInfo gi(1, fi);
                TLRENDER_ASSERT(1 == gi.code);
                TLRENDER_ASSERT(fi == gi.fontInfo);
            }
            {
                GlyphInfo a;
                GlyphInfo b;
                TLRENDER_ASSERT(a == b);
            }
            {
                GlyphInfo a;
                GlyphInfo b(1, FontInfo("NotoMono-Regular", 14));
                TLRENDER_ASSERT(a < b);
            }
            auto fontSystem = _context->getSystem<image::FontSystem>();
            for (auto fontSize : {14, 0})
            {
                _print(string::Format("Font size: {0}").arg(fontSize));
                FontInfo fi("NotoMono-Regular", fontSize);
                auto fm = fontSystem->getMetrics(fi);
                std::vector<std::string> text = {
                    "Hello world!", "Hello\nworld!", "Hello world!"};
                std::vector<uint16_t> maxLineWidth = {0, 0, 1};
                for (size_t i = 0; i < text.size(); ++i)
                {
                    _print(string::Format("Text: {0}").arg(text[i]));
                    const math::Size2i size =
                        fontSystem->getSize(text[i], fi, maxLineWidth[i]);
                    _print(string::Format("Size: {0}").arg(size));
                    const auto boxes =
                        fontSystem->getBox(text[i], fi, maxLineWidth[i]);
                    TLRENDER_ASSERT(text[i].size() == boxes.size());
                    for (size_t j = 0; j < text[i].size(); ++j)
                    {
                        _print(string::Format("Box '{0}': {1}")
                                   .arg(text[i][j])
                                   .arg(boxes[j]));
                    }
                    const auto glyphs = fontSystem->getGlyphs(text[i], fi);
                    TLRENDER_ASSERT(text[i].size() == glyphs.size());
                    for (size_t j = 0; j < text[i].size(); ++j)
                    {
                        image::Size size;
                        if (glyphs[j] && glyphs[j]->image)
                        {
                            size = glyphs[j]->image->getSize();
                        }
                        _print(string::Format("Glyph '{0}': {1}")
                                   .arg(text[i][j])
                                   .arg(size));
                    }
                    _print(string::Format("Glyph cache size: {0}")
                               .arg(fontSystem->getGlyphCacheSize()));
                    _print(string::Format("Glyph cache percentage: {0}%")
                               .arg(fontSystem->getGlyphCachePercentage()));
                }
            }
        }
    } // namespace core_tests
} // namespace tl
