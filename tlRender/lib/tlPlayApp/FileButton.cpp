// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesToolPrivate.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/ThumbnailSystem.h>

namespace tl
{
    namespace play_app
    {
        struct FileButton::Private
        {
            std::shared_ptr<play::FilesModelItem> item;

            struct SizeData
            {
                int sizeInit = true;
                int margin = 0;
                int spacing = 0;
                int border = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                math::Size2i textSize;

                bool thumbnailInit = true;
                float thumbnailScale = 1.F;
                int thumbnailHeight = 40;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > glyphs;
                ui::ThumbnailRequest thumbnailRequest;
                std::shared_ptr<image::Image> thumbnail;
            };
            DrawData draw;
        };

        void FileButton::_init(
            const std::shared_ptr<play::FilesModelItem>& item,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::play_app::FileButton", context, parent);
            TLRENDER_P();
            const std::string s =
                string::elide(item->path.get(-1, file::PathType::FileName));
            setText(s);
            setCheckable(true);
            setHStretch(ui::Stretch::Expanding);
            setAcceptsKeyFocus(true);
            _buttonRole = ui::ColorRole::None;
            p.item = item;
        }

        FileButton::FileButton() :
            _p(new Private)
        {
        }

        FileButton::~FileButton() {}

        std::shared_ptr<FileButton> FileButton::create(
            const std::shared_ptr<play::FilesModelItem>& item,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileButton>(new FileButton);
            out->_init(item, context, parent);
            return out;
        }

        void FileButton::tickEvent(
            bool parentsVisible, bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (p.draw.thumbnailRequest.future.valid() &&
                p.draw.thumbnailRequest.future.wait_for(
                    std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.draw.thumbnail = p.draw.thumbnailRequest.future.get();
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
        }

        void FileButton::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(
                    ui::SizeRole::MarginInside, _displayScale);
                p.size.spacing = event.style->getSizeRole(
                    ui::SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(
                    ui::SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo =
                    event.style->getFontRole(_fontRole, _displayScale);
                p.size.fontMetrics =
                    event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize =
                    event.fontSystem->getSize(_text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            if (_displayScale != p.size.thumbnailScale)
            {
                p.size.thumbnailInit = true;
                p.size.thumbnailScale = _displayScale;
                p.size.thumbnailHeight = 40 * _displayScale;
            }
            if (p.size.thumbnailInit)
            {
                p.size.thumbnailInit = false;
                if (auto context = _context.lock())
                {
                    auto thumbnailSystem =
                        context->getSystem<ui::ThumbnailSystem>();
                    p.draw.thumbnailRequest = thumbnailSystem->getThumbnail(
                        p.item->path, p.size.thumbnailHeight);
                }
            }

            math::Size2i thumbnailSize;
            if (p.draw.thumbnail)
            {
                const image::Size& size = p.draw.thumbnail->getSize();
                thumbnailSize =
                    math::Size2i(size.w * size.pixelAspectRatio, size.h);
            }
            _sizeHint.w = thumbnailSize.w + p.size.spacing + p.size.textSize.w +
                          p.size.margin * 2 + p.size.margin * 2 +
                          p.size.border * 4;
            _sizeHint.h =
                std::max(p.size.fontMetrics.lineHeight, thumbnailSize.h) +
                p.size.margin * 2 + p.size.border * 4;
        }

        void FileButton::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IButton::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void FileButton::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            const bool enabled = isEnabled();

            if (_keyFocus)
            {
                event.render->drawMesh(
                    ui::border(g, p.size.border * 2), math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::KeyFocus));
            }

            const math::Box2i g2 = g.margin(-p.size.border * 2);
            if (isChecked())
            {
                event.render->drawRect(
                    g2, event.style->getColorRole(ui::ColorRole::Checked));
            }
            if (_mouse.press && _geometry.contains(_mouse.pos))
            {
                event.render->drawRect(
                    g2, event.style->getColorRole(ui::ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g2, event.style->getColorRole(ui::ColorRole::Hover));
            }

            const math::Box2i g3 = g2.margin(-p.size.margin);
            int x = g3.min.x;
            if (p.draw.thumbnail)
            {
                const image::Size& size = p.draw.thumbnail->getSize();
                const math::Size2i thumbnailSize(
                    size.w * size.pixelAspectRatio, size.h);
                event.render->drawImage(
                    p.draw.thumbnail,
                    math::Box2i(x, g3.y(), thumbnailSize.w, thumbnailSize.h));
                x += thumbnailSize.w + p.size.spacing;
            }

            if (!_text.empty())
            {
                if (p.draw.glyphs.empty())
                {
                    p.draw.glyphs =
                        event.fontSystem->getGlyphs(_text, p.size.fontInfo);
                }
                const math::Vector2i pos(
                    x + p.size.margin, g3.y() + g3.h() / 2 -
                                           p.size.textSize.h / 2 +
                                           p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs, pos,
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void FileButton::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::Enter:
                    event.accept = true;
                    _click();
                    break;
                case ui::Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default:
                    break;
                }
            }
        }

        void FileButton::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }
    } // namespace play_app
} // namespace tl
