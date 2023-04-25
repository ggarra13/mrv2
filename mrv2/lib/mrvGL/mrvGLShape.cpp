// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvGLUtil.h"
#include "mrvGLShape.h"

#include "mrvCore/mrvI8N.h"

#include <tlGlad/gl.h>

#include <glm/gtc/matrix_transform.hpp>

namespace mrv
{
    void GLPathShape::draw(const std::shared_ptr<timeline::IRender>& render)
    {
        using namespace tl::draw;

        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        drawLines(
            render, pts, color, pen_size, matrix, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND);
    }

    void GLCircleShape::draw(const std::shared_ptr<timeline::IRender>& render)
    {
        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        drawCursor(render, center, radius, pen_size, color, matrix);
    }

    void GLTextShape::draw(const std::shared_ptr<timeline::IRender>& render)
    {
        if (text.empty())
            return;

        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics =
            fontSystem->getMetrics(fontInfo);
        auto height = fontMetrics.lineHeight;
        if (height == 0)
            throw std::runtime_error(_("Invalid font for text drawing"));

        // Set the projection matrix
        render->setTransform(matrix);

        // Copy the text to process it
        txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i pnt(x, y);
        std::size_t pos = txt.find('\n');
        for (; pos != std::string::npos; y += height, pos = txt.find('\n'))
        {
            pnt.y = y;
            std::string line = txt.substr(0, pos);
            const auto glyphs = fontSystem->getGlyphs(line, fontInfo);
            render->drawText(glyphs, pnt, color);
            if (txt.size() > pos)
                txt = txt.substr(pos + 1, txt.size());
        }
        if (!txt.empty())
        {
            pnt.y = y;
            const auto glyphs = fontSystem->getGlyphs(txt, fontInfo);
            render->drawText(glyphs, pnt, color);
        }
    }

    void
    GLErasePathShape::draw(const std::shared_ptr<timeline::IRender>& render)
    {
        using namespace tl::draw;
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // Set 1 into the stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

        drawLines(
            render, pts, color, pen_size, matrix, Polyline2D::JointStyle::ROUND,
            Polyline2D::EndCapStyle::ROUND);
    }

    void to_json(nlohmann::json& json, const GLPathShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "DrawPath";
    }

    void from_json(const nlohmann::json& json, GLPathShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLTextShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "Text";
        json["text"] = value.text;
        json["fontFamily"] = value.fontFamily;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, GLTextShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("fontFamily").get_to(value.fontFamily);
        json.at("fontSize").get_to(value.fontSize);
    }

    void to_json(nlohmann::json& json, const GL2TextShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "GL2Text";
        json["text"] = value.text;
        json["font"] = value.font;
        json["fontSize"] = value.fontSize;
    }

    void from_json(const nlohmann::json& json, GL2TextShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
        json.at("text").get_to(value.text);
        json.at("font").get_to(value.font);
        json.at("fontSize").get_to(value.fontSize);
    }

    void to_json(nlohmann::json& json, const GLCircleShape& value)
    {
        to_json(json, static_cast<const tl::draw::Shape&>(value));
        json["type"] = "Circle";
        json["center"] = value.center;
        json["radius"] = value.radius;
    }

    void from_json(const nlohmann::json& json, GLCircleShape& value)
    {
        from_json(json, static_cast<tl::draw::Shape&>(value));
        json.at("center").get_to(value.center);
        json.at("radius").get_to(value.radius);
    }

    void to_json(nlohmann::json& json, const GLArrowShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "Arrow";
    }

    void from_json(const nlohmann::json& json, GLArrowShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLRectangleShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "Rectangle";
    }

    void from_json(const nlohmann::json& json, GLRectangleShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
    }

    void to_json(nlohmann::json& json, const GLErasePathShape& value)
    {
        to_json(json, static_cast<const tl::draw::PathShape&>(value));
        json["type"] = "ErasePath";
    }

    void from_json(const nlohmann::json& json, GLErasePathShape& value)
    {
        from_json(json, static_cast<tl::draw::PathShape&>(value));
    }

} // namespace mrv
