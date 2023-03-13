// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"

#include "mrvFl/mrvIO.h"

#include <FL/Fl.H>

namespace
{
    const char* kModule = "hotkey";
}

namespace mrv
{
    // on macOS            ctrl, cmd. , alt, shift, key
    //                     ctrl, meta , alt, shift, key
    Hotkey kOpenDirectory(true, false, false, true, 'o');
    Hotkey kOpenImage(true, false, false, false, 'o');
    Hotkey kOpenSeparateAudio(false, false, false, false, 0);
    Hotkey kOpenSingleImage(true, true, false, false, 'o');
    Hotkey kOpenStereoImage(false, false, true, false, 'o');
    Hotkey kOpenAMF(false, false, false, true, 'o');
    Hotkey kOpenClipXMLMetadata(true, false, false, false, 'x');
    Hotkey kOpenSession(false, true, false, false, 'o');
    Hotkey kSaveReel(true, false, false, false, 0);
    Hotkey kSaveImage(true, false, false, false, 's');
    Hotkey kSaveSequence(true, false, false, true, 's');
    Hotkey kSaveSession(false, true, false, false, 's');

    Hotkey kCloseCurrent(false, false, false, false, 0);
    Hotkey kCloseAll(false, false, false, false, 0);

    Hotkey kQuitProgram(false, false, false, false, FL_Escape);

    Hotkey kZoomMin(false, false, false, false, '0');
    Hotkey kZoomMax(false, false, false, false, '9');

    Hotkey kZoomIn(false, false, false, false, 0);
    Hotkey kZoomOut(false, false, false, false, 0);
    Hotkey kFullScreen(false, false, false, false, FL_F + 11);
    Hotkey kToggleFloatOnTop(false, false, false, false, 0);
    Hotkey kToggleSecondary(false, false, false, false, 0);
    Hotkey kToggleSecondaryFloatOnTop(false, false, false, false, 0);

    Hotkey kToggleOnePanelOnly(false, false, false, false, 'p');

    Hotkey kFitScreen(false, false, false, false, 'f');
    Hotkey kResizeMainWindow(false, false, false, true, 'w');
    Hotkey kFitAll(false, false, false, false, 0);
    Hotkey kTextureFiltering(false, false, false, true, 'f');
    Hotkey kSafeAreas(false, false, false, false, 's');
    Hotkey kDisplayWindow(false, false, false, false, 'd');
    Hotkey kDataWindow(true, false, false, false, 'd');

    Hotkey kCompareWipe(false, false, false, false, 'w');
    Hotkey kCompareOverlay(false, false, false, false, 0);
    Hotkey kCompareDifference(false, false, false, false, 0);
    Hotkey kCompareHorizontal(false, false, false, false, 0);
    Hotkey kCompareVertical(false, false, false, false, 0);
    Hotkey kCompareTile(false, false, false, false, 0);

    Hotkey kColorChannel(false, false, false, false, 'c');
    Hotkey kRedChannel(false, false, false, false, 'r');
    Hotkey kGreenChannel(false, false, false, false, 'g');
    Hotkey kBlueChannel(false, false, false, false, 'b');
    Hotkey kAlphaChannel(false, false, false, false, 'a');

    Hotkey kFlipX(false, false, false, false, 'x');
    Hotkey kFlipY(false, false, false, false, 'y');
    Hotkey kCenterImage(false, false, false, false, 'h');

    Hotkey kShapeFrameStepBack(false, false, false, true, FL_Left, "");
    Hotkey kFrameStepBack(false, false, false, false, FL_Left, "");
    Hotkey kFrameStepFPSBack(true, false, false, false, FL_Left, "");
    Hotkey kFrameStepFwd(false, false, false, false, FL_Right, "");
    Hotkey kShapeFrameStepFwd(false, false, false, true, FL_Right, "");
    Hotkey kFrameStepFPSFwd(true, false, false, false, FL_Right, "");
    Hotkey kPlayBackHalfSpeed(false, false, false, false, 'j');
    Hotkey kPlayBack(false, false, false, false, FL_Up, "");
    Hotkey kPlayDirection(false, false, false, false, ' ');
    Hotkey kPlayFwd(false, false, false, false, FL_Down, "");
    Hotkey kPlayFwdTwiceSpeed(false, false, false, false, 'k');
    Hotkey kStop(false, false, false, false, FL_Enter);

    Hotkey kShapeFrameClear(false, false, false, false, 0, "");
    Hotkey kShapeFrameClearAll(false, false, false, false, 0, "");

