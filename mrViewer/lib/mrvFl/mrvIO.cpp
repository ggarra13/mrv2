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
 * @file   mrvIO.cpp
 * @author gga
 * @date   Wed Jul 11 12:04:58 2007
 *
 * @brief
 *
 *
 */


#include <cstring> // for strcpy

extern "C" {
#include "libavutil/mem.h"
}

#include "mrvCore/mrvHome.h"
// #include "mrvFl/mrvLogDisplay.h"
// #include "mrViewer.h" // for uiLog
#include "mrvFl/mrvAsk.h"
#include "mrvFl/mrvIO.h"

namespace mrv {

static char* _alert = NULL;

void alert( const char* str )
{
    av_free( _alert );
    if ( str == NULL )
    {
        _alert = NULL;
        return;
    }

    _alert = (char*) av_malloc( strlen(str) + 1 );
    strcpy( _alert, str );
    _alert[strlen(str)] = 0;

    mrv::fl_alert( "%s", _alert );
}

const char* alert()
{
    return _alert;
}


namespace trace
{



std::fstream logbuffer::out;
bool logbuffer::_debug = true;

logbuffer::~logbuffer() {
    if (out.is_open()) out.close();
};

int logbuffer::sync()
{
    if ( ! pbase() ) return 0;


    // make sure to null terminate the string
    sputc('\0');

    // freeze and call the virtual print method
    char* c = av_strdup( str().c_str() );
    if (!c) return 1;

    if ( _debug && out.is_open() ) out << c << std::flush;

    print( c );

    av_free(c);

    // reset iterator to first position & unfreeze
    seekoff( 0, std::ios::beg );
    return 0;
}

void logbuffer::open_stream()
{
    if ( !out.is_open() )
    {
        std::string file = mrv::homepath();
        file += "/.filmaura/errorlog.txt";
        std::cerr << file << std::endl;
        out.open( file.c_str(), std::ios_base::out );
    }
    if ( out.is_open() )
    {
        out << "DEBUG LOG" << std::endl
            << "=========" << std::endl << std::endl;
    }
}

void errorbuffer::print( const char* c )
{
    std::cerr << c << std::flush;

    // Send string to Log Window
    // if ( ViewerUI::uiLog && ViewerUI::uiLog->uiLogText )
    // {
    //     ViewerUI::uiLog->uiLogText->error( c );
    // }
}

void warnbuffer::print( const char* c )
{
    std::cerr << c << std::flush;

    // Send string to Log Window
    // if ( ViewerUI::uiLog && ViewerUI::uiLog->uiLogText )
    // {
    //     ViewerUI::uiLog->uiLogText->warning( c );
    // }
}

void infobuffer::print( const char* c )
{
    std::cout << c << std::flush;

    // Send string to Log Window
    // if ( ViewerUI::uiLog && ViewerUI::uiLog->uiLogText )
    // {
    //     ViewerUI::uiLog->uiLogText->info( c );
    // }
}

void connbuffer::print( const char* c )
{
    std::cout << c << std::flush;

    // Send string to Log Window in Connection panel
    // if ( ViewerUI::uiConnection && ViewerUI::uiConnection->uiLog )
    // {
    //     ViewerUI::uiConnection->uiLog->info( c );
    //     ViewerUI::uiConnection->uiLog->redraw();
    // }
}

connstream  conn;
infostream  info;
warnstream  warn;
errorstream error;
}

}
