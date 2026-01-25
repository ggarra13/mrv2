// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvURLLinkUI.h"

#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShape.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPathMapping.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMath.h"

#include <tlVk/Shader.h>
#include <tlVk/Util.h>

#include <tlCore/StringFormat.h>

#include <FL/filename.H>

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
