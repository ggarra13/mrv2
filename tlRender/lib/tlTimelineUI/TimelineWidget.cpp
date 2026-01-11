// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <TimelineWidget.h>

#include <tlUI/ScrollWidget.h>

#ifdef OPENGL_BACKEND
#    include <tlGL/GL.h>
#    include <tlGL/GLFWWindow.h>
#endif

namespace tl
{
    namespace TIMELINEUI
    {
        namespace
        {
            const float marginPercentage = .1F;
        }

        struct TimelineWidget::Private
        {
            std::shared_ptr<ItemData> itemData;
            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<observer::Value<bool> > editable;
            std::shared_ptr<observer::Value<timeline::EditMode> > editMode;
            std::shared_ptr<observer::Value<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::shared_ptr<observer::Value<bool> > scrollToCurrentFrame;
            ui::KeyModifier scrollKeyModifier = ui::KeyModifier::Control;
            float mouseWheelScale = 1.1F;
            std::shared_ptr<observer::Value<bool> > stopOnScrub;
            std::shared_ptr<observer::Value<bool> > scrub;
            std::shared_ptr<observer::Value<otime::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            std::shared_ptr<observer::Value<ItemOptions> > itemOptions;
            std::shared_ptr<observer::Value<DisplayOptions> > displayOptions;
            otime::TimeRange timeRange = time::invalidTimeRange;
            timeline::Playback playback = timeline::Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            double scale = 500.0;
            bool sizeInit = true;

#ifdef OPENGL_BACKEND
            std::shared_ptr<gl::GLFWWindow> window;
#endif
            
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            std::function<void(const std::vector<timeline::MoveData>&)>
                moveCallback;

            enum class MouseMode { kNone, Scroll };
            struct MouseData
            {
                MouseMode mode = MouseMode::kNone;
                math::Vector2i scrollPos;
                std::chrono::steady_clock::time_point wheelTimer;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> >
                playbackObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                timeScrubObserver;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineWidget", context, parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, static_cast<int>(p.scrollKeyModifier));

            p.itemData = std::make_shared<ItemData>();
            p.itemData->timeUnitsModel = timeUnitsModel;

            p.editMode = observer::Value<timeline::EditMode>::create(timeline::EditMode::Move);
            p.editable = observer::Value<bool>::create(false);
            p.frameView = observer::Value<bool>::create(true);
            p.scrollToCurrentFrame = observer::Value<bool>::create(true);
            p.stopOnScrub = observer::Value<bool>::create(true);
            p.scrub = observer::Value<bool>::create(false);
            p.timeScrub =
                observer::Value<otime::RationalTime>::create(time::invalidTime);
            p.itemOptions = observer::Value<ItemOptions>::create();
            p.displayOptions = observer::Value<DisplayOptions>::create();

#ifdef OPENGL_BACKEND
            p.window = gl::GLFWWindow::create(
                "tl::TIMELINEUI::TimelineWidget", math::Size2i(1, 1), context,
                static_cast<int>(gl::GLFWWindowOptions::kNone));
#endif
            
            p.scrollWidget = ui::ScrollWidget::create(
                context, ui::ScrollType::Both, shared_from_this());
            p.scrollWidget->setScrollEventsEnabled(false);
            p.scrollWidget->setBorder(false);
        }

        TimelineWidget::~TimelineWidget() {}

#ifdef OPENGL_BACKEND
        TimelineWidget::TimelineWidget() :
            _p(new Private)
        {
        }

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }
#endif

#ifdef VULKAN_BACKEND        
        TimelineWidget::TimelineWidget(Fl_Vk_Context& ctx) :
            ctx(ctx),
            _p(new Private)
        {
        }

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            Fl_Vk_Context& ctx,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget(ctx));
            out->_init(timeUnitsModel, context, parent);
            return out;
        }
