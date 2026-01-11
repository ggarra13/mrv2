// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <vector>

#include <opentimelineio/clip.h>

#include <tlTimeline/Edit.h>

#include <tlTimelineUI/IItem.h>

#include "mrvVk/mrvVkWindow.h"

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

    class TimelinePlayer;

    namespace vulkan
    {
        using namespace tl;

        //! Timeline widget.
        class TimelineWidget : public VkWindow
        {
        public:
            TimelineWidget(int X, int Y, int W, int H, const char* L = 0);

            ~TimelineWidget() override;

            //! Set tlRender's context
            void setContext(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&, ViewerUI*);

            void setStyle(const std::shared_ptr<ui::Style>& = nullptr);

            //! Get timelineUI's timelineWidget item options
            timelineui_vk::ItemOptions getItemOptions() const;

            //! Get timelineUI's timelineWidget display options
            timelineui_vk::DisplayOptions getDisplayOptions() const;

            //! Set the timeline player.
            void setTimelinePlayer(TimelinePlayer*);

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Set whether the to frame the view.
            void setFrameView(bool);

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Set whether to automatically scroll to the current frame.
            void setScrollToCurrentFrame(bool);

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(ui::KeyModifier);

            //! Set whether thumbnails are enabled.
            void setThumbnails(bool);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the Display options.
            void setItemOptions(const timelineui_vk::ItemOptions&);

            //! Set the Display options.
            void setDisplayOptions(const timelineui_vk::DisplayOptions&);

            //! Frame the view.
            void frameView();

            //! @{ Standard FLTK functions
            void resize(int X, int Y, int W, int H) FL_OVERRIDE;
            void draw() FL_OVERRIDE;
            int handle(int) FL_OVERRIDE;
            void hide() FL_OVERRIDE;

            void prepare() FL_OVERRIDE;
            void destroy() FL_OVERRIDE;

            //! @}

            //! Hide the thumbnail at least until user enters the timeline
            //! slider again.
            void hideThumbnail();

            //! Request a new thumbnail and reposition on timeline.
            int requestThumbnail(bool fetch = true);

            //! Reposition timeline based on last event or hide it.
            void repositionThumbnail();

            //! Set the time units.
            void setUnits(TimeUnits);

            //! Get whether the timeline is editable.
            bool isEditable() const;

            //! Toggle timeline editable
            void setEditable(bool);

            int mousePressEvent(int button, bool, int modifiers);
            int
            mouseReleaseEvent(int X, int Y, int button, bool, int modifiers);

            void mouseMoveEvent(int X, int Y);
            void scrollEvent(const float X, const float Y, const int modifiers);
            int mouseDragEvent(int X, int Y);
            int keyPressEvent(unsigned key, const int modifiers);
            int keyReleaseEvent(unsigned key, const int modifiers);

            void setEditMode(const timeline::EditMode);
            
            void moveCallback(const std::vector<tl::timeline::MoveData>&);

            void continuePlaying();

            void init_colorspace() FL_OVERRIDE;
            
            std::vector<const otio::Item* > getSelectedItems() const;
            
        protected:
            const float pixelRatio() const;

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
            void prepare_shaders();
            void prepare_mesh();
            void prepare_pipeline_layout();
            void prepare_pipeline();

            void _thumbnailEvent();

            void _createThumbnailWindow();
            void _getThumbnailPosition(int& X, int& Y, int& W, int& H);

            void _setTimeUnits(tl::timeline::TimeUnits);

            void _cancelThumbnailRequests();

            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<ui::IWidget>&, bool visible, bool enabled,
                const ui::TickEvent&);

            bool _getSizeUpdate(const std::shared_ptr<ui::IWidget>&) const;
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<ui::IWidget>&, const ui::SizeHintEvent&);

            void _setGeometry();

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<ui::IWidget>&, const math::Box2i&,
                bool clipped);

            bool _getDrawUpdate(const std::shared_ptr<ui::IWidget>&) const;
            void _drawEvent(
                const std::shared_ptr<ui::IWidget>&, const math::Box2i&,
                const ui::DrawEvent&);

            int _toUI(int) const;
            math::Vector2i _toUI(const math::Vector2i&) const;
            int _fromUI(int) const;
            math::Vector2i _fromUI(const math::Vector2i&) const;

            unsigned _changeKey(unsigned key);

            otime::RationalTime _posToTime(int) noexcept;

            //! Function used to send a seek to the network.
            int _seek();

            void _styleUpdate();

            void _deleteThumbnails();
            void _thumbnailsUpdate();

            TLRENDER_PRIVATE();
        };

    } // namespace vulkan

} // namespace mrv
