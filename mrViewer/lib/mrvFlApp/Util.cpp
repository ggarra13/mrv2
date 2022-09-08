// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "Util.h"

#include <tlCore/Color.h>
#include <tlCore/Math.h>

#include <cmath>

namespace mrv
{

    void drawHUDLabel(
        const std::shared_ptr<timeline::IRender>& render,
        const std::shared_ptr<imaging::FontSystem>& fontSystem,
        const imaging::Size& window,
        const std::string& text,
        imaging::FontFamily fontFamily,
        uint16_t fontSize,
        HUDElement hudElement)
    {
        const imaging::Color4f labelColor(1.F, 1.F, 1.F);
        const imaging::Color4f overlayColor(0.F, 0.F, 0.F, .7F);

        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);

        const int margin = fontSize;
        const math::BBox2i marginBBox = math::BBox2i(0, 0, window.w, window.h).margin(-margin);
        const math::Vector2i labelSize = fontSystem->measure(text, fontInfo);
        const int labelMargin = fontSize / 5;
        math::BBox2i bbox;
        math::Vector2i pos;
        switch (hudElement)
        {
        case HUDElement::UpperLeft:
            bbox = math::BBox2i(
                marginBBox.min.x,
                marginBBox.min.y,
                labelSize.x + labelMargin * 2,
                fontMetrics.lineHeight + labelMargin * 2);
            pos = math::Vector2i(
                marginBBox.min.x + labelMargin,
                marginBBox.min.y + labelMargin + fontMetrics.ascender);
            break;
        case HUDElement::UpperRight:
            bbox = math::BBox2i(
                marginBBox.max.x - labelMargin * 2 - labelSize.x,
                marginBBox.min.y,
                labelSize.x + labelMargin * 2,
                fontMetrics.lineHeight + labelMargin * 2);
            pos = math::Vector2i(
                marginBBox.max.x - labelMargin - labelSize.x,
                marginBBox.min.y + labelMargin + fontMetrics.ascender);
            break;
        case HUDElement::LowerLeft:
            bbox = math::BBox2i(
                marginBBox.min.x,
                marginBBox.max.y - labelMargin * 2 - fontMetrics.lineHeight,
                labelSize.x + labelMargin * 2,
                fontMetrics.lineHeight + labelMargin * 2);
            pos = math::Vector2i(
                marginBBox.min.x + labelMargin,
                marginBBox.max.y - labelMargin - fontMetrics.lineHeight + fontMetrics.ascender);
            break;
        case HUDElement::LowerRight:
            bbox = math::BBox2i(
                marginBBox.max.x - labelMargin * 2 - labelSize.x,
                marginBBox.max.y - labelMargin * 2 - fontMetrics.lineHeight,
                labelSize.x + labelMargin * 2,
                fontMetrics.lineHeight + labelMargin * 2);
            pos = math::Vector2i(
                marginBBox.max.x - labelMargin - labelSize.x,
                marginBBox.max.y - labelMargin - fontMetrics.lineHeight + fontMetrics.ascender);
            break;
        }


        render->drawRect(bbox, overlayColor);
        render->drawText(fontSystem->getGlyphs(text, fontInfo), pos, labelColor);
    }
}
