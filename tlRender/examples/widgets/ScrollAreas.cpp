// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ScrollAreas.h"

#include <tlUI/GridLayout.h>
#include <tlUI/LayoutUtil.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct ScrollAreasWidget::Private
            {
                math::Vector2i cellCount;
                int cellSize = 0;
                int margin = 0;
                std::vector<math::Size2i> textSize;
                std::vector<std::vector<std::shared_ptr<image::Glyph> > >
                    glyphs;
            };

            void ScrollAreasWidget::_init(
                const math::Vector2i& cellCount,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(
                    "tl::examples::widgets::ScrollAreasWidget", context,
                    parent);
                TLRENDER_P();
                p.cellCount = cellCount;
                p.textSize.resize(p.cellCount.x * p.cellCount.y);
                p.glyphs.resize(p.cellCount.x * p.cellCount.y);
            }

            ScrollAreasWidget::ScrollAreasWidget() :
                _p(new Private)
            {
            }

            ScrollAreasWidget::~ScrollAreasWidget() {}

            std::shared_ptr<ScrollAreasWidget> ScrollAreasWidget::create(
                const math::Vector2i& cellCount,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out =
                    std::shared_ptr<ScrollAreasWidget>(new ScrollAreasWidget);
                out->_init(cellCount, context, parent);
                return out;
            }

            void
            ScrollAreasWidget::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                TLRENDER_P();
                p.margin = event.style->getSizeRole(
                    ui::SizeRole::MarginLarge, _displayScale);
                const std::string format = string::Format("{0}, {1}")
                                               .arg(ui::format(p.cellCount.x))
                                               .arg(ui::format(p.cellCount.y));
                const auto fontInfo = event.style->getFontRole(
                    ui::FontRole::Label, _displayScale);
                const math::Size2i textSize =
                    event.fontSystem->getSize(format, fontInfo);
                p.cellSize = textSize.w + p.margin * 2;
                _sizeHint.w = p.cellCount.x * p.cellSize;
                _sizeHint.h = p.cellCount.y * p.cellSize;
            }

            void ScrollAreasWidget::clipEvent(
                const math::Box2i& clipRect, bool clipped)
            {
                IWidget::clipEvent(clipRect, clipped);
                TLRENDER_P();
                if (clipped)
                {
                    for (auto& i : p.glyphs)
                    {
                        i.clear();
                    }
                }
            }

            void ScrollAreasWidget::drawEvent(
                const math::Box2i& drawRect, const ui::DrawEvent& event)
            {
                IWidget::drawEvent(drawRect, event);
                TLRENDER_P();

                const math::Box2i& g = _geometry;

                const auto fontInfo = event.style->getFontRole(
                    ui::FontRole::Label, _displayScale);
                const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
                for (int y = 0; y < p.cellCount.y; ++y)
                {
                    for (int x = 0; x < p.cellCount.x; ++x)
                    {
                        const bool even = ((x + y) % 2 == 0);

                        const math::Box2i g2(
                            g.x() + x * p.cellSize, g.y() + y * p.cellSize,
                            p.cellSize, p.cellSize);
                        event.render->drawRect(
                            g2, event.style->getColorRole(
                                    even ? ui::ColorRole::Window
                                         : ui::ColorRole::Button));

                        const size_t i = y * p.cellCount.x + x;
                        if (p.glyphs[i].empty())
                        {
                            const std::string text =
                                string::Format("{0}, {1}").arg(y).arg(x);
                            p.textSize[i] =
                                event.fontSystem->getSize(text, fontInfo);
                            p.glyphs[i] =
                                event.fontSystem->getGlyphs(text, fontInfo);
                        }
                        event.render->drawText(
                            p.glyphs[i],
                            g2.getCenter() -
                                math::Vector2i(
                                    p.textSize[i].w, p.textSize[i].h) /
                                    2 +
                                math::Vector2i(0, fontMetrics.ascender),
                            event.style->getColorRole(ui::ColorRole::Text));
                    }
                }
            }

            struct ScrollAreas::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void ScrollAreas::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Scroll Areas", "tl::examples::widgets::ScrollAreas",
                    context, parent);
                TLRENDER_P();

                auto widget0 =
                    ScrollAreasWidget::create(math::Vector2i(10, 1), context);
                auto scrollWidget0 = ui::ScrollWidget::create(
                    context, ui::ScrollType::Horizontal);
                scrollWidget0->setWidget(widget0);

                auto widget1 =
                    ScrollAreasWidget::create(math::Vector2i(1, 10), context);
                auto scrollWidget1 =
                    ui::ScrollWidget::create(context, ui::ScrollType::Vertical);
                scrollWidget1->setWidget(widget1);

                auto widget2 =
                    ScrollAreasWidget::create(math::Vector2i(10, 10), context);
                auto scrollWidget2 =
                    ui::ScrollWidget::create(context, ui::ScrollType::Both);
                scrollWidget2->setWidget(widget2);
                scrollWidget2->setHStretch(ui::Stretch::Expanding);

                p.layout =
                    ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                scrollWidget0->setParent(p.layout);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setVStretch(ui::Stretch::Expanding);
                scrollWidget1->setParent(hLayout);
                scrollWidget2->setParent(hLayout);
            }

            ScrollAreas::ScrollAreas() :
                _p(new Private)
            {
            }

            ScrollAreas::~ScrollAreas() {}

            std::shared_ptr<ScrollAreas> ScrollAreas::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ScrollAreas>(new ScrollAreas);
                out->_init(context, parent);
                return out;
            }

            void ScrollAreas::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void ScrollAreas::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
