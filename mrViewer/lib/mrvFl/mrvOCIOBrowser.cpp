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
 * @file   mrvOCIOBrowser.cpp
 * @author gga
 * @date   Mon Jul  2 08:11:24 2007
 *
 * @brief
 *
 *
 */

extern "C" {
    #include <libavutil/mem.h>
}


#include <vector>
#include <string>


#include "mrvCore/mrvMedia.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvPreferences.h"

#include "mrvFl/mrvOCIOBrowser.h"


namespace
{
const char* kModule = "ocio";
}


namespace mrv {

OCIOBrowser::OCIOBrowser(int x, int y, int w, int h, const char* l) :
Fl_Browser( x, y, w, h, l ),
_type( kNone )
{
    type( FL_HOLD_BROWSER );
    textcolor( FL_BLACK );
    //   when( FL_WHEN_RELEASE );
}

OCIOBrowser::~OCIOBrowser()
{
}

void OCIOBrowser::fill_view()
{
    OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
    const char* display = Preferences::OCIO_Display.c_str();
    std::vector< std::string > views;
    int numViews = config->getNumViews(display);
    for(int i = 0; i < numViews; i++)
    {
        std::string view = config->getView(display, i);
        views.push_back( view );
    }

    value(1);
    std::sort( views.begin(), views.end() );
    for ( size_t i = 0; i < views.size(); ++i )
    {
        add( views[i].c_str() );
        if ( views[i] == _sel )
        {
            value(i+1);
        }
    }
}

void OCIOBrowser::fill_display()
{
    OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
    std::vector< std::string > displays;
    for(int i = 0; i < config->getNumDisplays(); ++i)
    {
        std::string display = config->getDisplay(i);
        displays.push_back( display );
    }

    value(1);
    std::sort( displays.begin(), displays.end() );
    for ( size_t i = 0; i < displays.size(); ++i )
    {
        add( displays[i].c_str() );
        if ( displays[i] == _sel )
        {
            value(i+1);
        }
    }
}

void OCIOBrowser::fill_input_color_space()
{
    OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
    std::vector< std::string > spaces;
    for(int i = 0; i < config->getNumColorSpaces(); ++i)
    {
        std::string csname = config->getColorSpaceNameByIndex(i);
        spaces.push_back( csname );
    }

    if ( std::find( spaces.begin(), spaces.end(), OCIO::ROLE_SCENE_LINEAR ) ==
         spaces.end() )
    {
        spaces.push_back( OCIO::ROLE_SCENE_LINEAR );
    }

    std::sort( spaces.begin(), spaces.end() );
    value(1);
    for ( size_t i = 0; i < spaces.size(); ++i )
    {
        const char* space = spaces[i].c_str();
        OCIO::ConstColorSpaceRcPtr cs = config->getColorSpace( space );
        add( space );  // was w = add( space ) @TODO: fltk1.4 impossible
        //w->tooltip( strdup( cs->getDescription() ) );
        if ( spaces[i] == _sel )
        {
            value(i+1);
        }
    }
}


int OCIOBrowser::handle( int event )
{
    return Fl_Browser::handle( event );
}

void OCIOBrowser::fill()
{
    this->clear();

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    switch( _type )
    {
    case kInputColorSpace:
        fill_input_color_space();
        break;
    case kView:
        fill_view();
        break;
    case kDisplay:
        fill_display();
        break;
    default:
        LOG_ERROR( _("Unknown type for mrvOCIOBrowser") );
    }

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );
}


} // namespace mrv
