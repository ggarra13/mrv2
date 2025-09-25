// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMath.h"

#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include "mrvGLUtil.h"
#include "mrvGLShape.h"

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

    void GLPathShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace tl;
        using namespace mrv::draw;

        CHECK_GL;
        gl::SetAndRestore(GL_BLEND, GL_TRUE);
        CHECK_GL;

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL;

        const bool catmullRomSpline = true;
        CHECK_GL;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
        CHECK_GL;
    }

    void GLErasePathShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

        color.r = color.g = color.b = 0.F;
        color.a = 1.F;

        if (rectangle)
        {           
            math::Box2i box(
                pts[0].x, pts[0].y, pts[2].x - pts[0].x, pts[2].y - pts[0].y);
            render->drawRect(box, color, "erase");
        }
        else
        {
            const bool catmullRomSpline = false;
            lines->drawLines(
                render, pts, color, pen_size, soft,
                Polyline2D::JointStyle::ROUND,
                Polyline2D::EndCapStyle::ROUND, catmullRomSpline);
        }
    }

    void GLPolygonShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

        const bool catmullRomSpline = false;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::JOINT, catmullRomSpline);
    }

    void GLCircleShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

        lines->drawCircle(render, center, radius, pen_size, color, soft);
    }

    void GLRectangleShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

        const bool catmullRomSpline = false;
        lines->drawLines(
            render, pts, color, pen_size, soft, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::JOINT, catmullRomSpline);
    }

    void GLFilledPolygonShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

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
        render->drawMesh(mesh, pos, color);
    }

    void GLFilledCircleShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

        math::Vector2i v(center.x, center.y);
        drawFilledCircle(render, v, radius, color, false);
    }

    void GLFilledRectangleShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

        math::Box2i box(
            pts[0].x, pts[0].y, pts[2].x - pts[0].x, pts[2].y - pts[0].y);
        render->drawRect(box, color);
    }

    void GLArrowShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        using namespace mrv::draw;

        gl::SetAndRestore(GL_BLEND, GL_TRUE);

        glBlendFuncSeparate(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
            GL_ONE_MINUS_SRC_ALPHA);

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

    void GLTextShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<opengl::Lines>& lines)
    {
        if (text.empty())
            return;

        const image::FontInfo fontInfo(fontFamily, fontSize);
        const image::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);
        auto height = fontMetrics.lineHeight;
        if (height == 0)
            throw std::runtime_error(_("Invalid font for text drawing"));

        // Set the projection matrix
        math::Matrix4x4f oldMatrix = render->getTransform();
        render->setTransform(matrix);

        // Copy the text to process it
        txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i pnt(x, y);
        std::size_t pos = txt.find('\n');
        std::vector<timeline::TextInfo> textInfos;
        for (; pos != std::string::npos; y += height, pos = txt.find('\n'))
        {
            pnt.y = y;
            std::string line = txt.substr(0, pos);
            const auto glyphs = fontSystem->getGlyphs(line, fontInfo);
            render->appendText(textInfos, glyphs, pnt);
            if (txt.size() > pos)
                txt = txt.substr(pos + 1, txt.size());
        }
        if (!txt.empty())
        {
            pnt.y = y;
            
            const auto glyphs = fontSystem->getGlyphs(txt, fontInfo);
            render->appendText(textInfos, glyphs, pnt);
        }
        for (const auto& textInfo : textInfos)
        {
            render->drawText(textInfo, math::Vector2i(), color);
        }
        render->setTransform(oldMatrix);
    }

    void to_json(nlohmann::json& json, const GLPathShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "DrawPath";
    }

    void from_json(const nlohmann::json& json, GLPathShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLPolygonShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Polygon";
    }

    void from_json(const nlohmann::json& json, GLPolygonShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLFilledPolygonShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "FilledPolygon";
    }

    void from_json(const nlohmann::json& json, GLFilledPolygonShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLTextShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Text";
        json["text"] = value.text;
        json["fontFamily"] = value.fontFamily;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, GLTextShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("fontFamily").get_to(value.fontFamily);
        json.at("fontSize").get_to(value.fontSize);
    }

#ifdef USE_OPENGL2
    void to_json(nlohmann::json& json, const GL2TextShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "GL2Text";
        json["text"] = value.text;
        json["font"] = value.font;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, GL2TextShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("font").get_to(value.font);
        json.at("fontSize").get_to(value.fontSize);
    }
#endif

    void to_json(nlohmann::json& json, const GLCircleShape& value)
    {
        to_json(json, static_cast<const draw::Shape&>(value));
        json["type"] = "Circle";
        json["center"] = value.center;
        json["radius"] = value.radius;
    }

    void from_json(const nlohmann::json& json, GLCircleShape& value)
    {
        from_json(json, static_cast<draw::Shape&>(value));
        json.at("center").get_to(value.center);
        json.at("radius").get_to(value.radius);
    }

    void to_json(nlohmann::json& json, const GLFilledCircleShape& value)
    {
        to_json(json, static_cast<const draw::Shape&>(value));
        json["type"] = "FilledCircle";
        json["center"] = value.center;
        json["radius"] = value.radius;
    }

    void from_json(const nlohmann::json& json, GLFilledCircleShape& value)
    {
        from_json(json, static_cast<draw::Shape&>(value));
        json.at("center").get_to(value.center);
        json.at("radius").get_to(value.radius);
    }

    void to_json(nlohmann::json& json, const GLArrowShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Arrow";
    }

    void from_json(const nlohmann::json& json, GLArrowShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLRectangleShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "Rectangle";
    }

    void from_json(const nlohmann::json& json, GLRectangleShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLFilledRectangleShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "FilledRectangle";
    }

    void from_json(const nlohmann::json& json, GLFilledRectangleShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLErasePathShape& value)
    {
        to_json(json, static_cast<const draw::PathShape&>(value));
        json["type"] = "ErasePath";
        json["rectangle"] = value.rectangle;
    }

    void from_json(const nlohmann::json& json, GLErasePathShape& value)
    {
        from_json(json, static_cast<draw::PathShape&>(value));
        json.at("rectangle").get_to(value.rectangle);
    }

    void GLVoiceOverShape::draw(
        const std::shared_ptr<timeline::IRender>& render,
        const voice::MouseData& mouse)
    {
        const image::Color4f color(1.F, 0.5F, 0.7F);
        const image::Color4f stoppedColor(0.7F, 0.4F, 6.F);
        const image::Color4f recordingColor(1.F, 0.F, 0.F);
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
            opengl::Lines lines;
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
            opengl::Lines lines;
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
            opengl::Lines lines;
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
