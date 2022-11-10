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
 * @file   mrvLogDisplay.cpp
 * @author gga
 * @date   Tue Oct  9 20:56:16 2007
 *
 * @brief
 *
 *
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <stdexcept>

#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl.H>

#include "mrvCore/mrvHome.h"
#include "mrvWidgets/mrvLogDisplay.h"
#include "mrViewer.h"

namespace mrv {

// Style table
static
Fl_Text_Display::Style_Table_Entry kLogStyles[] = {
    // FONT COLOR       FONT FACE   SIZE  ATTR
    // --------------- ------------ ---- ------
    {  FL_BLACK,       FL_HELVETICA, 14,   0 }, // A - Info
    {  FL_DARK_YELLOW, FL_HELVETICA, 14,   0 }, // B - Warning
    {  FL_RED,         FL_HELVETICA, 14,   0 }, // C - Error
};



    class LogData
    {
    public:
        LogData( LogDisplay* l, const char* msg, const char s ) :
            log( l ),
            message( strdup( msg ) )
            {
                size_t t = strlen(msg);
                style = (char*)malloc( t+1 );
                memset( style, s, t );
                style[t] = 0;
            }

        ~LogData()
            {
                free( message );
                free( style );
            }

    public:
        LogDisplay* log;
        char* message;
        char* style;
    };



    static void log_callback( void* v )
    {
        LogData* d = (LogData*) v;

        LogDisplay* log = d->log;
        log->style_buffer()->append( d->style );

        Fl_Text_Buffer* buffer = log->buffer();
        buffer->append( d->message );
        log->scroll( buffer->length(), 0 );

        delete d;
    }

    LogDisplay::LogDisplay( int x, int y, int w, int h, const char* l  ) :
        Fl_Text_Display( x, y, w, h, l )
    {

        color( FL_GRAY0 );

        scrollbar_align( FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT );

        wrap_mode( WRAP_AT_BOUNDS, 80 );

        delete mBuffer;
        delete mStyleBuffer;
        mBuffer = new Fl_Text_Buffer();
        mStyleBuffer = new Fl_Text_Buffer();
        highlight_data(mStyleBuffer, kLogStyles, 3, 'A', 0, 0);

        main_thread = std::this_thread::get_id();
    }

    LogDisplay::~LogDisplay()
    {
        delete mBuffer; mBuffer = NULL;
        delete mStyleBuffer; mStyleBuffer = NULL;
    }

    void LogDisplay::clear()
    {
        mStyleBuffer->text("");
        mBuffer->text("");
        redraw();
    }


    inline void LogDisplay::print( const char* x, const char style )
    {
        LogData* data = new LogData( this, x, style );
        if ( main_thread != std::this_thread::get_id() )
        {
            Fl::awake( (Fl_Awake_Handler)log_callback, data );
        }
        else
        {
            style_buffer()->append( data->style );
            buffer()->append( data->message );
            scroll( buffer()->length(), 0 );
            delete data;
        }
        
    }
    void LogDisplay::info( const char* x )
    {
        print( x, 'A' );
    }

    void LogDisplay::warning( const char* x )
    {
        print( x, 'B' );
    }

    void LogDisplay::error( const char* x )
    {
        print( x, 'C' );
    }

}
