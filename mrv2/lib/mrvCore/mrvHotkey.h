// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <iostream>

#include <FL/Enumerations.H>

namespace mrv
{

    /**
     * Struct used to hold a hotkey.
     *
     */
    struct Hotkey
    {
        Hotkey() :
            ctrl(false),
            meta(false),
            alt(false),
            shift(false),
            key(0),
            text("")
        {
        }

        Hotkey(
            const bool c, const bool m, const bool a, const bool s,
            const unsigned k, std::string t = "") :
            ctrl(c),
            meta(m),
            alt(a),
            shift(s),
            key(k),
            text(t){};

        Hotkey(const Hotkey& h) :
            ctrl(h.ctrl),
            meta(h.meta),
            alt(h.alt),
            shift(h.shift),
            key(h.key),
            text(h.text){};

        void clear()
        {
            ctrl = meta = alt = shift = false;
            key = 0;
            text.clear();
        }

        bool match(unsigned rawkey);

        bool operator==(const Hotkey& b) const;

        unsigned hotkey()
        {
            unsigned r = 0;
            if (ctrl)
                r += FL_CTRL;
            if (shift)
                r += FL_SHIFT;
            if (meta)
                r += FL_META;
            if (alt)
                r += FL_ALT;
            r += key;
            return r;
        }

        std::string to_s() const;

    public:
        bool ctrl;
        bool meta;
        bool alt;
        bool shift;
        unsigned key;
        std::string text;
    };

    extern Hotkey kOpenDirectory;
    extern Hotkey kOpenImage;
    extern Hotkey kOpenSeparateAudio;
    extern Hotkey kOpenSingleImage;
    extern Hotkey kOpenStereoImage;
    extern Hotkey kOpenSession;
    extern Hotkey kSaveReel;
    extern Hotkey kSaveImage;
    extern Hotkey kSaveSequence;
    extern Hotkey kSaveSession;

    extern Hotkey kCloseCurrent;
    extern Hotkey kCloseAll;

    extern Hotkey kQuitProgram;

    extern Hotkey kZoomMin;
    extern Hotkey kZoomMax;

    extern Hotkey kZoomIn;
    extern Hotkey kZoomOut;
    extern Hotkey kFullScreen;
    extern Hotkey kToggleFloatOnTop;
    extern Hotkey kToggleSecondary;
    extern Hotkey kToggleSecondaryFloatOnTop;
    extern Hotkey kCenterImage;
    extern Hotkey kFitScreen;
    extern Hotkey kResizeMainWindow;
    extern Hotkey kFitAll;
    extern Hotkey kMinifyTextureFiltering;
    extern Hotkey kMagnifyTextureFiltering;

    extern Hotkey kSafeAreas;

    // @todo:
    extern Hotkey kDisplayWindow;
    extern Hotkey kDataWindow;

    extern Hotkey kToggleBlackBackground;

    extern Hotkey kCompareWipe;
    extern Hotkey kCompareOverlay;
    extern Hotkey kCompareDifference;
    extern Hotkey kCompareHorizontal;
    extern Hotkey kCompareVertical;
    extern Hotkey kCompareTile;

    extern Hotkey kColorChannel;
    extern Hotkey kRedChannel;
    extern Hotkey kGreenChannel;
    extern Hotkey kBlueChannel;
    extern Hotkey kAlphaChannel;

    extern Hotkey kFlipX;
    extern Hotkey kFlipY;

    extern Hotkey kShapeFrameStepBack;
    extern Hotkey kShapeFrameStepFwd;

    extern Hotkey kShapeFrameClear;
    extern Hotkey kShapeFrameClearAll;

    extern Hotkey kFrameStepBack;
    extern Hotkey kFrameStepFPSBack;

    extern Hotkey kFrameStepFwd;
    extern Hotkey kFrameStepFPSFwd;

    extern Hotkey kPlayBack;
    extern Hotkey kPlayBackHalfSpeed;

    extern Hotkey kPlayDirection;

    extern Hotkey kPlayFwd;
    extern Hotkey kPlayFwdTwiceSpeed;

