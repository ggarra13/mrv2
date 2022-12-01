// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvColorSchemes.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "colors";
}

namespace mrv
{
    std::string ColorSchemes::remove_comments( std::string line )
    {
        size_t pos = line.find( "#" );
        if ( pos != std::string::npos )
        {
            return line.substr( 0, pos );
        }
        return line;
    }

    ColorSchemes::ColorSchemes()
    {
    }

    bool
    ColorSchemes::read_themes( const char* file )
    {
        FILE* f = fl_fopen( file, "r" );
        if (!f) {
            return false;
        }
        filename = file;
        char line[256];
        while ( fgets( line, sizeof(line), f ) != NULL )
        {
            std::string text = remove_comments( line );
            size_t pos = text.find( "theme" );
            if ( pos != std::string::npos )
            {
                pos = text.find( '"', pos+1 );
                size_t pos2 = text.find( '"', pos+1 );
                if ( pos2 == std::string::npos )
                {
                    pos2 = text.size();
                }

                std::string name = text.substr( pos+1, pos2-pos-1 );
                themes.push_back( Theme( name ) );


                while ( fgets( line, sizeof(line), f ) != NULL )
                {
                    text = line;
                    if ( text.find( '{' ) != std::string::npos )
                        break;
                }

                if ( ! read_colors( f, themes.back() ) )
                    return false;
            }
        }
        fclose( f );


        return true;
    }

    bool ColorSchemes::read_colors( FILE* f, Theme& theme )
    {
        char line[256];
        while ( fgets( line, sizeof(line), f ) != NULL )
        {
            std::string text = remove_comments( line );

            size_t pos = text.rfind( '}' );
            if ( pos != std::string::npos ) break;

            char cmap[24];
            int idx, ri, gi, bi;
            int num = sscanf( text.c_str(), "%s %d %d %d %d",
                              cmap, &idx, &ri, &gi, &bi );
            if ( num != 5 ) {
                continue;
            }

            uchar r, g, b;
            r = (uchar)ri; g = (uchar)gi; b = (uchar)bi;
            Fl_Color c = fl_rgb_color( r, g, b );
            theme.colormaps.insert( std::make_pair( idx, c ) );
        }

        return true;
    }

    void ColorSchemes::apply_colors( std::string n )
    {
        char buf[16];

        DBGM0( "Apply Default scheme" );
        for ( auto& s : themes )
        {
            if ( s.name == _("Default") ||
                 s.name == "Default" )
            {
                for ( auto& c: s.colormaps )
                {
                    sprintf( buf, "%08x", c.second );
                    Fl::set_color( c.first, Fl_Color(c.second) );
                }
            }
        }

        DBGM0( "Apply " << n << " scheme" );
        for ( auto& s : themes )
        {
            if ( s.name == n )
            {
                name = n;
                for ( auto& c: s.colormaps )
                {
                    sprintf( buf, "%08x", c.second );
                    Fl::set_color( c.first, Fl_Color(c.second) );
                }
            }
        }
        Fl::set_color(FL_FREE_COLOR, 0, 0, 0, 80);
    }

    void
    ColorSchemes::reload_theme( std::string t )
    {
        themes.clear();
        read_themes( filename.c_str() );
        apply_colors( t );
    }

    void
    ColorSchemes::debug()
    {
        char buf[16];
        for ( int i = 0; i < 256; ++i )
        {
            Fl_Color c = Fl::get_color( i );
            sprintf( buf, "%08x", c );
        }
    }
}
