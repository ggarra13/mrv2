// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvDraw/Annotation.h"

class ViewerUI;
class Fl_Menu_Button;

namespace mrv
{

    struct TimelineViewport::Private
    {
        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
        std::vector<tl::timeline::ImageOptions> imageOptions;
        std::vector<tl::timeline::DisplayOptions> displayOptions;
        timeline::CompareOptions compareOptions;
        EnvironmentMapOptions environmentMapOptions;

        std::vector<TimelinePlayer*> timelinePlayers;
        // These are not used in mrv2.
        // std::vector<imaging::Size> timelineSizes;
        // std::vector<imaging::Size> timelineSizesTmp;

        math::Vector2i viewPos;
        float viewZoom = 1.F;
        int zoomDir = 0;
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

        //! Last annotation undones
        std::shared_ptr< draw::Annotation > undoAnnotation = nullptr;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;

        std::shared_ptr<imaging::FontSystem> fontSystem;
        Fl_Menu_Button* popupMenu = nullptr;

        //! Compositing switch (render on black or on background color)
        static bool blackBackground;

        //! Temporary help text displayed in HUD
        static std::string helpText;
        static float helpTextFade;

        //! HUD display flags (ORed together)
        static bool hudActive;
        static HudDisplay hud;

        //! Action Mode
        static ActionMode actionMode;

        //! Rectangle selection ( Color area )
        static math::BBox2i selection;

        //! Last video size (if changed, clear selection)
        static imaging::Size videoSize;

        //! Color area information
        area::Info colorAreaInfo;

        //! Safe Areas
        static bool safeAreas;

        //! Masking
        static float masking;

        //! Last time shown
        static otio::RationalTime lastTime;

        //! Skipped frames
        static uint64_t skippedFrames;

        //! We store really imaging::Color4f but since we need to reverse
        //! the R and B channels (as they are read in BGR order), we process
        //! floats.
        float* image = nullptr;

        //! Mark the buffer as raw, so we will delete with free().
        bool rawImage = true;

        //! Store the size of previous buffer so we avoid allocating it again.
        size_t rawImageSize = 0;

        bool fullScreen = false;
        bool presentation = false;
    };

} // namespace mrv
