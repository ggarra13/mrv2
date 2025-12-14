// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FontSystem.h>

#include <tlCore/Context.h>
#include <tlCore/LRUCache.h>
#include <tlCore/Path.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <algorithm>
#include <filesystem>
#include <limits>
#include <locale>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

#include <FL/fl_utf8.h>

#ifdef _WIN32
# define NOMINMAX
# include <windows.h>
# include <shlobj.h>  // SHGetKnownFolderPath
# include <knownfolders.h>
#else
# include <cstdlib>  // getenv
# include <pwd.h>
# include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace
{
    typedef wchar_t tl_char_t;

    std::u32string utf8_to_utf32(const std::string& utf8_str)
    {
        const char* src = utf8_str.c_str();
        const char* end = src + utf8_str.size();
        std::u32string utf32;
        utf32.reserve(utf8_str.size());
        while (*src) {
            int len = 0;
            uint32_t ucs = fl_utf8decode(src, end, &len);
            // vvv --- optional
            if (*src & 0x80) {            // what should be a multibyte encoding
                if (len==1)
                    ucs = 0xFFFD;   // Turn errors into REPLACEMENT CHARACTER
            }
            // ^^^ --- optional
            utf32.push_back(ucs);
            src += len;
        }
        return utf32;
    }

    std::string utf32_to_utf8(const std::u32string& utf32_str)
    {
        std::string utf8;
        utf8.reserve(utf32_str.size());
        for (auto &ucs: utf32_str) {
            char buf[6];
            int len = fl_utf8encode(ucs, buf);
            utf8.append(buf, len);
        }
        return utf8;
    }
    
}

namespace
{
    std::vector<fs::path> getSystemFontPaths()
    {
        std::vector<fs::path> paths;

#ifdef _WIN32
        PWSTR fontDirW = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr,
                                           &fontDirW))) {
            char fontDir[MAX_PATH];
            wcstombs(fontDir, fontDirW, MAX_PATH);
            CoTaskMemFree(fontDirW);
            paths.emplace_back(fontDir);
        } else {
            paths.emplace_back("C:\\Windows\\Fonts\\");
        }

#elif __APPLE__
        const char* home = getenv("HOME");
        if (home) {
            paths.emplace_back(std::string(home) + "/Library/Fonts/");
        }
        paths.emplace_back("/System/Library/Fonts/");
        paths.emplace_back("/Library/Fonts/");

#else  // Linux / Unix
        const char* home = getenv("HOME");
        if (home) {
            paths.emplace_back(std::string(home) + "/.fonts/");
            paths.emplace_back(std::string(home) + "/.local/share/fonts/");
        }
        paths.emplace_back("/usr/share/fonts/");
        paths.emplace_back("/usr/local/share/fonts/");

#endif

        return paths;
    }

}

namespace tl
{
    namespace image
    {
        namespace
        {
#include <Fonts/NotoMono-Regular.font>
#include <Fonts/NotoSans-Regular.font>
#include <Fonts/NotoSans-Bold.font>
        } // namespace

        std::vector<uint8_t> getFontData(const std::string& name)
        {
            std::vector<uint8_t> out;
            if ("NotoMono-Regular" == name)
            {
                out.resize(NotoMono_Regular_ttf_len);
                memcpy(
                    out.data(), NotoMono_Regular_ttf, NotoMono_Regular_ttf_len);
            }
            else if ("NotoSans-Regular" == name)
            {
                out.resize(NotoSans_Regular_ttf_len);
                memcpy(
                    out.data(), NotoSans_Regular_ttf, NotoSans_Regular_ttf_len);
            }
            else if ("NotoSans-Bold" == name)
            {
                out.resize(NotoSans_Bold_ttf_len);
                memcpy(out.data(), NotoSans_Bold_ttf, NotoSans_Bold_ttf_len);
            }
            return out;
        }

        struct FontSystem::Private
        {
            std::shared_ptr<Glyph> getGlyph(uint32_t code, const FontInfo&);