#endif
        
        double TimelineWidget::getScale() const
        {
            TLRENDER_P();
            return p.scale;
        }

        const math::Box2i& TimelineWidget::getTimelineItemGeometry() const
        {
            TLRENDER_P();
            return p.timelineItem->getGeometry();
        }

        void TimelineWidget::setPlayer(
            const std::shared_ptr<timeline::Player>& player)
        {
            TLRENDER_P();
            if (player == p.player)
                return;

            p.itemData->info.clear();
            p.itemData->thumbnails.clear();
            p.itemData->waveforms.clear();
            p.timeRange = time::invalidTimeRange;
            p.playback = timeline::Playback::Stop;
            p.timelineObserver.reset();
            p.playbackObserver.reset();
            p.currentTimeObserver.reset();
            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            p.player = player;

            p.scale = _getTimelineScale();
            if (p.player)
            {
                p.timeRange = p.player->getTimeRange();

                p.timelineObserver = observer::ValueObserver<bool>::create(
                    p.player->getTimeline()->observeTimelineChanges(),
                    [this](bool) { _timelineUpdate(); });

                p.playbackObserver =
                    observer::ValueObserver<timeline::Playback>::create(
                        p.player->observePlayback(),
                        [this](timeline::Playback value)
                        { _p->playback = value; });

                p.currentTimeObserver =
                    observer::ValueObserver<otime::RationalTime>::create(
                        p.player->observeCurrentTime(),
                        [this](const otime::RationalTime& value)
                        {
                            _p->currentTime = value;
                            _scrollUpdate();
                        });

                setEditMode(p.editMode->get());
            }
            else
            {
                _timelineUpdate();
            }
        }

        bool TimelineWidget::isEditable() const
        {
            return _p->editable->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineWidget::observeEditable() const
        {
            return _p->editable;
        }

        void TimelineWidget::setEditable(bool value)
        {
            TLRENDER_P();
            if (p.editable->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setEditable(value);
                }
            }
        }

        void TimelineWidget::setEditMode(const timeline::EditMode value)
        {
            TLRENDER_P();
            p.editMode->setAlways(value);
            if (p.timelineItem)
            {
                p.timelineItem->setEditMode(value);
            }
        }
        
        void TimelineWidget::setViewZoom(double value)
        {
            setViewZoom(
                value, math::Vector2i(_geometry.w() / 2, _geometry.h() / 2));
        }

        void
        TimelineWidget::setViewZoom(double zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            _setViewZoom(zoom, p.scale, focus, p.scrollWidget->getScrollPos());
        }

        void TimelineWidget::frameView()
        {
            TLRENDER_P();
            p.scrollWidget->setScrollPos(math::Vector2i());
            const double scale = _getTimelineScale();
            if (scale != p.scale)
            {
                p.scale = scale;
                _setItemScale();
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineWidget::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (p.frameView->setIfChanged(value))
            {
                if (value)
                {
                    frameView();
                }
            }
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->scrollWidget->areScrollBarsVisible();
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->scrollWidget->setScrollBarsVisible(value);
        }

        bool TimelineWidget::hasScrollToCurrentFrame() const
        {
            return _p->scrollToCurrentFrame->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineWidget::observeScrollToCurrentFrame() const
        {
            return _p->scrollToCurrentFrame;
        }

        void TimelineWidget::setScrollToCurrentFrame(bool value)
        {
            TLRENDER_P();
            if (p.scrollToCurrentFrame->setIfChanged(value))
            {
                _scrollUpdate();
            }
        }

        ui::KeyModifier TimelineWidget::getScrollKeyModifier() const
        {
            return _p->scrollKeyModifier;
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            TLRENDER_P();
            p.scrollKeyModifier = value;
            _setMousePress(true, 0, static_cast<int>(p.scrollKeyModifier));
        }

        float TimelineWidget::getMouseWheelScale() const
        {
            return _p->mouseWheelScale;
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            TLRENDER_P();
            p.mouseWheelScale = value;
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->stopOnScrub->get();
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineWidget::observeStopOnScrub() const
        {
            return _p->stopOnScrub;
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            TLRENDER_P();
            if (p.stopOnScrub->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setStopOnScrub(value);
                }
            }
        }

        std::shared_ptr<observer::IValue<bool> >
        TimelineWidget::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> >
        TimelineWidget::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        const std::vector<int>& TimelineWidget::getFrameMarkers() const
        {
            return _p->frameMarkers;
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            if (p.timelineItem)
            {
                p.timelineItem->setFrameMarkers(value);
            }
        }

        const ItemOptions& TimelineWidget::getItemOptions() const
        {
            return _p->itemOptions->get();
        }

        std::shared_ptr<observer::IValue<ItemOptions> >
        TimelineWidget::observeItemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ItemOptions& value)
        {
            TLRENDER_P();
            if (p.itemOptions->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    _setItemOptions(p.timelineItem, value);
                }
            }
        }

        const DisplayOptions& TimelineWidget::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<observer::IValue<DisplayOptions> >
        TimelineWidget::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void TimelineWidget::setDisplayOptions(const DisplayOptions& value)
        {
            TLRENDER_P();
            if (p.displayOptions->setIfChanged(value))
            {
                p.itemData->info.clear();
                p.itemData->thumbnails.clear();
                p.itemData->waveforms.clear();
                if (p.timelineItem)
                {
                    _setDisplayOptions(p.timelineItem, value);
                }
            }
        }

        void TimelineWidget::setGeometry(const math::Box2i& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.scrollWidget->setGeometry(value);
            if (p.sizeInit || (changed && p.frameView->get()))
            {
                p.sizeInit = false;
                frameView();
            }
            else if (
                p.timelineItem && p.timelineItem->getSizeHint().w <
                                      p.scrollWidget->getViewport().w())
            {
                setFrameView(true);
            }
        }

        void TimelineWidget::tickEvent(
            bool parentsVisible, bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
        }

        void TimelineWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            const int b =
                event.style->getSizeRole(ui::SizeRole::Border, _displayScale);
            const int sa = event.style->getSizeRole(
                ui::SizeRole::ScrollArea, _displayScale);
            _sizeHint.w = sa;
            //! \bug This assumes the scroll bars are hidden.
            _sizeHint.h = p.timelineItem
                              ? (p.timelineItem->getMinimumHeight() + b * 2)
                              : sa;
            p.sizeInit |= displayScaleChanged;
        }

        void TimelineWidget::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = event.pos - _mouse.pressPos;
                p.scrollWidget->setScrollPos(p.mouse.scrollPos - d);
                setFrameView(false);
                break;
            }
            default:
                break;
            }
        }

        bool TimelineWidget::isDraggingClip() const
        {
            TLRENDER_P();
            return p.timelineItem->isDraggingClip();
        }

        //! Sets a callback for inserting items
        void TimelineWidget::setMoveCallback(
            const std::function<void(const std::vector<timeline::MoveData>&)>&
                value)
        {
            TLRENDER_P();
            p.moveCallback = value;

            if (p.timelineItem)
                p.timelineItem->setMoveCallback(value);
        }

        void TimelineWidget::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (p.itemOptions->get().inputEnabled && 0 == event.button &&
                event.modifiers & static_cast<int>(p.scrollKeyModifier))
            {
                takeKeyFocus();
                p.mouse.mode = Private::MouseMode::Scroll;
                p.mouse.scrollPos = p.scrollWidget->getScrollPos();
            }
            else
            {
                p.mouse.mode = Private::MouseMode::kNone;
            }
        }

        void TimelineWidget::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::kNone;
        }

        void TimelineWidget::scrollEvent(ui::ScrollEvent& event)
        {
            TLRENDER_P();
            if (p.itemOptions->get().inputEnabled)
            {
                event.accept = true;
                if (event.value.y > 0)
                {
                    const double zoom = p.scale * p.mouseWheelScale;
                    setViewZoom(zoom, event.pos);
                }
                else
                {
                    const double zoom = p.scale / p.mouseWheelScale;
                    setViewZoom(zoom, event.pos);
                }
            }
        }

        void TimelineWidget::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (p.itemOptions->get().inputEnabled && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::_0:
                    event.accept = true;
                    setViewZoom(1.F, event.pos);
                    break;
                case ui::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.scale * 2.F, event.pos);
                    break;
                case ui::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.scale / 2.F, event.pos);
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

        void TimelineWidget::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_releaseMouse()
        {
            IWidget::_releaseMouse();
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::kNone;
        }

        void TimelineWidget::_setViewZoom(
            double zoomNew, double zoomPrev, const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();
            const int w = _geometry.w();
            const double zoomMin = _getTimelineScale();
            const double zoomMax = _getTimelineScaleMax();
            const double zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);
            if (zoomClamped != p.scale)
            {
                p.scale = zoomClamped;
                _setItemScale();
                const double s = zoomClamped / zoomPrev;
                const math::Vector2i scrollPosNew(
                    (scrollPos.x + focus.x) * s - focus.x, scrollPos.y);
                p.scrollWidget->setScrollPos(scrollPosNew, false);

                setFrameView(zoomNew <= zoomMin);
            }
        }

        double TimelineWidget::_getTimelineScale() const
        {
            TLRENDER_P();
            double out = 1.0;
            if (p.player)
            {
                const otime::TimeRange& timeRange = p.player->getTimeRange();
                const double duration =
                    timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const math::Box2i scrollViewport =
                        p.scrollWidget->getViewport();
                    out = scrollViewport.w() / duration;
                }
            }
            return out;
        }

        double TimelineWidget::_getTimelineScaleMax() const
        {
            TLRENDER_P();
            double out = 1.0;
            if (p.player)
            {
                const math::Box2i scrollViewport =
                    p.scrollWidget->getViewport();
                const otime::TimeRange& timeRange = p.player->getTimeRange();
                const double duration =
                    timeRange.duration().rescaled_to(1.0).value();
                if (duration < 1.0)
                {
                    if (duration > 0.0)
                    {
                        out = scrollViewport.w() / duration;
                    }
                }
                else
                {
                    out = scrollViewport.w();
                }
            }
            return out;
        }

        void TimelineWidget::_setItemScale()
        {
            TLRENDER_P();
            p.itemData->waveforms.clear();
            if (p.timelineItem)
            {
                _setItemScale(p.timelineItem, p.scale);
            }
        }

        void TimelineWidget::_setItemScale(
            const std::shared_ptr<IWidget>& widget, double value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setScale(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemScale(child, value);
            }
        }

        void TimelineWidget::_setItemOptions(
            const std::shared_ptr<IWidget>& widget, const ItemOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemOptions(child, value);
            }
        }

        void TimelineWidget::_setDisplayOptions(
            const std::shared_ptr<IWidget>& widget, const DisplayOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setDisplayOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setDisplayOptions(child, value);
            }
        }

        void TimelineWidget::_scrollUpdate()
        {
            TLRENDER_P();
            if (p.timelineItem && p.scrollToCurrentFrame->get() &&
                !p.scrub->get() && Private::MouseMode::kNone == p.mouse.mode)
            {
                const int pos = p.timelineItem->timeToPos(p.currentTime);
                const math::Box2i vp = p.scrollWidget->getViewport();
                const int margin = vp.w() * marginPercentage;
                if (pos < (vp.min.x + margin) || pos > (vp.max.x - margin))
                {
                    const int offset = pos < (vp.min.x + margin)
                                           ? (vp.min.x + margin)
                                           : (vp.max.x - margin);
                    const otime::RationalTime t =
                        p.currentTime - p.timeRange.start_time();
                    math::Vector2i scrollPos = p.scrollWidget->getScrollPos();
                    scrollPos.x = _geometry.min.x - offset +
                                  t.rescaled_to(1.0).value() * p.scale;
                    p.scrollWidget->setScrollPos(scrollPos);
                }
            }
        }

        void TimelineWidget::_timelineUpdate()
        {
            TLRENDER_P();

            const math::Vector2i scrollPos = p.scrollWidget->getScrollPos();

            p.scrubObserver.reset();
            p.timeScrubObserver.reset();
            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            if (p.player)
            {
                if (auto context = _context.lock())
                {
                    p.itemData->speed = p.player->getDefaultSpeed();
                    p.itemData->directory = p.player->getPath().getDirectory();
                    p.itemData->options = p.player->getOptions();
#ifdef OPENGL_BACKEND
                    p.timelineItem = TimelineItem::create(
                        p.player,
                        p.player->getTimeline()->getTimeline()->tracks(),
                        p.scale, p.itemOptions->get(), p.displayOptions->get(),
                        p.itemData, p.window, context);
#endif
#ifdef VULKAN_BACKEND
                    p.timelineItem = TimelineItem::create(
                        p.player,
                        p.player->getTimeline()->getTimeline()->tracks(),
                        p.scale, p.itemOptions->get(), p.displayOptions->get(),
                        p.itemData, ctx, context);
#endif
                    p.timelineItem->setEditable(p.editable->get());
                    p.timelineItem->setEditMode(p.editMode->get());
                    p.timelineItem->setStopOnScrub(p.stopOnScrub->get());
                    p.timelineItem->setMoveCallback(p.moveCallback);
                    p.timelineItem->setFrameMarkers(p.frameMarkers);
                    p.scrollWidget->setScrollPos(scrollPos);
                    p.scrollWidget->setWidget(p.timelineItem);

                    p.scrubObserver = observer::ValueObserver<bool>::create(
                        p.timelineItem->observeScrub(),
                        [this](bool value)
                        {
                            _p->scrub->setIfChanged(value);
                            _scrollUpdate();
                        });

                    p.timeScrubObserver =
                        observer::ValueObserver<otime::RationalTime>::create(
                            p.timelineItem->observeTimeScrub(),
                            [this](const otime::RationalTime& value)
                            { _p->timeScrub->setIfChanged(value); });
                }
            }
        }
        
        std::vector<const otio::Item*> TimelineWidget::getSelectedItems() const
        {
            TLRENDER_P();
            
            std::vector<const otio::Item* > out;
            if (p.timelineItem)
                out = p.timelineItem->getSelectedItems();
            return out;
        }
    } // namespace TIMELINEUI
} // namespace tl
