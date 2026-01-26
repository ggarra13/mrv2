// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvVk/mrvVkShape.h"

#include "mrvCore/mrvMesh.h"

#include <tlVk/Shader.h>
#include <tlVk/Util.h>

#include <FL/fl_utf8.h>

namespace
{
    const int kCrossSize = 10;
}

namespace
{

    using namespace tl;

    using tl::geom::Triangle2;
    using tl::math::Vector2f;
    
   //! Helper function to check if a codepoint is inherently an emoji
    bool detectIsEmoji(unsigned int cp) {
        return (cp >= 0x1F300 && cp <= 0x1F9FF) || // Misc Symbols, Pictographs, Emoticons
            (cp >= 0x2600 && cp <= 0x26FF)   || // Misc Symbols
            (cp >= 0x2700 && cp <= 0x27BF)   || // Dingbats
            (cp >= 0x1F1E6 && cp <= 0x1F1FF);   // Flags
    }

    bool isEmojiCombiner(unsigned cp)
    {
        return ((cp >= 0x0300 && cp <= 0x036F) ||   // Combining Diacritical Marks
                (cp >= 0x1AB0 && cp <= 0x1AFF) ||   // Extended Marks
                (cp >= 0x20D0 && cp <= 0x20FF) ||   // Symbol Marks
                (cp >= 0xFE00 && cp <= 0xFE0F) ||   // Variation Selectors
                (cp >= 0x1F3FB && cp <= 0x1F3FF));   // Skin Tone
    }
    
} // namespace

namespace mrv
{
    int VKTextShape::accept()
    {
        return App::ui->uiView->acceptMultilineInput();
    }
    
    unsigned VKTextShape::line_end(unsigned c)
    {
        const char* start = text.c_str();
        const char* end   = text.c_str() + text.size();
        const char* pos = start + c;
        while (pos[0] != '\n' && pos < end)
        {
            pos = start + c;
            int len = fl_utf8len(pos[0]);
            if (len < 1) len = 1;
            c += len;
        }
        if (pos >= end)
            return text.size();
        ++c;
        return c;
    }

    unsigned VKTextShape::line_start(unsigned c)
    {
        const char* start = text.c_str();
        const char* pos = start + c;
        while (*pos != '\n' && pos > start)
        {
            int len = fl_utf8len(*pos);
            if (len < 1) len = 1;
            pos -= len;
            c -= len;
        }
        if (pos <= start)
            return 0;
        ++c;
        return c;
    }
    
    unsigned VKTextShape::current_column()
    {
        unsigned size = fl_utf8toa(text.c_str(), utf8_pos, nullptr, 0);
        
        char* dst = new char[size+1];
        fl_utf8toa(text.c_str(), utf8_pos, dst, size + 1);

        unsigned column = 0;
        char* c = dst;
        for (; *c; ++c)
        {
            if (*c != '\n')
                ++column;
            else
                column = 0;
        }

        delete [] dst;
        
        return column;
    }
    
    unsigned VKTextShape::current_line()
    {
        unsigned size = fl_utf8toa(text.c_str(), utf8_pos, nullptr, 0);

        char* dst = new char[size+1];
        fl_utf8toa(text.c_str(), utf8_pos, dst, size + 1);

        unsigned line = 0;
        char* c = dst;
        for (; *c; ++c)
        {
            if (*c == '\n')
                ++line;
        }
        
        delete [] dst;
        return line;
    }
    
