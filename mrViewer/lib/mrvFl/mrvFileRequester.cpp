// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.




#include <FL/Fl_Progress.H>
#include <FL/Fl_Output.H>
#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Native_File_Chooser.H>

#include <boost/filesystem.hpp>


#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvString.h"
#include "mrvCore/mrvHotkey.h"

#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvSaving.h"

#include "mrvFl/FLU/Flu_File_Chooser.h"

#include "mrvApp/App.h"

#include "mrvHotkeyUI.h"
#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace fs = boost::filesystem;

namespace {

    const char* kModule = "filereq";

#ifdef _WIN32
#define kSeparator ";"
#else
#define kSeparator ":"
#endif

// File extension patterns

static const std::string kReelPattern = "otio";

static const std::string kMoviePattern = "3gp,asf,avc,avchd,avi,braw,BRAW,divx,dv,flv,m2ts,m2t,mkv,m4v,mp4,mpg,mpeg,mov,mxf,ogm,ogv,qt,r3d,R3D,ts,vob,VOB,vp9,webm,wmv,y4m";

static const std::string kImagePattern =
    "3fr,arw,bay,bmp,bmq,bit,cap,cin,cine,cr2,crw,cs1,ct,dc2,dcr,dng,dpx,dsc,drf,erf,exr,fff,gif,hdr,hdri,k25,kc2,kdc,ia,iff,iiq,jpg,jpeg,map,mef,nt,mdc,miff,mos,mrw,mt,nef,nrw,orf,pef,pic,png,ppm,pnm,pgm,pbm,psd,ptx,pxn,qtk,raf,ras,raw,rdc,rgb,rla,rpf,rw2,rwl,rwz,shmap,sgi,sr2,srf,srw,st,sti,sun,sxr,tga,tif,tiff,tx,x3f,zt";

static const std::string kProfilePattern = "icc,icm";

static const std::string kAudioPattern = "au,aiff,flac,m4a,mp3,ogg,opus,wav";

static const std::string kSubtitlePattern = "srt,sub,ass,vtt";

static const std::string kOCIOPattern = "ocio";


// Actual FLTK file requester patterns



// static const std::string kSAVE_IMAGE_PATTERN = _("Images (*.{") +
//                                                kImagePattern + "})";

}


namespace mrv
{

// WINDOWS crashes if pattern is sent as is.  We need to sanitize the
// pattern here.  From:
// All (*.{png,ogg})\t
// to:
// All\t{*.png,ogg}\n
std::string pattern_to_native( std::string pattern )
{
    std::string ret;
    size_t pos = 0, pos2 = 0;
    while ( pos != std::string::npos )
    {
        pos = pattern.find( ' ' );
        if ( pos == std::string::npos ) break;
        ret += pattern.substr( 0, pos ) + '\t';
        size_t pos3 = pattern.find( '(', pos );
        size_t pos2 = pattern.find( ')', pos );
        ret += pattern.substr( pos3 + 1, pos2 - pos3 - 1 ) + '\n';
        if ( pos2 + 2 < pattern.size() )
            pattern = pattern.substr( pos2 + 2, pattern.size() );
        else
            pattern.clear();
    }
    return ret;
}

    const std::string file_save_single_requester(
        const std::shared_ptr<tl::system::Context>& context,
        const char* title,
        const char* pattern,
        const char* startfile,
        const bool compact_images = true
        )
{
    std::string file;
    try
    {
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.filename() ) {
                     file = native.filename();
                  }
           }
        }
        else
        {
            const char* f = flu_save_chooser( context, title, pattern, startfile, compact_images );
            if ( !f ) {
                return "";
            }
            file = f;
        }
    }
    catch ( const std::exception& e )
    {
    }

    return file;
}

stringArray file_multi_requester(
    const std::shared_ptr<tl::system::Context>& context,
    const char* title,
    const char* pattern,
    const char* startfile,
    const bool compact_images
)
{
    stringArray filelist;

    try
    {
        if ( !startfile ) startfile = "";
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.count() > 0 ) {
                      for ( int i = 0; i < native.count(); ++i )
                      {
                          filelist.push_back( native.filename(i) );
                      }
                  }
             }
        }
        else
        {
            flu_multi_file_chooser( context, title, pattern, startfile,
                                    filelist, compact_images);
        }
    }
    catch ( const std::exception& e )
    {
    }
    catch ( ... )
    {
    }
    return filelist;
}

