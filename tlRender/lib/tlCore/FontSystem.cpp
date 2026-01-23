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

#define USE_HARFBUZZ 1

namespace
{
    typedef wchar_t tl_char_t;

    constexpr bool isSpace(tl_char_t c)
    {
        return ' ' == c || '\t' == c;
    }

    constexpr bool isNewline(tl_char_t c)
    {
        return '\n' == c || '\r' == c;
    }

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
    
    FT_Error setFacePixelSize(FT_Face face, unsigned int size,
                              float& scale)
    {
        scale = 1.F;
        
        // If the font is scalable (TrueType/OpenType outlines), use the
        // requested size directly.
        if (FT_IS_SCALABLE(face))
        {
            return FT_Set_Pixel_Sizes(face, 0, size);
        }
    
        // If the font is NOT scalable (Bitmap/Emoji), snap to the closest
        // available fixed size.
        if (face->num_fixed_sizes > 0)
        {
            int bestIndex = 0;
            int minDiff = std::numeric_limits<int>::max();
        
            for (int i = 0; i < face->num_fixed_sizes; ++i)
            {
                // For bitmap fonts, check both height and y_ppem
                // y_ppem is usually more reliable for matching
                int height = face->available_sizes[i].height;
            
                // Some fonts store size info in y_ppem (26.6 fixed point)
                int ppem = face->available_sizes[i].y_ppem >> 6;
            
                // Use whichever gives better information
                int sizeToCompare = (ppem > 0) ? ppem : height;
            
                int diff = std::abs(sizeToCompare - static_cast<int>(size));
                if (diff < minDiff)
                {
                    minDiff = diff;
                    bestIndex = i;
                    scale = static_cast<float>(size) / static_cast<float>(sizeToCompare);                                  
                }
            }
            
            FT_Error err = FT_Select_Size(face, bestIndex);        
            return err;
        }

        return FT_Err_Invalid_Pixel_Size;
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

        std::shared_ptr<image::Image> scaleBitmap(
            const std::shared_ptr<image::Image>& source,
            int targetWidth, int targetHeight)
        {
            if (!source || targetWidth <= 0 || targetHeight <= 0)
                return source;
    
            const auto& srcInfo = source->getInfo();
            int srcWidth = srcInfo.size.w;
            int srcHeight = srcInfo.size.h;
    
            // If already the right size, return as-is
            if (srcWidth == targetWidth && srcHeight == targetHeight)
                return source;
    
            // Create new image at target size
            image::Info dstInfo(targetWidth, targetHeight, srcInfo.pixelType);
            auto dst = image::Image::create(dstInfo);
    
            const uint8_t* srcData = source->getData();
            uint8_t* dstData = dst->getData();
    
            int bytesPerPixel = (srcInfo.pixelType == image::PixelType::RGBA_U8) ? 4 : 1;
    
            // Simple nearest-neighbor scaling (fast, reasonable for small sizes)
            for (int dy = 0; dy < targetHeight; ++dy)
            {
                for (int dx = 0; dx < targetWidth; ++dx)
                {
                    // Map destination pixel to source pixel
                    int sx = (dx * srcWidth) / targetWidth;
                    int sy = (dy * srcHeight) / targetHeight;
            
                    // Clamp to valid range
                    sx = std::min(sx, srcWidth - 1);
                    sy = std::min(sy, srcHeight - 1);
            
                    // Copy pixel
                    const uint8_t* srcPixel = srcData + (sy * srcWidth + sx) * bytesPerPixel;
                    uint8_t* dstPixel = dstData + (dy * targetWidth + dx) * bytesPerPixel;
            
                    for (int b = 0; b < bytesPerPixel; ++b)
                    {
                        dstPixel[b] = srcPixel[b];
                    }
                }
            }
    
            return dst;
        }
        
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
            std::shared_ptr<Glyph> getGlyphByIndex(uint32_t glyphIndex,
                                                   const FontInfo&);
            FT_Face getFTFace(const FontInfo&);
            void measure(
                const std::u32string& utf32, const FontInfo&,
                int maxLineWidth, math::Size2i&,
                std::vector<math::Box2i>* = nullptr);
            std::map<std::string, std::vector<uint8_t> > fontData;
            FT_Library ftLibrary = nullptr;
            std::map<std::string, FT_Face> ftFaces;
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
        FT_Face FontSystem::Private::getFTFace(const FontInfo& fontInfo)
        {
            const auto i = ftFaces.find(fontInfo.family);
            if (i != ftFaces.end())
            {
                return i->second;
            }
            throw std::runtime_error("Font was not found in cache");
        }

        FontMetrics FontSystem::getMetrics(const FontInfo& fontInfo)
        {
            TLRENDER_P();
            FontMetrics out;
            FT_Face face = p.getFTFace(fontInfo);
            
            float scale;
            FT_Error ftError = setFacePixelSize(face, fontInfo.size, scale);
            if (ftError)
            {
                _log("Cannot set font size for metrics", log::Type::Error);
                // Optional: fall back to face defaults or return zeroed metrics
            }
            out.ascender = face->size->metrics.ascender / 64;
            out.descender = face->size->metrics.descender / 64;
            out.lineHeight = face->size->metrics.height / 64;
            return out;
        }

#ifdef USE_HARFBUZZ

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

        struct ShapedGlyph
        {
            hb_codepoint_t glyphIndex = 0;
            hb_position_t xAdvance;
            hb_position_t yAdvance;
            hb_position_t xOffset;
            hb_position_t yOffset;
            uint32_t cluster;
        };

        std::vector<ShapedGlyph> shapeText(const std::u32string& text,
                                           const FontInfo& fontInfo,
                                           FT_Face face)
        {
            hb_font_t* hbFont = hb_ft_font_create_referenced(face);
            
            hb_buffer_t* buf = hb_buffer_create();
            hb_buffer_add_utf32(buf, (const uint32_t*)text.data(), text.size(), 0, text.size());
    
            hb_buffer_guess_segment_properties(buf);
    
            hb_shape(hbFont, buf, NULL, 0);
    
            unsigned int glyphCount;
            hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
            hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);
    
            std::vector<ShapedGlyph> result;
            for (unsigned int i = 0; i < glyphCount; ++i)
            {
                result.push_back({
                        glyphInfo[i].codepoint, 
                        glyphPos[i].x_advance / 64,
                        glyphPos[i].y_advance / 64,
                        glyphPos[i].x_offset / 64,
                        glyphPos[i].y_offset / 64,
                        glyphInfo[i].cluster
                    });
            }
    
            hb_buffer_destroy(buf);
            hb_font_destroy(hbFont);
            return result;
        }

