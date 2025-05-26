// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"

#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShape.h"

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMath.h"

#include <tlVk/Shader.h>
#include <tlVk/Util.h>

namespace
{
    const int kCrossSize = 10;
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

        
        const bool catmullRomSpline = false;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline, false, "erase");
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

        // gl::SetAndRestore(VK_BLEND, VK_TRUE);

        // glBlendFuncSeparate(
        //     VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ONE,
        //     VK_ONE_MINUS_SRC_ALPHA);

        bool catmullRomSpline = false;
        std::vector< Point > line;

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

    int VKTextShape::accept()
    {
        return App::ui->uiView->acceptMultilineInput();
    }
    
    int VKTextShape::paste()
    {
        if (!Fl::event_text() || !Fl::event_length()) return 1;
            
        const char* t = Fl::event_text();
        const char* e = t + Fl::event_length();
        char* copy = new char[e - t + 1];

        memcpy(copy, t, e - t + 1);
        copy[e - t] = 0;

        std::string right;
        std::string left;
        if (cursor > 0)
            left = text.substr(0, cursor);
        if (cursor + 1 < text.size())
            right = text.substr(cursor + 1, text.size());

        text = left + copy + right;
        cursor = text.size();
            
        delete [] copy;
        return 1;
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
        case FL_ENTER:
        case FL_LEAVE:
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_PASTE:
        {
            return paste();
            break;
        }
        case FL_KEYBOARD:
            if (Fl::event_ctrl() && (rawkey == 'v' || rawkey == 'V'))
                return paste();
            switch(rawkey)
            {
            case FL_Escape:
                text = "";
                return accept();
                break;
            case FL_Delete:
            {
                if (text.empty())
                    break;
                int len = 1;
                size_t textLen = text.size();
                if (text[cursor] == '\n')
                    len = 2;
                std::string right;
                if (cursor < textLen - len)
                    right = text.substr(cursor + len, textLen);
                text = text.substr(0, cursor);
                text += right;
                break;
            }
            case FL_BackSpace:
            {
                if (text.empty())
                    break;
                int len = 1;
                size_t textLen = text.size();
                if (text[cursor] == '\n')
                    len = 2;
                std::string right;
                if (cursor < textLen - len + 1)
                    right = text.substr(cursor + 1, textLen);
                text = text.substr(0, cursor - len);
                text += right;
                cursor -= 1;
                break;
            }
            case FL_Left:
                if (cursor == 0)
                    break;
                --cursor;
                if (text[cursor] == '\n')
                    --cursor;
                break;
            case FL_Right:
                if (cursor >= text.size())
                    break;
                ++cursor;
                if (text[cursor] == '\n')
                    ++cursor;
                break;
            case FL_Enter:
            case FL_KP_Enter:
                text += '\n';
                ++cursor;
                break;
            default:
            {
                std::string key = Fl::event_text();
                if (key != "")
                {
                    text += Fl::event_text();
                    ++cursor;
                }
                break;
            }
            }
            return 1;
            break;
        }
        return 0;
    }
    
    void VKTextShape::draw(
        const std::shared_ptr<timeline_vlk::Render>& render,
        const std::shared_ptr<vulkan::Lines> lines)
    {
        if (!fontSystem->hasFont(fontFamily))
        {
            fontSystem->addFont(fontPath);
        }
        
        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);
        int ascender = fontMetrics.ascender;
        int descender = fontMetrics.descender;
        int height = fontSize; //fontMetrics.lineHeight;

        // Copy the text to process it
        std::string txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i cursor_pos(x, y);
        math::Vector2i pnt(x, y);
        std::size_t pos = txt.find('\n');
        std::vector<timeline::TextInfo> textInfos;
        unsigned cursor_count = 0;
        for (; pos != std::string::npos; y += height, pos = txt.find('\n'))
        {
            cursor_pos.x = x;
            cursor_pos.y += height;
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
                }
            }
        }
        if (!txt.empty())
        {
            cursor_pos.x = x;
            cursor_pos.y = y;
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
        math::Box2i cursorBox(cursor_pos.x, cursor_pos.y - ascender - descender, 2, height);
            
        if (editing)
        {
            box = math::Box2i(pts[0].x, pts[0].y - height / 2, 70, height / 2);
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
            
            box.expand(cursorBox);
            
            const image::Color4f bgcolor(0.F, 0.F, 0.F, 0.5F);
            box = box.margin(4);
            render->drawRect(box, bgcolor);
            
            box = box.margin(4);
            box.expand(cursorBox);
            render->drawRect(box, bgcolor);

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
        for (const auto& textInfo : textInfos)
        {
            render->drawText(textInfo, math::Vector2i(), color);
        }
        if (editing)
        {
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
        json["fontFamily"] = value.fontFamily;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, VKTextShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("fontFamily").get_to(value.fontFamily);
        json.at("fontSize").get_to(value.fontSize);
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
    }

    void from_json(const nlohmann::json& json, VKErasePathShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

} // namespace mrv
