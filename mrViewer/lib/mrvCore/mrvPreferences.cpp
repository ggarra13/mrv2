#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvMedia.h"
#include "mrvCore/mrvPreferences.h"
#include "mrvCore/mrvI8N.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvFl/mrvIO.h"

#include <FL/fl_utf8.h>   // for fl_getenv

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


#include "mrvFl/FLU/Flu_File_Chooser.h"

extern "C" {
#include <libavutil/mem.h>  // for av_free / av_strdup
}


#include "mrvPreferencesUI.h"

namespace {
    const char* kModule = "prefs";
}


extern float kCrops[];

namespace
{

/**
 * This function allows the user to override a preference setting by
 * using an environment variable.
 *
 * @param variable        environment variable to look for
 * @param defaultValue    default value to use if variable is not set
 * @param inPrefs         boolean specifying whether the value came from
 *                        saved preferences or not.  It is used to print
 *                        a warning if some setting is as of yet undefined.
 *
 * @return a float corresponding to the value set in the environment or to
 *         to the default value.
 */
int environmentSetting( const char* variable,
                        const int   defaultValue,
                        const bool  inPrefs )
{
    int r = defaultValue;
    const char* env = fl_getenv( variable );
    if ( !env )
    {
        if ( !inPrefs )
        {
            LOG_WARNING( _("Environment variable \"") << variable <<
                         _("\" is not set; using default value (")
                         << defaultValue << ").");
        }
    }
    else
    {
        int n = sscanf( env, " %d", &r );
        if (n != 1)
        {
            LOG_ERROR( _("Cannot parse environment variable \"") << variable
                       << _("\" as an integer value; using ")
                       << defaultValue  );
        }
    }
    return r;
}


/**
 * This function allows the user to override a preference setting by
 * using an environment variable.
 *
 * @param variable        environment variable to look for
 * @param defaultValue    default value to use if variable is not set
 * @param inPrefs         boolean specifying whether the value came from
 *                        saved preferences or not.  It is used to print
 *                        a warning if some setting is as of yet undefined.
 *
 * @return a float corresponding to the value set in the environment or to
 *         to the default value.
 */
float environmentSetting( const char* variable,
                          const float defaultValue,
                          const bool inPrefs )
{
    float r = defaultValue;
    const char* env = fl_getenv( variable );
    if ( !env )
    {
        if ( !inPrefs )
        {
            LOG_WARNING( _("Environment variable \"") << variable
                         << _("\" is not set; using default value (")
                         << defaultValue << ").");
        }
    }
    else
    {
        int n = sscanf( env, " %f", &r );
        if (n != 1)
        {
            LOG_ERROR( _("Cannot parse environment variable \"") << variable
                       << _("\" as a float value; using ")
                       << defaultValue );
        }
    }
    return r;
}

/**
 * This function allows the user to override a preference setting by
 * using an environment variable.
 *
 * @param variable        environment variable to look for
 * @param defaultValue    default value to use if variable is not set
 * @param inPrefs         boolean specifying whether the value came from
 *                        saved preferences or not.  It is used to print
 *                        a warning if some setting is as of yet undefined.
 *
 * @return a string corresponding to the value set in the environment or to
 *         to the default value.
 */
const char* environmentSetting( const char* variable,
                                const char* defaultValue,
                                const bool inPrefs )
{

    const char* env = fl_getenv( variable );
    if ( !env || strlen(env) == 0 )
    {
        env = defaultValue;
        if ( !inPrefs )
        {
            LOG_WARNING( _("Environment variable \"") << variable
                         << _("\" is not set; using default value (\"")
                         << defaultValue << "\").");
        }
    }
    return env;
}

} // anonymous namespace




// AboutUI*          ViewerUI::uiAbout = NULL;
// LogUI*            ViewerUI::uiLog   = NULL;
PreferencesUI*    ViewerUI::uiPrefs = NULL;
// ICCProfileListUI* ViewerUI::uiICCProfiles = NULL;
HotkeyUI*         ViewerUI::uiHotkey = NULL;
// ConnectionUI*     ViewerUI::uiConnection = NULL;

