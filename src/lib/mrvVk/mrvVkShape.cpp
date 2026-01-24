// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"
#include "mrvURLLinkUI.h"

#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShape.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPathMapping.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvMesh.h"

#include <tlVk/Shader.h>
#include <tlVk/Util.h>

#include <tlCore/StringFormat.h>

#include <FL/filename.H>
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
    
    // Check if vertex i in the polygon is an ear
    bool isEar(
        const std::vector<Vector2f>& points, const std::vector<int>& poly,
        int i)
    {
        int n = poly.size();
        int prevIdx = poly[(i - 1 + n) % n] - 1; // Convert to 0-based index
        int currIdx = poly[i] - 1;
        int nextIdx = poly[(i + 1) % n] - 1;

        const Vector2f& prev = points[prevIdx];
        const Vector2f& curr = points[currIdx];
        const Vector2f& next = points[nextIdx];

        // Ear condition: the triangle (prev, curr, next) must be convex
        if (mrv::crossProduct(prev, curr, next) <= 0)
        {
            return false;
        }

        // No other vertex should be inside the triangle formed by (prev, curr,
        // next)
        for (int j = 0; j < n; ++j)
        {
            if (j == (i - 1 + n) % n || j == i || j == (i + 1) % n)
            {
                continue;
            }
            int pointIdx = poly[j] - 1; // Convert to 0-based index
            if (mrv::isPointInTriangle(points[pointIdx], prev, curr, next))
            {
                return false;
            }
        }

        return true;
    }

    // Function to calculate polygon area to check the winding order
    float polygonArea(
        const std::vector<Vector2f>& points, const std::vector<int>& poly)
    {
        float area = 0;
        int n = poly.size();
        for (int i = 0; i < n; ++i)
        {
            const Vector2f& p1 = points[poly[i] - 1];
            const Vector2f& p2 = points[poly[(i + 1) % n] - 1];
            area += (p1.x * p2.y) - (p2.x * p1.y);
        }
        return 0.5f * area;
    }

    // Ensure counterclockwise winding
    void ensureCounterClockwise(
        std::vector<int>& poly, const std::vector<Vector2f>& points)
    {
        if (polygonArea(points, poly) < 0)
        {
            std::reverse(poly.begin(), poly.end());
        }
    }

    std::vector<geom::Triangle2>
    triangulatePolygon(std::vector<Vector2f>& points, std::vector<int>& poly)
    {
        std::vector<geom::Triangle2> triangles;

        geom::Triangle2 triangle;

        ensureCounterClockwise(poly, points);

        while (poly.size() > 3)
        {
            int n = poly.size();
            bool earFound = false;

            for (int i = 0; i < n; ++i)
            {
                if (isEar(points, poly, i))
                {
                    int prevIdx = poly[(i - 1 + n) % n];
                    int currIdx = poly[i];
                    int nextIdx = poly[(i + 1) % n];

                    triangle.v[0].v = prevIdx;
                    triangle.v[1].v = currIdx;
                    triangle.v[2].v = nextIdx;
                    triangles.push_back(triangle);

                    // Remove the ear vertex from the polygon
                    poly.erase(poly.begin() + i);

                    earFound = true;
                    break;
                }
            }

            if (!earFound)
            {
                break;
            }
        }

        if (poly.size() == 3)
        {
            triangle.v[0].v = poly[0];
            triangle.v[1].v = poly[1];
            triangle.v[2].v = poly[2];
            triangles.push_back(triangle);
        }

        return triangles;
    }

} // namespace

namespace mrv
{

    void VKPathShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl;
        using namespace tl::draw;
        