std::string file_single_requester(
    const std::shared_ptr<tl::system::Context>& context,
    const char* title,
    const char* pattern,
    const char* startfile,
    const bool compact_files
)
{
    std::string file;
    try {
        if ( !startfile ) startfile = "";
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.count() > 0 )
                  {
                      file = native.filename();
                  }
           }
        }
        else
        {
            const char* f = flu_file_chooser( context, title, pattern, startfile, compact_files );
            if ( f ) file = f;
        }
    }
    catch ( const std::exception& e )
    {
    }
    catch ( ... )
    {
    }


    return file;
}


/**
 * Opens a file requester to load a directory of image files
 *
 * @param startfile  start filename (directory)
 *
 * @return A directory to be opened or NULL
 */
std::string open_directory( const char* startfile, ViewerUI* ui )
{
    std::string dir;
    std::string title = _("Load Directory");
    bool native = mrv::Preferences::native_file_chooser;
    if ( native )
    {
        Fl_Native_File_Chooser native;
        native.title( title.c_str() );
        native.directory(startfile);
        native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
        // Show native chooser
        switch ( native.show() ) {
            case -1:
                break;	// ERROR
            case  1:
                break;		// CANCEL
            default:  // PICKED DIR
                if ( native.filename() ) {
                    dir = native.filename();
                }
                break;
        }
    }
    else
    {
        const auto& context = ui->app->getContext();
        const char* d = flu_dir_chooser( context, title.c_str(), startfile );
        if (d) dir = d;
    }
    fs::path path = dir;
    return path.generic_string();
}

/**
 * Opens a file requester to load image files
 *
 * @param startfile  start filename (directory)
 *
 * @return Each file to be opened
 */
stringArray open_image_file( const char* startfile, const bool compact_images,
                             ViewerUI* ui )
{
    const std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                      kReelPattern + "})\t";
    const std::string kAUDIO_PATTERN = ("Audios (*.{") + kAudioPattern +
                                       "})\t";
    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})\t";
    const std::string kALL_PATTERN = _("All (*.{") +
                                     kImagePattern + "," + kMoviePattern +
                                     "," + kReelPattern + "," +
                                     kAudioPattern + "," +
                                     kReelPattern + "})\t" +
                                     kIMAGE_PATTERN +
                                     kAUDIO_PATTERN +
                                     _("Movies (*.{") + kMoviePattern +
                                     "})\t" + kREEL_PATTERN;

    std::string pattern = kIMAGE_PATTERN + kAUDIO_PATTERN;
    std::string title = _("Load Image");
    if ( compact_images ) {
        title = _("Load Movie or Sequence");
        pattern = kALL_PATTERN;
    }

    const auto& context = ui->app->getContext();
    return file_multi_requester( context, title.c_str(), pattern.c_str(),
                                 startfile, compact_images );
}

/**
   * Opens a file requester to load a lut file.
   *
   * @param startfile  start filename (directory)
   * 
   * @return  opened lut file or empty
   */
std::string open_lut_file( const char* startfile,
                           ViewerUI* ui )
{
    std::string lut_pattern;
    for (const auto& i : tl::timeline::getLUTFormatExtensions())
    {
        if ( ! lut_pattern.empty() ) lut_pattern += ",";
        lut_pattern += i.substr( 1, i.size() ); // no period
    }
    std::string kLUT_PATTERN = _( "LUTS (*.{" ) + lut_pattern + "})\t";

    std::string title = _("Load LUT");

    const auto& context = ui->app->getContext();
    return file_single_requester( context, title.c_str(),
                                  kLUT_PATTERN.c_str(),
                                  startfile, false );
}


/**
   * Opens a file requester to load subtitle files
   *
   * @param startfile  start filename (directory)
   *
   * @return  opened subtitle file or empty
   */
std::string open_subtitle_file( const char* startfile,
                                ViewerUI* ui )
{
    std::string kSUBTITLE_PATTERN = _( "Subtitles (*.{" ) +
                                    kSubtitlePattern + "})\n";

    std::string title = _("Load Subtitle");


    const auto& context = ui->app->getContext();
    return file_single_requester( context, title.c_str(),
                                  kSUBTITLE_PATTERN.c_str(),
                                  startfile, false );
}

/**
   * Opens a file requester to load audio files
   *
   * @param startfile  start filename (directory)
   *
   * @return  opened audio file or null
   */
std::string open_audio_file( const char* startfile,
                             ViewerUI* ui )
{
    std::string kAUDIO_PATTERN = _( "Audios (*.{" ) +
                                 kAudioPattern + "})";

    std::string title = _("Load Audio");

    const auto& context = ui->app->getContext();
    return file_single_requester( context, title.c_str(),
                                  kAUDIO_PATTERN.c_str(),
                                  startfile, true );
}



