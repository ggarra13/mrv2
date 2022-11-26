
#include <cstring> // for strcpy

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvOS.h"

#include "mrvFl/mrvIO.h"

namespace mrv {



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
    char* c = strdup( str().c_str() );
    if (!c) return 1;

    if ( _debug && out.is_open() ) out << c << std::flush;

    print( c );

    free(c);

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
