/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"

#include <FL/Fl.H>


namespace mrv {
// ctrl, meta, alt, shift, key
Hotkey kOpenDirectory( true, false, false, true, 'o' );
Hotkey kOpenImage( true, false, false, false, 'o' );
Hotkey kOpenSingleImage( true, true, false, false, 'o' );
Hotkey kOpenStereoImage( false, false, true, false, 'o' );
Hotkey kOpenAMF( false, false, false, true, 'o' );
Hotkey kOpenClipXMLMetadata( true, false, false, false, 'x' );
Hotkey kOpenSession( false, true, false, false, 'o' );
Hotkey kSaveReel( true, false, false, false, 'r' );
Hotkey kSaveImage( true, false, false, false, 's' );
Hotkey kSaveSequence( true, false, false, true, 's' );
Hotkey kSaveSnapshot( false, false, true, false, 's' );
Hotkey kSaveClipXMLMetadata( false, false, true, false, 'x' );
Hotkey kIccProfile( true, false, false, false, 'i' );
Hotkey kIDTScript( true, false, false, true, 'i' );
Hotkey kLookModScript( true, false, false, false, 'l' );
Hotkey kCTLScript( true, false, false, false, 't' );
Hotkey kMonitorCTLScript( true, false, false, false, 'm' );
Hotkey kMonitorIccProfile( true, false, false, false, 'n' );
Hotkey kSaveSession( false, true, false, false, 's' );

Hotkey kQuitProgram( false, false, false, false, FL_Escape );

Hotkey kZoomMin( false, false, false, false, '0' );
Hotkey kZoomMax( false, false, false, false, '9' );

Hotkey kZoomIn( false, false, false, false, '.' );
Hotkey kZoomOut( false, false, false, false, ',' );
Hotkey kFullScreen( false, false, false, false, FL_F + 11 );
Hotkey kFitScreen( false, false, false, false, 'f' );
Hotkey kResizeMainWindow( false, false, false, true, 'w' );
Hotkey kFitAll( false, false, false, false, 'a' );
Hotkey kTextureFiltering( false, false, false, true, 'f' );
Hotkey kSafeAreas( false, false, false, false, 's' );
Hotkey kDisplayWindow( false, false, false, false, 'd' );
Hotkey kDataWindow( true, false, false, false, 'd' );
Hotkey kWipe( false, false, false, false, 'w' );
Hotkey kFlipX( false, false, false, false, 'x' );
Hotkey kFlipY( false, false, false, false, 'y' );
Hotkey kCenterImage( false, false, false, false, 'h' );

Hotkey kShapeFrameStepBack( false, false, false, true, FL_Left, "",
                            FL_KP + 4 );
Hotkey kFrameStepBack( false, false, false, false, FL_Left, "",
                       FL_KP + 4 );
Hotkey kFrameStepFPSBack( true, false, false, false, FL_Left, "",
                          FL_KP + 4 );
Hotkey kFrameStepFwd( false, false, false, false, FL_Right, "",
                      FL_KP + 6 );
Hotkey kShapeFrameStepFwd( false, false, false, true, FL_Right, "",
                           FL_KP + 6 );
Hotkey kFrameStepFPSFwd( true, false, false, false, FL_Right, "",
                         FL_KP + 6 );
Hotkey kPlayBackHalfSpeed( false, false, false, false, 'j' );
Hotkey kPlayBack( false, false, false, false, FL_Up, "", FL_KP + 8 );
Hotkey kPlayDirection( false, false, false, false, ' ' );
Hotkey kPlayFwd( false, false, false, false, FL_Down, "", FL_KP + 2 );
Hotkey kPlayFwdTwiceSpeed( false, false, false, false, 'k' );
Hotkey kStop( false, false, false, false, FL_Enter );

Hotkey kSwitchFGBG( true, false, false, false, 'j' );

Hotkey kFirstVersionImage( false, false, true, true, FL_Page_Up );
Hotkey kPreviousVersionImage( false, false, true, false, FL_Page_Up );
Hotkey kNextVersionImage( false, false, true, false, FL_Page_Down );
Hotkey kLastVersionImage( false, false, true, true, FL_Page_Down );

Hotkey kPreviousImage( false, false, false, false, FL_Page_Up );
Hotkey kNextImage( false, false, false, false, FL_Page_Down );

Hotkey kPreviousImageLimited( true, false, false, false, FL_Page_Up );
Hotkey kNextImageLimited( true, false, false, false, FL_Page_Down );

Hotkey kFirstFrame( false, false, false, false, FL_Home );
Hotkey kLastFrame( false, false, false, false, FL_End );
Hotkey kToggleBG( false, false, false, false, FL_Tab );
Hotkey kToggleEDL( false, false, false, false, FL_KP_Enter );

Hotkey kToggleMenuBar( false, false, false, true, FL_F + 1 );
Hotkey kToggleTopBar( false, false, false, false, FL_F + 1 );
Hotkey kTogglePixelBar( false, false, false, false, FL_F + 2 );
Hotkey kToggleTimeline( false, false, false, false, FL_F + 3 );
Hotkey kToggleToolBar( false, false, false, true, FL_F + 7 );
Hotkey kTogglePresentation( false, false, false, false, FL_F + 12 );

Hotkey kSwitchChannels( false, false, false, false, 'e' );
Hotkey kPreviousChannel( false, false, false, false, 0, "{" );
Hotkey kNextChannel( false, false, false, false, 0, "}" );

Hotkey kDrawTemporaryMode( false, false, false, false, 0 );
Hotkey kDrawMode( false, false, false, true, 'd' );
Hotkey kEraseTemporaryMode( false, false, false, false, 0 );
Hotkey kEraseMode( false, false, false, true, 'e' );
Hotkey kScrubMode( false, false, false, true, 's' );
Hotkey kAreaMode( false, false, false, true, 0 );
Hotkey kArrowMode( false, false, false, true, 'a' );
Hotkey kRectangleMode( false, false, false, true, 'r' );;
Hotkey kCircleMode;
Hotkey kRotateCanvasMode( false, false, false, false, 'r' );
Hotkey kTextMode( false, false, false, true, 't' );
Hotkey kMoveSizeMode( false, false, false, true, 'm' );

Hotkey kPenSizeMore( false, false, false, false, 0, "]" );
Hotkey kPenSizeLess( false, false, false, false, 0, "[" );

Hotkey kUndoDraw( false, false, false, false, '<' );
Hotkey kRedoDraw( false, false, false, false, '>' );

Hotkey kResetChanges( false, false, false, true, 'c' );
Hotkey kExposureMore( false, false, false, false, 0, "]" );
Hotkey kExposureLess( false, false, false, false, 0, "[" );
Hotkey kGammaMore( false, false, false, false, 0, ")" );
Hotkey kGammaLess( false, false, false, false, 0, "(" );

Hotkey kSetAsBG( false, false, false, false, 0 );
Hotkey kSelectSingleImage( false, false, false, false, 0 );
Hotkey kSelectMultiImage( false, false, false, false, 0 );

Hotkey kAddIPTCMetadata( false, false, false, false, 0 );
Hotkey kRemoveIPTCMetadata( false, false, false, false, 0 );

Hotkey kZDepthUp( false, false, false, false, 's' );
Hotkey kZDepthDown( false, false, false, false, 'a' );

Hotkey kDensityUp( false, false, false, false, 'c' );
Hotkey kDensityDown( false, false, false, false, 'd' );

Hotkey kSOPSatNodes( false, false, false, false, 0 );

Hotkey kAttachAudio( false, false, false, false, 0 );
Hotkey kEditAudio( false, false, false, false, 0 );
Hotkey kDetachAudio( false, false, false, false, 0 );
Hotkey kCopyFrameXYValues( true, false, false, true, 'c' );
Hotkey kCopyRGBAValues( true, false, false, false, 'c' );
Hotkey kCloneImage( false, false, false, false, 0 );

Hotkey kPreloadCache( false, false, false, false, 'p' );
Hotkey kClearCache( false, false, false, true, 'k' );
Hotkey kClearSingleFrameCache( false, false, false, false, 'u' );

Hotkey kSetInPoint( false, false, false, false, 'i' );
Hotkey kSetOutPoint( false, false, false, false, 'o' );

Hotkey kGridToggle( true, false, false, false, 'g' );
Hotkey kGridSize( true, true, false, false, 'g' );
Hotkey kHudToggle( true, false, false, false, 'h' );

Hotkey kOCIOInputColorSpace( false, false, false, false, 0 );
Hotkey kOCIODisplay( false, false, false, false, 0 );
Hotkey kOCIOView( false, false, false, false, 0 );

Hotkey kToggleReel( false, false, false, false, FL_F + 4 );
Hotkey kToggleMediaInfo( false, false, false, false, FL_F + 5 );
Hotkey kToggleColorInfo( false, false, false, false, FL_F + 6 );
Hotkey kToggleColorControls( false, false, false, false, 0 );
Hotkey kToggleAction( false, false, false, false, FL_F + 7 );
Hotkey kToggleStereoOptions( false, false, false, false, FL_F + 8 );
Hotkey kTogglePreferences( false, false, false, false, FL_F + 9 );
Hotkey kToggleEDLEdit( false, false, false, false, 0 );
Hotkey kToggle3dView( false, false, false, false, 0 );
Hotkey kToggleHistogram( false, false, false, false, 0 );
Hotkey kToggleVectorscope( false, false, false, false, 0 );
Hotkey kToggleWaveform( false, false, false, false, 0 );
Hotkey kToggleICCProfiles( false, false, false, false, 0 );
Hotkey kToggleConnections( false, false, false, false, 0 );
Hotkey kToggleHotkeys( false, false, false, false, 0 );
Hotkey kToggleLogs( false, false, false, false, FL_F + 10 );
Hotkey kToggleAbout( false, false, false, false, 0 );

Hotkey kRotatePlus10( false, false, false, false, '+' );
Hotkey kRotateMinus10( false, false, false, false, '-' );

