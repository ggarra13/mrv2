// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/Label.h>

#include <tlUI/LayoutUtil.h>

#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        struct Label::Private
        {
            std::string text;
            std::vector<std::string> lines;
            ColorRole textRole = ColorRole::Text;
            SizeRole marginRole = SizeRole::kNone;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                math::Size2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::vector<std::shared_ptr<image::Glyph> > >
                    glyphs;
            };
            DrawData draw;
        };

        void Label::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Label", context, parent);
            _hAlign = HAlign::Left;
        }

        Label::Label() :
            _p(new Private)
        {
        }

        Label::~Label() {}

        std::shared_ptr<Label> Label::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Label>(new Label);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<Label> Label::create(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Label>(new Label);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void Label::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _textUpdate();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setTextRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.textRole)
                return;
            p.textRole = value;
            _updates |= Update::Draw;
        }

        void Label::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin =
                    event.style->getSizeRole(p.marginRole, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo =
                    event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize =
                    event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w = p.size.textSize.w + p.size.margin * 2;
            _sizeHint.h = p.size.textSize.h + p.size.margin * 2;
        }

        void Label::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void
        Label::drawEvent(const math::Box2i& drawRect, const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            // event.render->drawRect(_geometry, image::Color4f(.5F, .3F, .3F));

            const math::Box2i g = align(
                                      _geometry, _sizeHint, Stretch::Fixed,
                                      Stretch::Fixed, _hAlign, _vAlign)
                                      .margin(-p.size.margin);

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                for (const auto& line : p.lines)
                {
                    p.draw.glyphs.push_back(
                        event.fontSystem->getGlyphs(line, p.size.fontInfo));
                }
            }
            math::Vector2i pos = g.min;
            for (const auto& glyphs : p.draw.glyphs)
            {
                event.render->drawText(
                    glyphs,
                    math::Vector2i(pos.x, pos.y + p.size.fontMetrics.ascender),
                    event.style->getColorRole(p.textRole));
                pos.y += p.size.fontMetrics.lineHeight;
            }
        }

        void Label::_textUpdate()
        {
            TLRENDER_P();
            const auto lines = string::split(
                p.text, {'\n', '\r'}, string::SplitOptions::KeepEmpty);
            p.lines.clear();
            for (const auto& line : lines)
            {
                p.lines.push_back(line);
            }
        }
    } // namespace ui
} // namespace tl
