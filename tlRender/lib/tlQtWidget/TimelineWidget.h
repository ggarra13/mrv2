// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace timeline
    {
        class Player;
    }

    namespace qtwidget
    {
        //! Timeline widget.
        class TimelineWidget : public QOpenGLWidget,
                               protected QOpenGLFunctions_4_1_Core
        {
            Q_OBJECT

        public:
            TimelineWidget(
                const std::shared_ptr<ui::Style>&,
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            virtual ~TimelineWidget();

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get whether the timeline is editable.
            bool isEditable() const;

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Get whether to automatically scroll to the current frame.
            bool hasScrollToCurrentFrame() const;

            //! Get the mouse scroll key modifier.
            ui::KeyModifier scrollKeyModifier() const;

            //! Get the mouse wheel scale.
            float mouseWheelScale() const;

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Get the frame markers.
            const std::vector<int>& frameMarkers() const;

            //! Get the item options.
            const timelineui::ItemOptions& itemOptions() const;

            //! Get the display options.
            const timelineui::DisplayOptions& displayOptions() const;

            QSize minimumSizeHint() const override;
            QSize sizeHint() const override;

        public Q_SLOTS:
            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Set whether to automatically scroll to the current frame.
            void setScrollToCurrentFrame(bool);

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(ui::KeyModifier);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Set the item options.
            void setItemOptions(const timelineui::ItemOptions&);

            //! Set the display options.
            void setDisplayOptions(const timelineui::DisplayOptions&);

        Q_SIGNALS:
            //! This signal is emitted when the editable timeline is changed.
            void editableChanged(bool);

            //! This signal is emitted when the frame view is changed.
            void frameViewChanged(bool);

            //! This signal is emitted when scrubbing is in progress.
            void scrubChanged(bool);

            //! This signal is emitted when the time is scrubbed.
            void timeScrubbed(const tl::otime::RationalTime&);

        protected:
            void initializeGL() override;
            void resizeGL(int w, int h) override;
            void paintGL() override;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            void enterEvent(QEvent*) override;
#else
            void enterEvent(QEnterEvent*) override;
#endif // QT_VERSION
            void leaveEvent(QEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;
            void wheelEvent(QWheelEvent*) override;
            void keyPressEvent(QKeyEvent*) override;
            void keyReleaseEvent(QKeyEvent*) override;

            bool event(QEvent*) override;

        private:
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

            void _inputUpdate();
            void _timerUpdate();
            void _styleUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