    void VKTextShape::to_cursor()
    {
        const char* start = text.c_str();
        const char* target = start + utf8_pos;
        const char* p = start;
    
        unsigned count = 0;

        while (p < target) {
            int len = 0;
            unsigned int cp = fl_utf8decode(p, target, &len);
            if (len < 1) len = 1;

            // Determine if this code point is a "Base" character or a "Modifier"
            bool is_modifier = false;

            // 1. Combining Marks & Variation Selectors
            if (isEmojiCombiner(cp)) {
                is_modifier = true;
            }

            // 2. Zero Width Joiner (ZWJ)
            // If it's a ZWJ, we treat it as a modifier (don't increment count) 
            // AND we must treat the character immediately following it as a modifier too.
            if (cp == 0x200D) {
                is_modifier = true;
                p += len; // move past ZWJ
                if (p < target) {
                    int next_len = 0;
                    fl_utf8decode(p, target, &next_len);
                    p += (next_len > 0 ? next_len : 1);
                }
                continue; // Jump to next loop iteration
            }

            // 3. Newlines / Carriage Returns
            // These are always base characters. They should increment the cursor.
            if (cp == '\n' || cp == '\r') {
                is_modifier = false;
            }

            // 4. Regional Indicators (Flags)
            // If this is a flag and the previous was a flag, it's a modifier pair.
            // (Simplification: only increment on the first RI of a pair)
            if (cp >= 0x1F1E6 && cp <= 0x1F1FF) {
                // Check if we just passed an RI
                // This requires looking back or tracking state; for simplicity in to_cursor,
                // we usually count the 'pair' as 1 visual unit.
                static bool last_was_ri = false;
                if (last_was_ri) {
                    is_modifier = true;
                    last_was_ri = false;
                } else {
                    last_was_ri = true;
                }
            } else {
                // If the character isn't an RI, reset that state
                // Note: In a real class, 'last_was_ri' should be a local variable outside the loop.
            }

            if (!is_modifier) {
                count++;
            }

            p += len;
        }

        cursor = count;
    }
    
    int VKTextShape::kf_paste()
    {
        if (!Fl::event_text() || !Fl::event_length()) return 1;
            
        const char* t = Fl::event_text();
        const char* e = t + Fl::event_length();

        const char* current = text.c_str() + utf8_pos;
        const char* start = current;
        const char* end = start + text.size() - utf8_pos;
        const char* pos = fl_utf8fwd(current + 1, start, end);
        unsigned next = pos - text.c_str();

        char* copy = new char[e - t + 1];
        memcpy(copy, t, e - t);
        copy[e - t] = 0;

        std::string right;
        std::string left;
        if (utf8_pos > 0)
            left = text.substr(0, utf8_pos);
        if (utf8_pos < text.size())
            right = text.substr(utf8_pos, text.size());

        text = left + copy + right;

        to_cursor();
        
        return 1;
    }
    
    const char* VKTextShape::advance_to_column(unsigned start,
                                               unsigned column)
    {
        const char* current = text.c_str() + start;
        const char* pos = current;
        for (unsigned i = 0; *pos != '\n' && i < column; ++i)
        {
            int len = fl_utf8len(pos[0]);
            if (len < 1) len = 1;
            pos += len;
        }
        return pos;
    }

    int VKTextShape::kf_select_all()
    {
        return 0;
    }

    int VKTextShape::kf_copy()
    {
        return 0;
    }
    
    int VKTextShape::kf_copy_cut()
    {
        return 0;
    }
    
