// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <deque>

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/Player.h>

#include "mrvDraw/Annotation.h"

class ViewerUI;
class Fl_Menu_Button;

namespace mrv
{

    struct TimelineViewport::Private
    {
        static timeline::BackgroundOptions backgroundOptions;
        timeline::OCIOOptions ocioOptions;
        timeline::LUTOptions lutOptions;
        std::vector<tl::timeline::ImageOptions> imageOptions;
        std::vector<tl::timeline::DisplayOptions> displayOptions;
        timeline::CompareOptions compareOptions;
        Stereo3DOptions stereo3DOptions;
        static EnvironmentMapOptions environmentMapOptions;

        std::vector<TimelinePlayer*> timelinePlayers;
        std::vector<image::Size> timelineSizes;

        math::Vector2i viewPos;
        float viewZoom = 1.F;
        bool frameView = false;
        int event_x, event_y, last_x;
        math::Vector2i mousePos;
        math::Vector2i mousePress;
        math::Vector2i viewPosMousePress;

        //! Used to handle spinning in environment map mode.
        math::Vector2f viewSpin;

        //! Used to handle play/stop with a single quick click in the
        //! view window.
        int lastEvent = 0;

        //! Show annotations
        bool showAnnotations = true;

        short ghostPrevious = 5;
        short ghostNext = 5;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;

        //! Last valid video frame and data
        tl::timeline::VideoData lastVideoData;

        //! OpenGL3 fontSystem (used for HUD)
        std::shared_ptr<image::FontSystem> fontSystem;

        //! Right mouse menu
        Fl_Menu_Button* popupMenu = nullptr;

        //! Temporary help text displayed in HUD
        static std::string helpText;
        static float helpTextFade;

        //! HUD display flags (ORed together)
        static bool hudActive;
        static HudDisplay hud;

        //! Action Mode
        static ActionMode actionMode;

        //! Playback mode before scrubbing
        static tl::timeline::Playback playbackMode;

        //! Rectangle selection ( Color area )
        static math::Box2i selection;

        //! Last video size (if changed, clear selection)
        static image::Size videoSize;

        //! Color area information
        area::Info colorAreaInfo;

        //! Safe Areas
        static bool safeAreas;

        //! Data Window
        static bool dataWindow;

        //! Display Window
        static bool displayWindow;

        //! Masking
        static float masking;

        //! Last time shown
        static otio::RationalTime lastTime;

        //! Skipped frames
        static uint64_t skippedFrames;

        //! We store really image::Color4f but since we need to reverse
        //! the R and B channels (as they are read in BGR order), we process
        //! floats.
        float* image = nullptr;

        //! Mark the buffer as raw, so we will delete with free().
        bool rawImage = true;

        //! Store the size of previous buffer so we avoid allocating it again.
        size_t rawImageSize = 0;

        //! Whether the view is in full screen mode
        bool fullScreen = false;

        //! Whether the view is in presentation mode (full screen with no menus,
        //! bars or dock tools).
        bool presentation = false;

        //! Whether the current frame represengs a missing frame in a sequence.
        bool missingFrame = false;

        //! Default missing frame type.  Should be static.
        MissingFrameType missingFrameType = kBlackFrame;

        //! Auxiliary variable used to hide cursor in presentation mode.
        std::chrono::high_resolution_clock::time_point presentationTime;

        //! Auxiliary variables to count FPS
        std::deque<double> frameTimes;
        std::chrono::high_resolution_clock::time_point startTime;
    };

} // namespace mrv