            std::map<std::string, std::vector<uint8_t> > fontData;
            FT_Library ftLibrary = nullptr;
            std::map<std::string, FT_Face> ftFaces;
            void measure(
                const std::u32string& utf32, const FontInfo&,
                int maxLineWidth, math::Size2i&,
                std::vector<math::Box2i>* = nullptr);
            memory::LRUCache<GlyphInfo, std::shared_ptr<Glyph> > glyphCache;
        };

        void FontSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::image::FontSystem", context);
            TLRENDER_P();

            try
            {
                FT_Error ftError = FT_Init_FreeType(&p.ftLibrary);
                if (ftError)
                {
                    throw std::runtime_error("FreeType cannot be initialized");
                }

                ftError = FT_New_Memory_Face(
                    p.ftLibrary, NotoSans_Regular_ttf, NotoSans_Regular_ttf_len,
                    0, &p.ftFaces["NotoSans-Regular"]);
                if (ftError)
                {
                    throw std::runtime_error("Cannot create font");
                }
                ftError = FT_New_Memory_Face(
                    p.ftLibrary, NotoSans_Bold_ttf, NotoSans_Bold_ttf_len, 0,
                    &p.ftFaces["NotoSans-Bold"]);
                if (ftError)
                {
                    throw std::runtime_error("Cannot create font");
                }
                ftError = FT_New_Memory_Face(
                    p.ftLibrary, NotoMono_Regular_ttf, NotoMono_Regular_ttf_len,
                    0, &p.ftFaces["NotoMono-Regular"]);
                if (ftError)
                {
                    throw std::runtime_error("Cannot create font");
                }
            }
            catch (const std::exception& e)
            {
                _log(e.what(), log::Type::Error);
            }
        }

        FontSystem::FontSystem() :
            _p(new Private)
        {
        }

        FontSystem::~FontSystem()
        {
            TLRENDER_P();
            if (p.ftLibrary)
            {
                for (const auto& i : p.ftFaces)
                {
                    FT_Done_Face(i.second);
                }
                FT_Done_FreeType(p.ftLibrary);
            }
        }