        const bool catmullRomSpline = true;
        
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
    }

    void VKErasePathShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        color.r = color.g = color.b = 0.F;
        color.a = 1.F;

        if (rectangle)
        {
            math::Box2i box(
                pts[0].x, pts[0].y, pts[2].x - pts[0].x, pts[2].y - pts[0].y);
            render->drawRect("annotation", box, color, true, "erase");

            if (drawing)
            {
                const bool catmullRomSpline = false;
                color.r = 0.F; color.g = 1.F;

                int lineSize = 3 * mult;
                if (lineSize < 1) lineSize = 1;
                
                lines->drawLines(
                    render, pts, color, lineSize, false,
                    Polyline2D::JointStyle::ROUND,
                    Polyline2D::EndCapStyle::JOINT, catmullRomSpline);
            }
        }
        else
        {
            const bool catmullRomSpline = false;
            lines->drawLines(
                render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
                Polyline2D::EndCapStyle::ROUND, catmullRomSpline, false, "erase");
        }
    }

    void VKPolygonShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        const bool catmullRomSpline = false;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::JOINT, catmullRomSpline);
    }

    void VKCircleShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        lines->drawCircle(render, center, radius, pen_size, color, soft);
    }

    void VKRectangleShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        const bool catmullRomSpline = false;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::JOINT, catmullRomSpline);
    }

    void VKFilledPolygonShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        bool enableBlending = false;
        if (color.a < 0.99F)
            enableBlending = true;

        if (pts.size() < 3)
        {
            lines->drawLines(render, pts, color, pen_size);
            return;
        }
        
        geom::TriangleMesh2 mesh;
        mesh.v.reserve(pts.size() + 64);

        size_t numVertices = pts.size();
        mesh.v.reserve(numVertices);
        for (size_t i = 0; i < numVertices; ++i)
            mesh.v.push_back(math::Vector2f(pts[i].x, pts[i].y));

        std::vector<int> poly;
        for (int i = 0; i < pts.size(); ++i)
        {
            poly.push_back(i + 1);
        }
        auto triangles = triangulatePolygon(mesh.v, poly);
        mesh.triangles = triangles;
        
        math::Vector2i pos;
        render->drawMesh("annotation", "rect", "rect", "mesh", mesh, pos, color,
                         enableBlending);
    }

    void VKFilledCircleShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        bool enableBlending = false;
        if (color.a < 0.99F)
            enableBlending = true;

        math::Vector2i v(center.x, center.y);
        util::drawFilledCircle(render, "annotation", v, radius, color,
                               enableBlending);
    }

    void VKFilledRectangleShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        bool enableBlending = false;
        if (color.a < 0.99F)
            enableBlending = true;

        math::Box2i box(
            pts[0].x, pts[0].y, pts[2].x - pts[0].x, pts[2].y - pts[0].y);
        render->drawRect(box, color);
    }

    void VKArrowShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace tl::draw;

        bool catmullRomSpline = false;
        std::vector< draw::Point > line;

        line.push_back(pts[1]);
        line.push_back(pts[2]);
        lines->drawLines(
            render, line, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);

        line.clear();
        line.push_back(pts[1]);
        line.push_back(pts[4]);

        lines->drawLines(
            render, line, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);

        line.clear();
        line.push_back(pts[0]);
        line.push_back(pts[1]);
        lines->drawLines(
            render, line, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
    }

    const math::Box2f VKLinkShape::getBBox(float scale) const
    {
        // --- 1. Get Stored Data (using the "hack" method) ---
        const draw::Point& pnt = pts[0]; // Anchor (raster space)
        const draw::Point& pixel_dims = pts[1]; // L-shape (pixel space)
        
        // Calculate current raster offsets
        float L_height_raster = pixel_dims.y * scale;
        float L_width_raster = pixel_dims.x * scale;

        // --- 3. Calculate Final Raster Points ---
        draw::Point pnt2(pnt.x, pnt.y + L_height_raster);
        draw::Point pnt3(pnt.x + L_width_raster, pnt.y + L_height_raster);
        
        const float radius = std::fabs(pts[0].y - pnt2.y) * 1.05;

        math::Vector2f center;
        center.x = (pts[0].x + pnt2.x + pnt3.x) / 3;
        center.y = (pts[0].y + pnt2.y + pnt3.y) / 3;
        
        return math::Box2f(center.x - radius, center.y - radius,
                           radius * 2, radius * 2);
                           
    }
    
    void VKLinkShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace mrv::draw;

        math::Vector2f center;
        const image::Color4f shadowColor(0.F, 0.F, 0.F, color.a);
        const bool catmullRomSpline = false;

        // --- 1. Get Stored Data ---
        const draw::Point& pnt = pts[0]; // Anchor (raster space)
        const draw::Point& pixel_dims = pts[1]; // L-shape (pixel space)
        const float pen_size_px = pen_size; // Pen (pixel space)

        // --- 2. Convert Pixel-Space values to Raster-Space ---
        // Calculate current raster pen size
        float pen_size_raster = pen_size_px * mult;
        
        // Calculate current raster offsets
        float L_height_raster = pixel_dims.y * mult;
        float L_width_raster = pixel_dims.x * mult;

        // --- 3. Calculate Final Raster Points ---
        draw::Point pnt2(pnt.x, pnt.y + L_height_raster);
        draw::Point pnt3(pnt.x + L_width_raster, pnt.y + L_height_raster);

        // --- 4. Calculate radius of circle and shadow offset
        const float radius = std::fabs(pts[0].y - pnt2.y) * 1.05;
        const float offset = radius * 0.15F;

        std::vector< draw::Point > line;
        line.push_back(pts[0]);
        line.push_back(pnt2);
        line.push_back(pnt3);

        line[0].x += offset;
        line[0].y += offset;
        line[1].x += offset;
        line[1].y += offset;
        line[2].x += offset;
        line[2].y += offset;
        
        lines->drawLines(
            render, line, shadowColor, pen_size_raster,
            soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
        
        center.x = (pts[0].x + pnt2.x + pnt3.x) / 3 + offset;
        center.y = (pts[0].y + pnt2.y + pnt3.y) / 3 + offset;
        
        lines->drawCircle(render, center, radius,
                          pen_size_raster, shadowColor, soft);

        line[0].x -= offset;
        line[0].y -= offset;
        line[1].x -= offset;
        line[1].y -= offset;
        line[2].x -= offset;
        line[2].y -= offset;
        
        lines->drawLines(
            render, line, color, pen_size_raster, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
        
        center.x -= offset;
        center.y -= offset;
        
        lines->drawCircle(render, center, radius, pen_size_raster, color, soft);
    }

    void VKLinkShape::open()
    {
        const char* kModule = "link";
        
        if (url.substr(0, 4) == "http" ||
            url.substr(0, 3) == "ftp")
        {
            char errmsg[512];
            if (!fl_open_uri(url.c_str(), errmsg, sizeof(errmsg)))
            {
                LOG_ERROR(errmsg);
            }
        }
        else if (file::isDirectory(url))
        {
            char errmsg[512];
            std::string uri = url;
            if (uri.substr(0, 6) != "file://")
                uri = "file://" + url;
            if (!fl_open_uri(uri.c_str(), errmsg, sizeof(errmsg)))
            {
                LOG_ERROR(errmsg);
            }
        }
        else if (file::isReadable(url))
        {
            char errmsg[512];
            std::string uri = url;
            if (uri.substr(0, 6) != "file://")
                uri = "file://" + url;
            if (!fl_open_uri(uri.c_str(), errmsg, sizeof(errmsg)))
            {
                LOG_ERROR(errmsg);
            }
        }
        else
        {
            const std::string err = string::Format(_("'{0}' is not a file, directory or url.")).arg(url);
            LOG_ERROR(err); 
        }
    }

    bool VKLinkShape::edit()
    {
        URLLinkUI linkEdit(this);
        if (linkEdit.cancel)
            return false;

        url = linkEdit.uiURL->value();
        if (url.empty())
            return false;
        
        if (url.substr(0, 4) == "www.")
            url = "http://" + url;
        
        title = linkEdit.uiTitle->value();
        return true;
    }
    
    int VKLinkShape::handle(int event)
    {
        if (event == FL_PUSH)
        {
            if (Fl::event_button1())
            {
                if (Fl::event_alt())
                {
                    edit();
                }
                else
                {
                    open();
                }
                return 1;
            }
        }
        return 0;
    }
    
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
        const char* prev_ptr = current_ptr;
    
        bool keep_deleting = true;

        // Loop to consume the entire Grapheme Cluster
        while (keep_deleting && prev_ptr > start) {
        
            // 1. Step back one UTF-8 code point
            const char* temp_ptr = fl_utf8back(prev_ptr - 1, start, prev_ptr);
        
            // 2. Decode that code point to check its properties
            unsigned int cp = fl_utf8decode(temp_ptr, prev_ptr, NULL);
        
            // Move our 'deletion head' to this new point
            prev_ptr = temp_ptr;
        
            // Default assumption: we stop after deleting one character, unless specific rules apply
            keep_deleting = false;

            // --- RULE A: Current char is a Modifier/Combiner ---
            // If the character we just ate is meant to modify the previous one,
            // we must continue backward to eat the base character too.
            if (isEmojiCombiner(cp) ||
                (cp == 0x200D))
            {
                keep_deleting = true;
            }

            // --- RULE B: Look-ahead (technically Look-behind) for ZWJ ---
            // Even if the current char is normal (e.g., "Boy" emoji), if the *previous*
            // char is a ZWJ, this current char is part of a sequence (e.g. Family).
            if (prev_ptr > start) {
                const char* lookback_ptr = fl_utf8back(prev_ptr - 1, start, prev_ptr);
                unsigned int prev_cp = fl_utf8decode(lookback_ptr, prev_ptr, NULL);

                // If the char BEFORE the one we just ate is a ZWJ, we must eat it too.
                if (prev_cp == 0x200D) {
                    keep_deleting = true;
                }
            
                // --- RULE C: Regional Indicators (Flags) ---
                // Flags are two RI characters (e.g. US = U + S). We should delete both.
                bool is_curr_ri = (cp >= 0x1F1E6 && cp <= 0x1F1FF);
                bool is_prev_ri = (prev_cp >= 0x1F1E6 && prev_cp <= 0x1F1FF);
            
                if (is_curr_ri && is_prev_ri) {
                    keep_deleting = true;
                }
            }
        }

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
        const char* new_ptr = current_ptr;

        bool keep_moving = true;
        while (keep_moving && new_ptr > start) {
            // Step back one code point
            const char* temp_ptr = fl_utf8back(new_ptr - 1, start, new_ptr);
            unsigned int cp = fl_utf8decode(temp_ptr, new_ptr, NULL);
        
            new_ptr = temp_ptr;
            keep_moving = false;

            // Rule: If we land on a combiner/modifier, we must go back further
            if ((cp >= 0x0300 && cp <= 0x036F) || (cp >= 0x1AB0 && cp <= 0x1AFF) ||
                (cp >= 0x20D0 && cp <= 0x20FF) || (cp >= 0xFE00 && cp <= 0xFE0F) ||
                (cp >= 0x1F3FB && cp <= 0x1F3FF) || (cp == 0x200D)) {
                keep_moving = true;
            }

            // Rule: If the char BEFORE the one we just landed on is a ZWJ, keep going
            if (new_ptr > start) {
                const char* lookback = fl_utf8back(new_ptr - 1, start, new_ptr);
                if (fl_utf8decode(lookback, new_ptr, NULL) == 0x200D) {
                    keep_moving = true;
                }
            }
        }

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
    
        int len = 0;
        unsigned int current_cp = fl_utf8decode(current_ptr, end, &len);
        if (len < 1) len = 1;
    
        const char* new_ptr = current_ptr + len;
        bool keep_scanning = true;

        while (keep_scanning && new_ptr < end) {
            int next_len = 0;
            unsigned int next_cp = fl_utf8decode(new_ptr, end, &next_len);
            if (next_len < 1) next_len = 1;

            keep_scanning = false;

            // 1. Check for modifiers/combiners
            if (isEmojiCombiner(next_cp))
            {
                new_ptr += next_len;
                keep_scanning = true;
            }
            // 2. Check for ZWJ (Joiner)
            else if (next_cp == 0x200D)
            {
                new_ptr += next_len; // eat ZWJ
                if (new_ptr < end) {
                    int joined_len = 0;
                    fl_utf8decode(new_ptr, end, &joined_len);
                    new_ptr += (joined_len > 0) ? joined_len : 1;
                    keep_scanning = true;
                }
            }
            // 3. Check for Regional Indicator (Flag) pairs
            else {
                bool is_curr_ri = (current_cp >= 0x1F1E6 && current_cp <= 0x1F1FF);
                bool is_next_ri = (next_cp >= 0x1F1E6 && next_cp <= 0x1F1FF);
                if (is_curr_ri && is_next_ri) {
                    new_ptr += next_len;
                }
            }
            current_cp = next_cp;
        }

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
    
        // 1. Get the length of the CURRENT character to be deleted
        int first_char_len = 0;
        // fl_utf8decode returns the codepoint and puts the byte length in first_char_len
        unsigned int current_cp = fl_utf8decode(current_ptr, end, &first_char_len);
    
        // Safety check: ensure we advance at least 1 byte to prevent infinite loops/zero delete
        if (first_char_len < 1) first_char_len = 1;

        // Set our deletion end pointer to the end of the first character
        const char* delete_end_ptr = current_ptr + first_char_len;
    
        bool keep_scanning = true;

        // 2. Look ahead for combiners
        while (keep_scanning && delete_end_ptr < end) {
        
            int next_char_len = 0;
            unsigned int next_cp = fl_utf8decode(delete_end_ptr, end, &next_char_len);
            if (next_char_len < 1) next_char_len = 1;

            keep_scanning = false; // Assume we stop unless we find a combiner

            // --- RULE A: The NEXT char is a Modifier/Combiner ---
            if (isEmojiCombiner(next_cp))
            {
                delete_end_ptr += next_char_len;
                keep_scanning = true;
            }

            // --- RULE B: Zero Width Joiner (ZWJ) Sequences ---
            else if (next_cp == 0x200D)
            {
                // Consume the ZWJ
                delete_end_ptr += next_char_len;
            
                // Check if there is a character AFTER the ZWJ
                if (delete_end_ptr < end) {
                    int after_zwj_len = 0;
                    fl_utf8decode(delete_end_ptr, end, &after_zwj_len);
                    if (after_zwj_len < 1) after_zwj_len = 1;

                    // Consume the character that the ZWJ was joining
                    delete_end_ptr += after_zwj_len;
                
                    // Keep scanning, because that character might be followed by another ZWJ
                    keep_scanning = true;
                }
            }
        
            // --- RULE C: Regional Indicators (Flags) ---
            else {
                bool is_curr_ri = (current_cp >= 0x1F1E6 && current_cp <= 0x1F1FF);
                bool is_next_ri = (next_cp >= 0x1F1E6 && next_cp <= 0x1F1FF);

                // If we are deleting a flag part, and the next one is also a flag part, delete both.
                if (is_curr_ri && is_next_ri) {
                    delete_end_ptr += next_char_len;
                    keep_scanning = false; // Flags are only pairs, we can stop
                }
            }
        
            // Update current_cp to the last character we "ate" (needed for chained logic if we loop)
            current_cp = next_cp;
        }

        // 3. Perform the erase
        // Calculate strict integer length
        unsigned int delete_len = (unsigned int)(delete_end_ptr - current_ptr);
        text.erase(utf8_pos, delete_len);
    
        to_cursor();
        return 1;
    }
    
    int VKTextShape::handle_mouse_click(int event, const math::Vector2i& local)
    {
        file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        const image::FontInfo fontInfo(fontFamily, fontSize);
        
        // Copy the text to process it line by line
        std::string txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i cursor_pos(x, y);
        std::size_t pos = txt.find('\n');

        utf8_pos = 0;
        const char* text_start = text.c_str();
        const char* text_it = text_start; 
        const char* text_end = text_it + text.size();
        
        for (; pos != std::string::npos; pos = txt.find('\n'))
        {
            const std::string line = txt.substr(0, pos);
            const auto& glyphs = fontSystem->getGlyphs(line, fontInfo);
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
                            text_it = fl_utf8fwd(text_it + 1, text_start, text_end);
                            utf8_pos += (text_it - old_it);
                        }
                    }
                }
            }
            if (txt.size() > pos)
            {
                txt = txt.substr(pos + 1, txt.size());
                if (local.y > cursor_pos.y)
                {
                    cursor_pos.x = x;
                    cursor_pos.y += fontSize;
                    if (text_it < text_end && *text_it == '\n')
                    {
                        utf8_pos += 1;
                        text_it++; // Advance past the single-byte newline
                    }
                }
            }
        }
        if (!txt.empty())
        {
            const auto& glyphs = fontSystem->getGlyphs(txt, fontInfo);
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (local.x > (cursor_pos.x + glyph->advance / 2) ||
                        local.y > cursor_pos.y)
                    {
                        cursor_pos.x += glyph->advance;
                        
                        // --- Update utf8_pos using 3-parameter fl_utf8fwd ---
                        if (text_it < text_end)
                        {
                            const char* old_it = text_it;
                            text_it = fl_utf8fwd(text_it + 1, text_start, text_end); 
                            utf8_pos += (text_it - old_it);
                        }
                    }
                }
            }
        }
        to_cursor();
        return 1;
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
        int ascender = fontMetrics.ascender;
        int descender = fontMetrics.descender;

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

    void to_json(nlohmann::json& json, const VKPathShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "DrawPath";
    }

    void from_json(const nlohmann::json& json, VKPathShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const VKPolygonShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Polygon";
    }

    void from_json(const nlohmann::json& json, VKPolygonShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const VKFilledPolygonShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "FilledPolygon";
    }

    void from_json(const nlohmann::json& json, VKFilledPolygonShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
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

    void to_json(nlohmann::json& json, const VKLinkShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Link";
        json["url"] = value.url;
        json["title"] = value.title;
    }

    void from_json(const nlohmann::json& json, VKLinkShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("url").get_to(value.url);
        json.at("title").get_to(value.title);
        replace_path(value.url);
    }
    
    void to_json(nlohmann::json& json, const VKCircleShape& value)
    {
        to_json(json, static_cast<const draw::Shape&>(value));
        json["type"] = "Circle";
        json["center"] = value.center;
        json["radius"] = value.radius;
    }

    void from_json(const nlohmann::json& json, VKCircleShape& value)
    {
        from_json(json, static_cast<draw::Shape&>(value));
        json.at("center").get_to(value.center);
        json.at("radius").get_to(value.radius);
    }

    void to_json(nlohmann::json& json, const VKFilledCircleShape& value)
    {
        to_json(json, static_cast<const draw::Shape&>(value));
        json["type"] = "FilledCircle";
        json["center"] = value.center;
        json["radius"] = value.radius;
    }

    void from_json(const nlohmann::json& json, VKFilledCircleShape& value)
    {
        from_json(json, static_cast<draw::Shape&>(value));
        json.at("center").get_to(value.center);
        json.at("radius").get_to(value.radius);
    }

    void to_json(nlohmann::json& json, const VKArrowShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Arrow";
    }

    void from_json(const nlohmann::json& json, VKArrowShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const VKRectangleShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Rectangle";
    }

    void from_json(const nlohmann::json& json, VKRectangleShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const VKFilledRectangleShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "FilledRectangle";
    }

    void from_json(const nlohmann::json& json, VKFilledRectangleShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const VKErasePathShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "ErasePath";
        json["rectangle"] = value.rectangle;
    }

    void from_json(const nlohmann::json& json, VKErasePathShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("rectangle").get_to(value.rectangle);
    }

    void VKVoiceOverShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const voice::MouseData& mouse)
    {
        const image::Color4f color(1.F, 0.5F, 0.7F);
        const image::Color4f stoppedColor(0.7F, 0.4F, 6.F);
        const image::Color4f recordingColor(blinkingIndex / 255.F, 0.F, 0.F);
        const image::Color4f yellowColor(1.F, 1.F, 0.F);
        const image::Color4f blackColor(0.F, 0.F, 0.F);
        const image::Color4f lineColor(0.F, 0.F, 1.F);

        image::Color4f cursorColor(1.F, 1.F, 1.F);

        const int radius = 3 * mult;
        const int boxRadius = 6 * mult;
        const int lineSize = 3 * mult;

        if (mouse.pressed)
            cursorColor = image::Color4f(0.F, 1.F, 0.F);

        const math::Vector2i c(int(center.x), int(center.y));
        const math::Vector2i e(int(mouse.pos.x), int(mouse.pos.y));
        
        switch(status)
        {
        case voice::RecordStatus::Stopped:
        {
            //
            // Draw box and rectangle icon
            //
            math::Box2i box(c.x - boxRadius, c.y - boxRadius,
                            boxRadius * 2, boxRadius * 2);
            render->drawRect(box, stoppedColor);
            
            box = math::Box2i(c.x - radius, c.y - radius,
                              radius * 2, radius * 2);
            render->drawRect(box, blackColor);
            break;
        }
        case voice::RecordStatus::Saved:
        {
            //
            // Draw box and icon
            //
            vulkan::Lines lines(render->getContext(), VK_NULL_HANDLE);
            math::Box2i box(c.x - boxRadius, c.y - boxRadius,
                            boxRadius * 2, boxRadius * 2);
            render->drawRect(box, stoppedColor);

            unsigned numSides = 4; // A rotated square
            lines.drawFilledCircle(render, center, radius, yellowColor, numSides);
            break;
        }
        case voice::RecordStatus::Playing:
        {
            //
            // Draw the connecting line and the cursor
            //
            vulkan::Lines lines(render->getContext(), VK_NULL_HANDLE);
            lines.drawLine(render, c, e, lineColor, 2);
            
            math::Box2i box(int(e.x - radius), int(e.y - radius),
                            radius * 2, radius * 2);
            render->drawRect(box, cursorColor);


            //
            // Draw box and a triangle icon
            //
            box = math::Box2i(c.x - boxRadius, c.y - boxRadius,
                              boxRadius * 2, boxRadius * 2);
            render->drawRect(box, stoppedColor);

            unsigned numSides = 3;
            lines.drawFilledCircle(render, center, radius, blackColor,
                                   numSides);
            break;
        }
        case voice::RecordStatus::Recording:
        {
            //
            // Draw the connecting line and the cursor
            //
            vulkan::Lines lines(render->getContext(), VK_NULL_HANDLE);
            lines.drawLine(render, c, e, lineColor, 2);
            
            math::Box2i box(int(e.x - radius), int(e.y - radius),
                            radius * 2, radius * 2);
            render->drawRect(box, cursorColor);
            
            //
            // Draw box and icon
            //
            box = math::Box2i(c.x - boxRadius, c.y - boxRadius,
                              boxRadius * 2, boxRadius * 2);
            render->drawRect(box, stoppedColor);
            
            lines.drawFilledCircle(render, center, radius, recordingColor);

            break;
        }
        default:
            throw std::runtime_error("Unknown voice::RecordStatus");
            break;
        }

    }
} // namespace mrv