        void FontSystem::Private::measure(
            const std::u32string& utf32, const FontInfo& fontInfo,
            int maxLineWidth, math::Size2i& size,
            std::vector<math::Box2i>* glyphGeom)
        {
            size = math::Size2i(0, 0);
            if (utf32.empty())
            {
                return;
            }

            FT_Face face = getFTFace(fontInfo);
            float scale;
            FT_Error err = setFacePixelSize(face, fontInfo.size, scale);
            if (err)
            {
                // Handle error (log or throw)
                return;
            }

            int h = face->size->metrics.height >> 6;

            // Split text into hard lines (explicit newlines)
            std::vector<std::u32string> lines;
            std::u32string current;
            for (auto c : utf32)
            {
                if (isNewline(c))
                {
                    lines.push_back(current);
                    current.clear();
                }
                else
                {
                    current.push_back(c);
                }
            }
            lines.push_back(current);

            int pos_y = 0;
            int max_w = 0;

            for (const auto& line : lines)
            {
                pos_y += h;

                auto shaped = shapeText(line, fontInfo, face);

                auto j = shaped.begin();
                double pos_x = 0.0;
                double current_line_w = 0.0;

                auto textLine = shaped.end();
                double textLineX = 0.0;

                while (j != shaped.end())
                {
                    const auto& sg = *j;
                    double x = sg.xAdvance;

                    bool space = (sg.cluster < line.size() && isSpace(line[sg.cluster]));

                    if (glyphGeom)
                    {
                        math::Box2i box(
                            static_cast<int>(pos_x),
                            pos_y - h,
                            static_cast<int>(x),
                            h);
                        glyphGeom->push_back(box);
                    }

                    if (maxLineWidth > 0 && pos_x > 0 && pos_x + (space ? 0.0 : x) >= maxLineWidth)
                    {
                        if (textLine != shaped.end())
                        {
                            j = textLine;
                            current_line_w = std::max(current_line_w, textLineX);
                            pos_x = 0.0;
                            pos_y += h;
                            continue;
                        }
                        else
                        {
                            current_line_w = std::max(current_line_w, pos_x);
                            pos_x = x;
                            pos_y += h;
                            continue;
                         }
                    }
                    else
                    {
                        if (space)
                        {
                            textLine = j;
                            textLineX = pos_x;
                        }
                        pos_x += x;
                    }

                    ++j;
                }

                current_line_w = std::max(current_line_w, pos_x);
                max_w = std::max(max_w, static_cast<int>(current_line_w));
            }

            size.w = max_w;
            size.h = pos_y;
        }
        
#else
        void FontSystem::Private::measure(
            const std::u32string& utf32, const FontInfo& fontInfo,
            int maxLineWidth, math::Size2i& size,
            std::vector<math::Box2i>* glyphGeom)
        {
            FT_Face face = getFTFace(fontInfo);
            
            math::Vector2i pos;
            float scale;
            FT_Error ftError = setFacePixelSize(
                face, static_cast<int>(fontInfo.size), scale);
            if (ftError)
            {
                throw std::runtime_error("Cannot set pixel sizes");
            }

            const int h = face->size->metrics.height / 64;
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
#endif
        
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

        std::vector<std::shared_ptr<Glyph> >
        FontSystem::getGlyphs(const std::string& text, const FontInfo& fontInfo)
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<Glyph> > out;
            try
            {
                const auto utf32 = utf8_to_utf32(text);
#ifdef USE_HARFBUZZ
                FT_Face face = p.getFTFace(fontInfo);

                float scale;
                FT_Error ftError = setFacePixelSize(face,
                                                    fontInfo.size, scale);
                
                const auto& shaped = shapeText(utf32, fontInfo, face);
                
                for (const auto& sg : shaped) {
                    auto glyph = p.getGlyphByIndex(sg.glyphIndex,
                                                   fontInfo); 
        
                    // Store the offsets provided by HarfBuzz for correct emoji positioning
                    // \@bug from Grok: this makes the text be placed half to
                    //                  the bottom.
                    // glyph->offset.x = sg.xOffset;
                    // glyph->offset.y = sg.yOffset;
                    // glyph->advance = sg.xAdvance;
                    out.push_back(glyph);
                }
#else
                for (const auto& i : utf32)
                {
                    out.push_back(p.getGlyph(i, fontInfo));
                }
#endif
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
                FT_Face face = getFTFace(fontInfo);

                float scale;
                FT_Error ftError = setFacePixelSize(face,
                                                    fontInfo.size, scale);
                if (ftError)
                {
                    throw std::runtime_error("setFacePixelSize failed");
                }
                    
                if (auto ftGlyphIndex = FT_Get_Char_Index(face, code))
                {
                    FT_UInt base_flags = 0;
                    if (FT_IS_SCALABLE(face))
                    {
                        base_flags |= FT_LOAD_FORCE_AUTOHINT;
                    }

                    bool tried_color = false;
                    FT_UInt load_flags = base_flags;

                    if (FT_HAS_COLOR(face))
                    {
                        load_flags |= FT_LOAD_COLOR;
                        tried_color = true;
                    }

                    FT_Error ftError = FT_Load_Glyph(face,
                                                     ftGlyphIndex,
                                                     load_flags);

                    if (ftError == FT_Err_Unimplemented_Feature &&
                        tried_color)
                    {
                        // Fallback: retry without color (likely old FreeType + COLR font)
                        load_flags = base_flags;  // remove FT_LOAD_COLOR
                        ftError = FT_Load_Glyph(face,
                                                ftGlyphIndex,
                                                load_flags);
                    }

                    if (ftError)
                    {
                        throw std::runtime_error("Cannot load glyph (error: " + std::to_string(ftError) + ")");
                    }

                    // Proceed to FT_Render_Glyph as before
                    ftError = FT_Render_Glyph(face->glyph,
                                              FT_RENDER_MODE_NORMAL);
                    if (ftError)
                    {
                        throw std::runtime_error("Render glyph (error: " + std::to_string(ftError) + ")");
                        return out;
                    }
                        
                    out = std::make_shared<Glyph>();
                    out->info = GlyphInfo(code, fontInfo);
                    auto ftBitmap = face->glyph->bitmap;
                    const image::Info imageInfo(
                        ftBitmap.width, ftBitmap.rows,
                        image::PixelType::L_U8);
                    out->image = image::Image::create(imageInfo);
                        
                    // Pixel Conversion Logic
                    // FontSystem expects L_U8 (1 byte gray), but Emoji might be BGRA (4 bytes).
                    uint8_t* outData = out->image->getData();
                    if (ftBitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
                    {
                        for (size_t y = 0; y < ftBitmap.rows; ++y)
                        {
                            uint8_t* rowDataP = outData + ftBitmap.width * y;
                            const unsigned char* bitmapP =
                                ftBitmap.buffer + y * ftBitmap.pitch;
                            memcpy(rowDataP, bitmapP, ftBitmap.width);
                        }
                    }
                    // Case B: Color Bitmap Font (Emoji) -> Convert to Monochrome (Luminance)
                    else if (ftBitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
                    {
                        for (size_t y = 0; y < ftBitmap.rows; ++y)
                        {
                            uint8_t* rowDataP = outData + ftBitmap.width * y;
                            const unsigned char* bitmapP =
                                ftBitmap.buffer + y * ftBitmap.pitch;  // * 4?
                            for (size_t x = 0; x < ftBitmap.width; ++x)
                            {
                                const unsigned char* p = bitmapP + (x * 4);
                                // Simple luminance: 0.2126 R + 0.7152 G + 0.0722 B
                                // Or simple average for speed: (R+G+B)/3
                                // BGRA layout: B=0, G=1, R=2, A=3
                                unsigned int b = p[0];
                                unsigned int g = p[1];
                                unsigned int r = p[2];
                                unsigned int a = p[3];
                                    
                                // Calculate grayscale intensity
                                unsigned int lum = (r * 6966 + g * 23436 + b * 2366) >> 15; // fast approx
                                        
                                // Apply Alpha blending against black background for the glyph
                                // (Since L_U8 has no alpha channel in this context, we bake it)
                                rowDataP[x] = static_cast<uint8_t>((lum * a) / 255);
                            }
                        }
                    }
                    // Case C: 1-bit Monochrome (rare but possible)
                    else if (ftBitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                    {
                        for (size_t y = 0; y < ftBitmap.rows; ++y)
                        {
                            uint8_t* rowDataP = outData + ftBitmap.width * y;
                            const unsigned char* bitmapP =
                                ftBitmap.buffer + y * ftBitmap.pitch;
                                
                            for (size_t x = 0; x < ftBitmap.width; ++x)
                            {
                                int byteIndex = x / 8;
                                int bitIndex  = 7 - (x % 8);
                                int bit = (bitmapP[byteIndex] >> bitIndex) & 1;
                                rowDataP[x] = bit ? 255 : 0;
                            }
                        }
                    }
                    else 
                    {
                        // Fallback zero
                        memset(out->image->getData(), 0,
                               ftBitmap.width * ftBitmap.rows);
                    }

                    out->offset = math::Vector2i(
                        face->glyph->bitmap_left,
                        face->glyph->bitmap_top);
                    out->advance = face->glyph->advance.x / 64;
                    out->lsbDelta = face->glyph->lsb_delta;
                    out->rsbDelta = face->glyph->rsb_delta;

                    // Scale bitmap fonts to requested size
                    if (!FT_IS_SCALABLE(face) && out->image)
                    {
                        const auto& imgInfo = out->image->getInfo();
                        int actualHeight = imgInfo.size.h;
    
                        // Calculate target size based on requested font size
                        // Use the selected bitmap size as reference
                        float scale = static_cast<float>(fontInfo.size) / actualHeight;
    
                        if (scale != 1.0f && scale > 0.1f && scale < 10.0f) // Reasonable scale range
                        {
                            int targetWidth = static_cast<int>(imgInfo.size.w * scale);
                            int targetHeight = static_cast<int>(fontInfo.size);
        
                            out->image = scaleBitmap(out->image, targetWidth, targetHeight);
        
                            // Adjust metrics proportionally
                            out->offset.x = static_cast<int>(out->offset.x * scale);
                            out->offset.y = static_cast<int>(out->offset.y * scale);
                            out->advance = static_cast<int>(out->advance * scale);
                        }
                    }
                    glyphCache.add(out->info, out);
                }
            }
            return out;
        }

        std::shared_ptr<Glyph> 
        FontSystem::Private::getGlyphByIndex(uint32_t ftGlyphIndex,
                                             const FontInfo& fontInfo)
        {
            std::shared_ptr<Glyph> out;

            GlyphInfo key(ftGlyphIndex, fontInfo); 
            if (!glyphCache.get(key, out))
            {
                FT_Face face = getFTFace(fontInfo);
                
                float scale;
                FT_Error ftError = setFacePixelSize(face,
                                                    fontInfo.size, scale);
                if (ftError)
                {
                    throw std::runtime_error("setFacePixelSize failed");
                }

                FT_UInt base_flags = 0;
                if (FT_IS_SCALABLE(face))
                {
                    base_flags |= FT_LOAD_FORCE_AUTOHINT;
                }
                        
                bool tried_color = false;
                FT_UInt load_flags = base_flags;

                if (FT_HAS_COLOR(face))
                {
                    load_flags |= FT_LOAD_COLOR;
                    tried_color = true;
                }

                ftError = FT_Load_Glyph(face, ftGlyphIndex, load_flags);
                    
                if (ftError == FT_Err_Unimplemented_Feature &&
                    tried_color)
                {
                    // Fallback: retry without color (likely old FreeType + COLR font)
                    load_flags = base_flags;  // remove FT_LOAD_COLOR
                    ftError = FT_Load_Glyph(face,
                                            ftGlyphIndex,
                                            load_flags);
                }
                        
                if (ftError)
                {
                    throw std::runtime_error("Cannot load glyph (error: " + std::to_string(ftError) + ")");
                }

                if (!face->glyph)
                {
                    throw std::runtime_error("No glyph pointer");
                }
                
                // Proceed to FT_Render_Glyph
                ftError = FT_Render_Glyph(face->glyph,
                                          FT_RENDER_MODE_NORMAL);
                if (ftError)
                {
                    throw std::runtime_error("Render glyph (error: " + std::to_string(ftError) + ")");
                    return out;
                }
                
                
                out = std::make_shared<Glyph>();
                out->info = key;
            
                FT_Bitmap& ftBitmap = face->glyph->bitmap;
                const image::Info imageInfo(
                    ftBitmap.width, ftBitmap.rows,
                    image::PixelType::L_U8);
                out->image = image::Image::create(imageInfo);


                // Pixel Conversion Logic
                // FontSystem expects L_U8 (1 byte gray), but Emoji might be BGRA (4 bytes).
                uint8_t* outData = out->image->getData();
                if (ftBitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
                {
                    for (size_t y = 0; y < ftBitmap.rows; ++y)
                    {
                        uint8_t* rowDataP = outData + ftBitmap.width * y;
                        const unsigned char* bitmapP =
                            ftBitmap.buffer + y * ftBitmap.pitch;
                        memcpy(rowDataP, bitmapP, ftBitmap.width);
                    }
                }
                // Case B: Color Bitmap Font (Emoji) -> Convert to Monochrome (Luminance)
                else if (ftBitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
                {
                    for (size_t y = 0; y < ftBitmap.rows; ++y)
                    {
                        uint8_t* rowDataP = outData + ftBitmap.width * y;
                        const unsigned char* bitmapP =
                            ftBitmap.buffer + y * ftBitmap.pitch;  // * 4?
                        for (size_t x = 0; x < ftBitmap.width; ++x)
                        {
                            const unsigned char* p = bitmapP + (x * 4);
                            // Simple luminance: 0.2126 R + 0.7152 G + 0.0722 B
                            // Or simple average for speed: (R+G+B)/3
                            // BGRA layout: B=0, G=1, R=2, A=3
                            unsigned int b = p[0];
                            unsigned int g = p[1];
                            unsigned int r = p[2];
                            unsigned int a = p[3];
                                    
                            // Calculate grayscale intensity
                            unsigned int lum = (r * 6966 + g * 23436 + b * 2366) >> 15; // fast approx
                                        
                            // Apply Alpha blending against black background for the glyph
                            // (Since L_U8 has no alpha channel in this context, we bake it)
                            rowDataP[x] = static_cast<uint8_t>((lum * a) / 255);
                        }
                    }
                }
                // Case C: 1-bit Monochrome (rare but possible)
                else if (ftBitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    for (size_t y = 0; y < ftBitmap.rows; ++y)
                    {
                        uint8_t* rowDataP = outData + ftBitmap.width * y;
                        const unsigned char* bitmapP =
                            ftBitmap.buffer + y * ftBitmap.pitch;
                                
                        for (size_t x = 0; x < ftBitmap.width; ++x)
                        {
                            int byteIndex = x / 8;
                            int bitIndex  = 7 - (x % 8);
                            int bit = (bitmapP[byteIndex] >> bitIndex) & 1;
                            rowDataP[x] = bit ? 255 : 0;
                        }
                    }
                }
                else 
                {
                    // Fallback zero
                    memset(out->image->getData(), 0,
                           ftBitmap.width * ftBitmap.rows);
                }

                out->offset = math::Vector2i(
                    face->glyph->bitmap_left,
                    face->glyph->bitmap_top);
                out->advance = face->glyph->advance.x / 64;
                out->lsbDelta = face->glyph->lsb_delta;
                out->rsbDelta = face->glyph->rsb_delta;

                // Scale bitmap fonts to requested size
                if (!FT_IS_SCALABLE(face) && out->image)
                {
                    const auto& imgInfo = out->image->getInfo();
                    int actualHeight = imgInfo.size.h;
    
                    // Calculate target size based on requested font size
                    // Use the selected bitmap size as reference
                    float scale = static_cast<float>(fontInfo.size) / actualHeight;
    
                    if (scale != 1.0f && scale > 0.1f && scale < 10.0f) // Reasonable scale range
                    {
                        int targetWidth = static_cast<int>(imgInfo.size.w * scale);
                        int targetHeight = static_cast<int>(fontInfo.size);
        
                        out->image = scaleBitmap(out->image, targetWidth, targetHeight);
                        
                        // Adjust metrics proportionally
                        out->offset.x = static_cast<int>(out->offset.x * scale);
                        out->offset.y = static_cast<int>(out->offset.y * scale);
                        out->advance = static_cast<int>(out->advance * scale);
                    }
                }
                glyphCache.add(out->info, out);
            }
            return out;
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

        file::Path emojiFont()
        {
            file::Path out;
            const auto& fonts = discoverSystemFonts();
            for (const auto& font : fonts)
            {
                const std::string baseName = font.stem().string();
                if (baseName == "NotoColorEmoji" ||
                    baseName == "Apple Color Emoji" ||
                    baseName == "seguiemj")
                {
                    out = file::Path(font.u8string());
                    break;
                }
            }
            return out;
        }

    } // namespace image
} // namespace tl
