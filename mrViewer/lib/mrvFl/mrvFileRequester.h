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
/**
 * @file   mrvFileRequester.h
 * @author gga
 * @date   Fri Jul  6 17:38:52 2007
 *
 * @brief  This file defines several helper functions for file requesters.
 *
 *
 */

#pragma once

#include <mrvCore/mrvString.h>

#include <FL/Fl_Preferences.H>

class ViewerUI;

namespace mrv
{


std::string open_directory( const char* startfile,
                            ViewerUI* main );

std::string open_session( const char* startfile,
                          ViewerUI* main );

/**
 * Opens a file requester to load a reel
 *
 * @param startfile directory to start from
 *
 * @return opened reel(s)
 */
stringArray open_reel( const char* startfile,
                       ViewerUI* main );

/**
 * Opens a file requester to load an image
 *
 * @param startfile start filename (directory)
 *
 * @return opened filename(s)
 */
stringArray open_image_file( const char* startfile,
                             const bool compact_files,
                             ViewerUI* main );


// void attach_ocio_input_color_space( CMedia* img, ImageView* view );
// void attach_ocio_display( CMedia* img, ImageView* view );
// void attach_ocio_view( CMedia* img, ImageView* view );

/**
 * Opens a file requester to load a lut file
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened subtitle file or empty
 */
std::string open_lut_file( const char* startfile,
                           ViewerUI* main );

/**
 * Opens a file requester to load a subtitle file
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened subtitle file or empty
 */
    std::string open_subtitle_file( const char* startfile,
                                    ViewerUI* main  );

/**
 * Opens a file requester to load audio files
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened audio file or null
 */
    std::string open_audio_file( const char* startfile,
                                 ViewerUI* main );


std::string open_ocio_config( const char* startfile,
                              ViewerUI* main  );

    void load_hotkeys( ViewerUI* uiMain, std::string filename = "" );
    void load_hotkeys( ViewerUI* uiMain, Fl_Preferences* prefs );
    void save_hotkeys( ViewerUI* uiMain, std::string filename = "" );
    void save_hotkeys( Fl_Preferences& keys );

} // namespace mrv
