// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Viewport.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct Viewport::Private
        {
            std::shared_ptr<observer::Value<bool> > hud;
            double fps = 0.0;
            size_t droppedFrames = 0;
            std::vector<std::string> text;

            struct SizeData
            {
                int sizeInit = true;
                int margin = 0;
                int spacing = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                std::vector<math::Size2i> textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::vector<std::shared_ptr<image::Glyph> > >
                    glyphs;
            };
            DrawData draw;

            std::shared_ptr<observer::ValueObserver<double> > fpsObserver;
            std::shared_ptr<observer::ValueObserver<size_t> >
                droppedFramesObserver;
        };

        void Viewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            TimelineViewport::_init(context, parent);
            TLRENDER_P();
            p.hud = observer::Value<bool>::create(false);
            p.fpsObserver = observer::ValueObserver<double>::create(
                observeFPS(),
                [this](double value)
                {
                    _p->fps = value;
                    _textUpdate();
                });
            p.droppedFramesObserver = observer::ValueObserver<size_t>::create(
                observeDroppedFrames(),
                [this](size_t value)
                {
                    _p->droppedFrames = value;
                    _textUpdate();
                });
        }

        Viewport::Viewport() :
            _p(new Private)
        {
        }

        Viewport::~Viewport() {}

        std::shared_ptr<Viewport> Viewport::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Viewport>(new Viewport);
            out->_init(context, parent);
            return out;
        }

        bool Viewport::hasHUD() const
        {
            return _p->hud->get();
        }

        std::shared_ptr<observer::IValue<bool> > Viewport::observeHUD() const
        {
            return _p->hud;
        }

        void Viewport::setHUD(bool value)
        {
            TLRENDER_P();
            if (p.hud->setIfChanged(value))
            {
                _updates |= ui::Update::Draw;
            }
        }

        void Viewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            TimelineViewport::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(
                    ui::SizeRole::MarginSmall, _displayScale);
                p.size.spacing = event.style->getSizeRole(
                    ui::SizeRole::SpacingSmall, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo =
                    event.style->getFontRole(ui::FontRole::Mono, _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize.clear();
                for (const auto& text : p.text)
                {
                    p.size.textSize.push_back(
                        event.fontSystem->getSize(text, p.size.fontInfo));
                }
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;
        }

        void Viewport::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            TimelineViewport::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void Viewport::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TimelineViewport::drawEvent(drawRect, event);
            TLRENDER_P();
            const math::Box2i& g = _geometry;
            if (p.hud->get())
            {
                if (!p.text.empty() && p.draw.glyphs.empty())
                {
                    for (const auto& text : p.text)
                    {
                        p.draw.glyphs.push_back(
                            event.fontSystem->getGlyphs(text, p.size.fontInfo));
                    }
                }

                const math::Box2i g2 = g.margin(-p.size.margin);
                int x = g2.min.x;
                int y = g2.min.y;
                for (size_t i = 0;
                     i < p.text.size() && i < p.size.textSize.size() &&
                     i < p.draw.glyphs.size();
                     ++i)
                {
                    const math::Box2i g3(
                        x, y, p.size.textSize[i].w + p.size.margin * 2,
                        p.size.textSize[i].h + p.size.margin * 2);
                    event.render->drawRect(
                        g3, event.style->getColorRole(ui::ColorRole::Overlay));
                    const math::Vector2i pos(
                        g3.min.x + p.size.margin,
                        g3.min.y + p.size.margin + p.size.fontMetrics.ascender);
                    event.render->drawText(
                        p.draw.glyphs[i], pos,
                        event.style->getColorRole(ui::ColorRole::Text));
                    y += g3.h();
                }
            }
        }

        void Viewport::_textUpdate()
        {
            TLRENDER_P();
            p.text.clear();
            p.text.push_back(string::Format("FPS: {0} ({1} dropped)")
                                 .arg(p.fps, 2, 4)
                                 .arg(p.droppedFrames));
            p.size.textInit = true;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    } // namespace play_app
} // namespace tl