    extern Hotkey kStop;

    extern Hotkey kPlaybackLoop;
    extern Hotkey kPlaybackOnce;
    extern Hotkey kPlaybackPingPong;

    extern Hotkey kFirstVersionImage;
    extern Hotkey kPreviousVersionImage;
    extern Hotkey kNextVersionImage;
    extern Hotkey kLastVersionImage;

    extern Hotkey kPreviousImage;
    extern Hotkey kNextImage;

    extern Hotkey kPreviousImageLimited;
    extern Hotkey kNextImageLimited;

    extern Hotkey kPreviousChannel;
    extern Hotkey kNextChannel;

    extern Hotkey kFirstFrame;
    extern Hotkey kLastFrame;

    extern Hotkey kToggleMenuBar;
    extern Hotkey kToggleTopBar;
    extern Hotkey kTogglePixelBar;
    extern Hotkey kToggleTimeline;
    extern Hotkey kToggleStatusBar;
    extern Hotkey kToggleToolBar;
    extern Hotkey kTogglePresentation;

    extern Hotkey kToggleOnePanelOnly;

    extern Hotkey kDrawMode;
    extern Hotkey kEraseMode;
    extern Hotkey kCircleMode;
    extern Hotkey kArrowMode;
    extern Hotkey kRectangleMode;
    extern Hotkey kScrubMode;
    extern Hotkey kTextMode;
    extern Hotkey kAreaMode;

    extern Hotkey kPenSizeMore;
    extern Hotkey kPenSizeLess;

    extern Hotkey kUndoDraw;
    extern Hotkey kRedoDraw;

    extern Hotkey kResetChanges;
    extern Hotkey kExposureMore;
    extern Hotkey kExposureLess;
    extern Hotkey kGammaMore;
    extern Hotkey kGammaLess;

    extern Hotkey kSetInPoint;
    extern Hotkey kSetOutPoint;

    // @todo:

    extern Hotkey kCopyFrameXYValues;
    extern Hotkey kCopyRGBAValues;

    // @todo:
    extern Hotkey kGridToggle;
    extern Hotkey kGridSize;

    //

    extern Hotkey kHudToggle;

    // OCIO Hotkeys
    extern Hotkey kOCIOInputColorSpace;
    extern Hotkey kOCIODisplay;
    extern Hotkey kOCIOView;

    // Windows hotkeys
    extern Hotkey kToggleReel;
    extern Hotkey kToggleMediaInfo;
    extern Hotkey kToggleColorInfo;
    extern Hotkey kToggleColorControls;
    extern Hotkey kToggleCompare;
    extern Hotkey kTogglePlaylist;
    extern Hotkey kToggleDevices;
    extern Hotkey kToggleAnnotation;
    extern Hotkey kToggleSettings;
    extern Hotkey kTogglePreferences;
    extern Hotkey kToggleHistogram;
    extern Hotkey kToggleVectorscope;
    extern Hotkey kToggleWaveform;
    extern Hotkey kToggleEnvironmentMap;
    extern Hotkey kToggleHotkeys;
    extern Hotkey kToggleLogs;
    extern Hotkey kTogglePythonConsole;
    extern Hotkey kToggleAbout;
    extern Hotkey kToggleNetwork;

    // @todo:
    extern Hotkey kRotatePlus90;
    extern Hotkey kRotateMinus90;

    /**
     * @brief Struct used to hold a hotkey entry.
     *
     */
    struct HotkeyEntry
    {
        /**
         * HotkeyEntry constructor.
         *
         * @param n name of the hotkey command.
         * @param h hotkey to u se.
         * @param f whether to force its use.
         *
         */
        HotkeyEntry(const std::string n, Hotkey& h, bool f = false) :
            force(f),
            name(n),
            hotkey(h){};

        bool force;
        std::string name;
        Hotkey& hotkey;
    };

    /**
     * @brief Table used to hold a key and the corresponding name for it.
     *
     */
    struct TableText
    {
        unsigned n;
        const char* text;
    };

    extern struct TableText table[];
    extern HotkeyEntry hotkeys[];
} // namespace mrv