namespace mrv {



ColorSchemes        Preferences::schemes;
ViewerUI*           Preferences::uiMain = NULL;
bool                Preferences::native_file_chooser;
bool                Preferences::use_ocio = true;
OCIO::ConstConfigRcPtr Preferences::config;
std::string         Preferences::OCIO_Display;
std::string         Preferences::OCIO_View;

mrv::Preferences::MissingFrameType      Preferences::missing_frame;

std::string         Preferences::video_threads;

std::string         Preferences::root;
int                 Preferences::debug = 0;
int                 Preferences::language_index = 2;
std::string         Preferences::tempDir = "/usr/tmp/";
std::string         Preferences::hotkeys_file = "mrViewer.keys";


int Preferences::R3dScale  = 4;
int Preferences::BRAWScale = 3;

int Preferences::switching_images = 0;

int Preferences::bgcolor;
int Preferences::textcolor;
int Preferences::selectioncolor;
int Preferences::selectiontextcolor;

static std::string expandVariables( const std::string &s,
                                    const char* START_VARIABLE,
                                    const char END_VARIABLE)
{


    size_t p = s.find( START_VARIABLE );

    if( p == std::string::npos ) return s;

    std::string pre  = s.substr( 0, p );
    std::string post = s.substr( p + strlen(START_VARIABLE) );

    size_t e = post.find( END_VARIABLE );

    if( e == std::string::npos ) return s;

    std::string variable = post.substr( 0, e );
    std::string value    = "";

    post = post.substr( e + 1 );

    const char *v = fl_getenv( variable.c_str() );
    if( v != NULL ) value = std::string( v );

    return expandVariables( pre + value + post, START_VARIABLE,
                            END_VARIABLE );
}

Preferences::Preferences( PreferencesUI* uiPrefs )
{

    bool ok;
    int version;
    int tmp;
    float tmpF;
    char  tmpS[2048];



    Fl_Preferences base( prefspath().c_str(), "filmaura",
                         "mrViewer" );


    base.get( "version", version, 6 );

    //
    // Get ui preferences
    //

    Fl_Preferences ui( base, "ui" );

    ui.get( "single_instance", tmp, 0 );
    uiPrefs->uiPrefsSingleInstance->value( (bool) tmp );

    ui.get( "menubar", tmp, 1 );
    uiPrefs->uiPrefsMenuBar->value( (bool) tmp );

    ui.get( "topbar", tmp, 1 );
    uiPrefs->uiPrefsTopbar->value( (bool) tmp );

    ui.get( "pixel_toolbar", tmp, 1 );
    uiPrefs->uiPrefsPixelToolbar->value( (bool) tmp );


    ui.get( "timeline_toolbar", tmp, 1 );
    uiPrefs->uiPrefsTimeline->value( (bool) tmp );

    ui.get( "action_toolbar", tmp, 1 );
    uiPrefs->uiPrefsToolBar->value( (bool) tmp );

    ui.get( "macOS_menus", tmp, 0 );
    uiPrefs->uiPrefsMacOSMenus->value( (bool) tmp );

    ui.get( "reel_list", tmp, 0 );
    uiPrefs->uiPrefsReelList->value( (bool) tmp );


    ui.get( "edl_edit", tmp, 0 );
    uiPrefs->uiPrefsEDLEdit->value(tmp);

    ui.get( "stereo3d_options", tmp, 0 );
    uiPrefs->uiPrefsStereoOptions->value(tmp);


    ui.get( "action_tools", tmp, 0 );
    uiPrefs->uiPrefsPaintTools->value(tmp);

    ui.get( "image_info", tmp, 0 );
    uiPrefs->uiPrefsImageInfo->value(tmp);

    ui.get( "color_area", tmp, 0 );
    uiPrefs->uiPrefsColorArea->value(tmp);


    ui.get( "histogram", tmp, 0 );
    uiPrefs->uiPrefsHistogram->value(tmp);

    ui.get( "vectorscope", tmp, 0 );
    uiPrefs->uiPrefsVectorscope->value(tmp);


    ui.get( "waveform", tmp, 0 );
    uiPrefs->uiPrefsWaveform->value(tmp);

    ui.get( "timeline_display", tmp, 0 );
    uiPrefs->uiPrefsTimelineDisplay->value(tmp);

    ui.get( "timeline_thumbnails", tmp, 1 );
    uiPrefs->uiPrefsTimelineThumbnails->value(tmp);

    ui.get( "max_cacheline_frames", tmp, 5000 );
    uiPrefs->uiPrefsMaxCachelineFrames->value(tmp);



    //
    // ui/window preferences
    //
    {
        Fl_Preferences win( ui, "window" );

        win.get( "auto_fit_image", tmp, 1 );
        uiPrefs->uiPrefsAutoFitImage->value( tmp );


        win.get( "always_on_top", tmp, 0 );
        uiPrefs->uiPrefsAlwaysOnTop->value( tmp );


        win.get( "open_mode", tmp, 0 );

        {
            Fl_Round_Button* r;
            for ( int i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i )
            {

                r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child( i );
                r->value(0);
            }

            r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child( tmp );
            r->value(1);
        }

    }



    //
    // ui/view
    //

    Fl_Preferences view( ui, "view" );

    view.get("gain", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGain->value( tmpF );

    view.get("gamma", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGamma->value( tmpF );


    view.get("compensate_pixel_ratio", tmp, 1 );
    uiPrefs->uiPrefsViewPixelRatio->value( (bool) tmp );



    view.get("safe_areas", tmp, 0 );
    uiPrefs->uiPrefsSafeAreas->value( (bool) tmp );

    view.get("crop_area", tmp, 0 );
    uiPrefs->uiPrefsCropArea->value( tmp );

    view.get( "zoom_speed", tmp, 2 );
    uiPrefs->uiPrefsZoomSpeed->value( tmp );


    view.get("display_window", tmp, 1 );
    uiPrefs->uiPrefsViewDisplayWindow->value( (bool)tmp );

    view.get("data_window", tmp, 1 );
    uiPrefs->uiPrefsViewDataWindow->value( (bool)tmp );

    //
    // ui/colors
    //

    Fl_Preferences colors( ui, "colors" );

    colors.get( "background_color", bgcolor, 0x43434300 );

    colors.get( "text_color", textcolor, 0xababab00 );

    colors.get( "selection_color", selectioncolor, 0x97a8a800 );

    colors.get( "selection_text_color", selectiontextcolor, 0x00000000 );

    bool loaded = false;

    std::string colorname = prefspath() + "mrViewer.colors";
    if ( ! (loaded = schemes.read_themes( colorname.c_str() )) )
    {

        colorname = root + "/colors/mrViewer.colors";
        if ( ! (loaded = schemes.read_themes( colorname.c_str() )) )
        {
            LOG_ERROR( _("Could not open \"") << colorname << "\"" );
        }
    }

    if ( loaded )
    {

        LOG_INFO( _("Loaded color themes from ") << colorname << "." );
    }


    for ( auto& t: schemes.themes )
    {

        uiPrefs->uiColorTheme->add( t.name.c_str() );
    }

    colors.get( "scheme", tmpS, "plastic", 2048 );

    const Fl_Menu_Item* item = uiPrefs->uiScheme->find_item( tmpS );
    if ( item )
    {

        uiPrefs->uiScheme->picked( item );
        Fl::scheme( tmpS );
    }

    colors.get( "theme", tmpS, "Black", 2048 );

    item = uiPrefs->uiColorTheme->find_item( tmpS );
    if ( item )
    {

        uiPrefs->uiColorTheme->picked( item );

    }

    const char* language = getenv( "LANGUAGE" );
    if ( !language || language[0] == '\0' ) language = getenv( "LC_ALL" );
    if ( !language || language[0] == '\0' ) language = getenv( "LC_MESSAGES" );
    if ( !language || language[0] == '\0' ) language = getenv( "LANG" );

#ifdef _WIN32
    if ( ! language )
    {
        WCHAR wcBuffer[LOCALE_NAME_MAX_LENGTH];
        int iResult = GetUserDefaultLocaleName( wcBuffer,
                                                LOCALE_NAME_MAX_LENGTH );
        if ( iResult )
        {
            _bstr_t b( wcBuffer );
            language = b;
        }
    }
#else
    if ( !language ) language = setlocale( LC_MESSAGES, NULL );
#endif

    int uiIndex = 3;
    if ( language && strlen(language) > 1 )
    {
        for ( unsigned i = 0; i < sizeof( kLanguages ) / sizeof(LanguageTable);
              ++i )
        {
            if ( strcmp( language, "C" ) == 0 )
            {
                language_index = 2;
                break;
            }
            LOG_INFO( "Comparing " << language << " to " <<
                      kLanguages[i].code );
            if ( strncmp( language, kLanguages[i].code, 2 ) == 0 )
            {
                uiIndex = i;
                language_index = kLanguages[i].index;
                break;
            }
        }
    }

    LOG_INFO( _("Setting language to ") << kLanguages[uiIndex].code );
    uiPrefs->uiLanguage->value( uiIndex );

    //
    // ui/view/colors
    //
    {

        Fl_Preferences colors( view, "colors" );

        colors.get("background_color", tmp, 0x20202000 );
        uiPrefs->uiPrefsViewBG->color( tmp );

        colors.get("text_overlay_color", tmp, 0xFFFF0000 );
        uiPrefs->uiPrefsViewTextOverlay->color( tmp );

        colors.get("selection_color", tmp, 0x0000FF00 );
        uiPrefs->uiPrefsViewSelection->color( tmp );


        colors.get("hud_color", tmp, 0xF0F08000 );
        uiPrefs->uiPrefsViewHud->color( tmp );
    }


    Fl_Preferences ocio( view, "ocio" );
    if ( version < 3 )
    {
        ocio.get( "use_ocio", tmp, 0 );
        const char* var = fl_getenv( "OCIO" );


        if ( var && strlen(var) > 0 )
            tmp = true;
    }
    else
    {

        ocio.get( "use_ocio", tmp, 1 );
    }

    uiPrefs->uiPrefsUseOcio->value( tmp );
    use_ocio = (bool)tmp;



    ocio.get( "save_config", tmp, 0 );
    uiPrefs->uiPrefsSaveOcio->value( tmp );


    ocio.get( "config", tmpS, "", 2048 );
    uiPrefs->uiPrefsOCIOConfig->value( tmpS );


    Fl_Preferences ics( ocio, "ICS" );
    {
#define OCIO_ICS(x, d)                                  \
        ok = ics.get( #x, tmpS, d, 2048 );              \
        environmentSetting( "MRV_OCIO_" #x "_ICS" ,     \
                            tmpS, ok );                 \
        uiPrefs->uiOCIO_##x##_ics->value( tmpS );

        OCIO_ICS( 8bits,  "" );

        OCIO_ICS( 16bits, "" );

        OCIO_ICS( 32bits, "" );

        OCIO_ICS( float,  "" );


    }

    //
    // ui/view/hud
    //
    Fl_Preferences hud( view, "hud" );

    hud.get("filename", tmp, 0 );

    uiPrefs->uiPrefsHudFilename->value( (bool) tmp );
    hud.get("directory", tmp, 0 );
    uiPrefs->uiPrefsHudDirectory->value( (bool) tmp );
    hud.get("fps", tmp, 0 );
    uiPrefs->uiPrefsHudFPS->value( (bool) tmp );
    hud.get("av_difference", tmp, 0 );

    uiPrefs->uiPrefsHudAVDifference->value( (bool) tmp );
    hud.get("frame", tmp, 0 );
    uiPrefs->uiPrefsHudFrame->value( (bool) tmp );
    hud.get("timecode", tmp, 0 );
    uiPrefs->uiPrefsHudTimecode->value( (bool) tmp );
    hud.get("resolution", tmp, 0 );

    uiPrefs->uiPrefsHudResolution->value( (bool) tmp );
    hud.get("frame_range", tmp, 0 );
    uiPrefs->uiPrefsHudFrameRange->value( (bool) tmp );
    hud.get("frame_count", tmp, 0 );
    uiPrefs->uiPrefsHudFrameCount->value( (bool) tmp );
    hud.get("memory", tmp, 0 );

    uiPrefs->uiPrefsHudMemory->value( (bool) tmp );
    hud.get("attributes", tmp, 0 );

    uiPrefs->uiPrefsHudAttributes->value( (bool) tmp );
    hud.get("center", tmp, 0 );

    uiPrefs->uiPrefsHudCenter->value( (bool) tmp );

    Fl_Preferences win( view, "window" );
    win.get("fixed_position", tmp, 0 );
    uiPrefs->uiWindowFixedPosition->value( (bool) tmp );
    win.get("x_position", tmp, 0 );

    uiPrefs->uiWindowXPosition->value( tmp );
    win.get("y_position", tmp, 0 );
    uiPrefs->uiWindowYPosition->value( tmp );
    win.get("fixed_size", tmp, 0 );

    uiPrefs->uiWindowFixedSize->value( (bool) tmp );
    win.get("x_size", tmp, 640 );
    uiPrefs->uiWindowXSize->value( tmp );
    win.get("y_size", tmp, 530 );

    uiPrefs->uiWindowYSize->value( tmp );

    Fl_Preferences flu( ui, "file_requester" );
    //


    flu.get("quick_folder_travel", tmp, 1 );
    uiPrefs->uiPrefsFileReqFolder->value( (bool) tmp );
    Flu_File_Chooser::singleButtonTravelDrawer = (bool) tmp;

    flu.get("thumbnails", tmp, 1 );
    uiPrefs->uiPrefsFileReqThumbnails->value( (bool) tmp );
    Flu_File_Chooser::thumbnailsFileReq = (bool) tmp;

    //
    // playback
    //
    Fl_Preferences playback( base, "playback" );

    playback.get( "auto_playback", tmp, 1 );
    uiPrefs->uiPrefsAutoPlayback->value(tmp);


    playback.get( "play_all_frames", tmp, 1 );
    uiPrefs->uiPrefsPlayAllFrames->value(tmp);


    playback.get( "override_fps", tmp, 0 );
    uiPrefs->uiPrefsOverrideFPS->value(tmp);


    playback.get( "fps", tmpF, 24.0 );
    uiPrefs->uiPrefsFPS->value(tmpF);


    playback.get( "loop_mode", tmp, 1 );
    uiPrefs->uiPrefsLoopMode->value(tmp);


    playback.get( "scrubbing_sensitivity", tmpF, 5.0f );
    uiPrefs->uiPrefsScrubbingSensitivity->value(tmpF);


    playback.get( "selection_display_mode", tmp, 0 );
    uiPrefs->uiPrefsTimelineSelectionDisplay->value(tmp);

    Fl_Preferences pixel_toolbar( base, "pixel_toolbar" );

    pixel_toolbar.get( "RGBA_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelRGBA->value( tmp );


    pixel_toolbar.get( "pixel_values", tmp, 0 );
    uiPrefs->uiPrefsPixelValues->value( tmp );


    pixel_toolbar.get( "HSV_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelHSV->value( tmp );


    pixel_toolbar.get( "Lumma_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelLumma->value( tmp );


    Fl_Preferences action( base, "action" );

    action.get( "scrubbing", tmp, 1 );
    uiPrefs->uiScrub->value( (bool) tmp );
    action.get( "move_picture", tmp, 0 );
    uiPrefs->uiMovePicture->value( (bool) tmp );
    action.get( "color_area", tmp, 0 );

    uiPrefs->uiSelection->value( (bool) tmp );
    action.get( "pencil", tmp, 0 );
    uiPrefs->uiDraw->value( (bool) tmp );
    action.get( "text", tmp, 0 );

    uiPrefs->uiText->value( (bool) tmp );
    action.get( "eraser", tmp, 0 );
    uiPrefs->uiErase->value( (bool) tmp );


    Fl_Preferences caches( base, "caches" );


    caches.get( "active", tmp, 1 );
    uiPrefs->uiPrefsCacheActive->value( (bool) tmp );


    caches.get( "preload", tmp, 1 );
    uiPrefs->uiPrefsPreloadCache->value( (bool) tmp );


    caches.get( "scale", tmp, 0 );
    uiPrefs->uiPrefsCacheScale->value( tmp );



    caches.get( "8bit_caches", tmp, 0 );
    uiPrefs->uiPrefs8BitCaches->value( (bool) tmp );


    //
    // audio
    //
    Fl_Preferences audio( base, "audio" );
    char device[256];
    audio.get( "device", device, "default", 255 );



    audio.get( "override_audio", tmp, 0 );
    uiPrefs->uiPrefsOverrideAudio->value( tmp );


    audio.get( "volume", tmpF, 1.0f );
    uiPrefs->uiPrefsAudioVolume->value( tmpF );


    audio.get( "volume_mute", tmp, 0 );
    uiPrefs->uiPrefsAudioMute->value( tmp );

    // Images

    Fl_Preferences images( base, "images" );
    images.get( "editable_metadata", tmp, 0 );
    uiPrefs->uiMetadataEditable->value( tmp );

    images.get( "all_layers", tmp, 0 );
    uiPrefs->uiPrefsAllLayers->value( tmp );


    images.get( "aces_metadata", tmp, 0 );
    uiPrefs->uiPrefsACESClipMetadata->value( tmp );

    // OpenEXR

    Fl_Preferences openexr( base, "openexr" );
    openexr.get( "thread_count", tmp, 4 );
    uiPrefs->uiPrefsOpenEXRThreadCount->value( tmp );


    openexr.get( "gamma", tmpF, 2.2f );
    if ( !use_ocio ) {

        uiPrefs->uiPrefsOpenEXRGamma->value( tmpF );
    }
    else
    {
        uiPrefs->uiPrefsOpenEXRGamma->value( 1.0f );
    }


    openexr.get( "compression", tmp, 4 );   // PIZ default
    uiPrefs->uiPrefsOpenEXRCompression->value( tmp );


    openexr.get( "dwa_compression", tmpF, 45.0f );
    uiPrefs->uiPrefsOpenEXRDWACompression->value( tmpF );


    Fl_Preferences red3d( base, "red3d" );
    red3d.get( "proxy_scale", tmp, 4 );   // 1:16 default
    R3dScale = tmp;
    uiPrefs->uiPrefsR3DScale->value( tmp );

    Fl_Preferences braw( base, "braw" );
    braw.get( "proxy_scale", tmp, 3 );   // 1:8 default
    BRAWScale = tmp;
    uiPrefs->uiPrefsBRAWScale->value( tmp );


    Fl_Preferences loading( base, "loading" );

    loading.get( "load_library", tmp, 1 );
    uiPrefs->uiPrefsLoadLibrary->value( tmp );


    loading.get( "missing_frames", tmp, 0 );
    uiPrefs->uiPrefsMissingFrames->value( tmp );

    loading.get( "drag_load_seq", tmp, 1 );
    uiPrefs->uiPrefsLoadSequence->value( (bool) tmp );


    loading.get( "file_assoc_load_seq", tmp, 1 );
    uiPrefs->uiPrefsLoadSequenceOnAssoc->value( (bool) tmp );


    loading.get( "autoload_images", tmp, 0 );
    uiPrefs->uiPrefsAutoLoadImages->value( (bool) tmp );

#if defined( _WIN32 ) || defined( OSX )
    loading.get( "native_file_chooser", tmp, 1 );
#  ifdef OSX
    if ( version < 5 ) tmp = 1;
#  endif
#else
    loading.get( "native_file_chooser", tmp, 0 );
#endif
    uiPrefs->uiPrefsNativeFileChooser->value( (bool) tmp );


    loading.get( "thumbnail_percent", tmpF, 0.0 );
    uiPrefs->uiPrefsThumbnailPercent->value( tmpF );


    loading.get( "uses_16bits", tmp, 0 );
    uiPrefs->uiPrefsUses16Bits->value( (bool) tmp );

    loading.get( "image_version_prefix", tmpS, "_v", 256 );
    uiPrefs->uiPrefsImageVersionPrefix->value( tmpS );

    loading.get( "max_images_apart", tmp, 10 );
    uiPrefs->uiPrefsMaxImagesApart->value( tmp );


    Fl_Preferences saving( base, "saving" );
    saving.get( "use_relative_paths", tmp, 1 );
    uiPrefs->uiPrefsRelativePaths->value( tmp );

    saving.get( "use_image_path", tmp, 1 );
    uiPrefs->uiPrefsImagePathReelPath->value( tmp );


    Fl_Preferences video( base, "video" );
    video.get( "filtering", tmp, 0 );
    uiPrefs->uiPrefsFiltering->value( tmp );
    video.get( "video_codec", tmp, 0 );
    uiPrefs->uiPrefsVideoCodec->value(tmp);
    video.get( "yuv_hint", tmp, 0 );
    if ( version < 4 ) tmp = 0;
    uiPrefs->uiPrefsYUVConversion->value(tmp);
    video.get( "thread_count", tmp, 0 );

    uiPrefs->uiPrefsVideoThreadCount->value( tmp );

    Fl_Preferences comp( base, "compositing" );
    comp.get( "blend_mode", tmp, 0 );

    uiPrefs->uiPrefsBlendMode->value(tmp);
    comp.get( "resize_bg", tmp, 1 );

    uiPrefs->uiPrefsResizeBackground->value(tmp);

    Fl_Preferences subtitles( base, "subtitles" );

    subtitles.get( "font", tmpS, "Arial", 2048 );
    uiPrefs->uiPrefsSubtitleFont->value(0);  // in case no font is found
    for (unsigned i = 0; i < uiPrefs->uiPrefsSubtitleFont->children(); ++i )
    {

        const char* label = uiPrefs->uiPrefsSubtitleFont->child(i)->label();
        if ( label && strcmp( label, tmpS ) == 0 )
        {

            uiPrefs->uiPrefsSubtitleFont->value(i);
            break;
        }
    }

    subtitles.get( "encoding", tmpS, "ISO-8859-1", 2048 );
    uiPrefs->uiPrefsSubtitleEncoding->value( tmpS );


    Fl_Preferences errors( base, "errors" );

    errors.get( "raise_log_window_on_error", tmp, 0 );
    uiPrefs->uiPrefsRaiseLogWindowOnError->value(tmp);


    //
    // Hotkeys
    //
    Fl_Preferences* keys;

    Fl_Preferences hotkeys( base, "hotkeys" );
    hotkeys.get( "default", tmpS, "mrViewer.keys", 2048 );

    hotkeys_file = tmpS;

    if ( version >= 6 )
    {
        if ( hotkeys_file.empty() ) hotkeys_file = _("mrViewer.keys");
        LOG_INFO( _("Loading hotkeys from ") << prefspath()
                  << _( hotkeys_file.c_str() ) << ".prefs" );
        keys = new Fl_Preferences( prefspath().c_str(), "filmaura",
                                   tmpS );
    }
    else
    {
        keys = new Fl_Preferences( base, "hotkeys" );
    }
    //load_hotkeys(uiMain, keys);
}



void Preferences::save()
{
    int i;
    PreferencesUI* uiPrefs = ViewerUI::uiPrefs;

    Fl_Preferences base( prefspath().c_str(), "filmaura",
                         "mrViewer" );
    base.set( "version", 6 );

    // Save ui preferences
    Fl_Preferences ui( base, "ui" );

    //
    // window options
    //
    {
        Fl_Preferences win( ui, "window" );
        win.set( "auto_fit_image", (int) uiPrefs->uiPrefsAutoFitImage->value() );
        win.set( "always_on_top", (int) uiPrefs->uiPrefsAlwaysOnTop->value() );
        int tmp = 0;
        for ( i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i ) {
            Fl_Round_Button* r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(i);
            if ( r->value() ) {
                tmp = i;
                break;
            }
        }
        win.set( "open_mode", tmp );
    }

    //
    // ui options
    //

    ui.set( "language", language_index );

    ui.set( "menubar", (int) uiPrefs->uiPrefsMenuBar->value() );
    ui.set( "topbar", (int) uiPrefs->uiPrefsTopbar->value() );
    ui.set( "single_instance", (int) uiPrefs->uiPrefsSingleInstance->value() );
    ui.set( "pixel_toolbar", (int) uiPrefs->uiPrefsPixelToolbar->value() );
    ui.set( "timeline_toolbar", (int) uiPrefs->uiPrefsTimeline->value() );
    ui.set( "action_toolbar", (int) uiPrefs->uiPrefsToolBar->value() );
    ui.set( "macOS_menus", (int) uiPrefs->uiPrefsMacOSMenus->value() );
    ui.set( "reel_list", (int) uiPrefs->uiPrefsReelList->value() );
    ui.set( "edl_edit", (int) uiPrefs->uiPrefsEDLEdit->value() );
    ui.set( "stereo3d_options", (int) uiPrefs->uiPrefsStereoOptions->value() );
    ui.set( "action_tools", (int) uiPrefs->uiPrefsPaintTools->value() );
    ui.set( "image_info", (int) uiPrefs->uiPrefsImageInfo->value() );
    ui.set( "color_area", (int) uiPrefs->uiPrefsColorArea->value() );
    ui.set( "histogram", (int) uiPrefs->uiPrefsHistogram->value() );
    ui.set( "vectorscope", (int) uiPrefs->uiPrefsVectorscope->value() );
    ui.set( "waveform", (int) uiPrefs->uiPrefsWaveform->value() );


    ui.set( "timeline_display",
            uiPrefs->uiPrefsTimelineDisplay->value() );

    ui.set( "timeline_thumbnails",
            uiPrefs->uiPrefsTimelineThumbnails->value() );

    ui.set( "max_cacheline_frames",
            uiPrefs->uiPrefsMaxCachelineFrames->value() );
    //
    // ui/view prefs
    //
    Fl_Preferences view( ui, "view" );
    view.set("gain", uiPrefs->uiPrefsViewGain->value() );
    view.set("gamma", uiPrefs->uiPrefsViewGamma->value() );
    view.set("compensate_pixel_ratio", uiPrefs->uiPrefsViewPixelRatio->value() );
    view.set("display_window", uiPrefs->uiPrefsViewDisplayWindow->value() );
    view.set("data_window", uiPrefs->uiPrefsViewDataWindow->value() );

    view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value() );
    view.set("crop_area", uiPrefs->uiPrefsCropArea->value() );
    view.set( "zoom_speed", (int) uiPrefs->uiPrefsZoomSpeed->value() );

    //
    // view/colors prefs
    //
    {
        Fl_Preferences colors( view, "colors" );
        int tmp = uiPrefs->uiPrefsViewBG->color();
        colors.set("background_color", tmp );
        tmp = uiPrefs->uiPrefsViewTextOverlay->color();
        colors.set("text_overlay_color", tmp );
        tmp = uiPrefs->uiPrefsViewSelection->color();
        colors.set("selection_color", tmp );
        tmp = uiPrefs->uiPrefsViewHud->color();
        colors.set("hud_color", tmp );
    }

    {
        Fl_Preferences ocio( view, "ocio" );
        int tmp = uiPrefs->uiPrefsUseOcio->value();
        ocio.set( "use_ocio", tmp );


        if ( uiPrefs->uiPrefsSaveOcio->value() )
        {
            ocio.set( "save_config", 1 );
            ocio.set( "config", uiPrefs->uiPrefsOCIOConfig->value() );
        }
        else
        {
            ocio.set( "save_config", 0 );
            ocio.set( "config", "" );
        }

        Fl_Preferences ics( ocio, "ICS" );
        {
            ics.set( "8bits",  uiPrefs->uiOCIO_8bits_ics->value() );
            ics.set( "16bits", uiPrefs->uiOCIO_16bits_ics->value() );
            ics.set( "32bits", uiPrefs->uiOCIO_32bits_ics->value() );
            ics.set( "float",  uiPrefs->uiOCIO_float_ics->value() );
        }

    }

    //
    // view/hud prefs
    //
    Fl_Preferences hud( view, "hud" );
    hud.set("filename", uiPrefs->uiPrefsHudFilename->value() );
    hud.set("directory", uiPrefs->uiPrefsHudDirectory->value() );
    hud.set("fps", uiPrefs->uiPrefsHudFPS->value() );
    hud.set("av_difference", uiPrefs->uiPrefsHudAVDifference->value() );
    hud.set("non_drop_timecode", uiPrefs->uiPrefsHudTimecode->value() );
    hud.set("frame", uiPrefs->uiPrefsHudFrame->value() );
    hud.set("resolution", uiPrefs->uiPrefsHudResolution->value() );
    hud.set("frame_range", uiPrefs->uiPrefsHudFrameRange->value() );
    hud.set("frame_count", uiPrefs->uiPrefsHudFrameCount->value() );
    hud.set("memory", uiPrefs->uiPrefsHudMemory->value() );
    hud.set("attributes", uiPrefs->uiPrefsHudAttributes->value() );
    hud.set("center", uiPrefs->uiPrefsHudCenter->value() );

    {
        Fl_Preferences win( view, "window" );
        win.set("fixed_position", uiPrefs->uiWindowFixedPosition->value() );
        win.set("x_position", uiPrefs->uiWindowXPosition->value() );
        win.set("y_position", uiPrefs->uiWindowYPosition->value() );
        win.set("fixed_size", uiPrefs->uiWindowFixedSize->value() );
        win.set("x_size", uiPrefs->uiWindowXSize->value() );
        win.set("y_size", uiPrefs->uiWindowYSize->value() );
    }

    //
    // ui/colors prefs
    //
    Fl_Preferences colors( ui, "colors" );
    colors.set( "scheme", uiPrefs->uiScheme->text() );
    colors.set( "theme", uiPrefs->uiColorTheme->text() );
    colors.set( "background_color", bgcolor );
    colors.set( "text_color", textcolor );
    colors.set( "selection_color", selectioncolor );
    colors.set( "selection_text_color", selectiontextcolor );
    colors.set( "theme", uiPrefs->uiColorTheme->text() );

    Fl_Preferences flu( ui, "file_requester" );
    flu.set("quick_folder_travel", uiPrefs->uiPrefsFileReqFolder->value());
    flu.set("thumbnails", uiPrefs->uiPrefsFileReqThumbnails->value());

    // @TODO: fltk1.4
    Flu_File_Chooser::singleButtonTravelDrawer =
        uiPrefs->uiPrefsFileReqFolder->value();
    Flu_File_Chooser::thumbnailsFileReq =
        uiPrefs->uiPrefsFileReqThumbnails->value();

    //
    // playback prefs
    //
    Fl_Preferences playback( base, "playback" );
    playback.set( "auto_playback", (int) uiPrefs->uiPrefsAutoPlayback->value() );
    playback.set( "play_all_frames",
                  (int) uiPrefs->uiPrefsPlayAllFrames->value() );
    playback.set( "override_fps", uiPrefs->uiPrefsOverrideFPS->value() );
    playback.set( "fps", uiPrefs->uiPrefsFPS->value() );
    playback.set( "loop_mode", uiPrefs->uiPrefsLoopMode->value() );
    playback.set( "scrubbing_sensitivity",
                  uiPrefs->uiPrefsScrubbingSensitivity->value() );
    playback.set( "selection_display_mode",
                  uiPrefs->uiPrefsTimelineSelectionDisplay->value() );

    Fl_Preferences pixel_toolbar( base, "pixel_toolbar" );
    pixel_toolbar.set( "RGBA_pixel", uiPrefs->uiPrefsPixelRGBA->value() );
    pixel_toolbar.set( "pixel_values", uiPrefs->uiPrefsPixelValues->value() );
    pixel_toolbar.set( "HSV_pixel", uiPrefs->uiPrefsPixelHSV->value() );
    pixel_toolbar.set( "Lumma_pixel", uiPrefs->uiPrefsPixelLumma->value() );


    Fl_Preferences action( base, "action" );

    action.set( "scrubbing", (int)uiPrefs->uiScrub->value() );
    action.set( "move_picture", (int)uiPrefs->uiMovePicture->value() );
    action.set( "color_area", (int)uiPrefs->uiSelection->value() );
    action.set( "pencil", (int)uiPrefs->uiDraw->value() );
    action.set( "text", (int) uiPrefs->uiText->value() );
    action.set( "eraser", (int)  uiPrefs->uiErase->value() );

    Fl_Preferences caches( base, "caches" );
    caches.set( "active", (int) uiPrefs->uiPrefsCacheActive->value() );
    caches.set( "preload", (int) uiPrefs->uiPrefsPreloadCache->value() );
    caches.set( "scale", (int) uiPrefs->uiPrefsCacheScale->value() );
    caches.set( "8bit_caches", (int) uiPrefs->uiPrefs8BitCaches->value() );
    caches.set( "fps", (int) uiPrefs->uiPrefsCacheFPS->value() );
    caches.set( "size", (int) uiPrefs->uiPrefsCacheSize->value() );

    caches.set( "cache_memory", (float)uiPrefs->uiPrefsCacheMemory->value() );

    Fl_Preferences loading( base, "loading" );
    loading.set( "load_library", uiPrefs->uiPrefsLoadLibrary->value() );
    loading.set( "missing_frames", uiPrefs->uiPrefsMissingFrames->value());
    loading.set( "drag_load_seq", (int) uiPrefs->uiPrefsLoadSequence->value() );
    loading.set( "file_assoc_load_seq",
                 (int) uiPrefs->uiPrefsLoadSequenceOnAssoc->value() );
    loading.set( "autoload_images",
                 (int) uiPrefs->uiPrefsAutoLoadImages->value() );
    loading.set( "native_file_chooser", (int) uiPrefs->uiPrefsNativeFileChooser->value() );
    loading.set( "thumbnail_percent",
                 uiPrefs->uiPrefsThumbnailPercent->value() );

    loading.set( "uses_16bits", (int) uiPrefs->uiPrefsUses16Bits->value() );
    loading.set( "image_version_prefix",
                 uiPrefs->uiPrefsImageVersionPrefix->value() );
    loading.set( "max_images_apart", uiPrefs->uiPrefsMaxImagesApart->value() );

    Fl_Preferences saving( base, "saving" );
    saving.set( "use_relative_paths", (int)
                uiPrefs->uiPrefsRelativePaths->value() );

    saving.set( "use_image_path", (int)
                uiPrefs->uiPrefsImagePathReelPath->value() );

    Fl_Preferences video( base, "video" );
    video.set( "filtering", (int) uiPrefs->uiPrefsFiltering->value() );
    video.set( "video_codec", (int) uiPrefs->uiPrefsVideoCodec->value() );
    video.set( "yuv_hint", (int) uiPrefs->uiPrefsYUVConversion->value() );
    video.set( "thread_count", (int) uiPrefs->uiPrefsVideoThreadCount->value());

    Fl_Preferences comp( base, "compositing" );
    comp.set( "blend_mode", (int) uiPrefs->uiPrefsBlendMode->value() );
    comp.set( "resize_bg", (int) uiPrefs->uiPrefsResizeBackground->value() );

    //
    // Audio prefs
    //
    Fl_Preferences audio( base, "audio" );
    unsigned int idx = uiPrefs->uiPrefsAudioDevice->value();



    audio.set( "override_audio", uiPrefs->uiPrefsOverrideAudio->value() );
    audio.set( "volume", uiPrefs->uiPrefsAudioVolume->value() );

    audio.set( "volume_mute", uiPrefs->uiPrefsAudioMute->value() );




    {
        Fl_Preferences subtitles( base, "subtitles" );
        int idx = uiPrefs->uiPrefsSubtitleFont->value();
        if ( idx >= 0 )
        {
            subtitles.set( "font",
                           uiPrefs->uiPrefsSubtitleFont->child(idx)->label() );
        }
        subtitles.set( "encoding",
                       uiPrefs->uiPrefsSubtitleEncoding->value() );
    }

    Fl_Preferences errors( base, "errors" );
    errors.set( "raise_log_window_on_error",
                uiPrefs->uiPrefsRaiseLogWindowOnError->value() );

    // Images
    Fl_Preferences images( base, "images" );
    images.set( "editable_metadata", uiPrefs->uiMetadataEditable->value());
    images.set( "all_layers", (int) uiPrefs->uiPrefsAllLayers->value() );
    images.set( "aces_metadata",
                (int) uiPrefs->uiPrefsACESClipMetadata->value());

    // OpenEXR
    Fl_Preferences openexr( base, "openexr" );
    openexr.set( "thread_count", (int) uiPrefs->uiPrefsOpenEXRThreadCount->value() );
    openexr.set( "gamma", uiPrefs->uiPrefsOpenEXRGamma->value() );
    openexr.set( "compression",
                 (int) uiPrefs->uiPrefsOpenEXRCompression->value() );
    openexr.set( "dwa_compression",
                 uiPrefs->uiPrefsOpenEXRDWACompression->value() );

    Fl_Preferences red3d( base, "red3d" );
    red3d.set( "proxy_scale", (int) uiPrefs->uiPrefsR3DScale->value() );

    Fl_Preferences braw( base, "braw" );
    braw.set( "proxy_scale", (int) uiPrefs->uiPrefsBRAWScale->value() );


    Fl_Preferences hotkeys( base, "hotkeys" );
    hotkeys.set( "default", hotkeys_file.c_str() );

    if ( ! fs::exists( prefspath() + hotkeys_file ) )
    {
        Fl_Preferences keys( prefspath().c_str(), "filmaura",
                             hotkeys_file.c_str() );
        save_hotkeys( keys );
    }

    base.flush();

    LOG_INFO( _("Preferences have been saved to: ") << prefspath() << "mrViewer.prefs." );

    check_language( uiPrefs, language_index );
}


bool Preferences::set_transforms()
{
    return true;
}


Preferences::~Preferences()
{
}



void Preferences::run( ViewerUI* main )
{
    DBG;
    uiMain = main;
    PreferencesUI* uiPrefs = main->uiPrefs;

    check_language( uiPrefs, language_index );
    DBG;

#ifdef OSX
    if ( uiPrefs->uiPrefsMacOSMenus->value() )
    {
        uiMain->uiMenuBar->clear();
        uiMain->uiMenuGroup->redraw();
        delete uiMain->uiMenuBar;
        uiMain->uiMenuBar = new Fl_Sys_Menu_Bar( 0, 0, 0, 25 );
    }
    else
    {
        Fl_Sys_Menu_Bar* smenubar =
            dynamic_cast< Fl_Sys_Menu_Bar* >( uiMain->uiMenuBar );
        if ( smenubar )
        {
            smenubar->clear();
            delete uiMain->uiMenuBar;
            uiMain->uiMenuBar = new Fl_Menu_Bar( 0, 0,
                                                 uiMain->uiStatus->x(), 25 );
            uiMain->uiMenuGroup->add( uiMain->uiMenuBar );
            uiMain->uiMenuGroup->redraw();
        }
    }
#endif



    //
    // Already shown on mrViewer.fl
    //
    //main->uiMain->show();

    //
    // Windows
    //



    // PaintUI* uiPaint = main->uiPaint;

    // if ( uiPrefs->uiPrefsPaintTools->value() )
    // {
    //     uiPaint->uiMain->show();
    // }
    // else
    //     uiPaint->uiMain->hide();



    // if ( uiPrefs->uiPrefsStereoOptions->value() )
    // {
    //     main->uiStereo->uiMain->show();
    // }
    // else
    //     main->uiStereo->uiMain->hide();


    // if ( uiPrefs->uiPrefsReelList->value() )
    // {
    //     main->uiReelWindow->uiMain->show();
    // }
    // else
    //     main->uiReelWindow->uiMain->hide();


    // mrv::GLViewport* v = uiMain->uiView;
    // if ( uiPrefs->uiPrefsImageInfo->value() )
    //     v->toggle_window( GLViewport::kMediaInfo,
    //                       uiPrefs->uiPrefsImageInfo->value() );


    // if ( uiPrefs->uiPrefsColorArea->value() )
    //     v->toggle_window( GLViewport::kColorInfo,
    //                       uiPrefs->uiPrefsColorArea->value() );


    // if ( uiPrefs->uiPrefsHistogram->value() )
    //     v->toggle_window( GLViewport::kHistogram,
    //                       uiPrefs->uiPrefsHistogram->value() );


    // if ( uiPrefs->uiPrefsVectorscope->value() )
    //     v->toggle_window( GLViewport::kVectorscope,
    //                       uiPrefs->uiPrefsVectorscope->value() );


    // if ( uiPrefs->uiPrefsWaveform->value() )
    //     v->toggle_window( GLViewport::kWaveform,
    //                       uiPrefs->uiPrefsWaveform->value() );

    //
    // Toolbars
    //

    GLViewport* view = uiMain->uiView;

    //uiMain->uiView->fill_menu( uiMain->uiMenuBar );
    if ( uiPrefs->uiPrefsMenuBar->value() )
    {
        uiMain->uiMenuGroup->show();
    }
    else {
        uiMain->uiMenuGroup->hide();
    }


    if ( uiPrefs->uiPrefsTopbar->value() )
    {
        main->uiTopBar->show();
    }
    else
    {
        main->uiTopBar->hide();
    }


    DBG;
    if ( uiPrefs->uiPrefsPixelToolbar->value() )
    {
        main->uiPixelBar->show();
    }
    else
    {
        main->uiPixelBar->hide();
    }

    DBG;

    if ( uiPrefs->uiPrefsTimeline->value() )
    {
        main->uiBottomBar->show();
    }
    else
    {
        main->uiBottomBar->hide();
    }

    DBG;
    if ( uiPrefs->uiPrefsToolBar->value() )
    {
        main->uiToolsGroup->show();
        main->uiToolsGroup->size( 45, 433 );
        main->uiViewGroup->layout();
        main->uiViewGroup->init_sizes();
    }
    else
    {
        main->uiToolsGroup->hide();
        main->uiViewGroup->layout();
        main->uiViewGroup->init_sizes();
    }
    DBG;

    // @BUG: WINDOWS NEEDS THIS
    ///      To fix to uiRegion scaling badly (too much or too little)
    // main->uiView->resize_main_window();
    // main->uiRegion->size( main->uiRegion->w(), main->uiMain->h() );

    //
    // Widget/Viewer settings
    //


    DBG;
    main->uiLoopMode->value( uiPrefs->uiPrefsLoopMode->value() );
    main->uiLoopMode->do_callback();


    main->uiGain->value( uiPrefs->uiPrefsViewGain->value() );
    main->uiGamma->value( uiPrefs->uiPrefsViewGamma->value() );



    main->uiPixelRatio->value( uiPrefs->uiPrefsViewPixelRatio->value() );
    // if ( main->uiPixelRatio->value() )
    //     view->show_pixel_ratio( main->uiPixelRatio->value() );

    // view->texture_filtering( GLViewport::kNearestNeighbor );xs
    // if ( main->uiPrefs->uiPrefsFiltering->value() ==
    //      GLViewport::kBilinearFiltering )
    //     view->texture_filtering( GLViewport::kBilinearFiltering );

    // view->display_window( uiPrefs->uiPrefsViewDisplayWindow->value() );
    // view->data_window( uiPrefs->uiPrefsViewDataWindow->value() );


    // if ( uiPrefs->uiScrub->value() )
    //     view->scrub_mode();
    // else if ( uiPrefs->uiMovePicture->value() )
    //     view->move_pic_mode();
    // else if ( uiPrefs->uiSelection->value() )
    //     view->selection_mode();
    // else if ( uiPrefs->uiDraw->value() )
    //     view->draw_mode();
    // else if ( uiPrefs->uiText->value() )
    //     view->text_mode();
    // else if ( uiPrefs->uiErase->value() )
    //     view->erase_mode();


    DBG;
    missing_frame = (MissingFrameType)uiPrefs->uiPrefsMissingFrames->value();


    //////////////////////////////////////////////////////
    // OCIO
    /////////////////////////////////////////////////////


    use_ocio = (bool) uiPrefs->uiPrefsUseOcio->value();

    const char* var = environmentSetting( "OCIO",
                                          uiPrefs->uiPrefsOCIOConfig->value(),
                                          true );
    const char* envvar = var;

    std::string tmp = root + "/ocio/nuke-default/config.ocio";

    if ( uiPrefs->uiPrefsSaveOcio->value() && use_ocio )
    {
        Fl_Preferences base( prefspath().c_str(), "filmaura",
                             "mrViewer" );
        Fl_Preferences ui( base, "ui" );
        Fl_Preferences view( ui, "view" );
        Fl_Preferences ocio( view, "ocio" );
        char tmpS[2048];
        ocio.get( "config", tmpS, "", 2048 );
        uiPrefs->uiPrefsOCIOConfig->value( tmpS );

        var = uiPrefs->uiPrefsOCIOConfig->value();
    }

    if (  ( !var || strlen(var) == 0 || tmp == var ) && use_ocio )
    {
        var = av_strdup( tmp.c_str() );
    }


    bool nuke_default = false;

    if ( var && use_ocio && strlen(var) > 0 )
    {
        static std::string old_ocio;


        if ( old_ocio != var )
        {

            if ( var == envvar )
            {
                mrvLOG_INFO( "ocio", _("Setting OCIO config from OCIO "
                                    "environment variable:")
                            << std::endl );
            }
            else
            {
                mrvLOG_INFO( "ocio", _("Setting OCIO config to:")
                            << std::endl );
            }
            old_ocio = var;
            mrvLOG_INFO( "ocio", old_ocio << std::endl );
        }

        std::string parsed = expandVariables( var, "%", '%' );
        parsed = expandVariables( parsed, "${", '}' );
        if ( old_ocio != parsed )
        {
          mrvLOG_INFO( "ocio", _("Expanded OCIO config to:")
                                  << std::endl );
          mrvLOG_INFO( "ocio", parsed << std::endl );

        }

        if ( parsed.rfind( "nuke-default" ) != std::string::npos )
            nuke_default = true;

        // sprintf( buf, "OCIO=%s", parsed.c_str() );
        // putenv( av_strdup(buf) );
        uiPrefs->uiPrefsOCIOConfig->value( var );


        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );


        try
        {

            config = OCIO::Config::CreateFromFile( parsed.c_str() );

            uiPrefs->uiPrefsOCIOConfig->tooltip( config->getDescription() );


            OCIO_Display = config->getDefaultDisplay();

            OCIO_View = config->getDefaultView( OCIO_Display.c_str() );


            // First, remove all additional defaults if any from pulldown menu
            if ( use_ocio && !OCIO_View.empty() && !OCIO_Display.empty() )
            {
                main->gammaDefaults->clear();
            }



            int numDisplays = config->getNumDisplays();


            stringArray active_displays;
            const char* displaylist = config->getActiveDisplays();
            if ( displaylist )
            {
                mrv::split( active_displays, displaylist, ',' );

                // Eliminate forward spaces in names
                for ( unsigned i = 0; i < active_displays.size(); ++i )
                {
                    while ( active_displays[i][0] == ' ' )
                        active_displays[i] =
                        active_displays[i].substr( 1, active_displays[i].size() );
                }
            }
            else
            {
                int num = config->getNumDisplays();
                for ( int i = 0; i < num; ++i )
                {
                    active_displays.push_back( config->getDisplay( i ) );
                }
            }

            stringArray active_views;
            const char* viewlist = config->getActiveViews();
            if ( viewlist )
            {
                mrv::split( active_views, viewlist, ',' );

                // Eliminate forward spaces in names
                for ( unsigned i = 0; i < active_views.size(); ++i )
                {
                    while ( active_views[i][0] == ' ' )
                        active_views[i] =
                        active_views[i].substr( 1, active_views[i].size() );
                }
            }

            size_t num_active_displays = active_displays.size();
            size_t num_active_views = active_views.size();

            for ( size_t j = 0; j < num_active_displays; ++j )
            {
                std::string display = active_displays[j];


                int numViews = config->getNumViews(display.c_str());

                // Collect all views

                if ( num_active_views )
                {
                    for ( size_t h = 0; h < num_active_views; ++h )
                    {
                        std::string view;
                        bool add = false;

                        for (int i = 0; i < numViews; ++i)
                        {
                            view = config->getView(display.c_str(), i);
                            if ( active_views[h] == view )
                            {
                                add = true; break;
                            }
                        }

                        if ( add )
                        {
                            std::string name;
                            if ( num_active_displays > 1 )
                            {
                                name = display;
                                name += "/";
                                name += view;
                            }
                            else
                            {
                                name = view;
                                name += " (" + display + ")";
                            }

                            main->gammaDefaults->add( name.c_str() );

                            if ( view == OCIO_View && !OCIO_View.empty() )
                            {
                                main->gammaDefaults->copy_label( view.c_str() );
                                main->uiGamma->value( 1.0f );
                                main->uiGammaInput->value( 1.0f );
                            }
                        }
                    }
                }
                else
                {
                    for(int i = 0; i < numViews; i++)
                    {
                        std::string view = config->getView(display.c_str(), i);

                        std::string name;
                        if ( num_active_displays > 1 )
                        {
                            name = display;
                            name += "/";
                            name += view;
                        }
                        else
                        {
                            name = view;
                            name += " (" + display + ")";
                        }

                        main->gammaDefaults->add( name.c_str() );

                        if ( view == OCIO_View && !OCIO_View.empty() )
                        {
                            main->gammaDefaults->copy_label( view.c_str() );
                            main->uiGamma->value( 1.0f );
                            main->uiGammaInput->value( 1.0f );
                        }
                    }
                }
            }




            main->gammaDefaults->redraw();
        }
        catch( const OCIO::Exception& e )
        {

            LOG_ERROR( e.what() );
            use_ocio = false;
        }
        catch( const std::exception& e )
        {
            LOG_ERROR( e.what() );
            use_ocio = false;
        }


        setlocale(LC_NUMERIC, oldloc );
        av_free( oldloc );


    }
    else
    {
        use_ocio = false;
    }

    if ( use_ocio )
    {
        // @todo: handle OCIO
        //
        DBGM1( "use_OCIO" );

        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );
        try
        {

            std::vector< std::string > spaces;
            for(int i = 0; i < config->getNumColorSpaces(); ++i)
            {

                std::string csname = config->getColorSpaceNameByIndex(i);
                spaces.push_back( csname );
            }


            if ( std::find( spaces.begin(), spaces.end(),
                            OCIO::ROLE_SCENE_LINEAR ) == spaces.end() )
            {
                spaces.push_back( OCIO::ROLE_SCENE_LINEAR );

            }


            mrv::PopupMenu* w = main->uiICS;
            w->clear();
            std::sort( spaces.begin(), spaces.end() );
            size_t idx = 0;
            for ( size_t i = 0; i < spaces.size(); ++i )
            {
                const char* space = spaces[i].c_str();
                OCIO::ConstColorSpaceRcPtr cs = config->getColorSpace( space );
                const char* family = cs->getFamily();
                std::string menu;
                if ( family && strlen(family) > 0 ) {
                    menu = family; menu += "/";
                }
                menu += space;
                w->add( menu.c_str() );

                // if ( img && img->ocio_input_color_space() == space )
                // {

                //     w->copy_label( space );
                // }
            }

            for ( size_t i = 0; i < w->children(); ++i )
            {
                const Fl_Menu_Item* o = w->child(i);
                if ( !o || !o->label() ) continue;

                // if ( img->ocio_input_color_space() == o->label() )
                // {
                //     w->value(i);
                //     w->do_callback();
                //     break;
                // }
            }

            w->redraw();
        }
        catch( const OCIO::Exception& e )
        {
            LOG_ERROR( e.what() );
        }
        catch( const std::exception& e )
        {
            LOG_ERROR( e.what() );
        }

        main->uiICS->show();
        setlocale(LC_NUMERIC, oldloc );
        av_free( oldloc );

    }


    char buf[64];
    sprintf( buf, "%d", (int) uiPrefs->uiPrefsVideoThreadCount->value() );
    video_threads = buf;

    //
    // Handle file requester
    //

    Flu_File_Chooser::thumbnailsFileReq = (bool)
                                          uiPrefs->uiPrefsFileReqThumbnails->value();


    Flu_File_Chooser::singleButtonTravelDrawer = (bool)
            uiPrefs->uiPrefsFileReqFolder->value();


    native_file_chooser = uiPrefs->uiPrefsNativeFileChooser->value();

    //
    // Handle pixel values
    //

    main->uiAColorType->value( uiPrefs->uiPrefsPixelRGBA->value() );
    main->uiAColorType->redraw();

    main->uiPixelValue->value( uiPrefs->uiPrefsPixelValues->value() );
    main->uiPixelValue->redraw();

    main->uiBColorType->value( uiPrefs->uiPrefsPixelHSV->value() );
    main->uiBColorType->redraw();

    main->uiLType->value( uiPrefs->uiPrefsPixelLumma->value() );
    main->uiLType->redraw();


    //
    // Handle crop area (masking)
    //

    // int crop = uiPrefs->uiPrefsCropArea->value();
    // float mask = kCrops[crop];
    // view->masking( mask );


    //
    // Handle HUD
    //
    unsigned int hud = mrv::GLViewport::kHudNone;
    if ( uiPrefs->uiPrefsHudFilename->value() )
        hud |= mrv::GLViewport::kHudFilename;

    if ( uiPrefs->uiPrefsHudFPS->value() )
        hud |= mrv::GLViewport::kHudFPS;

    if ( uiPrefs->uiPrefsHudAVDifference->value() )
        hud |= mrv::GLViewport::kHudAVDifference;

    if ( uiPrefs->uiPrefsHudTimecode->value() )
        hud |= mrv::GLViewport::kHudTimecode;

    if ( uiPrefs->uiPrefsHudFrame->value() )
        hud |= mrv::GLViewport::kHudFrame;

    if ( uiPrefs->uiPrefsHudResolution->value() )
        hud |= mrv::GLViewport::kHudResolution;

    if ( uiPrefs->uiPrefsHudFrameRange->value() )
        hud |= mrv::GLViewport::kHudFrameRange;

    if ( uiPrefs->uiPrefsHudFrameCount->value() )
        hud |= mrv::GLViewport::kHudFrameCount;

    if ( uiPrefs->uiPrefsHudMemory->value() )
        hud |= mrv::GLViewport::kHudMemoryUse;

    if ( uiPrefs->uiPrefsHudAttributes->value() )
        hud |= mrv::GLViewport::kHudAttributes;

    if ( uiPrefs->uiPrefsHudCenter->value() )
        hud |= mrv::GLViewport::kHudCenter;


    // view->hud( (mrv::GLViewport::HudDisplay) hud );


    if ( uiPrefs->uiPrefsOverrideAudio->value() )
    {
        double x = uiPrefs->uiPrefsAudioVolume->value();
        if ( uiPrefs->uiPrefsAudioMute->value() )
            x = 0.0;
    }


    //
    // Handle fullscreen and presentation mode
    //
    if ( uiPrefs->uiWindowFixedPosition->value() )
    {
        int x = int(uiPrefs->uiWindowXPosition->value());
        int y = int(uiPrefs->uiWindowYPosition->value());
        main->uiMain->position( x, y );
    }

    if ( uiPrefs->uiWindowFixedSize->value() )
    {
        int w = int(uiPrefs->uiWindowXSize->value());
        int h = int(uiPrefs->uiWindowYSize->value());
        main->uiMain->resize( main->uiMain->x(),
                              main->uiMain->y(),
                              w, h );
    }

    //
    // Handle FPS
    //


#if 0 // defined(_WIN32) || defined(_WIN64)
    main->uiMain->resize(  main->uiMain->x(), main->uiMain->y(),
                           main->uiMain->w(), main->uiMain->h()-20 );
#endif


    Fl_Round_Button* r;
    r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(1);

    if ( r->value() == 1 )
    {

        // Fullscreen mode
        // view->toggle_fullscreen();
    }


    r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(2);

    if ( r->value() == 1 )
    {
        // Go to presentation mode - window must be shown first, thou.

         // view->toggle_presentation();
    }

    R3dScale = main->uiPrefs->uiPrefsR3DScale->value();
    BRAWScale = main->uiPrefs->uiPrefsBRAWScale->value();





    size_t idx = main->uiPrefs->uiPrefsSubtitleFont->value();
    size_t num = main->uiPrefs->uiPrefsSubtitleFont->children();
    if ( (int)idx < num )
    {

        const char* font = main->uiPrefs->uiPrefsSubtitleFont->child(idx)->label();
        if ( font )
            mrv::Media::default_subtitle_font = font;
    }
    const char* enc = main->uiPrefs->uiPrefsSubtitleEncoding->value();

    if ( enc )
        mrv::Media::default_subtitle_encoding = enc;

    // LogDisplay::prefs = (LogDisplay::ShowPreferences)
    //                     main->uiPrefs->uiPrefsRaiseLogWindowOnError->value();
    // LogDisplay::shown = false;


    main->uiMain->always_on_top( uiPrefs->uiPrefsAlwaysOnTop->value() );


    if ( debug > 1 )
        schemes.debug();
}


} // namespace mrv
