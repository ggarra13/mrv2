// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstring>

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
    Hotkey kOpenSingleImage(false, false, false, false, 0);
    Hotkey kOpenImage(true, false, false, false, 'o');
    Hotkey kOpenSeparateAudio(false, false, false, false, 0);
    Hotkey kOpenSession(false, false, true, false, 'o');

    Hotkey kSaveImage(false, false, false, false, 0);
    Hotkey kSaveImageToFolder(false, false, false, false, 0);
    Hotkey kSaveSequence(true, false, false, true, 's');
    Hotkey kSaveAudio(false, false, false, false, 0);
    Hotkey kSaveAnnotationsOnly(false, false, false, false, 0);
    Hotkey kSaveAnnotationsAsJson(false, false, false, false, 0);
    Hotkey kSaveOTIOEDL(false, false, false, false, 0);
    Hotkey kSavePDF(false, false, false, false, 0);
    Hotkey kSaveSession(true, false, false, false, 's');
    Hotkey kSaveSessionAs(false, false, false, false, 0);

    Hotkey kCloseCurrent(false, false, false, false, 0);
    Hotkey kCloseAll(false, false, false, false, 0);

    Hotkey kQuitProgram(false, false, false, false, FL_Escape);

    Hotkey kZoomMin(false, false, false, false, '0');
    Hotkey kZoomMax(false, false, false, false, '9');

    Hotkey kZoomIn(false, false, false, false, 0);
    Hotkey kZoomOut(false, false, false, false, 0);
    Hotkey kFullScreen(false, false, false, false, FL_F + 11);
    Hotkey kToggleFloatOnTop(false, true, false, false, 'w');
    Hotkey kToggleSecondary(false, false, false, false, 0);
    Hotkey kToggleSecondaryFloatOnTop(false, false, false, false, 0);

    Hotkey kToggleOnePanelOnly(false, false, false, false, 'p');

    Hotkey kToggleShowAnnotations(false, false, false, false, 0);

    Hotkey kToggleBackground(true, false, false, false, 'b');

    Hotkey kFitScreen(false, false, false, false, 'f');
    Hotkey kResizeMainWindow(false, false, false, true, 'w');
    Hotkey kFitAll(false, false, false, false, 0);
    Hotkey kMinifyTextureFiltering(false, false, false, false, 0);
    Hotkey kMagnifyTextureFiltering(false, false, false, true, 'f');
    Hotkey kFrameView(false, false, false, false, 0);
    Hotkey kSafeAreas(false, false, false, false, 's');
    Hotkey kIgnoreDisplayWindow(false, false, false, false, 0);
    Hotkey kIgnoreChromaticities(false, false, false, false, 0);
    Hotkey kAutoNormalize(false, false, false, false, 0);
    Hotkey kInvalidValues(false, false, false, false, 0);

    
    Hotkey kHDRDataFromFile(false, false, false, false, 0);
    Hotkey kHDRDataFalse(false, false, false, false, 0);
    Hotkey kHDRDataTrue(false, false, false, false, 0);
    
    Hotkey kToggleHDRTonemap(false, false, false, false, 0);

    Hotkey kDisplayWindow(true, false, false, false, 'd');
    Hotkey kDataWindow(false, false, false, false, 'd');

    Hotkey kCompareNone(false, false, false, false, 0);
    Hotkey kCompareWipe(false, false, true, false, 'w');
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
    Hotkey kLummaChannel(false, false, false, false, 'l');

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

    Hotkey kNextClip(true, false, false, true, FL_Right);
    Hotkey kPreviousClip(true, false, false, true, FL_Left);

    Hotkey kToggleMenuBar(false, false, false, true, FL_F + 1);
    Hotkey kToggleTopBar(false, false, false, false, FL_F + 1);
    Hotkey kToggleTimeline(false, false, false, false, FL_F + 2);
    Hotkey kTogglePixelBar(false, false, false, false, FL_F + 3);
    Hotkey kToggleStatusBar(false, false, false, false, 0);
    Hotkey kToggleToolBar(false, false, false, true, FL_F + 7);
    Hotkey kTogglePresentation(false, false, false, false, FL_F + 12);

    Hotkey kPreviousChannel(false, false, false, false, 0, "{");
    Hotkey kNextChannel(false, false, false, false, 0, "}");

    Hotkey kClearCache(false, false, false, false, 0);
    Hotkey kUpdateVideoFrame(false, false, false, true, 'u');

    Hotkey kDrawMode(false, false, false, true, 'd');
    Hotkey kEraseMode(false, false, false, true, 'e');
    Hotkey kScrubMode(false, false, false, true, 's');
    Hotkey kAreaMode(false, false, false, true, 0);
    Hotkey kPolygonMode(false, false, false, true, 'p');
    Hotkey kArrowMode(false, false, false, true, 'a');
    Hotkey kRectangleMode(false, false, false, true, 'r');
    Hotkey kCircleMode(false, false, false, true, 'c');
    Hotkey kTextMode(false, false, false, true, 't');

    Hotkey kPenSizeMore(false, false, false, false, 0, "+");
    Hotkey kPenSizeLess(false, false, false, false, 0, "-");

