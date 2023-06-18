// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <FL/Fl_Gl_Window.H>

#include "mrvFl/mrvTimeObject.h"

namespace tl
{
    namespace timeline
    {
        class Player;
    }
} // namespace tl

namespace mrv
{
    using namespace tl;

    //! Timeline widget.
    class TimelineWidget : public Fl_Gl_Window
    {
    public:
        TimelineWidget(int X, int Y, int W, int H, const char* L = 0);

        ~TimelineWidget() override;

        //! Set tlRender's context
        void setContext(const std::shared_ptr<system::Context>&);

        void setStyle(const std::shared_ptr<ui::Style>& = nullptr);

        //! Set the time object.
        // @todo:
        // void setTimeObject(qt::TimeObject*);

        //! Set the timeline player.
        void setPlayer(const std::shared_ptr<timeline::Player>&);

    public: // Q_SLOTS:
        //! Set whether the to frame the view.
        void setFrameView(bool);

        //! Set whether the scroll bars are visible.
        void setScrollBarsVisible(bool);

        //! Set the mouse scroll key modifier.
        void setScrollKeyModifier(ui::KeyModifier);

        //! Set whether to stop playback when scrubbing.
        void setStopOnScrub(bool);

        //! Set whether thumbnails are enabled.
        void setThumbnails(bool);

        //! Set the mouse wheel scale.
        void setMouseWheelScale(float);

        //! Set the item options.
        void setItemOptions(const timelineui::ItemOptions&);

        // Q_SIGNALS:
        //! This signal is emitted when the frame view is changed.
        void frameViewChanged(bool);

        void resize(int X, int Y, int W, int H) override;
        void draw() override;
        int handle(int) override;

    protected:
        void initializeGL();

        int enterEvent();
        int leaveEvent();
        int mousePressEvent();
        int mouseReleaseEvent();
        int mouseMoveEvent();
        int wheelEvent();
        int keyPressEvent();
        int keyReleaseEvent();

        static void timerEvent_cb(void* data);
        void timerEvent();

    private: // Q_SLOTS:
        void _setTimeUnits(tl::timeline::TimeUnits);

    private:
        int _toUI(int) const;
        math::Vector2i _toUI(const math::Vector2i&) const;
        int _fromUI(int) const;
        math::Vector2i _fromUI(const math::Vector2i&) const;

        void _styleUpdate();

        TLRENDER_PRIVATE();
    };
} // namespace mrv
