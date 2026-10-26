// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvOS/mrvString.h"

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/Player.h>

#include "tlDraw/Annotation.h"

#include <chrono>
#include <deque>
#include <cstdint>

class ViewerUI;
class Fl_Menu_Button;

namespace mrv
{
    namespace opengl
    {

        struct TimelineViewport::Private
        {
            static image::Tags tagData;
            std::shared_ptr<observer::Value<timeline::BackgroundOptions> > backgroundOptions;
            static float rotation;
            static bool resizeWindow;

            static float pixelAspectRatio;

            //! Ghosting options
            static short ghostPrevious;
            static short ghostNext;

            bool hdrMonitorFound = false;

            //! Boolean to turn off ocio when playing a video or showing
            //! BT709 or sRGB images.
            bool ocio_disabled = false;

            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            static timeline::HDROptions hdrOptions;
            static timeline::ShaderOptions shaderOptions;

            std::vector<tl::timeline::ImageOptions> imageOptions;
            std::vector<tl::timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            Stereo3DOptions stereo3DOptions;
            static EnvironmentMapOptions environmentMapOptions;

            float videoRotation = 0.F;
            TimelinePlayer* player = nullptr;

            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = false;
            bool switchClip = true;

            int event_x, event_y, last_x;
            math::Vector2i mousePos;
            math::Vector2i mousePress;
            math::Vector2i viewPosMousePress;

            //! Previous screen where window was opened.  Used to avoid an
            //! expensive string copy.
            int previous_screen = -1;

            //! Timer used to stop scrubbing if there's no action on the user's
            //! side.
            std::chrono::high_resolution_clock::time_point lastScrubTime;

            //! Used to handle spinning in environment map mode.
            math::Vector2f viewSpin;

            //! Last environment map rotation in x and y (dx and dy)
            math::Vector2i rotDir;

            //! Used to handle play/stop with a single quick click in the
            //! view window.
            int lastEvent = 0;

            //! Show annotations
            bool showAnnotations = true;

            //! Show video
            bool showVideo = true;

            //! Main ui pointer
            ViewerUI* ui = nullptr;

            //! Video frame and data
            std::vector<tl::timeline::VideoFrame> videoData;

            //! Last valid video frame and data
            tl::timeline::VideoFrame lastVideoFrame;

            //! OpenGL3 fontSystem (used for HUD)
            std::shared_ptr<image::FontSystem> fontSystem;

            //! Right mouse menu
            Fl_Menu_Button* popupMenu = nullptr;

            //! HUD display flags (ORed together).
            static bool hudActive;
            static HudDisplay hud;

            //! Per-monitor Display/View OCIO Options.
            static std::vector<timeline::OCIOOptions> monitorOCIOOptions;

            //! Action Mode.
            static ActionMode actionMode;

            //! Edit Mode.
            static timeline::EditMode editMode;

            //! Playback mode before scrubbing.
            static tl::timeline::Playback playbackMode;

            //! Boolean flag used to mark that scrubbing is being used.
            static bool isScrubbing;

            //! Rectangle selection ( Color area )
            static math::Box2i selection;
            math::Vector2i selectionAnchor;

            //! Last video size (if changed, clear selection)
            static image::Size videoSize;

            //! Last FLTK cursor set on window.
            Fl_Cursor lastCursor = FL_CURSOR_ARROW;

            //! Color area information
            area::Info colorAreaInfo;

            //! Safe Areas
            static bool safeAreas;

            //! Data Window
            static bool dataWindow;

            //! Display Window
            static bool displayWindow;

            //! Ignore Display Window
            static bool ignoreDisplayWindow;

            //! Masking
            static float masking;

            //! Playback statistics for the HUD (FPS and DF).  Static so every
            //! viewport (main, secondary) shares one measurement.
            struct PlaybackStats
            {
                using Clock = std::chrono::steady_clock;

                uint64_t droppedFrames = 0;
                double fps = 0.0;

                //! Forget the baseline (play, stop, seek, scrub, step, ...).
                void restart(bool clearDropped = false)
                    {
                        _valid = false;
                        _shown.clear();
                        fps = 0.0;
                        if (clearDropped)
                            droppedFrames = 0;
                    }

                //! Call for every draw while playing.
                void update(int64_t frame, double speed, bool forward,
                            int64_t loopFrames, Clock::time_point now)
                    {
                        bool continuous = false;
                        if (_valid && forward == _forward)
                        {
                            if (frame == _frame)
                                return;  // same frame redrawn (mouse move, zoom, 2nd viewport)

                            int64_t advance = forward ? frame - _frame : _frame - frame;
                            if (advance < 1 && loopFrames > 0)
                                advance += loopFrames;  // wrapped around the loop

                            const double dt = std::chrono::duration<double>(now - _time).count();
                            const double expected = dt * speed;  // what the clock allows
                            if (advance >= 1 && advance <= expected + kJitterFrames)
                            {
                                droppedFrames += advance - 1;
                                continuous = true;
                            }
                        }
                        if (!continuous)
                            _shown.clear();  // seek / direction change / first frame

                        _valid = true;
                        _frame = frame;
                        _forward = forward;
                        _time = now;

                        _shown.push_back(now);
                        while (_shown.size() > 2 &&
                               now - _shown.front() > std::chrono::seconds(1))
                            _shown.pop_front();

                        if (_shown.size() >= 2)
                        {
                            const double span =
                                std::chrono::duration<double>(_shown.back() - _shown.front())
                                .count();
                            if (span > 0.0)
                                fps = (_shown.size() - 1) / span;
                        }
                    }

            private:
                static constexpr double kJitterFrames = 2.0;
                bool _valid = false;
                bool _forward = true;
                int64_t _frame = 0;
                Clock::time_point _time;
                std::deque<Clock::time_point> _shown;
            };
            static PlaybackStats stats;

            //! Whether the one view is in presentation mode (full screen with
            //! no menus, bars or dock tools).
            static bool presentation;

            //! We store really image::Color4f but since we need to reverse
            //! the R and B channels (as they are read in BGR order), we process
            //! floats.
            float* image = nullptr;

            //! Mark the buffer as raw, so we will delete with free().
            bool rawImage = true;

            //! Store the size of previous buffer so we avoid allocating it
            //! again.
            size_t rawImageSize = 0;

            //! Whether the view is in full screen mode
            bool fullScreen = false;

            //! Sets whether the window starts in maximized mode.
            bool maximized = false;

            //! Whether the current frame represengs a missing frame in a
            //! sequence.
            bool missingFrame = false;

            //! Default missing frame type.
            tl::io::MissingFrames missingFrameType =
                tl::io::MissingFrames::Black;

            //! Auxiliary variable used to hide cursor in presentation mode.
            std::chrono::high_resolution_clock::time_point presentationTime;

            //! Auxiliary variables to count FPS
            std::deque<double> frameTimes;
            std::chrono::high_resolution_clock::time_point startTime;

            // Observers.
            std::shared_ptr<observer::ListObserver<timeline::VideoFrame> >
            videoDataObserver;

            //! Overlay.
            std::shared_ptr<image::Image> overlayImage;

            //! Tooltip.
            Tooltip* tooltip = nullptr;

            //! Tablet information.
            float pressure = 0.F;
            math::Vector2f lastPenPosition;

            // libplacebo toggle.
            bool tonemap = true;
        };
    }  // namespace opengl
} // namespace mrv
