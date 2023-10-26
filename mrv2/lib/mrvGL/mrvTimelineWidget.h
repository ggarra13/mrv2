// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <vector>

#include <opentimelineio/clip.h>

#include <tlTimeline/Edit.h>

#include <tlTimelineUI/IItem.h>

#include "mrvFl/mrvTimelinePlayer.h"

#ifdef TLRENDER_GL
#    include "mrvGL/mrvGLWindow.h"
#    define Fl_SuperClass GLWindow
#endif

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
    class TimelineWidget : public Fl_SuperClass
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

        //! @{ Standard FLTK functions
        void resize(int X, int Y, int W, int H) FL_OVERRIDE;
        void draw() FL_OVERRIDE;
        int handle(int) FL_OVERRIDE;
        //! @}

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

        //! Get whether the timeline is editable.
        bool isEditable() const;

        //! Toggle timeline editable
        void setEditable(bool);

        int mousePressEvent(int button, bool, int modifiers);
        int mouseReleaseEvent(int X, int Y, int button, bool, int modifiers);

        void mouseMoveEvent(int X, int Y);
        void scrollEvent(const float X, const float Y, const int modifiers);
        int mouseDragEvent(int X, int Y);
        int keyPressEvent(unsigned key, const int modifiers);
        int keyReleaseEvent(unsigned key, const int modifiers);

        void moveCallback(const std::vector<tl::timeline::MoveData>&);

    protected:
        void _initializeGL();
        void _initializeGLResources();

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

    private:
        void _setTimeUnits(tl::timeline::TimeUnits);

        int _toUI(int) const;
        math::Vector2i _toUI(const math::Vector2i&) const;
        int _fromUI(int) const;
        math::Vector2i _fromUI(const math::Vector2i&) const;

        unsigned _changeKey(unsigned key);
        void _drawAnnotationMarks() const noexcept;

        otime::RationalTime _posToTime(int) const noexcept;
        int _timeToPos(const otime::RationalTime&) const noexcept;

        //! Function used to send a seek to the network.
        int _seek();

        void _styleUpdate();

        int _requestThumbnail(bool fetch = true);
        void _deleteThumbnails();
        void _thumbnailsUpdate();

        TLRENDER_PRIVATE();
    };
} // namespace mrv