    Hotkey kRotatePlus90; //( false, false, false, false, '+' );
    Hotkey kRotateMinus90; //( false, false, false, false, '-' );

Hotkey kTogglePixelRatio( true, false, false, false, 'p' );
Hotkey kToggleLut( false, false, false, false, 't' );
Hotkey kToggleICS( false, false, false, true, 'i' );

bool Hotkey::match( unsigned rawkey )
{
    bool ok = false;

    const char* t = Fl::event_text();
    if ( ( !ctrl && !shift && !alt && !meta ) &&
         ( ! Fl::event_state( FL_CTRL ) ) &&
         ( ! Fl::event_state( FL_ALT ) ) &&
         ( ! Fl::event_state( FL_META ) ) &&
         ( (key && (int)key == t[0]) || ( text.size() && text == t ) ) )
    {
        return true;
    }

    if ( ctrl )
    {
        if ( Fl::event_state( FL_CTRL ) )
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
        if ( Fl::event_state( FL_CTRL ) )
            return false;
    }

    if ( shift )
    {
        if ( Fl::event_state( FL_SHIFT ) )
            ok = true;
        else
            return false;
    }
    else
    {
        if ( Fl::event_state( FL_SHIFT ) )
            return false;
    }
    if ( alt )
    {
        if ( Fl::event_state( FL_ALT ) )
            ok = true;
        else
            return false;
    }
    else
    {
        if ( Fl::event_state( FL_ALT ) )
            return false;
    }
    if ( meta )
    {
        if ( Fl::event_state( FL_META ) )
            ok = true;
        else
            return false;
    }
    else
    {
        if ( Fl::event_state( FL_META ) )
            return false;
    }

    if ( rawkey != 0 && (key != 0 || key2 != 0) )
    {
        if  ( rawkey == key || rawkey == key2 )
        {
            ok = true;
        }
        else if ( (!text.empty()) && text == t )
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
    HotkeyEntry( _("3dView Z Depth Up"), kZDepthUp),
    HotkeyEntry( _("3dView Z Depth Down"), kZDepthDown),
    HotkeyEntry( _("3dView Density Up"), kDensityUp),
    HotkeyEntry( _("3dView Density Down"), kDensityDown),
    HotkeyEntry( _("Open Directory"), kOpenDirectory),
    HotkeyEntry( _("Open Movie or Sequence"), kOpenImage),
    HotkeyEntry( _("Open Single Image"), kOpenSingleImage),
    HotkeyEntry( _("Open Aces Metadata File (AMF)"), kOpenAMF, true),
    HotkeyEntry( _("Open Clip XML Metadata"), kOpenClipXMLMetadata),
    HotkeyEntry( _("Open Session"), kOpenSession, true),
    HotkeyEntry( _("Save Reel"), kSaveReel),
    HotkeyEntry( _("Save Image"), kSaveImage),
    HotkeyEntry( _("Save GL Snapshot"), kSaveSnapshot),
    HotkeyEntry( _("Save Sequence"), kSaveSequence),
    HotkeyEntry( _("Save Session"), kSaveSession, true),
    HotkeyEntry( _("Quit Program"), kQuitProgram, true),
    HotkeyEntry( _("Save Clip XML Metadata"), kSaveClipXMLMetadata),
    HotkeyEntry( _("Image Icc Profile"), kIccProfile ),
    HotkeyEntry( _("Image CTL script"), kCTLScript ),
    HotkeyEntry( _("Monitor Icc Profile"), kMonitorIccProfile ),
    HotkeyEntry( _("Monitor CTL script"), kMonitorCTLScript ),
//HotkeyEntry( _("Zoom Minimum"), kZoomMin),
//HotkeyEntry( _("Zoom Maximum"), kZoomMax),
    HotkeyEntry( _("Center Image"), kCenterImage ),
    HotkeyEntry( _("Fit Screen"), kFitScreen),
    HotkeyEntry( _("Resize Main Window to Fit"), kResizeMainWindow, true),
    HotkeyEntry( _("Fit All"), kFitAll),
    HotkeyEntry( _("TextureFiltering"), kTextureFiltering),
    HotkeyEntry( _("Safe Areas"), kSafeAreas),
    HotkeyEntry( _("Display Window"), kDisplayWindow),
    HotkeyEntry( _("Data Window"), kDataWindow),
    HotkeyEntry( _("Wipe"), kWipe),
    HotkeyEntry( _("Flip X"), kFlipX),
    HotkeyEntry( _("Flip Y"), kFlipY),
    HotkeyEntry( _("Annotation Frame Step Backwards"), kShapeFrameStepBack,
                 true),
    HotkeyEntry( _("Frame Step Backwards"), kFrameStepBack),
    HotkeyEntry( _("Frame Step FPS Backwards"), kFrameStepFPSBack),
    HotkeyEntry( _("Annotation Frame Step Forwards"), kShapeFrameStepFwd, true),
    HotkeyEntry( _("Frame Step Forwards"), kFrameStepFwd),
    HotkeyEntry( _("Frame Step FPS Forwards"), kFrameStepFPSFwd),
    HotkeyEntry( _("Play Backwards"), kPlayBack),
    HotkeyEntry( _("Play Backwards / Change Speed"), kPlayBackHalfSpeed),
    HotkeyEntry( _("Play in Current Direction"), kPlayDirection),
    HotkeyEntry( _("Play Forwards"), kPlayFwd),
    HotkeyEntry( _("Play Forwards / Change Speed"), kPlayFwdTwiceSpeed),
    HotkeyEntry( _("Preload Image Cache"), kPreloadCache),
    HotkeyEntry( _("Clear Image Cache"), kClearCache),
    HotkeyEntry( _("Update Frame in Cache"), kClearSingleFrameCache),
    HotkeyEntry( _("Stop"), kStop),
    HotkeyEntry( _("First Image Version"), kFirstVersionImage, true ),
    HotkeyEntry( _("Previous Image Version"), kPreviousVersionImage ),
    HotkeyEntry( _("Next Image Version"), kNextVersionImage ),
    HotkeyEntry( _("Last Image Version"), kLastVersionImage, true ),
    HotkeyEntry( _("Previous Image"), kPreviousImage ),
    HotkeyEntry( _("Next Image"), kNextImage ),
    HotkeyEntry( _("Previous Image Limited"), kPreviousImageLimited ),
    HotkeyEntry( _("Next Image Limited"), kNextImageLimited ),
    HotkeyEntry( _("Switch Channels"), kSwitchChannels ),
    HotkeyEntry( _("Previous Channel"), kPreviousChannel ),
    HotkeyEntry( _("Next Channel"), kNextChannel ),
    HotkeyEntry( _("First Frame"), kFirstFrame ),
    HotkeyEntry( _("Last Frame"), kLastFrame ),
    HotkeyEntry( _("Toggle Background Composite"), kToggleBG ),
    HotkeyEntry( _("Toggle EDL"), kToggleEDL ),
    HotkeyEntry( _("Toggle Menu Bar"), kToggleMenuBar ),
    HotkeyEntry( _("Toggle Top Bar"), kToggleTopBar ),
    HotkeyEntry( _("Toggle Pixel Bar"), kTogglePixelBar ),
    HotkeyEntry( _("Toggle Bottom Bar"), kToggleTimeline ),
    HotkeyEntry( _("Toggle Tool Dock"), kToggleToolBar ),
    HotkeyEntry( _("Toggle Full Screen"), kFullScreen),
    HotkeyEntry( _("Toggle Presentation"), kTogglePresentation ),
    HotkeyEntry( _("Reset Gain/Gamma"), kResetChanges),
    HotkeyEntry( _("Exposure More"), kExposureMore),
    HotkeyEntry( _("Exposure Less"), kExposureLess),
    HotkeyEntry( _("OCIO Input Color Space"), kOCIOInputColorSpace ),
    HotkeyEntry( _("OCIO Display"), kOCIODisplay ),
    HotkeyEntry( _("OCIO View"), kOCIOView ),
    HotkeyEntry( _("Scrub Mode"), kScrubMode ),
    HotkeyEntry( _("Area Selection Mode"), kAreaMode ),
    HotkeyEntry( _("Draw Mode"), kDrawMode ),
    HotkeyEntry( _("Draw Temporary Mode"), kDrawTemporaryMode ),
    HotkeyEntry( _("Erase Mode"), kEraseMode ),
    HotkeyEntry( _("Erase Temporary Mode"), kEraseTemporaryMode ),
    HotkeyEntry( _("Arrow Mode"), kArrowMode, true ),
    HotkeyEntry( _("Rectangle Mode"), kRectangleMode, true ),
    HotkeyEntry( _("Circle Mode"), kCircleMode ),
    HotkeyEntry( _("Rotate Canvas Mode"), kRotateCanvasMode ),
    HotkeyEntry( _("Text Mode"), kTextMode ),
    HotkeyEntry( _("Move/Size Mode"), kMoveSizeMode ),
    HotkeyEntry( _("Pen Size More"), kPenSizeMore),
    HotkeyEntry( _("Pen Size Less"), kPenSizeLess),
    HotkeyEntry( _("Undo Draw"), kUndoDraw),
    HotkeyEntry( _("Redo Draw"), kRedoDraw),
    HotkeyEntry( _("Gamma More"), kGammaMore),
    HotkeyEntry( _("Gamma Less"), kGammaLess),
    HotkeyEntry( _("Switch FG/BG Images"), kSwitchFGBG ),
    HotkeyEntry( _("Set As BG Image"), kSetAsBG),
    HotkeyEntry( _("Attach Audio File"), kAttachAudio),
    HotkeyEntry( _("Edit Audio Frame Offset"), kEditAudio),
    HotkeyEntry( _("Copy Frame, X, Y Values"), kCopyFrameXYValues, true),
    HotkeyEntry( _("Copy RGBA Values"), kCopyRGBAValues),
    HotkeyEntry( _("Clone Image"), kCloneImage),
    HotkeyEntry( _("Set In Point"), kSetInPoint),
    HotkeyEntry( _("Set Out Point"), kSetOutPoint),
    HotkeyEntry( _("Toggle Reel Window"), kToggleReel),
    HotkeyEntry( _("Toggle Media Info Window"), kToggleMediaInfo),
    HotkeyEntry( _("Toggle Color Area Info Window"), kToggleColorInfo),
    HotkeyEntry( _("Toggle Color Controls Window"), kToggleColorControls),
    HotkeyEntry( _("Toggle Action Window"), kToggleAction),
    HotkeyEntry( _("Toggle 3D Stereo Options Window"), kToggleStereoOptions),
    HotkeyEntry( _("Toggle 3D View Window"), kToggle3dView),
    HotkeyEntry( _("Toggle EDL Edit Window"), kToggleEDLEdit),
    HotkeyEntry( _("Toggle Histogram Window"), kToggleHistogram),
    HotkeyEntry( _("Toggle Vectorscope Window"), kToggleVectorscope),
    HotkeyEntry( _("Toggle Waveform Window"), kToggleWaveform),
    HotkeyEntry( _("Toggle ICC Profiles Window"), kToggleICCProfiles),
    HotkeyEntry( _("Toggle Connections Window"), kToggleConnections),
    HotkeyEntry( _("Toggle Preferences Window"), kTogglePreferences),
    HotkeyEntry( _("Toggle Hotkeys Window"), kToggleHotkeys),
    HotkeyEntry( _("Toggle Log Window"), kToggleLogs),
    HotkeyEntry( _("Toggle About Window"), kToggleAbout),
    HotkeyEntry( _("Toggle Hud"), kHudToggle),
    HotkeyEntry( _("Toggle Input Color Space"), kToggleICS),
    HotkeyEntry( _("Select Single Image"), kSelectSingleImage ),
    HotkeyEntry( _("Select Multi Image"), kSelectMultiImage ),
    HotkeyEntry( _("Toggle LUT"), kToggleLut),
    HotkeyEntry( _("Toggle Pixel Ratio"), kTogglePixelRatio),
    HotkeyEntry( _("Rotate Image +10 Degrees"), kRotatePlus10),
    HotkeyEntry( _("Rotate Image -10 Degrees"), kRotateMinus10),
    HotkeyEntry( _("Rotate Image +90 Degrees"), kRotatePlus90),
    HotkeyEntry( _("Rotate Image -90 Degrees"), kRotateMinus90),
    HotkeyEntry( N_("END"), kGammaLess),
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
    {' ',_("Space (' ')")},

    {FL_KP + '*', _("Multiply")},
    {FL_KP + '+', _("Add")},
    {FL_KP + '-', _("Subtract")},
    {FL_KP + '.', _("Decimal")},
    {FL_KP + '/', _("Divide")},
};



std::string Hotkey::to_s() const
{
    std::string r;
    if ( ctrl ) r += "Ctrl+";
    if ( alt ) r += "Alt+";
    if ( meta ) r += "Meta+";
    if ( shift ) r += "Shift+";

    unsigned k = key;

    bool special = false;
    for ( unsigned j = 0; j < sizeof(table)/sizeof(TableText); ++j )
      {
          if ( k == table[j].n )
            {
                r += table[j].text;
                special = true;
                break;
            }
      }

    if ( !special )
      {
          if (k >= FL_F && k <= FL_F_Last) {
              char buf[16];
              sprintf(buf, "F%d", k - FL_F);
              r += buf;
          }
          else
            {
                if ( key != 0 ) r += (char) key;
                if ( key == 0 && !text.empty() )
                    r += text;
            }
        }
    return r;
}


std::string Hotkey::to_s2() const
{
    std::string r;

    unsigned k = key2;
    if ( k == 0 ) return r;


    if ( ctrl ) r += "Ctrl+";
    if ( alt ) r += "Alt+";
    if ( meta ) r += "Meta+";
    if ( shift ) r += "Shift+";


    bool special = false;
    for ( unsigned j = 0; j < sizeof(table)/sizeof(TableText); ++j )
      {
          if ( k == table[j].n )
            {
                r += table[j].text;
                special = true;
                break;
            }
      }

    if ( !special )
      {
          if (k >= FL_F && k <= FL_F_Last) {
              char buf[16];
              sprintf(buf, "F%d", k - FL_F);
              r += buf;
          }
          else
            {
                if ( key2 != 0 ) r += (char) key2;
            }
        }
    return r;
}

bool Hotkey::operator==( const Hotkey& b ) const
{
    const std::string& A = to_s();
    const std::string& B = b.to_s();
    if ( A == B && !A.empty() && !B.empty() ) return true;
    const std::string& A2 = to_s2();
    const std::string& B2 = b.to_s2();
    if ( A2 == B2 && !A2.empty() && !B2.empty() ) return true;
    if ( A  == B2 && !A.empty() && !B2.empty() ) return true;
    if ( A2 == B  && !A2.empty() && !B.empty() ) return true;
    return false;
}



} // namespace mrv
