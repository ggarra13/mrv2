// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <iostream>

#include <FL/Enumerations.H>

#include "mrvCore/mrvI8N.h"

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
            text(t) {};

        Hotkey(const Hotkey& h) :
            ctrl(h.ctrl),
            meta(h.meta),
            alt(h.alt),
            shift(h.shift),
            key(h.key),
            text(h.text) {};

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
    extern Hotkey kOpenSingleImage;
    extern Hotkey kOpenImage;
    extern Hotkey kOpenSeparateAudio;
    extern Hotkey kOpenSingleImage;
    extern Hotkey kOpenStereoImage;
    extern Hotkey kOpenSession;

    //! Application behavior
    extern Hotkey kOpenNewInstance;
    
    extern Hotkey kSaveImage;
    extern Hotkey kSaveImageToFolder;
    extern Hotkey kSaveOTIOEDL;
    extern Hotkey kSaveSequence;
    extern Hotkey kSaveAudio;
    extern Hotkey kSaveAnnotationsOnly;
    extern Hotkey kSaveAnnotationsAsJson;
    extern Hotkey kSavePDF;
    extern Hotkey kSaveSession;
    extern Hotkey kSaveSessionAs;

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

    extern Hotkey kToggleShowAnnotations;

    extern Hotkey kFrameView;
    extern Hotkey kSafeAreas;

    extern Hotkey kIgnoreDisplayWindow;
    extern Hotkey kIgnoreChromaticities;
    extern Hotkey kAutoNormalize;
    extern Hotkey kInvalidValues;
    
    extern Hotkey kHDRDataFromFile;
    extern Hotkey kHDRDataFalse;
    extern Hotkey kHDRDataTrue;
    
    extern Hotkey kToggleHDRTonemap;

    extern Hotkey kDisplayWindow;
    extern Hotkey kDataWindow;

    extern Hotkey kToggleBackground;

    extern Hotkey kCompareNone;
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
    extern Hotkey kLummaChannel;

    extern Hotkey kFlipX;
    extern Hotkey kFlipY;

    extern Hotkey kRotatePlus90;
    extern Hotkey kRotateMinus90;

    extern Hotkey kVideoLevelsFile;
    extern Hotkey kVideoLevelsLegalRange;
    extern Hotkey kVideoLevelsFullRange;

    extern Hotkey kAlphaBlendNone;
    extern Hotkey kAlphaBlendStraight;
    extern Hotkey kAlphaBlendPremultiplied;

    //! Playback hotkeys
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

    extern Hotkey kFirstFrame;
    extern Hotkey kLastFrame;

    extern Hotkey kStop;

    //! Looping hotkeys.
    extern Hotkey kSetInPoint;
    extern Hotkey kSetOutPoint;
    extern Hotkey kToggleOtioClipInOut;
    extern Hotkey kToggleInOutPoint;

    extern Hotkey kPlaybackLoop;
    extern Hotkey kPlaybackOnce;
    extern Hotkey kPlaybackPingPong;

    //! Versioning hotkeys.
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

    extern Hotkey kPreviousClip;
    extern Hotkey kNextClip;

    //! Cache hotkeys.
    extern Hotkey kClearCache;
    extern Hotkey kUpdateVideoFrame;

    //! Menu Bar hotkeys.
    extern Hotkey kToggleMenuBar;
    extern Hotkey kToggleTopBar;
    extern Hotkey kTogglePixelBar;
    extern Hotkey kToggleTimeline;
    extern Hotkey kToggleStatusBar;
    extern Hotkey kToggleToolBar;
    extern Hotkey kTogglePresentation;

    //! Drawing hotkeys.
    extern Hotkey kDrawMode;
    extern Hotkey kEraseMode;
    extern Hotkey kPolygonMode;
    extern Hotkey kCircleMode;
    extern Hotkey kArrowMode;
    extern Hotkey kRectangleMode;
    extern Hotkey kScrubMode;
    extern Hotkey kTextMode;
    extern Hotkey kVoiceMode;
    extern Hotkey kAreaMode;

    extern Hotkey kPenSizeMore;
    extern Hotkey kPenSizeLess;

    extern Hotkey kUndoDraw;
    extern Hotkey kRedoDraw;

    extern Hotkey kSwitchPenColor;

    //! Display hotkeys.
    extern Hotkey kResetChanges;
    extern Hotkey kExposureMore;
    extern Hotkey kExposureLess;
    extern Hotkey kSaturationMore;
    extern Hotkey kSaturationLess;
    extern Hotkey kGammaMore;
    extern Hotkey kGammaLess;

    extern Hotkey kHudToggle;

    extern Hotkey kOCIOToggle;
    extern Hotkey kOCIOPresetsToggle;
    extern Hotkey kOCIOInTopBarToggle;

    // @todo:
    extern Hotkey kGridToggle;
    extern Hotkey kGridSize;
    extern Hotkey kCopyFrameXYValues;
    extern Hotkey kCopyRGBAValues;

    //! OCIO Hotkeys.
    extern Hotkey kOCIOInputColorSpace;
    extern Hotkey kOCIODisplay;
    extern Hotkey kOCIOView;

    //! Windows/Panels hotkeys.
    extern Hotkey kToggleOnePanelOnly;
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
    extern Hotkey kToggleNDI;
    extern Hotkey kToggleNetwork;
    extern Hotkey kToggleStereo3D;
    extern Hotkey kToggleUSD;
    
    //! Window behavior
    extern Hotkey kToggleClickThrough;
    extern Hotkey kUITransparencyLess;
    extern Hotkey kUITransparencyMore;

    //! Editing hotkeys.
    extern Hotkey kToggleEditMode;
    extern Hotkey kToggleTimelineThumbnails;
    extern Hotkey kToggleTimelineTransitions;
    extern Hotkey kToggleTimelineMarkers;
    extern Hotkey kToggleTimelineEditable;
    extern Hotkey kToggleEditAssociatedClips;
    extern Hotkey kToggleTimelineFrameView;
    extern Hotkey kToggleTimelineScrollToCurrentFrame;
    extern Hotkey kToggleTimelineTrackInfo;
    extern Hotkey kToggleTimelineClipInfo;

    extern Hotkey kEditCutFrame;
    extern Hotkey kEditCopyFrame;
    extern Hotkey kEditPasteFrame;
    extern Hotkey kEditInsertFrame;

    extern Hotkey kEditSliceClip;
    extern Hotkey kEditRemoveClip;
    extern Hotkey kEditInsertAudioClip;
    extern Hotkey kEditRemoveAudioClip;
    extern Hotkey kEditInsertAudioGap;
    extern Hotkey kEditRemoveAudioGap;

    extern Hotkey kToggleMuteAudio;
    
    extern Hotkey kEditUndo;
    extern Hotkey kEditRedo;

    extern Hotkey kRotatePlus90;
    extern Hotkey kRotateMinus90;

    /**
     * @brief Struct used to hold a hotkey entry.
     *
     */
    struct HotkeyEntry
    {
        /**
         * Default constructor.
         *
         */
        HotkeyEntry() :
            force(false),
            hotkey(nullptr) {};

        /**
         * HotkeyEntry constructor.
         *
         * @param n name of the hotkey command.
         * @param h hotkey to u se.
         * @param f whether to force its use.
         *
         */
        HotkeyEntry(const std::string n, Hotkey* h, bool f = false) :
            force(f),
            name(n),
            hotkey(h) {};

        ~HotkeyEntry() { /*delete hotkey;*/ }

        bool force;
        std::string name;
        Hotkey* hotkey;
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

    static struct TableText table[] = {
        {FL_Escape, _("Escape")},
        {FL_BackSpace, _("BackSpace")},
        {FL_Tab, _("Tab")},
        {FL_Enter, _("Return")},
        {FL_Print, _("Print")},

        {FL_Scroll_Lock, _("ScrollLock")},
        {FL_Pause, _("Pause")},
        {FL_Insert, _("Insert")},
        {FL_Home, _("Home")},
        {FL_Page_Up, _("PageUp")},

        {FL_Delete, _("Delete")},
        {FL_End, _("End")},
        {FL_Page_Down, _("PageDown")},
        {FL_Left, _("Left")},
        {FL_Up, _("Up")},

        {FL_Right, _("Right")},
        {FL_Down, _("Down")},
        {FL_Shift_L, _("LeftShift")},
        {FL_Shift_R, _("RightShift")},
        {FL_Control_L, _("LeftCtrl")},

        {FL_Control_R, _("RightCtrl")},
        {FL_Caps_Lock, _("CapsLock")},
        {FL_Alt_L, _("LeftAlt")},
        {FL_Alt_R, _("RightAlt")},
        {FL_Meta_L, _("LeftMeta")},

        {FL_Meta_R, _("RightMeta")},
        {FL_Menu, _("Menu")},
        {FL_Num_Lock, _("NumLock")},
        {FL_KP_Enter, _("padEnter")},
        {FL_KP + '0', _("pad0")},

        {FL_KP + '1', _("pad1")},
        {FL_KP + '2', _("pad2")},
        {FL_KP + '3', _("pad3")},
        {FL_KP + '4', _("pad4")},
        {FL_KP + '5', _("pad5")},

        {FL_KP + '6', _("pad6")},
        {FL_KP + '7', _("pad7")},
        {FL_KP + '8', _("pad8")},
        {FL_KP + '9', _("pad9")},
        {' ', _("Space (' ')")},

        {FL_KP + '*', _("Multiply")},
        {FL_KP + '+', _("Add")},
        {FL_KP + '-', _("Subtract")},
        {FL_KP + '.', _("Decimal")},
        {FL_KP + '/', _("Divide")},
        {FL_Help, "FL_Help"},
    };

    extern HotkeyEntry hotkeys[];

    void store_default_hotkeys();
    void reset_hotkeys();
} // namespace mrv
