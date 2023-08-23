// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <vector>

#include <opentimelineio/clip.h>

#include <tlTimelineUI/IItem.h>

#include <FL/Fl_Gl_Window.H>

#include "mrvFl/mrvTimeObject.h"
#include "mrvFl/mrvTimelinePlayer.h"

namespace tl
{
    namespace timeline
    {
        class Player;
    }
} // namespace tl

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class ThumbnailCreator;

    //! Timeline widget.
    class TimelineWidget : public Fl_Gl_Window
    {
    public:
        TimelineWidget(int X, int Y, int W, int H, const char* L = 0);

        ~TimelineWidget() override;

        //! Set tlRender's context
        void setContext(
            const std::shared_ptr<system::Context>&,
            const std::shared_ptr<timeline::TimeUnitsModel>&, ViewerUI*);

        void setStyle(const std::shared_ptr<ui::Style>& = nullptr);

        //! Set the LUT configuration.
        void setLUTOptions(const timeline::LUTOptions&);

        //! Set the color configuration.
        void setColorConfigOptions(const timeline::ColorConfigOptions&);

        //! Get timelineUI's timelineWidget item options
        timelineui::ItemOptions getItemOptions() const;

        //! Set the timeline player.
        void setTimelinePlayer(TimelinePlayer*);

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

        //! Frame the view.
        void frameView();

        // Q_SIGNALS:
        //! This signal is emitted when the frame view is changed.
        void frameViewChanged(bool);

        void resize(int X, int Y, int W, int H) override;
        void draw() override;
        int handle(int) override;

        void single_thumbnail(
            const int64_t,
            const std::vector<
                std::pair<otime::RationalTime, Fl_RGB_Image*> >&);

        static void single_thumbnail_cb(
            const int64_t,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&,
            void* data);

        //! Hide the thumbnail at least until user enters the timeline slider
        //! again.
        void hideThumbnail();

        //! @bug: A static one time timeout callback used to avoid a bug
        //! FLTK when hiding a window from an event of another widget.
        static void hideThumbnail_cb(TimelineWidget* t);

        //! Get the thumbnail creator
        ThumbnailCreator* thumbnailCreator();

        //! Set the time units.
        void setUnits(TimeUnits);

        //! Refresh the GL objects
        void refresh();

        void mouseMoveEvent(const int X, const int Y);
        void scrollEvent(const float X, const float Y, const int modifiers);

    protected:
        void _initializeGL();
        void _initializeGLResources();

        int enterEvent();
        int leaveEvent();
        int mousePressEvent();
        int mouseDragEvent();
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

        unsigned _changeKey(unsigned key);
        void _drawAnnotationMarks() const noexcept;

        otime::RationalTime _posToTime(float) const noexcept;
        int _timeToPos(const otime::RationalTime&) const noexcept;

        //! Function used to send a seek to the network.
        void _seek();

        void _styleUpdate();

        int _requestThumbnail(bool fetch = true);
        void _deleteThumbnails();
        void _thumbnailsUpdate();

        TLRENDER_PRIVATE();
    };
} // namespace mrv