    Hotkey kPlaybackLoop(false, false, false, false, 0);
    Hotkey kPlaybackOnce(false, false, false, false, 0);
    Hotkey kPlaybackPingPong(false, false, false, false, 0);

    Hotkey kFirstVersionImage(false, false, true, true, FL_Page_Up);
    Hotkey kPreviousVersionImage(false, false, true, false, FL_Page_Up);
    Hotkey kNextVersionImage(false, false, true, false, FL_Page_Down);
    Hotkey kLastVersionImage(false, false, true, true, FL_Page_Down);

    Hotkey kPreviousImage(false, false, false, false, FL_Page_Up);
    Hotkey kNextImage(false, false, false, false, FL_Page_Down);

    Hotkey kPreviousImageLimited(true, false, false, false, FL_Page_Up);
    Hotkey kNextImageLimited(true, false, false, false, FL_Page_Down);

    Hotkey kFirstFrame(false, false, false, false, FL_Home);
    Hotkey kLastFrame(false, false, false, false, FL_End);

    Hotkey kToggleMenuBar(false, false, false, true, FL_F + 1);
    Hotkey kToggleTopBar(false, false, false, false, FL_F + 1);
    Hotkey kTogglePixelBar(false, false, false, false, FL_F + 2);
    Hotkey kToggleTimeline(false, false, false, false, FL_F + 3);
    Hotkey kToggleStatusBar(false, false, false, false, 0);
    Hotkey kToggleToolBar(false, false, false, true, FL_F + 7);
    Hotkey kTogglePresentation(false, false, false, false, FL_F + 12);

    Hotkey kPreviousChannel(false, false, false, false, 0, "{");
    Hotkey kNextChannel(false, false, false, false, 0, "}");

    Hotkey kDrawMode(false, false, false, true, 'd');
    Hotkey kEraseMode(false, false, false, true, 'e');
    Hotkey kScrubMode(false, false, false, true, 's');
    Hotkey kAreaMode(false, false, false, true, 0);
    Hotkey kArrowMode(false, false, false, true, 'a');
    Hotkey kRectangleMode(false, false, false, true, 'r');
    Hotkey kCircleMode(false, false, false, true, 'c');
    Hotkey kTextMode(false, false, false, true, 't');

    Hotkey kPenSizeMore(false, false, false, false, 0, "]");
    Hotkey kPenSizeLess(false, false, false, false, 0, "[");

    Hotkey kUndoDraw(false, true, false, false, 'z');
    Hotkey kRedoDraw(false, true, false, true, 'z');

    Hotkey kResetChanges(true, false, false, false, 'r');
    Hotkey kExposureMore(false, false, false, false, '.');
    Hotkey kExposureLess(false, false, false, false, ',');
    Hotkey kGammaMore(false, false, false, false, 0, ")");
    Hotkey kGammaLess(false, false, false, false, 0, "(");

    Hotkey kSelectSingleImage(false, false, false, false, 0);
    Hotkey kSelectMultiImage(false, false, false, false, 0);

    Hotkey kSOPSatNodes(false, false, false, false, 0);

    Hotkey kCopyFrameXYValues(true, false, false, true, 'c');
    Hotkey kCopyRGBAValues(true, false, false, false, 'c');

    Hotkey kSetInPoint(false, false, false, false, 'i');
    Hotkey kSetOutPoint(false, false, false, false, 'o');

    Hotkey kGridToggle(true, false, false, false, 'g');
    Hotkey kGridSize(true, true, false, false, 'g');
    Hotkey kHudToggle(true, false, false, false, 'h');

    Hotkey kOCIOInputColorSpace(false, false, false, false, 0);
    Hotkey kOCIODisplay(false, false, false, false, 0);
    Hotkey kOCIOView(false, false, false, false, 0);

    Hotkey kToggleReel(false, false, false, false, FL_F + 4);
    Hotkey kToggleMediaInfo(false, false, false, false, FL_F + 5);
    Hotkey kToggleColorControls(false, false, false, false, FL_F + 6);
    Hotkey kToggleColorInfo(false, false, false, false, FL_F + 7);
    Hotkey kTogglePlaylist(false, false, false, false, 0);
    Hotkey kToggleCompare(false, false, false, false, FL_F + 8);
    Hotkey kToggleDevices(false, false, false, false, 0);
    Hotkey kToggleAnnotation(false, false, false, false, 0);
    Hotkey kToggleSettings(false, false, false, false, FL_F + 9);
    Hotkey kTogglePreferences(false, false, false, false, FL_F + 10);
    Hotkey kToggleHistogram(false, false, false, false, 0);
    Hotkey kToggleVectorscope(false, false, false, false, 0);
    Hotkey kToggleEnvironmentMap(false, false, false, false, 0);
    Hotkey kToggleWaveform(false, false, false, false, 0);
    Hotkey kToggleHotkeys(false, false, false, false, 0);
    Hotkey kTogglePythonConsole(false, false, false, false, 0);
    Hotkey kToggleLogs(false, false, false, false, 0);
    Hotkey kToggleAbout(false, false, false, false, 0);