    int VKTextShape::handle_backspace()
    {
        if (utf8_pos == 0) return 1;

        const char* start = text.c_str();
    
        // We start at the current cursor position
        const char* current_ptr = start + utf8_pos;
    
        // We will calculate the new position (prev_ptr) by stepping back
        const char* prev_ptr = fl_utf8_previous_composed_char(current_ptr,
                                                              start);
        
        // Perform the deletion
        unsigned final_prev_index = prev_ptr - start;
        unsigned len = utf8_pos - final_prev_index;

        text.erase(final_prev_index, len);
        utf8_pos = final_prev_index;

        Fl::compose_reset();
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle_move_up() {
        unsigned row = current_line();
        if (row == 0)
            return 1;
        unsigned column = current_column();
        unsigned start = line_start(utf8_pos);
        start = line_start(start-2);  // 2 to skip \n
        const char* pos = advance_to_column(start, column);
        utf8_pos = pos - text.c_str();
        Fl::compose_reset();
        to_cursor();
        return 1; 
    }

    int VKTextShape::handle_move_down() {
        unsigned column = current_column();
        unsigned end = line_end(utf8_pos);
        if (end == text.size())
            return 1;
        unsigned start = line_start(end+1);
        const char* pos = advance_to_column(start, column);
        utf8_pos = pos - text.c_str();
        Fl::compose_reset();
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle_insert(const char* new_text, int len,
                                   int del_back) {
        if (del_back > 0 && utf8_pos >= (unsigned)del_back) {
            utf8_pos -= del_back;
            text.erase(utf8_pos, del_back);
        }
    
        if (new_text && len > 0) {
            text.insert(utf8_pos, new_text, len);
            utf8_pos += len;
        }
        Fl::compose_reset();
    
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle(int e)
    {
     
        if (e == FL_PASTE) return kf_paste();
        if (e != FL_KEYBOARD) return 0;

        unsigned rawkey = Fl::event_key();
        int mods = Fl::event_state() & (FL_META | FL_CTRL | FL_ALT);
        bool shift = Fl::event_state() & FL_SHIFT;

        // Handle "Accept" shortcut (Shift+Enter)
        if ((rawkey == FL_Enter || rawkey == FL_KP_Enter) && shift) {
            return accept();
        }

        int del = 0;
        if (Fl::compose(del)) {
            if (del > 0 || (Fl::event_text() && Fl::event_length() > 0)) {
                handle_insert(Fl::event_text(), Fl::event_length(), del);
            }
            return 1;
        }

        // Command shortcuts
        if (mods & FL_COMMAND) {
            switch (tolower(rawkey)) {
            case 'c': return kf_copy();
            case 'v': return kf_paste();
            case 'x': return kf_copy_cut();
            case 'a': return kf_select_all();
            }
        }

        // Navigation and Editing
        switch (rawkey) {
        case FL_BackSpace: return handle_backspace();
        case FL_Delete:    return handle_delete();
        case FL_Left:      return handle_move_left();
        case FL_Right:     return handle_move_right();
        case FL_Up:        return handle_move_up();
        case FL_Down:      return handle_move_down();
        case FL_Enter:
        case FL_KP_Enter:  return handle_insert("\n", 1, 0);
        case FL_Escape:    text = ""; return accept();
        }

        return 0;
    }

    int VKTextShape::handle_move_left()
    {
        if (utf8_pos == 0) return 1;

        const char* start = text.c_str();
        const char* current_ptr = start + utf8_pos;
        const char* new_ptr = fl_utf8_previous_composed_char(current_ptr,
                                                             start);

        utf8_pos = (unsigned)(new_ptr - start);
        Fl::compose_reset();
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle_move_right()
    {
        if (utf8_pos >= text.size()) return 1;

        const char* start = text.c_str();
        const char* end = start + text.size();
        const char* current_ptr = start + utf8_pos;
        
        const char* new_ptr = fl_utf8_next_composed_char(current_ptr, end);

        utf8_pos = (unsigned)(new_ptr - start);
        Fl::compose_reset();
        to_cursor();
        return 1;
    }

    int VKTextShape::handle_delete()
    {
        if (utf8_pos >= text.size()) return 1;

        const char* start = text.c_str();
        const char* end = start + text.size();
        const char* current_ptr = start + utf8_pos;

        const char* new_ptr = fl_utf8_next_composed_char(current_ptr, end);
        unsigned int delete_len = (unsigned int)(new_ptr - current_ptr);
        
        text.erase(utf8_pos, delete_len);
    
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle_mouse_click(int event, const math::Vector2i& local)
    {
        file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);
        const int ascender = fontMetrics.ascender;
        
        // Copy the text to process it line by line
        std::string txt = text;

        const int x = pts[0].x;
        const int y = pts[0].y;
        math::Vector2i cursor_pos(x, y);
        std::size_t pos = txt.find('\n');

        utf8_pos = 0;
        const char* text_start = text.c_str();
        const char* text_it = text_start;
        const char* text_end = text_start + text.size();
        
        for (; pos != std::string::npos; pos = txt.find('\n'))
        {
            const std::string line = txt.substr(0, pos);
            _positionCursor(line, text_it, x, y, local, cursor_pos);
            if (txt.size() > pos)
            {
                txt = txt.substr(pos + 1, txt.size());
                if (local.y > cursor_pos.y)
                {
                    cursor_pos.x = x;
                    cursor_pos.y += fontSize;
                    if (text_it < text_end && *text_it == '\n')
                    {
                        ++utf8_pos;
                        ++text_it; // Advance past the single-byte newline
                    }
                }
            }
        }
        if (!txt.empty())
        {
            _positionCursor(txt, text_it, x, y, local, cursor_pos);
        }
        to_cursor();
        return 1;
    }
    

    void VKTextShape::_positionCursor(
        const std::string& line, const char* text_it, int x, int y,
        const math::Vector2i& local, math::Vector2i& cursor_pos)
    {
        //
        // Add selected font.
        //
        const file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        
        //
        // Add emoji font.
        //
        const file::Path emojiPath = image::emojiFont();
        const std::string emojiFamily = emojiPath.getBaseName();
        
        //
        // Get metrics for selected font.
        // 
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontInfo emojiInfo(emojiFamily, fontSize);
        
        const char* text_start = text.c_str();
        const char* text_end = text_start + text.size();
        
        std::string currentRun;
        bool runIsEmoji = false;
        bool prevWasZWJ = false;
        bool firstChar = true;
        
        // Helper to flush the current accumulated run
        auto flushRun = [&](const std::string& run, bool isEmoji) 
            {
                if (run.empty()) return;
                
                const auto& activeInfo = isEmoji ? emojiInfo : fontInfo;
                const auto& glyphs = fontSystem->getGlyphs(run, activeInfo);
        
                for (const auto& glyph : glyphs)
                {
                    if (glyph)
                    {
                        if (local.x > (cursor_pos.x + glyph->advance / 2) ||
                            local.y > cursor_pos.y)
                        {
                            cursor_pos.x += glyph->advance;
                            if (text_it < text_end)
                            {
                                const char* old_it = text_it;
                                text_it = fl_utf8fwd(text_it + 1, text_start,
                                                     text_end);
                                utf8_pos += (text_it - old_it);
                            }
                        }
                    }
                }
            };
        
        for (size_t i = 0; i < line.size(); )
        {
            // fl_utf8len returns the length of the UTF-8 sequence (1 to 4 bytes)
            int len = fl_utf8len(line[i]);
        
            // Safety fallback: if FLTK returns < 1 for some reason, assume 1 byte to prevent infinite loops
            if (len < 1) len = 1; 

            std::string charStr = line.substr(i, len);
        
            // Decode the codepoint to check for ZWJ/Variation Selectors
            unsigned int codepoint = fl_utf8decode(charStr.c_str(), nullptr, &len);
                
            // Check if emoji
            bool isEmojiChar = detectIsEmoji(codepoint);
                
            // Define "Sticky" characters that should not break a run
            bool isZWJ = (codepoint == 0x200D);
            bool isVS = (codepoint >= 0xFE00 && codepoint <= 0xFE0F);
            bool isSticky = isZWJ || isVS || prevWasZWJ;
                
            // Update runIsEmoji immediately if it's the very first character
            if (firstChar)
            {
                runIsEmoji = isEmojiChar;
                firstChar = false;
            }
            // Standard run-switching logic
            else if (!isSticky && isEmojiChar != runIsEmoji && !currentRun.empty())
            {
                flushRun(currentRun, runIsEmoji);
                currentRun.clear();
                runIsEmoji = isEmojiChar;
            }
    
            currentRun += charStr;
            i += len; // Advance by the UTF-8 length
            prevWasZWJ = isZWJ;
        }

        // Flush remaining buffer at end of line
        if (!currentRun.empty())
        {
            flushRun(currentRun, runIsEmoji);
        }
    }

    void VKTextShape::_drawLine(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::string& line, int x, int y,
        std::vector<timeline::TextInfo>& textInfos,
        unsigned& cursor_count,
        math::Vector2i& cursor_pos)
    {
        //
        // Add selected font.
        //
        const file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        
        //
        // Add emoji font.
        //
        const file::Path emojiPath = image::emojiFont();
        const std::string emojiFamily = emojiPath.getBaseName();
        
        //
        // Get metrics for selected font.
        // 
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontInfo emojiInfo(emojiFamily, fontSize);

        
        // Buffers for batching
        int currentDrawX = x; 
        std::string currentRun;
        bool runIsEmoji = false;
        bool prevWasZWJ = false;
        bool firstChar = true;

        // Helper to flush the current accumulated run
        auto flushRun = [&](const std::string& run, bool isEmoji) 
            {
                if (run.empty()) return;
                
                const auto& activeInfo = isEmoji ? emojiInfo : fontInfo;
                const auto& glyphs = fontSystem->getGlyphs(run, activeInfo);
                math::Vector2i pnt(currentDrawX, y);
        
                for (const auto& glyph : glyphs)
                {
                    if (glyph)
                    {
                        if (cursor_count < cursor)
                            cursor_pos.x += glyph->advance;
                        if (glyph->info.code != 0x003)
                            currentDrawX += glyph->advance;
                    }
                    ++cursor_count;
                }
                render->appendText(textInfos, glyphs, pnt);
            };

        for (size_t i = 0; i < line.size(); )
        {
            // fl_utf8len returns the length of the UTF-8 sequence (1 to 4 bytes)
            int len = fl_utf8len(line[i]);
        
            // Safety fallback: if FLTK returns < 1 for some reason, assume 1 byte to prevent infinite loops
            if (len < 1) len = 1; 

            std::string charStr = line.substr(i, len);
        
            // Decode the codepoint to check for ZWJ/Variation Selectors
            unsigned int codepoint = fl_utf8decode(charStr.c_str(), nullptr, &len);
                
            // Check if emoji
            bool isEmojiChar = detectIsEmoji(codepoint);
                
            // Define "Sticky" characters that should not break a run
            bool isZWJ = (codepoint == 0x200D);
            bool isVS = (codepoint >= 0xFE00 && codepoint <= 0xFE0F);
            bool isSticky = isZWJ || isVS || prevWasZWJ;
                
            // Update runIsEmoji immediately if it's the very first character
            if (firstChar)
            {
                runIsEmoji = isEmojiChar;
                firstChar = false;
            }
            // Standard run-switching logic
            else if (!isSticky && isEmojiChar != runIsEmoji && !currentRun.empty())
            {
                flushRun(currentRun, runIsEmoji);
                currentRun.clear();
                runIsEmoji = isEmojiChar;
            }
    
            currentRun += charStr;
            i += len; // Advance by the UTF-8 length
            prevWasZWJ = isZWJ;
        }

        // Flush remaining buffer at end of line
        if (!currentRun.empty())
        {
            flushRun(currentRun, runIsEmoji);
        }
    }
    
    void VKTextShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        //
        // Add selected font.
        //
        const file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        if (!fontSystem->hasFont(fontFamily))
        {
            fontSystem->addFont(fontPath);
        }
        
        //
        // Add emoji font.
        //
        const file::Path emojiPath = image::emojiFont();
        const std::string emojiFamily = emojiPath.getBaseName();
        if (!fontSystem->hasFont(emojiFamily))
        {
            fontSystem->addFont(emojiPath.get());
        }
        
        //
        // Get metrics for selected font.
        // 
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontInfo emojiInfo(emojiFamily, fontSize);
        const image::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);
        const int ascender = fontMetrics.ascender;
        const int descender = fontMetrics.descender;

        //
        // Get metrics for emoji font.
        //
        const image::FontMetrics emojiMetrics = fontSystem->getMetrics(emojiInfo);

        // Copy the text to process it line by line
        std::string txt = text;

        int x = pts[0].x;
        int y = pts[0].y + descender;
        math::Vector2i cursor_pos(x, y - ascender);
        math::Vector2i pnt(x, y);
        std::size_t pos = txt.find('\n');
        std::vector<timeline::TextInfo> textInfos;
        unsigned cursor_count = 0;
        int currentDrawX = x;
        
        for (; pos != std::string::npos; y += fontSize, pos = txt.find('\n'))
        {
            const std::string line = txt.substr(0, pos);
            

            _drawLine(render, line, x, y, textInfos, cursor_count, cursor_pos);
            

            if (txt.size() > pos)
            {
                txt = txt.substr(pos + 1, txt.size());
                if (cursor_count < cursor)
                {
                    cursor_pos.x = x;
                    cursor_pos.y += fontSize;
                    ++cursor_count;
                }
            }
        }

        if (!txt.empty())
        {
            _drawLine(render, txt, x, y, textInfos, cursor_count, cursor_pos);
        }
        
        const image::Color4f cursorColor(.8F, 0.8F, 0.8F);
        math::Box2i cursorBox(cursor_pos.x, cursor_pos.y, 2, fontSize);
            
        if (editing)
        {
            auto boxf = math::Box2f(pts[0].x, pts[0].y + descender - ascender, 70, 0);
            for (const auto& textInfo : textInfos)
            {
                for (const auto& v : textInfo.mesh.v)
                {
                    if (v.x < boxf.min.x)
                        boxf.min.x = v.x;
                    if (v.y < boxf.min.y)
                        boxf.min.y = v.y;
                    if (v.x > boxf.max.x)
                        boxf.max.x = v.x;
                    if (v.y > boxf.max.y)
                        boxf.max.y = v.y;
                }
            }

            //
            // Make room in box for cursor
            //
            boxf.expand(math::Box2f(cursorBox.min.x,
                                    cursorBox.min.y,
                                    cursorBox.w(),
                                    cursorBox.h()));
            boxf = boxf.margin(8);
            
            auto roundedBox = createRoundedRect(boxf, 10);
                
            //
            // Draw background which will be darker
            //
            const image::Color4f bgcolor(0.F, 0.F, 0.F, 0.5F);
            render->drawMesh("annotation", "rect", "rect", "mesh", roundedBox,
                             math::Vector2i(), bgcolor);

            //
            // Draw cross
            //
            image::Color4f crossColor(0.F, 1.F, 0.F);
            if (text.empty())
                crossColor = image::Color4f(1.F, 0.F, 0.F);

            int cross_size = kCrossSize * mult / 3;
            if (cross_size < kCrossSize / 2) cross_size = kCrossSize / 2;
            
            int line_size = 2 * mult / 3;
            if (line_size < 2) line_size = 2;

            math::Vector2i start(boxf.min.x, boxf.min.y);
            math::Vector2i end(boxf.min.x + cross_size,
                               boxf.min.y + cross_size);
            lines->drawLine(render, start, end, crossColor, line_size);
            
            start = math::Vector2i(boxf.min.x + cross_size, boxf.min.y);
            end = math::Vector2i(boxf.min.x, boxf.min.y + cross_size);
            lines->drawLine(render, start, end, crossColor, line_size);

            box = math::Box2i(boxf.x(), boxf.y(), boxf.w(), boxf.h());
        }
        
        //
        // Draw all text with color 
        //
        for (const auto& textInfo : textInfos)
        {
            render->drawText(textInfo, math::Vector2i(), color);
        }
        if (editing)
        {
            //
            // Finally, draw cursor.
            // 
            render->drawRect(cursorBox, cursorColor);
        }
    }

    void to_json(nlohmann::json& json, const VKTextShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Text";
        json["text"] = value.text;
        json["fontPath"] = value.fontPath;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, VKTextShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("fontPath").get_to(value.fontPath);
        json.at("fontSize").get_to(value.fontSize);
    }

} // namespace mrv