        std::shared_ptr<FontSystem>
        FontSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FontSystem>(new FontSystem);
            out->_init(context);
            return out;
        }

        bool FontSystem::hasFont(const std::string& name)
        {
            TLRENDER_P();
            return p.fontData.find(name) != p.fontData.end();
        }
        
        void FontSystem::addFont(
            const std::string& name, const uint8_t* data, size_t size)
        {
            TLRENDER_P();
            p.fontData[name] = std::vector<uint8_t>(size);
            memcpy(p.fontData[name].data(), data, size);
            FT_Error ftError = FT_New_Memory_Face(
                p.ftLibrary, p.fontData[name].data(), size, 0,
                &p.ftFaces[name]);
            if (ftError)
            {
                _log("Cannot create font", log::Type::Error);
            }
        }

        void FontSystem::addFont(const fs::path& filePath)
        {
            TLRENDER_P();
            
            file::Path fileName(filePath.filename().generic_string());
            const std::string name = fileName.getBaseName();
            if (p.fontData.find(name) != p.fontData.end())
                return;
            
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if (!file) {
                std::cerr << "Failed to open font: " << filePath << std::endl;
                return;
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer(size);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                std::cerr << "Failed to read font: " << filePath << std::endl;
                return;
            }

            
            addFont(name, buffer.data(), buffer.size());
        }

        size_t FontSystem::getGlyphCacheSize() const
        {
            return _p->glyphCache.getSize();
        }

        float FontSystem::getGlyphCachePercentage() const
        {
            return _p->glyphCache.getPercentage();
        }

        FontMetrics FontSystem::getMetrics(const FontInfo& info)
        {
            TLRENDER_P();
            FontMetrics out;
            const auto i = p.ftFaces.find(info.family);
            if (i != p.ftFaces.end())
            {
                FT_Error ftError = FT_Set_Pixel_Sizes(i->second, 0, info.size);
                if (ftError)
                {
                    _log("Cannot set pixel sizes", log::Type::Error);
                }
                out.ascender = i->second->size->metrics.ascender / 64;
                out.descender = i->second->size->metrics.descender / 64;
                out.lineHeight = i->second->size->metrics.height / 64;
            }
            return out;
        }

        math::Size2i FontSystem::getSize(
            const std::string& text, const FontInfo& fontInfo, int maxLineWidth)
        {
            TLRENDER_P();
            math::Size2i out;
            try
            {
                 const auto utf32 = utf8_to_utf32(text);
                 p.measure(utf32, fontInfo, maxLineWidth, out);
            }
            catch (const std::exception& e)
            {
                _log(e.what(), log::Type::Error);
            } 
            return out;
        }

        std::vector<math::Box2i> FontSystem::getBox(
            const std::string& text, const FontInfo& fontInfo, int maxLineWidth)
        {
            TLRENDER_P();
            std::vector<math::Box2i> out;
            try
            {
                const auto utf32 = utf8_to_utf32(text);
                math::Size2i size;
                p.measure(utf32, fontInfo, maxLineWidth, size, &out);
            }
            catch (const std::exception& e)
            {
                _log(e.what(), log::Type::Error);
            }
            return out;
        }

        std::vector<std::shared_ptr<Glyph> >
        FontSystem::getGlyphs(const std::string& text, const FontInfo& fontInfo)
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<Glyph> > out;
            try
            {
                const auto utf32 = utf8_to_utf32(text);
                for (const auto& i : utf32)
                {
                    out.push_back(p.getGlyph(i, fontInfo));
                }
            }
            catch (const std::exception& e)
            {
                _log(e.what(), log::Type::Error);
            }
            return out;
        }

        std::shared_ptr<Glyph>
        FontSystem::Private::getGlyph(uint32_t code, const FontInfo& fontInfo)
        {
            std::shared_ptr<Glyph> out;
            if (!glyphCache.get(GlyphInfo(code, fontInfo), out))
            {
                const auto i = ftFaces.find(fontInfo.family);
                if (i != ftFaces.end())
                {
                    FT_Error ftError = FT_Set_Pixel_Sizes(
                        i->second, 0, static_cast<int>(fontInfo.size));
                    if (ftError)
                    {
                        throw std::runtime_error("Cannot set pixel sizes");
                    }
                    if (auto ftGlyphIndex = FT_Get_Char_Index(i->second, code))
                    {
                        ftError = FT_Load_Glyph(
                            i->second, ftGlyphIndex, FT_LOAD_FORCE_AUTOHINT);
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot load glyph");
                        }
                        FT_Render_Mode renderMode = FT_RENDER_MODE_NORMAL;
                        uint8_t renderModeChannels = 1;
                        ftError = FT_Render_Glyph(i->second->glyph, renderMode);
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot render glyph");
                        }

                        out = std::make_shared<Glyph>();
                        out->info = GlyphInfo(code, fontInfo);
                        auto ftBitmap = i->second->glyph->bitmap;
                        const image::Info imageInfo(
                            ftBitmap.width, ftBitmap.rows,
                            image::PixelType::L_U8);
                        out->image = image::Image::create(imageInfo);
                        for (size_t y = 0; y < ftBitmap.rows; ++y)
                        {
                            uint8_t* dataP =
                                out->image->getData() + ftBitmap.width * y;
                            const unsigned char* bitmapP =
                                ftBitmap.buffer + y * ftBitmap.pitch;
                            for (size_t x = 0; x < ftBitmap.width; ++x)
                            {
                                dataP[x] = bitmapP[x];
                            }
                        }
                        out->offset = math::Vector2i(
                            i->second->glyph->bitmap_left,
                            i->second->glyph->bitmap_top);
                        out->advance = i->second->glyph->advance.x / 64;
                        out->lsbDelta = i->second->glyph->lsb_delta;
                        out->rsbDelta = i->second->glyph->rsb_delta;

                        glyphCache.add(out->info, out);
                    }
                }
            }
            return out;
        }

        namespace
        {
            constexpr bool isSpace(tl_char_t c)
            {
                return ' ' == c || '\t' == c;
            }

            constexpr bool isNewline(tl_char_t c)
            {
                return '\n' == c || '\r' == c;
            }
        } // namespace

        // Defined in C++ 11
        void FontSystem::Private::measure(
            const std::u32string& utf32, const FontInfo& fontInfo,
            int maxLineWidth, math::Size2i& size,
            std::vector<math::Box2i>* glyphGeom)
        {
            const auto i = ftFaces.find(fontInfo.family);
            if (i != ftFaces.end())
            {
                math::Vector2i pos;
                FT_Error ftError = FT_Set_Pixel_Sizes(
                    i->second, 0, static_cast<int>(fontInfo.size));
                if (ftError)
                {
                    throw std::runtime_error("Cannot set pixel sizes");
                }

                const int h = i->second->size->metrics.height / 64;
                pos.y = h;
                auto textLine = utf32.end();
                int textLineX = 0;
                int32_t rsbDeltaPrev = 0;
                for (auto j = utf32.begin(); j != utf32.end(); ++j)
                {
                    const auto glyph = getGlyph(*j, fontInfo);
                    if (glyphGeom)
                    {
                        math::Box2i box;
                        if (glyph)
                        {
                            box = math::Box2i(
                                pos.x, pos.y - h, glyph->advance, h);
                        }
                        glyphGeom->push_back(box);
                    }

                    int32_t x = 0;
                    math::Vector2i posAndSize;
                    if (glyph)
                    {
                        x = glyph->advance;
                        if (rsbDeltaPrev - glyph->lsbDelta > 32)
                        {
                            x -= 1;
                        }
                        else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                        {
                            x += 1;
                        }
                        rsbDeltaPrev = glyph->rsbDelta;
                    }
                    else
                    {
                        rsbDeltaPrev = 0;
                    }

                    if (isNewline(*j))
                    {
                        size.w = std::max(size.w, pos.x);
                        pos.x = 0;
                        pos.y += h;
                        rsbDeltaPrev = 0;
                    }
                    else if (
                        maxLineWidth > 0 && pos.x > 0 &&
                        pos.x + (!isSpace(*j) ? x : 0) >= maxLineWidth)
                    {
                        if (textLine != utf32.end())
                        {
                            j = textLine;
                            textLine = utf32.end();
                            size.w = std::max(size.w, textLineX);
                            pos.x = 0;
                            pos.y += h;
                        }
                        else
                        {
                            size.w = std::max(size.w, pos.x);
                            pos.x = x;
                            pos.y += h;
                        }
                        rsbDeltaPrev = 0;
                    }
                    else
                    {
                        if (isSpace(*j) && j != utf32.begin())
                        {
                            textLine = j;
                            textLineX = pos.x;
                        }
                        pos.x += x;
                    }
                }
                size.w = std::max(size.w, pos.x);
                size.h = pos.y;
            }
        }

        std::vector<fs::path> discoverSystemFonts()
        {
            std::vector<fs::path> out;
            const std::vector<fs::path> fontDirs = getSystemFontPaths();

            for (const auto& dir : fontDirs)
            {
                if (!fs::exists(dir) || !fs::is_directory(dir))
                    continue;

                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (!entry.is_regular_file())
                        continue;

                    const auto ext = entry.path().extension().string();
                    if (ext == ".ttf" || ext == ".otf" || ext == ".ttc")
                    {
                        out.emplace_back(entry.path());
                    }
                }
            }
            
            // Sort by filename
            std::sort(out.begin(), out.end(),
                      [](const fs::path& a, const fs::path& b)
                          {
                              return (a.filename().string() <
                                      b.filename().string());
                      });
            return out;
        }

    } // namespace image
} // namespace tl