    Hotkey kRotatePlus90;  //( false, false, false, false, '+' );
    Hotkey kRotateMinus90; //( false, false, false, false, '-' );

    inline bool has_shift(unsigned rawkey)
    {
        return Fl::event_key(FL_Shift_L) || Fl::event_key(FL_Shift_R);
    }

    inline bool has_ctrl(unsigned rawkey)
    {
        return Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R);
    }

    inline bool has_alt(unsigned rawkey)
    {
        return Fl::event_key(FL_Alt_L) || Fl::event_key(FL_Alt_R);
    }

    inline bool has_meta(unsigned rawkey)
    {
        return Fl::event_key(FL_Meta_L) || Fl::event_key(FL_Meta_R);
    }

    bool Hotkey::match(unsigned rawkey)
    {
        bool ok = false;

        const char* t = Fl::event_text();

        if ((!ctrl && !shift && !alt && !meta) && (!has_shift(rawkey)) &&
            (!has_ctrl(rawkey)) && (!has_alt(rawkey)) && (!has_meta(rawkey)) &&
            ((key && (int)key == t[0]) || (text.size() && text == t)))
        {
            return true;
        }

        if (ctrl)
        {
            if (has_ctrl(rawkey))
            {
                ok = true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (has_ctrl(rawkey))
                return false;
        }

        if (shift)
        {
            if (has_shift(rawkey))
                ok = true;
            else
                return false;
        }
        else
        {
            // We need to check for text as we get text like "("
            if (has_shift(rawkey) && text.empty())
                return false;
        }
        if (alt)
        {
            if (has_alt(rawkey))
                ok = true;
            else
                return false;
        }
        else
        {
            if (has_alt(rawkey))
                return false;
        }
        if (meta)
        {
            if (has_meta(rawkey))
                ok = true;
            else
                return false;
        }
        else
        {
            if (has_meta(rawkey))
                return false;
        }

        if (rawkey != 0)
        {
            if ((!text.empty()) && text == t)
            {
                ok = true;
            }
            else if (rawkey == key)
            {
                ok = true;
            }
            else
            {
                ok = false;
            }
        }

        return ok;
    }

    HotkeyEntry hotkeys[] = {
        HotkeyEntry(_("Open Directory"), kOpenDirectory),
        HotkeyEntry(_("Open Movie or Sequence"), kOpenImage),
        HotkeyEntry(_("Open Single Image"), kOpenSingleImage),
        // HotkeyEntry( _("Open Session"), kOpenSession, true),
        // HotkeyEntry( _("Save OTIO file"), kSaveReel),
        HotkeyEntry(_("Save Image"), kSaveImage),
        HotkeyEntry(_("Save Sequence"), kSaveSequence),
        // HotkeyEntry( _("Save Session"), kSaveSession, true),
        HotkeyEntry(_("Close Current"), kCloseCurrent, true),
        HotkeyEntry(_("Close All"), kCloseAll, true),
        HotkeyEntry(_("Quit Program"), kQuitProgram, true),
        HotkeyEntry(_("Center Image"), kCenterImage),
        HotkeyEntry(_("Fit Screen"), kFitScreen),
        HotkeyEntry(_("Resize Main Window to Fit"), kResizeMainWindow, true),
        HotkeyEntry(_("Fit All"), kFitAll),
        // HotkeyEntry( _("TextureFiltering"), kTextureFiltering),
        HotkeyEntry(_("Safe Areas"), kSafeAreas),
        // HotkeyEntry( _("Display Window"), kDisplayWindow),
        // HotkeyEntry( _("Data Window"), kDataWindow),
        HotkeyEntry(_("Compare Wipe"), kCompareWipe),
        HotkeyEntry(_("Compare Overlay"), kCompareOverlay),
        HotkeyEntry(_("Compare Difference"), kCompareDifference),
        HotkeyEntry(_("Compare Horizontal"), kCompareHorizontal),
        HotkeyEntry(_("Compare Vertical"), kCompareVertical),
        HotkeyEntry(_("Compare Tile"), kCompareTile),
        HotkeyEntry(_("Color Channel"), kColorChannel, true),
        HotkeyEntry(_("Red Channel"), kRedChannel, true),
        HotkeyEntry(_("Green Channel"), kGreenChannel, true),
        HotkeyEntry(_("Blue Channel"), kBlueChannel, true),
        HotkeyEntry(_("Alpha Channel"), kAlphaChannel, true),
        HotkeyEntry(_("Flip X"), kFlipX),
        HotkeyEntry(_("Flip Y"), kFlipY),
        HotkeyEntry(_("Annotation Clear Frame"), kShapeFrameClear, true),
        HotkeyEntry(_("Annotation Clear All Frames"), kShapeFrameClear, true),
        HotkeyEntry(
            _("Annotation Frame Step Backwards"), kShapeFrameStepBack, true),
        HotkeyEntry(_("Frame Step Backwards"), kFrameStepBack),
        HotkeyEntry(_("Frame Step FPS Backwards"), kFrameStepFPSBack),
        HotkeyEntry(
            _("Annotation Frame Step Forwards"), kShapeFrameStepFwd, true),
        HotkeyEntry(_("Frame Step Forwards"), kFrameStepFwd),
        HotkeyEntry(_("Frame Step FPS Forwards"), kFrameStepFPSFwd),
        HotkeyEntry(_("Play Backwards"), kPlayBack),
        HotkeyEntry(_("Play Backwards / Change Speed"), kPlayBackHalfSpeed),
        HotkeyEntry(_("Play in Current Direction"), kPlayDirection),
        HotkeyEntry(_("Play Forwards"), kPlayFwd),
        HotkeyEntry(_("Play Forwards / Change Speed"), kPlayFwdTwiceSpeed),
        HotkeyEntry(_("Stop"), kStop),
        HotkeyEntry(_("Loop Playback"), kPlaybackLoop),
        HotkeyEntry(_("Playback Once"), kPlaybackOnce),
        HotkeyEntry(_("Playback Ping Pong"), kPlaybackPingPong),
        HotkeyEntry(_("First Image Version"), kFirstVersionImage, true),
        HotkeyEntry(_("Previous Image Version"), kPreviousVersionImage),
        HotkeyEntry(_("Next Image Version"), kNextVersionImage),
        HotkeyEntry(_("Last Image Version"), kLastVersionImage, true),
        HotkeyEntry(_("Previous Image"), kPreviousImage),
        HotkeyEntry(_("Next Image"), kNextImage),
        HotkeyEntry(_("Previous Image Limited"), kPreviousImageLimited),
        HotkeyEntry(_("Next Image Limited"), kNextImageLimited),
        HotkeyEntry(_("Previous Channel"), kPreviousChannel),
        HotkeyEntry(_("Next Channel"), kNextChannel),
        HotkeyEntry(_("First Frame"), kFirstFrame),
        HotkeyEntry(_("Last Frame"), kLastFrame),
        HotkeyEntry(_("Toggle Menu Bar"), kToggleMenuBar),
        HotkeyEntry(_("Toggle Top Bar"), kToggleTopBar),
        HotkeyEntry(_("Toggle Pixel Bar"), kTogglePixelBar),
        HotkeyEntry(_("Toggle Timeline"), kToggleTimeline, true),
        HotkeyEntry(_("Toggle Status Bar"), kToggleStatusBar),
        HotkeyEntry(_("Toggle Tool Dock"), kToggleToolBar),
        HotkeyEntry(_("Toggle Full Screen"), kFullScreen),
        HotkeyEntry(_("Toggle Presentation"), kTogglePresentation),
        HotkeyEntry(_("Toggle Float On Top"), kToggleFloatOnTop),
        HotkeyEntry(_("Toggle Secondary"), kToggleSecondary),
        HotkeyEntry(
            _("Toggle Secondary Float On Top"), kToggleSecondaryFloatOnTop),
        HotkeyEntry(_("Reset Gain/Gamma"), kResetChanges),
        HotkeyEntry(_("Exposure More"), kExposureMore),
        HotkeyEntry(_("Exposure Less"), kExposureLess),
        HotkeyEntry(_("OCIO Input Color Space"), kOCIOInputColorSpace),
        HotkeyEntry(_("OCIO Display"), kOCIODisplay),
        HotkeyEntry(_("OCIO View"), kOCIOView),
        HotkeyEntry(_("Scrub Mode"), kScrubMode),
        HotkeyEntry(_("Area Selection Mode"), kAreaMode),
        HotkeyEntry(_("Draw Mode"), kDrawMode),
        HotkeyEntry(_("Erase Mode"), kEraseMode),
        HotkeyEntry(_("Arrow Mode"), kArrowMode, true),
        HotkeyEntry(_("Rectangle Mode"), kRectangleMode, true),
        HotkeyEntry(_("Circle Mode"), kCircleMode),
        HotkeyEntry(_("Text Mode"), kTextMode),
        HotkeyEntry(_("Pen Size More"), kPenSizeMore),
        HotkeyEntry(_("Pen Size Less"), kPenSizeLess),
        HotkeyEntry(_("Undo Draw"), kUndoDraw),
        HotkeyEntry(_("Redo Draw"), kRedoDraw),
        HotkeyEntry(_("Gamma More"), kGammaMore),
        HotkeyEntry(_("Gamma Less"), kGammaLess),
        HotkeyEntry(_("Set In Point"), kSetInPoint),
        HotkeyEntry(_("Set Out Point"), kSetOutPoint),
        HotkeyEntry(_("Toggle One Panel Only"), kToggleOnePanelOnly),
        HotkeyEntry(_("Toggle Files Panel"), kToggleReel),
        HotkeyEntry(_("Toggle Media Info Panel"), kToggleMediaInfo),
        HotkeyEntry(_("Toggle Color Area Info Panel"), kToggleColorInfo),
        HotkeyEntry(_("Toggle Color Controls Panel"), kToggleColorControls),
        HotkeyEntry(_("Toggle Playlist Panel"), kTogglePlaylist),
        HotkeyEntry(_("Toggle Compare Panel"), kToggleCompare),
        HotkeyEntry(_("Toggle Devices Panel"), kToggleDevices),
        HotkeyEntry(_("Toggle Annotation Panel"), kToggleAnnotation),
        HotkeyEntry(_("Toggle Settings Panel"), kToggleSettings),
        HotkeyEntry(_("Toggle Histogram Panel"), kToggleHistogram),
        HotkeyEntry(_("Toggle Vectorscope Panel"), kToggleVectorscope),
        HotkeyEntry(_("Toggle Waveform Panel"), kToggleWaveform),
        HotkeyEntry(_("Toggle Environment Map Panel"), kToggleEnvironmentMap),
        HotkeyEntry(_("Toggle Preferences Panel"), kTogglePreferences),
        HotkeyEntry(_("Toggle Hotkeys Panel"), kToggleHotkeys),
        HotkeyEntry(_("Toggle Python Panel"), kTogglePythonConsole),
        HotkeyEntry(_("Toggle Log Panel"), kToggleLogs),
        HotkeyEntry(_("Toggle About Panel"), kToggleAbout),
        HotkeyEntry(_("Toggle Hud"), kHudToggle),
        // HotkeyEntry( _("Select Single Image"), kSelectSingleImage ),
        // HotkeyEntry( _("Select Multi Image"), kSelectMultiImage ),
        // HotkeyEntry( _("Rotate Image +90 Degrees"), kRotatePlus90),
        // HotkeyEntry( _("Rotate Image -90 Degrees"), kRotateMinus90),
        HotkeyEntry("END", kGammaLess),
    };

    struct TableText table[] = {
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
    };

    std::string Hotkey::to_s() const
    {
        std::string r;
        if (ctrl)
            r += "Ctrl+";
        if (alt)
            r += "Alt+";
        if (meta)
            r += "Meta+";
        if (shift)
            r += "Shift+";

        unsigned k = key;
        if (k == 0 && (ctrl || alt || meta || shift))
            r = r.substr(0, r.size() - 1);

        bool special = false;
        for (unsigned j = 0; j < sizeof(table) / sizeof(TableText); ++j)
        {
            if (k == table[j].n)
            {
                r += table[j].text;
                special = true;
                break;
            }
        }

        if (!special)
        {
            if (k >= FL_F && k <= FL_F_Last)
            {
                char buf[16];
                snprintf(buf, 16, "F%d", k - FL_F);
                r += buf;
            }
            else
            {
                if (key != 0)
                    r += (char)key;
                if (key == 0 && !text.empty())
                    r += text;
            }
        }
        return r;
    }

    bool Hotkey::operator==(const Hotkey& b) const
    {
        const std::string& A = to_s();
        const std::string& B = b.to_s();
        if (A == B && !A.empty() && !B.empty())
            return true;
        return false;
    }

} // namespace mrv