#ifdef __APPLE__
    Hotkey kUndoDraw(false, true, false, false, 'z');
    Hotkey kRedoDraw(false, true, false, true, 'z');
#else
    Hotkey kUndoDraw(true, false, false, false, 'z');
    Hotkey kRedoDraw(true, false, false, true, 'z');
#endif

    Hotkey kSwitchPenColor(false, false, false, false, 0);

    Hotkey kResetChanges(true, false, false, false, 'r');

    Hotkey kExposureMore(false, false, false, false, '.');
    Hotkey kExposureLess(false, false, false, false, ',');

    Hotkey kSaturationMore(false, false, false, false, 0, ">");
    Hotkey kSaturationLess(false, false, false, false, 0, "<");

    Hotkey kGammaMore(false, false, false, false, 0, ")");
    Hotkey kGammaLess(false, false, false, false, 0, "(");

    // Hotkey kCopyFrameXYValues(true, false, false, true, 'c');
    // Hotkey kCopyRGBAValues(true, false, true, false, 'c');

    Hotkey kSetInPoint(false, false, false, false, 'i');
    Hotkey kSetOutPoint(false, false, false, false, 'o');
    Hotkey kToggleOtioClipInOut(false, false, false, false, 'm');

    Hotkey kGridToggle(true, false, false, false, 'g');
    Hotkey kGridSize(true, true, false, false, 'g');

    Hotkey kHudToggle(true, false, false, false, 'h');

    Hotkey kOCIOPresetsToggle(false, false, false, false, 0);
    Hotkey kOCIOInTopBarToggle(false, false, false, false, 't');
    Hotkey kOCIOInputColorSpace(false, false, false, false, 0);
    Hotkey kOCIODisplay(false, false, false, false, 0);
    Hotkey kOCIOView(false, false, false, false, 0);
    Hotkey kOCIOToggle(false, false, true, false, 'p');

    Hotkey kToggleReel(false, false, false, false, FL_F + 4);
    Hotkey kToggleMediaInfo(false, false, false, false, FL_F + 5);
    Hotkey kToggleColorControls(false, false, false, false, FL_F + 6);
    Hotkey kToggleColorInfo(false, false, false, false, FL_F + 7);
    Hotkey kTogglePlaylist(true, false, false, false, 'p');
    Hotkey kToggleCompare(false, false, false, false, FL_F + 8);
    Hotkey kToggleDevices(false, false, false, false, 0);
    Hotkey kToggleAnnotation(true, false, false, false, 'a');
    Hotkey kToggleSettings(false, false, false, false, FL_F + 9);
    Hotkey kTogglePreferences(false, false, false, false, FL_F + 10);
    Hotkey kToggleHistogram(false, false, false, false, 0);
    Hotkey kToggleVectorscope(false, false, false, false, 0);
    Hotkey kToggleEnvironmentMap(true, false, false, false, 'e');
    Hotkey kToggleWaveform(false, false, false, false, 0);
    Hotkey kToggleHotkeys(false, false, false, false, 0);
    Hotkey kTogglePythonConsole(false, false, false, false, 0);
    Hotkey kToggleLogs(false, false, false, false, 0);
    Hotkey kToggleAbout(false, false, false, false, 0);
    Hotkey kToggleNDI(false, false, false, false, 0);
    Hotkey kToggleNetwork(false, false, false, false, 'n');
    Hotkey kToggleUSD(false, false, false, true, 'u');
    Hotkey kToggleStereo3D(false, false, false, false, 0);
    Hotkey kToggleEditMode(false, false, false, false, 'e');
    Hotkey kToggleTimelineThumbnails(false, false, false, false, 0);
    Hotkey kToggleTimelineTransitions(false, false, false, false, 0);
    Hotkey kToggleTimelineMarkers(false, false, false, false, 0);

    // Hotkey to allow mouse click handling to pass through the FLTK window to
    // the OS windows behind it.
    Hotkey kToggleClickThrough(true, false, false, false, 't');

    Hotkey kUITransparencyMore(true, false, false, false, '.');
    Hotkey kUITransparencyLess(true, false, false, false, ',');

    Hotkey kToggleTimelineEditable(false, false, false, false, 0);
    Hotkey kToggleEditAssociatedClips(false, false, false, false, 0);
    Hotkey kToggleTimelineFrameView(false, false, false, false, 0);
    Hotkey kToggleTimelineScrollToCurrentFrame(false, false, false, false, 0);
    Hotkey kToggleTimelineTrackInfo(false, false, false, false, 0);
    Hotkey kToggleTimelineClipInfo(false, false, false, false, 0);

    Hotkey kEditCutFrame(true, false, false, false, 'x');
    Hotkey kEditCopyFrame(true, false, false, false, 'c');
    Hotkey kEditPasteFrame(true, false, false, false, 'v');
    Hotkey kEditInsertFrame(true, false, false, false, 'i');

    Hotkey kEditSliceClip(false, false, false, false, 0);
    Hotkey kEditRemoveClip(false, false, false, false, 0);
    Hotkey kEditInsertAudioClip(false, false, false, false, 0);
    Hotkey kEditRemoveAudioClip(false, false, false, false, 0);
    Hotkey kEditInsertAudioGap(false, false, false, false, 0);
    Hotkey kEditRemoveAudioGap(false, false, false, false, 0);

    Hotkey kEditUndo(false, false, false, false, ';');
    Hotkey kEditRedo(false, false, false, false, ':');

    Hotkey kRotatePlus90(false, false, false, false, '+');
    Hotkey kRotateMinus90(false, false, false, false, '-');

    Hotkey kVideoLevelsFile(false, false, false, false, 0);
    Hotkey kVideoLevelsLegalRange(false, false, false, false, 0);
    Hotkey kVideoLevelsFullRange(false, false, false, false, 0);

    Hotkey kAlphaBlendNone(false, false, false, false, 0);
    Hotkey kAlphaBlendStraight(false, false, false, false, 0);
    Hotkey kAlphaBlendPremultiplied(false, false, false, false, 0);

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

    HotkeyEntry* kDefaultHotkeys = nullptr;

    HotkeyEntry hotkeys[] = {

        HotkeyEntry(_("Open Directory"), &kOpenDirectory),
        HotkeyEntry(_("Open Movie or Sequence"), &kOpenImage),
        HotkeyEntry(_("Open Single Image"), &kOpenSingleImage),
        HotkeyEntry(_("Open Session"), &kOpenSession),
        HotkeyEntry(_("Save Image"), &kSaveImage),
        HotkeyEntry(_("Save Audio"), &kSaveAudio),
        HotkeyEntry(_("Save Frames To Folder"), &kSaveImageToFolder),

        HotkeyEntry(_("Save Movie or Sequence"), &kSaveSequence),
        HotkeyEntry(_("Save OTIO Timeline"), &kSaveOTIOEDL),
        HotkeyEntry(_("Save Annotations Only"), &kSaveAnnotationsOnly),
        HotkeyEntry(_("Save Annotations as JSON"), &kSaveAnnotationsAsJson),
        HotkeyEntry(_("Save PDF Document"), &kSavePDF),
        HotkeyEntry(_("Save Session"), &kSaveSession),
        HotkeyEntry(_("Save Session As"), &kSaveSessionAs),

        HotkeyEntry(_("Close Current"), &kCloseCurrent),
        HotkeyEntry(_("Close All"), &kCloseAll),

        HotkeyEntry(_("Quit Program"), &kQuitProgram),

        HotkeyEntry(_("Zoom Minimum"), &kZoomMin),
        HotkeyEntry(_("Zoom Maximum"), &kZoomMax),

        HotkeyEntry(_("Center Image"), &kCenterImage),
        HotkeyEntry(_("Fit Screen"), &kFitScreen),
        HotkeyEntry(_("Resize Main Window to Fit"), &kResizeMainWindow),
        HotkeyEntry(_("Fit All"), &kFitAll),
        HotkeyEntry(
            _("Toggle Minify Texture Filtering"), &kMinifyTextureFiltering),
        HotkeyEntry(
            _("Toggle Magnify Texture Filtering"), &kMagnifyTextureFiltering),
        HotkeyEntry(_("Auto Frame View"), &kFrameView),
        HotkeyEntry(_("Toggle Click Through"), &kToggleClickThrough),
        HotkeyEntry(_("More UI Transparency"), &kUITransparencyMore),
        HotkeyEntry(_("Less UI Transparency"), &kUITransparencyLess),
        HotkeyEntry(_("Safe Areas"), &kSafeAreas),
        HotkeyEntry(_("Ignore Display Window"), &kIgnoreDisplayWindow),
        HotkeyEntry(_("Ignore Chromaticities"), &kIgnoreChromaticities),
        HotkeyEntry(_("Auto Normalize"), &kAutoNormalize),
        HotkeyEntry(_("Invalid Values"), &kInvalidValues),
        HotkeyEntry(_("HDR Data From File"), &kHDRDataFromFile),
        HotkeyEntry(_("HDR Data Inactive"), &kHDRDataFalse),
        HotkeyEntry(_("HDR Data Active"), &kHDRDataTrue),
        HotkeyEntry(_("Toggle HDR tonemap"), &kToggleHDRTonemap),
        HotkeyEntry(_("Display Window"), &kDisplayWindow),
        HotkeyEntry(_("Data Window"), &kDataWindow),
        HotkeyEntry(_("Compare None"), &kCompareNone),
        HotkeyEntry(_("Compare Wipe"), &kCompareWipe),
        HotkeyEntry(_("Compare Overlay"), &kCompareOverlay),
        HotkeyEntry(_("Compare Difference"), &kCompareDifference),
        HotkeyEntry(_("Compare Horizontal"), &kCompareHorizontal),
        HotkeyEntry(_("Compare Vertical"), &kCompareVertical),
        HotkeyEntry(_("Compare Tile"), &kCompareTile),
        HotkeyEntry(_("Color Channel"), &kColorChannel),
        HotkeyEntry(_("Red Channel"), &kRedChannel),
        HotkeyEntry(_("Green Channel"), &kGreenChannel),
        HotkeyEntry(_("Blue Channel"), &kBlueChannel),
        HotkeyEntry(_("Alpha Channel"), &kAlphaChannel),
        HotkeyEntry(_("Lumma Channel"), &kLummaChannel),
        HotkeyEntry(_("Flip X"), &kFlipX),
        HotkeyEntry(_("Flip Y"), &kFlipY),

        HotkeyEntry(_("Rotate Images +90 Degrees"), &kRotatePlus90),
        HotkeyEntry(_("Rotate Images -90 Degrees"), &kRotateMinus90),

        HotkeyEntry(_("Video Levels from File"), &kVideoLevelsFile),
        HotkeyEntry(_("Video Levels Legal Range"), &kVideoLevelsLegalRange),
        HotkeyEntry(_("Video Levels Full Range"), &kVideoLevelsFullRange),

        HotkeyEntry(_("Alpha Blend None"), &kAlphaBlendNone),
        HotkeyEntry(_("Alpha Blend Straight"), &kAlphaBlendStraight),
        HotkeyEntry(_("Alpha Blend Premultiplied"), &kAlphaBlendPremultiplied),

        HotkeyEntry(_("Annotation Clear Frame"), &kShapeFrameClear),
        HotkeyEntry(_("Annotation Clear All Frames"), &kShapeFrameClearAll),
        HotkeyEntry(_("Annotation Frame Step Backwards"), &kShapeFrameStepBack),
        HotkeyEntry(_("Toggle Show Annotations"), &kToggleShowAnnotations),

        HotkeyEntry(_("Frame Step Backwards"), &kFrameStepBack),
        HotkeyEntry(_("Frame Step FPS Backwards"), &kFrameStepFPSBack),
        HotkeyEntry(_("Annotation Frame Step Forwards"), &kShapeFrameStepFwd),
        HotkeyEntry(_("Frame Step Forwards"), &kFrameStepFwd),
        HotkeyEntry(_("Frame Step FPS Forwards"), &kFrameStepFPSFwd),
        HotkeyEntry(_("Play Backwards"), &kPlayBack),
        HotkeyEntry(_("Play Backwards / Change Speed"), &kPlayBackHalfSpeed),
        HotkeyEntry(_("Play in Current Direction"), &kPlayDirection),
        HotkeyEntry(_("Play Forwards"), &kPlayFwd),
        HotkeyEntry(_("Play Forwards / Change Speed"), &kPlayFwdTwiceSpeed),
        HotkeyEntry(_("Stop"), &kStop),
        HotkeyEntry(_("First Frame"), &kFirstFrame),
        HotkeyEntry(_("Last Frame"), &kLastFrame),

        HotkeyEntry(_("Next Clip"), &kNextClip),
        HotkeyEntry(_("Previous Clip"), &kPreviousClip),

        HotkeyEntry(_("Loop Playback"), &kPlaybackLoop),
        HotkeyEntry(_("Playback Once"), &kPlaybackOnce),
        HotkeyEntry(_("Playback Ping Pong"), &kPlaybackPingPong),

        HotkeyEntry(_("First Image Version"), &kFirstVersionImage),
        HotkeyEntry(_("Previous Image Version"), &kPreviousVersionImage),
        HotkeyEntry(_("Next Image Version"), &kNextVersionImage),
        HotkeyEntry(_("Last Image Version"), &kLastVersionImage),

        HotkeyEntry(_("Previous Image"), &kPreviousImage),
        HotkeyEntry(_("Next Image"), &kNextImage),

        HotkeyEntry(_("Previous Image Limited"), &kPreviousImageLimited),
        HotkeyEntry(_("Next Image Limited"), &kNextImageLimited),

        HotkeyEntry(_("Previous Channel"), &kPreviousChannel),
        HotkeyEntry(_("Next Channel"), &kNextChannel),

        HotkeyEntry(_("Clear Cache"), &kClearCache),
        HotkeyEntry(_("Update Video Frame"), &kUpdateVideoFrame),

        HotkeyEntry(_("Cut Frame"), &kEditCutFrame),
        HotkeyEntry(_("Copy Frame"), &kEditCopyFrame),
        HotkeyEntry(_("Paste Frame"), &kEditPasteFrame),
        HotkeyEntry(_("Insert Frame"), &kEditInsertFrame),

        HotkeyEntry(_("Slice Clip"), &kEditSliceClip),
        HotkeyEntry(_("Remove Clip"), &kEditRemoveClip),
        HotkeyEntry(_("Insert Audio Clip"), &kEditInsertAudioClip),
        HotkeyEntry(_("Remove Audio Clip"), &kEditRemoveAudioClip),
        HotkeyEntry(_("Insert Audio Gap"), &kEditInsertAudioGap),
        HotkeyEntry(_("Remove Audio Gap"), &kEditRemoveAudioGap),

        HotkeyEntry(_("Edit Undo"), &kEditUndo),
        HotkeyEntry(_("Edit Redo"), &kEditRedo),

        HotkeyEntry(_("Toggle Menu Bar"), &kToggleMenuBar),
        HotkeyEntry(_("Toggle Top Bar"), &kToggleTopBar),
        HotkeyEntry(_("Toggle Pixel Bar"), &kTogglePixelBar),
        HotkeyEntry(_("Toggle Timeline"), &kToggleTimeline),
        HotkeyEntry(_("Toggle Status Bar"), &kToggleStatusBar),
        HotkeyEntry(_("Toggle Tool Dock"), &kToggleToolBar),
        HotkeyEntry(_("Toggle Full Screen"), &kFullScreen),
        HotkeyEntry(_("Toggle Presentation"), &kTogglePresentation),
        HotkeyEntry(_("Toggle Float On Top"), &kToggleFloatOnTop),
        HotkeyEntry(_("Toggle Secondary"), &kToggleSecondary),
        HotkeyEntry(
            _("Toggle Secondary Float On Top"), &kToggleSecondaryFloatOnTop),
        HotkeyEntry(_("Toggle NDI"), &kToggleNDI),
        HotkeyEntry(_("Toggle Network"), &kToggleNetwork),
        HotkeyEntry(_("Toggle USD"), &kToggleUSD),
        HotkeyEntry(_("Toggle Stereo 3D"), &kToggleStereo3D),
        HotkeyEntry(_("Toggle Edit Mode"), &kToggleEditMode),
        HotkeyEntry(_("Toggle Timeline Editable"), &kToggleTimelineEditable),
        HotkeyEntry(
            _("Toggle Edit Associated Clips"), &kToggleEditAssociatedClips),
        HotkeyEntry(_("Timeline Frame View"), &kToggleTimelineFrameView),
        HotkeyEntry(
            _("Toggle Timeline Scroll To Current Frame"),
            &kToggleTimelineScrollToCurrentFrame),
        HotkeyEntry(_("Toggle Timeline Track Info"), &kToggleTimelineTrackInfo),
        HotkeyEntry(_("Toggle Timeline Clip Info"), &kToggleTimelineClipInfo),
        HotkeyEntry(
            _("Toggle Timeline Thumbnails"), &kToggleTimelineThumbnails),
        HotkeyEntry(
            _("Toggle Timeline Transitions"), &kToggleTimelineTransitions),
        HotkeyEntry(_("Toggle Timeline Markers"), &kToggleTimelineMarkers),

        HotkeyEntry(_("Reset Gain/Gamma"), &kResetChanges),
        HotkeyEntry(_("Exposure More"), &kExposureMore),
        HotkeyEntry(_("Exposure Less"), &kExposureLess),
        HotkeyEntry(_("Saturation More"), &kSaturationMore),
        HotkeyEntry(_("Saturation Less"), &kSaturationLess),
        HotkeyEntry(_("Gamma More"), &kGammaMore),
        HotkeyEntry(_("Gamma Less"), &kGammaLess),

        HotkeyEntry(_("OCIO Presets"), &kOCIOPresetsToggle),
        HotkeyEntry(_("OCIO In Top Bar"), &kOCIOInTopBarToggle),
        HotkeyEntry(_("OCIO Input Color Space"), &kOCIOInputColorSpace),
        HotkeyEntry(_("OCIO Display"), &kOCIODisplay),
        HotkeyEntry(_("OCIO View"), &kOCIOView),
        HotkeyEntry(_("OCIO Toggle"), &kOCIOToggle),

        HotkeyEntry(_("Scrub Mode"), &kScrubMode),
        HotkeyEntry(_("Area Selection Mode"), &kAreaMode),
        HotkeyEntry(_("Draw Mode"), &kDrawMode),
        HotkeyEntry(_("Erase Mode"), &kEraseMode),
        HotkeyEntry(_("Polygon Mode"), &kPolygonMode),
        HotkeyEntry(_("Arrow Mode"), &kArrowMode),
        HotkeyEntry(_("Rectangle Mode"), &kRectangleMode),
        HotkeyEntry(_("Circle Mode"), &kCircleMode),
        HotkeyEntry(_("Text Mode"), &kTextMode),
        HotkeyEntry(_("Pen Size More"), &kPenSizeMore),
        HotkeyEntry(_("Pen Size Less"), &kPenSizeLess),
        HotkeyEntry(_("Undo Draw"), &kUndoDraw),
        HotkeyEntry(_("Redo Draw"), &kRedoDraw),
        HotkeyEntry(_("Switch Pen Color"), &kSwitchPenColor),

        HotkeyEntry(_("Set In Point"), &kSetInPoint),
        HotkeyEntry(_("Set Out Point"), &kSetOutPoint),

        HotkeyEntry(_("Hud Window"), &kHudToggle),

        HotkeyEntry(_("Toggle One Panel Only"), &kToggleOnePanelOnly),
        HotkeyEntry(_("Toggle Files Panel"), &kToggleReel),
        HotkeyEntry(_("Toggle Media Info Panel"), &kToggleMediaInfo),
        HotkeyEntry(_("Toggle Color Area Info Panel"), &kToggleColorInfo),
        HotkeyEntry(_("Toggle Color Controls Panel"), &kToggleColorControls),
        HotkeyEntry(_("Toggle Playlist Panel"), &kTogglePlaylist, true),
        HotkeyEntry(_("Toggle Compare Panel"), &kToggleCompare),
        HotkeyEntry(_("Toggle Devices Panel"), &kToggleDevices),
        HotkeyEntry(_("Toggle Annotation Panel"), &kToggleAnnotation),
        HotkeyEntry(_("Toggle Background Panel"), &kToggleBackground),
        HotkeyEntry(_("Toggle Settings Panel"), &kToggleSettings),
        HotkeyEntry(_("Toggle Histogram Panel"), &kToggleHistogram),
        HotkeyEntry(_("Toggle Vectorscope Panel"), &kToggleVectorscope),
        HotkeyEntry(_("Toggle Waveform Panel"), &kToggleWaveform),
        HotkeyEntry(
            _("Toggle Environment Map Panel"), &kToggleEnvironmentMap, true),
        HotkeyEntry(_("Toggle Preferences Window"), &kTogglePreferences),
        HotkeyEntry(_("Toggle Python Panel"), &kTogglePythonConsole),
        HotkeyEntry(_("Toggle Log Panel"), &kToggleLogs),

        HotkeyEntry(_("Toggle Hotkeys Window"), &kToggleHotkeys),
        HotkeyEntry(_("Toggle About Window"), &kToggleAbout),
        HotkeyEntry("END", nullptr),
    };

    std::string Hotkey::to_s() const
    {
        std::string r;
        if (ctrl)
            r += "<Ctrl>";
        if (alt)
            r += "<Alt>";
        if (meta)
            r += "<Meta>";
        if (shift)
            r += "<Shift>";

        unsigned k = key;
        bool special = false;
        for (unsigned j = 0; j < sizeof(table) / sizeof(TableText); ++j)
        {
            if (k == table[j].n)
            {
                r += _(table[j].text);
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

    void store_default_hotkeys()
    {
        size_t numHotkeys = sizeof(hotkeys) / sizeof(HotkeyEntry);
        kDefaultHotkeys = new HotkeyEntry[numHotkeys];
        for (size_t i = 0; i < numHotkeys; ++i)
        {
            kDefaultHotkeys[i] = hotkeys[i];
            if (hotkeys[i].hotkey)
                kDefaultHotkeys[i].hotkey = new Hotkey(*hotkeys[i].hotkey);
        }
    }

    void reset_hotkeys()
    {
        size_t numHotkeys = sizeof(hotkeys) / sizeof(HotkeyEntry);
        for (size_t i = 0; i < numHotkeys; ++i)
        {
            if (kDefaultHotkeys[i].hotkey && hotkeys[i].hotkey)
            {
                hotkeys[i].hotkey->ctrl = kDefaultHotkeys[i].hotkey->ctrl;
                hotkeys[i].hotkey->meta = kDefaultHotkeys[i].hotkey->meta;
                hotkeys[i].hotkey->alt = kDefaultHotkeys[i].hotkey->alt;
                hotkeys[i].hotkey->shift = kDefaultHotkeys[i].hotkey->shift;
                hotkeys[i].hotkey->key = kDefaultHotkeys[i].hotkey->key;
                hotkeys[i].hotkey->text = kDefaultHotkeys[i].hotkey->text;
            }
        }
    }
} // namespace mrv
