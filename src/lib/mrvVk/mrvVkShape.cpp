// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"
#include "mrvURLLinkUI.h"

#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShape.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMath.h"

#include <tlVk/Shader.h>
#include <tlVk/Util.h>

#include <tlCore/StringFormat.h>

#include <FL/filename.H>
#include <FL/fl_utf8.h>

namespace
{
    const int kCrossSize = 10;
    const char* kModule = "shape";
}

namespace
{
    using tl::geom::Triangle2;
    using tl::math::Vector2f;

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

    std::vector<Triangle2>
    triangulatePolygon(std::vector<Vector2f>& points, std::vector<int>& poly)
    {
        std::vector<Triangle2> triangles;

        Triangle2 triangle;

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

    void VKLinkShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        using namespace mrv::draw;

        const bool catmullRomSpline = false;
        std::vector< draw::Point > line;

        line.push_back(pts[0]);
        line.push_back(pts[1]);
        line.push_back(pts[2]);
        lines->drawLines(
            render, line, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
        
        math::Vector2f center;
        center.x = (pts[0].x + pts[1].x + pts[2].x) / 3;
        center.y = (pts[0].y + pts[1].y + pts[2].y) / 3;
        float radius = std::abs(pts[0].y - pts[1].y) * 1.05;
        
        lines->drawCircle(render, center, radius, pen_size, color, soft);
    }

    void VKLinkShape::open()
    {
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
                open();
                return 1;
            }
            else if (Fl::event_button2())
            {
                edit();
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
        const char* start   = text.c_str();
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
        const char* start   = text.c_str();
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
        unsigned size = fl_utf8toa(text.c_str(), utf8_pos, nullptr, 0);
        cursor = size;
    }
    
    int VKTextShape::paste()
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
    
    int VKTextShape::handle(int e)
    {
        unsigned rawkey = Fl::event_key();
        if ((rawkey == FL_KP_Enter || rawkey == FL_Enter) &&
            Fl::event_shift())
        {
            return accept();
        }
             
        switch(e)
        {
        case FL_PASTE:
        {
            return paste();
            break;
        }
        case FL_KEYBOARD:
        {
            int del = 0;
            if (!Fl::compose(del))
            {
                switch(rawkey)
                {
                case FL_Escape:
                    text = "";
                    return accept();
                    break; 
                case FL_Delete:
                {
                    if (utf8_pos >= text.size())
                        break;
                    const char* current = text.c_str() + utf8_pos;
                    const char* start = current;
                    const char* end = start + text.size() - utf8_pos;
                    const char* pos = fl_utf8fwd(current + 1, start, end);
                    unsigned next = pos - text.c_str();
                    std::string left;
                    std::string right;
                    if (utf8_pos > 0)
                        left = text.substr(0, utf8_pos);
                    if (next < text.size())
                        right = text.substr(next, text.size());
                    text = left + right;
                    break;
                }
                case FL_BackSpace:
                {
                    if (utf8_pos == 0)
                        break;
                    unsigned last = utf8_pos;
                    const char* current = text.c_str() + utf8_pos;
                    const char* start = text.c_str();
                    const char* end = current;
                    const char* pos = fl_utf8back(current - 1, start, end);
                    utf8_pos = pos - text.c_str();
                    std::string left;
                    std::string right;
                    if (utf8_pos > 0)
                        left = text.substr(0, utf8_pos);
                    if (last < text.size())
                        right = text.substr(last, text.size());
                    text = left + right;
                    Fl::compose_reset();
                    break;
                }
                case FL_Up:
                {
                    unsigned row = current_line();
                    if (row == 0)
                        break;
                    unsigned column = current_column();
                    unsigned start = line_start(utf8_pos);
                    start = line_start(start-2);  // 2 to skip \n
                    const char* pos = advance_to_column(start, column);
                    utf8_pos = pos - text.c_str();
                    Fl::compose_reset();
                    break;
                }
                case FL_Down:
                {
                    unsigned column = current_column();
                    unsigned end = line_end(utf8_pos);
                    if (end == text.size())
                        break;
                    unsigned start = line_start(end+1);
                    const char* pos = advance_to_column(start, column);
                    utf8_pos = pos - text.c_str();
                    Fl::compose_reset();
                    break;
                }
                case FL_Left:
                {
                    if (utf8_pos == 0)
                        break;
                    const char* current = text.c_str() + utf8_pos;
                    const char* start = text.c_str();
                    const char* end = current;
                    const char* pos = fl_utf8back(current - 1, start, end);
                    utf8_pos = pos - text.c_str();
                    Fl::compose_reset();
                    break;
                }
                case FL_Right:
                {
                    if (utf8_pos >= text.size())
                        break;
                    const char* current = text.c_str() + utf8_pos;
                    const char* start = current;
                    const char* end = start + text.size() - utf8_pos;
                    const char* pos = fl_utf8fwd(current + 1, start, end);
                    utf8_pos = pos - text.c_str();
                    Fl::compose_reset();
                    break;
                }
                case FL_Enter:
                case FL_KP_Enter:
                {
                    std::string left;
                    std::string right;
                    if (utf8_pos > 0)
                        left = text.substr(0, utf8_pos); 

                    const char* current = text.c_str() + utf8_pos;
                    const char* start = current;
                    const char* end = start + text.size() - utf8_pos;
                    const char* pos = fl_utf8fwd(current + 1, start, end);
                    if (utf8_pos < text.size())
                        right = text.substr(utf8_pos, text.size());
                
                    text = left + '\n' + right;
                    utf8_pos += Fl::event_length();
                    Fl::compose_reset();
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                int len = Fl::event_length();
                std::string key(Fl::event_text(), len);
                if (del > 0 || Fl::event_text() && len > 0)
                {
                    std::string left;
                    std::string right;
                    if (utf8_pos > 0)
                        left = text.substr(0, utf8_pos - del); 
                    
                    const char* current = text.c_str() + utf8_pos;
                    const char* start = current;
                    const char* end = start + text.size() - utf8_pos;
                    const char* pos = fl_utf8fwd(current + 1, start, end);
                    
                    if (utf8_pos < text.size())
                        right = text.substr(utf8_pos, text.size());
                    
                    text = left + key + right;
                    utf8_pos += len - del;
                }
            }
            to_cursor();
            return 1;
            break;
        }
        }
        return 0;
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
                    // --- Update utf8_pos for the newline ('\n') ---
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
    
    void VKTextShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        file::Path path(fontPath);
        const std::string fontFamily = path.getBaseName();
        if (!fontSystem->hasFont(fontFamily))
        {
            fontSystem->addFont(fontPath);
        }
        
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);
        int ascender = fontMetrics.ascender;
        int descender = fontMetrics.descender;

        // Copy the text to process it line by line
        std::string txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i cursor_pos(x, y);
        math::Vector2i pnt(x, y);
        std::size_t pos = txt.find('\n');
        std::vector<timeline::TextInfo> textInfos;
        unsigned cursor_count = 0;
        for (; pos != std::string::npos; y += fontSize, pos = txt.find('\n'))
        {
            pnt.y = y;
            
            const std::string line = txt.substr(0, pos);
            const auto& glyphs = fontSystem->getGlyphs(line, fontInfo);
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (cursor_count < cursor)
                    {
                        cursor_pos.x += glyph->advance;
                    }
                }
                ++cursor_count;
            }
            render->appendText(textInfos, glyphs, pnt);
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
            pnt.y = y;
            const auto& glyphs = fontSystem->getGlyphs(txt, fontInfo);
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (cursor_count < cursor)
                    {
                        cursor_pos.x += glyph->advance;
                    }
                }
                ++cursor_count;
            }
            render->appendText(textInfos, glyphs, pnt);
        }

        const image::Color4f cursorColor(.8F, 0.8F, 0.8F);
        math::Box2i cursorBox(cursor_pos.x,
                              cursor_pos.y - ascender + descender, 2, fontSize);
            
        if (editing)
        {
            box = math::Box2i(pts[0].x, pts[0].y - fontSize / 2,
                              70, fontSize / 2);
            for (const auto& textInfo : textInfos)
            {
                for (const auto& v : textInfo.mesh.v)
                {
                    if (v.x < box.min.x)
                        box.min.x = v.x;
                    if (v.y < box.min.y)
                        box.min.y = v.y;
                    if (v.x > box.max.x)
                        box.max.x = v.x;
                    if (v.y > box.max.y)
                        box.max.y = v.y;
                }
            }

            //
            // Make room in box for cursor
            //
            box.expand(cursorBox);

            //
            // Draw background which will be darker
            //
            const image::Color4f bgcolor(0.F, 0.F, 0.F, 0.5F);
            box = box.margin(4);
            render->drawRect(box, bgcolor);

            //
            // Draw second background (lighter)
            //
            box = box.margin(4);
            box.expand(cursorBox);
            render->drawRect(box, bgcolor);

            //
            // Draw cross
            //
            image::Color4f crossColor(0.F, 1.F, 0.F);
            if (text.empty())
                crossColor = image::Color4f(1.F, 0.F, 0.F);
            
            math::Vector2i start(box.min.x, box.min.y);
            math::Vector2i end(box.min.x + kCrossSize, box.min.y + kCrossSize);
            lines->drawLine(render, start, end, crossColor, 2);
            
            start = math::Vector2i(box.min.x + kCrossSize, box.min.y);
            end = math::Vector2i(box.min.x, box.min.y + kCrossSize);
            lines->drawLine(render, start, end, crossColor, 2);

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
        to_json(json, static_cast<const draw::Shape&>(value));
        json["type"] = "Link";
        json["url"] = value.url;
        json["title"] = value.title;
    }

    void from_json(const nlohmann::json& json, VKLinkShape& value)
    {
        from_json(json, static_cast<draw::Shape&>(value));
        json.at("url").get_to(value.url);
        json.at("title").get_to(value.title);
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