void save_sequence_file( ViewerUI* ui,
                         const char* startdir, const bool opengl)
{
    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})\t";
    const std::string kALL_PATTERN = _("All (*.{") +
                                     kImagePattern + "," + kMoviePattern
                                     + "})\t" +
                                     kIMAGE_PATTERN +
                                     _("Movies (*.{") + kMoviePattern +
                                     "})\t";

    std::string title = _("Save Sequence");
    if ( opengl ) title = _("Save GL Snapshot");

    stringArray filelist;
    if ( !startdir ) startdir = "";

    const auto& context = ui->app->getContext();
    const std::string file = file_save_single_requester( context,
                                                         title.c_str(),
                                                         kALL_PATTERN.c_str(),
                                                         startdir, true );
    if ( file.empty() ) return;

    // save_movie_or_sequence( file.c_str(), ui, opengl );

}




void save_movie_file( ViewerUI* ui, const char* startdir)
{
    const std::string kMOVIE_PATTERN = _("Movies (*.{") +
                                       kMoviePattern
                                       + "})";

    std::string title = _("Save Movie");

    if ( !startdir ) startdir = "";

    const auto& context = ui->app->getContext();
    const std::string file = file_save_single_requester( context,
                                                         title.c_str(),
                                                         kMOVIE_PATTERN.c_str(),
                                                         startdir, true );
    if ( file.empty() ) return;
    
    save_movie( file, ui );
}




std::string open_ocio_config( const char* startfile,
                              ViewerUI* ui )
{
    std::string kOCIO_PATTERN = _("OCIO config (*.{") +
                                kOCIOPattern + "})";
    std::string title = _("Load OCIO Config");

    const auto& context = ui->app->getContext();
    std::string file = file_single_requester( context, title.c_str(),
                                              kOCIO_PATTERN.c_str(),
                                              startfile, false );
    return file;
}


void save_hotkeys( Fl_Preferences& keys )
{
    keys.set( "version", 11 );
    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
        keys.set( (hotkeys[i].name + " ctrl").c_str(),
                  hotkeys[i].hotkey.ctrl );
        keys.set( (hotkeys[i].name + " alt").c_str(),
                  hotkeys[i].hotkey.alt );
        keys.set( (hotkeys[i].name + " meta").c_str(),
                  hotkeys[i].hotkey.meta );
        keys.set( (hotkeys[i].name + " shift").c_str(),
                  hotkeys[i].hotkey.shift );
        keys.set( (hotkeys[i].name + " key").c_str(),
                  (int)hotkeys[i].hotkey.key );
        keys.set( (hotkeys[i].name + " text").c_str(),
                  hotkeys[i].hotkey.text.c_str() );

    }
}


