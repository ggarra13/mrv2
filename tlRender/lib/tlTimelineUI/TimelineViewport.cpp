// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineViewport.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineViewport::Private
        {
            timeline::CompareOptions compareOptions;
            std::function<void(timeline::CompareOptions)> compareCallback;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::BackgroundOptions backgroundOptions;
            image::PixelType colorBuffer = image::PixelType::RGBA_U8;
            std::shared_ptr<timeline::Player> player;
            std::vector<timeline::VideoData> videoData;
            math::Vector2i viewPos;
            double viewZoom = 1.0;
            std::shared_ptr<observer::Value<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::function<void(const math::Vector2i&, double)>
                viewPosAndZoomCallback;
            std::shared_ptr<observer::Value<double> > fps;
            struct FpsData
            {
                std::chrono::steady_clock::time_point timer;
                size_t frameCount = 0;
            };
            FpsData fpsData;
            std::shared_ptr<observer::Value<size_t> > droppedFrames;
            struct DroppedFramesData
            {
                bool init = true;
                double frame = 0.0;
            };
            DroppedFramesData droppedFramesData;

            bool doRender = false;
            std::shared_ptr<gl::OffscreenBuffer> buffer;

            enum class MouseMode { None, View, Wipe };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                math::Vector2i viewPos;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<timeline::Playback> >
                playbackObserver;
            std::shared_ptr<observer::ListObserver<timeline::VideoData> >
                videoDataObserver;
        };

        void TimelineViewport::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineViewport", context, parent);
            TLRENDER_P();

            setHStretch(ui::Stretch::Expanding);
            setVStretch(ui::Stretch::Expanding);

            _setMouseHover(true);
            _setMousePress(true);

            p.frameView = observer::Value<bool>::create(true);
            p.fps = observer::Value<double>::create(0.0);
            p.droppedFrames = observer::Value<size_t>::create(0);
        }

        TimelineViewport::TimelineViewport() :
            _p(new Private)
        {
        }

        TimelineViewport::~TimelineViewport() {}

        std::shared_ptr<TimelineViewport> TimelineViewport::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineViewport>(new TimelineViewport);
            out->_init(context, parent);
            return out;
        }

        void TimelineViewport::setCompareOptions(
            const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setCompareCallback(
            const std::function<void(timeline::CompareOptions)>& value)
        {
            _p->compareCallback = value;
        }

        void
        TimelineViewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            if (value == p.ocioOptions)
                return;
            p.ocioOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setImageOptions(
            const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setDisplayOptions(
            const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setBackgroundOptions(
            const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            if (value == p.backgroundOptions)
                return;
            p.backgroundOptions = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        image::PixelType TimelineViewport::getColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void TimelineViewport::setColorBuffer(image::PixelType value)
        {
            TLRENDER_P();
            if (value == p.colorBuffer)
                return;
            p.colorBuffer = value;
            p.doRender = true;
            _updates |= ui::Update::Draw;
        }

        void TimelineViewport::setPlayer(
            const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();

            p.fpsData.timer = std::chrono::steady_clock::now();
            p.fpsData.frameCount = 0;
            p.fps->setIfChanged(0.0);
            p.droppedFramesData.init = true;
            p.droppedFrames->setIfChanged(0);
            p.playbackObserver.reset();
            p.videoDataObserver.reset();

            p.player = value;

            if (p.player)
            {
                p.playbackObserver =
                    observer::ValueObserver<timeline::Playback>::create(
                        p.player->observePlayback(),
                        [this](timeline::Playback value)
                        {
                            switch (value)
                            {
                            case timeline::Playback::Forward:
                            case timeline::Playback::Reverse:
                                _p->fpsData.timer =
                                    std::chrono::steady_clock::now();
                                _p->fpsData.frameCount = 0;
                                _p->droppedFramesData.init = true;
                                break;
                            default:
                                break;
                            }
                        });
                p.videoDataObserver =
                    observer::ListObserver<timeline::VideoData>::create(
                        p.player->observeCurrentVideo(),
                        [this](const std::vector<timeline::VideoData>& value)
                        {
                            _p->videoData = value;

                            _p->fpsData.frameCount = _p->fpsData.frameCount + 1;
                            const auto now = std::chrono::steady_clock::now();
                            const std::chrono::duration<double> diff =
                                now - _p->fpsData.timer;
                            if (diff.count() > 1.0)
                            {
                                const double fps =
                                    _p->fpsData.frameCount / diff.count();
                                // std::cout << "FPS: " << fps << std::endl;
                                _p->fps->setIfChanged(fps);
                                _p->fpsData.timer = now;
                                _p->fpsData.frameCount = 0;
                            }

                            _p->doRender = true;
                            _updates |= ui::Update::Draw;
                        });
            }
            else if (!p.videoData.empty())
            {
                p.videoData.clear();
                p.doRender = true;
                _updates |= ui::Update::Draw;
            }
        }

        const math::Vector2i& TimelineViewport::getViewPos() const
        {
            return _p->viewPos;
        }

        double TimelineViewport::getViewZoom() const
        {
            return _p->viewZoom;
        }

        void TimelineViewport::setViewPosAndZoom(
            const math::Vector2i& pos, double zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.doRender = true;
            _updates |= ui::Update::Draw;
            if (p.viewPosAndZoomCallback)
            {
                p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
            }
            setFrameView(false);
        }

        void
        TimelineViewport::setViewZoom(double zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            math::Vector2i pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineViewport::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineViewport::setFrameView(bool value)
        {
            TLRENDER_P();
            if (p.frameView->setIfChanged(value))
            {
                if (p.frameViewCallback)
                {
                    p.frameViewCallback(value);
                }
                p.doRender = true;
                _updates |= ui::Update::Draw;
            }
        }

        void TimelineViewport::setFrameViewCallback(
            const std::function<void(bool)>& value)
        {
            _p->frameViewCallback = value;
        }

        void TimelineViewport::viewZoom1To1()
        {
            TLRENDER_P();
            setViewZoom(1.F, _getViewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.0, _getViewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.0, _getViewportCenter());
        }

        void TimelineViewport::setViewPosAndZoomCallback(
            const std::function<void(const math::Vector2i&, double)>& value)
        {
            _p->viewPosAndZoomCallback = value;
        }

        double TimelineViewport::getFPS() const
        {
            return _p->fps->get();
        }

        std::shared_ptr<observer::IValue<double> >
        TimelineViewport::observeFPS() const
        {
            return _p->fps;
        }

        size_t TimelineViewport::getDroppedFrames() const
        {
            return _p->droppedFrames->get();
        }

        std::shared_ptr<observer::IValue<size_t> >
        TimelineViewport::observeDroppedFrames() const
        {
            return _p->droppedFrames;
        }

        void TimelineViewport::setGeometry(const math::Box2i& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            TLRENDER_P();
            if (changed)
            {
                p.doRender = true;
            }
        }

        void TimelineViewport::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            const int sa = event.style->getSizeRole(
                ui::SizeRole::ScrollArea, _displayScale);
            _sizeHint.w = sa;
            _sizeHint.h = sa;
        }

        void TimelineViewport::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            if (p.frameView->get())
            {
                _frameView();
            }

            const math::Box2i& g = _geometry;
            if (p.doRender)
            {
                p.doRender = false;

                const timeline::ViewportState viewportState(event.render);
                const timeline::ClipRectEnabledState clipRectEnabledState(
                    event.render);
                const timeline::ClipRectState clipRectState(event.render);
                const timeline::TransformState transformState(event.render);
                const timeline::RenderSizeState renderSizeState(event.render);

                const math::Size2i size = g.getSize();
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = p.colorBuffer;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
#if defined(TLRENDER_API_GL_4_1)
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
#elif defined(TLRENDER_API_GLES_2)
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
#endif // TLRENDER_API_GL_4_1
                if (gl::doCreate(p.buffer, size, offscreenBufferOptions))
                {
                    p.buffer = gl::OffscreenBuffer::create(
                        size, offscreenBufferOptions);
                }
                if (p.buffer)
                {
                    gl::OffscreenBufferBinding binding(p.buffer);
                    event.render->setRenderSize(size);
                    event.render->setViewport(math::Box2i(0, 0, g.w(), g.h()));
                    event.render->setClipRectEnabled(false);
                    event.render->clearViewport(image::Color4f(0.F, 0.F, 0.F));
                    event.render->setOCIOOptions(p.ocioOptions);
                    event.render->setLUTOptions(p.lutOptions);
                    if (!p.videoData.empty())
                    {
                        math::Matrix4x4f vm;
                        vm = vm * math::translate(math::Vector3f(
                                      p.viewPos.x, p.viewPos.y, 0.F));
                        vm = vm * math::scale(math::Vector3f(
                                      p.viewZoom, p.viewZoom, 1.F));
                        const auto pm = math::ortho(
                            0.F, static_cast<float>(g.w()), 0.F,
                            static_cast<float>(g.h()), -1.F, 1.F);
                        event.render->setTransform(pm * vm);
                        event.render->drawVideo(
                            p.videoData,
                            timeline::getBoxes(
                                p.compareOptions.mode, p.videoData),
                            p.imageOptions, p.displayOptions, p.compareOptions,
                            p.backgroundOptions);

                        _droppedFramesUpdate(p.videoData[0].time);
                    }
                }
            }

            if (p.buffer)
            {
                const unsigned int id = p.buffer->getColorID();
                event.render->drawTexture(id, g);
            }
        }

        void TimelineViewport::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::View:
                p.viewPos.x =
                    p.mouse.viewPos.x + (event.pos.x - _mouse.pressPos.x);
                p.viewPos.y =
                    p.mouse.viewPos.y + (event.pos.y - _mouse.pressPos.y);
                p.doRender = true;
                _updates |= ui::Update::Draw;
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
                setFrameView(false);
                break;
            case Private::MouseMode::Wipe:
            {
                if (p.player)
                {
                    const io::Info& ioInfo = p.player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x =
                            (event.pos.x - _geometry.min.x - p.viewPos.x) /
                            p.viewZoom /
                            static_cast<float>(
                                imageInfo.size.w *
                                imageInfo.size.pixelAspectRatio);
                        p.compareOptions.wipeCenter.y =
                            (event.pos.y - _geometry.min.y - p.viewPos.y) /
                            p.viewZoom / static_cast<float>(imageInfo.size.h);
                        p.doRender = true;
                        _updates |= ui::Update::Draw;
                        if (p.compareCallback)
                        {
                            p.compareCallback(p.compareOptions);
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        void TimelineViewport::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            takeKeyFocus();
            if (0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                p.mouse.mode = Private::MouseMode::View;
                p.mouse.viewPos = p.viewPos;
            }
            else if (
                0 == event.button &&
                event.modifiers & static_cast<int>(ui::KeyModifier::Alt))
            {
                p.mouse.mode = Private::MouseMode::Wipe;
            }
        }

        void TimelineViewport::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineViewport::scrollEvent(ui::ScrollEvent& event)
        {
            TLRENDER_P();
            if (static_cast<int>(ui::KeyModifier::None) == event.modifiers)
            {
                event.accept = true;
                const double mult = 1.1;
                const double zoom = event.value.y < 0
                                        ? p.viewZoom / (-event.value.y * mult)
                                        : p.viewZoom * (event.value.y * mult);
                setViewZoom(zoom, event.pos - _geometry.min);
            }
            else if (
                event.modifiers & static_cast<int>(ui::KeyModifier::Control))
            {
                event.accept = true;
                if (p.player)
                {
                    const otime::RationalTime t = p.player->getCurrentTime();
                    p.player->seek(
                        t + otime::RationalTime(event.value.y, t.rate()));
                }
            }
        }

        void TimelineViewport::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::_0:
                    event.accept = true;
                    setViewZoom(1.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.viewZoom * 2.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.viewZoom / 2.0, event.pos - _geometry.min);
                    break;
                case ui::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default:
                    break;
                }
            }
        }

        void TimelineViewport::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineViewport::_releaseMouse()
        {
            IWidget::_releaseMouse();
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        math::Size2i TimelineViewport::_getRenderSize() const
        {
            TLRENDER_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.videoData);
        }

        math::Vector2i TimelineViewport::_getViewportCenter() const
        {
            return math::Vector2i(_geometry.w() / 2, _geometry.h() / 2);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const math::Size2i viewportSize(_geometry.w(), _geometry.h());
            const math::Size2i renderSize = _getRenderSize();
            double zoom = viewportSize.w / static_cast<double>(renderSize.w);
            if (zoom * renderSize.h > viewportSize.h)
            {
                zoom = viewportSize.h / static_cast<double>(renderSize.h);
            }
            const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
            const math::Vector2i viewPos(
                viewportSize.w / 2.F - c.x * zoom,
                viewportSize.h / 2.F - c.y * zoom);
            if (viewPos != p.viewPos || zoom != p.viewZoom)
            {
                p.viewPos = viewPos;
                p.viewZoom = zoom;
                if (p.viewPosAndZoomCallback)
                {
                    p.viewPosAndZoomCallback(p.viewPos, p.viewZoom);
                }
            }
        }

        void
        TimelineViewport::_droppedFramesUpdate(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (value != time::invalidTime && p.droppedFramesData.init)
            {
                p.droppedFramesData.init = false;
                p.droppedFrames->setIfChanged(0);
            }
            else
            {
                const double frameDiff =
                    value.value() - p.droppedFramesData.frame;
                if (std::abs(frameDiff) > 1.0)
                {
                    p.droppedFrames->setIfChanged(p.droppedFrames->get() + 1);
                }
            }
            p.droppedFramesData.frame = value.value();
        }
    } // namespace timelineui
} // namespace tl