void save_hotkeys( ViewerUI* ui, std::string filename )
{

    const auto& context = ui->app->getContext();
    //
    // Hotkeys
    //
    size_t pos = filename.rfind( ".prefs" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    pos = filename.rfind( ".keys" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    std::string path = prefspath() + filename + ".keys.prefs";
    std::string title = _("Save Hotkeys Preferences");
    filename = file_save_single_requester(context,
                                          title.c_str(),
                                          _("Hotkeys (*.{keys.prefs})"),
                                          path.c_str() );
    if ( filename.empty() ) return;

    size_t start = filename.rfind( '/' );
    if ( start == std::string::npos )
        start = filename.rfind( '\\' );;
    if ( start != std::string::npos )
        filename = filename.substr( start+1, filename.size() );

    pos = filename.rfind( ".prefs" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    pos = filename.rfind( ".keys" );
    if ( pos == std::string::npos ) filename += ".keys";

    Preferences::hotkeys_file = filename;
    Fl_Preferences keys( prefspath().c_str(), "filmaura",
                         filename.c_str() );
    save_hotkeys( keys );

    HotkeyUI* h = ui->uiHotkey;
    h->uiHotkeyFile->value( mrv::Preferences::hotkeys_file.c_str() );

    // Update menubar in case some hotkey shortcut changed
    ui->uiMain->fill_menu( ui->uiMenuBar );
}


void load_hotkeys( ViewerUI* ui, std::string filename )
{
    const auto& context = ui->app->getContext();
    size_t pos = filename.rfind( ".prefs" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    pos = filename.rfind( ".keys" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    std::string path = prefspath() + filename + ".keys.prefs";
    std::string title = _("Load Hotkeys Preferences");
    filename = file_single_requester(context, title.c_str(),
                                     _("Hotkeys (*.{keys.prefs})"),
                                     path.c_str(), false );
    if ( filename.empty() ) return;

    size_t start = filename.rfind( '/' );
    if ( start == std::string::npos )
        start = filename.rfind( '\\' );
    if ( start != std::string::npos )
        filename = filename.substr( start+1, filename.size() );


    if ( ! fs::exists( prefspath() + '/' + filename ) )
    {
        LOG_ERROR( _("Hotkeys file ") << prefspath() << '/' << filename
                   << _(" does not exist!") );
        return;
    }


    pos = filename.rfind( ".prefs" );
    if ( pos != std::string::npos )
        filename = filename.replace( pos, filename.size(), "" );

    Preferences::hotkeys_file = filename;


    Fl_Preferences* keys = new Fl_Preferences( prefspath().c_str(),
                                               "filmaura",
                                               filename.c_str() );
    load_hotkeys( ui, keys );
}

void load_hotkeys( ViewerUI* ui, Fl_Preferences* keys )
{
    int version = 0;
    keys->get( "version", version, 11 );
    int tmp = 0;
    char  tmpS[2048];

    if ( fs::exists( prefspath() + Preferences::hotkeys_file + ".prefs" ) )
    {
        for ( int i = 0; hotkeys[i].name != "END"; ++i )
        {
            if ( hotkeys[i].force == true ) continue;
            hotkeys[i].hotkey.shift = hotkeys[i].hotkey.ctrl =
              hotkeys[i].hotkey.alt = hotkeys[i].hotkey.meta = false;
            hotkeys[i].hotkey.key = 0;
            hotkeys[i].hotkey.text.clear();
        }
    }

    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
        // If version >= 1 of preferences, do not set scrub

        Hotkey saved = hotkeys[i].hotkey;

        keys->get( (hotkeys[i].name + " key").c_str(),
                   tmp, (int)hotkeys[i].hotkey.key );
        if (tmp) hotkeys[i].force = false;
        hotkeys[i].hotkey.key = unsigned(tmp);

        keys->get( (hotkeys[i].name + " text").c_str(),
                   tmpS,
                   hotkeys[i].hotkey.text.c_str(), 16 );
        if ( strlen(tmpS) > 0 ) {
            hotkeys[i].force = false;
            hotkeys[i].hotkey.text = tmpS;
        }
        else hotkeys[i].hotkey.text.clear();

        if ( hotkeys[i].force ) {
            hotkeys[i].hotkey = saved;
            continue;
        }
        keys->get( (hotkeys[i].name + " ctrl").c_str(),
                   tmp, (int)hotkeys[i].hotkey.ctrl );
        if ( tmp ) hotkeys[i].hotkey.ctrl = true;
        else       hotkeys[i].hotkey.ctrl = false;
        keys->get( (hotkeys[i].name + " alt").c_str(),
                   tmp, (int)hotkeys[i].hotkey.alt );
        if ( tmp ) hotkeys[i].hotkey.alt = true;
        else       hotkeys[i].hotkey.alt = false;

        keys->get( (hotkeys[i].name + " meta").c_str(),
                   tmp, (int)hotkeys[i].hotkey.meta );
        if ( tmp ) hotkeys[i].hotkey.meta = true;
        else       hotkeys[i].hotkey.meta = false;

        keys->get( (hotkeys[i].name + " shift").c_str(),
                   tmp, (int)hotkeys[i].hotkey.shift );
        if ( tmp ) hotkeys[i].hotkey.shift = true;
        else       hotkeys[i].hotkey.shift = false;


        for ( int j = 0; hotkeys[j].name != "END"; ++j )
        {
            bool view3d = false;
            if ( j < 4 ) view3d = true;

            if ( hotkeys[j].hotkey == hotkeys[i].hotkey && j != i &&
                 (i > 4 && !view3d) &&
                 hotkeys[j].hotkey.to_s() != "[" &&
                 hotkeys[j].hotkey.to_s() != "]" )
            {
                LOG_ERROR( _("Corruption in hotkeys preferences. ")
                           << _("Hotkey '") << hotkeys[j].hotkey.to_s()
                           << _("' for ") << _(hotkeys[j].name.c_str())
                           << _(" will not be available.  ")
                           << _("Already used in ")
                           << _(hotkeys[i].name.c_str()) );
                hotkeys[j].hotkey = Hotkey();
            }
        }
    }

    HotkeyUI* h = ui->uiHotkey;
    h->uiHotkeyFile->value( mrv::Preferences::hotkeys_file.c_str() );
    fill_ui_hotkeys( h->uiFunction );
}
}  // namespace mrv
