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


#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/fl_draw.H>


#include "mrvGLShape.h"

namespace {
    const char* kModule = "gl2text";
}

//#define CHECK_CLIPPING


namespace mrv {

#ifdef USE_OPENGL2

    bool GL2TextShape::setRasterPos( double x, double y, size_t pos )
    {
        GLboolean result = GL_TRUE;

        double height = (gl_height() / zoom);
        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double width  = gl_width( txt.c_str(), pos ) / zoom;
            double xMove, yMove, bxMove, byMove;
            xMove = width;
            yMove = height;
            bxMove = -xMove * m * zoom;
            byMove = yMove * m * zoom;
#ifdef CHECK_CLIPPING
            std::cerr << "**** xMove=" << xMove << " yMove="
                      << yMove << std::endl;
#endif 
            glRasterPos2d( x + xMove, y + yMove );
            result = GL_TRUE;
            glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            if ( result == GL_FALSE )
            {
                byMove = -byMove;
                yMove = -yMove;
                glRasterPos2d( x + xMove, y + yMove );
                result = GL_TRUE;
                glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
                if ( result == GL_FALSE )
                {
                    bxMove = 0;
                    glRasterPos2d( x, y + yMove );
                    result = GL_TRUE;
                    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,
                                  &result);
                }
            }
#ifdef CHECK_CLIPPING
            std::cerr << ">>>> xMove=" << xMove << " yMove="
                      << yMove << std::endl;
            std::cerr << "txt= " << txt << " bitmap bxMove=" << bxMove
                      << " byMove=" << byMove << std::endl;
#endif
            glBitmap( 0, 0, 0, 0, bxMove, byMove, NULL );
        }
        return (bool)result;
    }
    
    void GL2TextShape::draw( 
        const std::shared_ptr<timeline::IRender>& render )
    {
        int textSize = int( fontSize * zoom );
        if ( text.empty() || textSize < 1 ) return;
        
        //Turn on Color Buffer and Depth Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        //Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);

        //Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glEnable( GL_BLEND );

        glLoadMatrixf( matrix.e );

        // So compositing works properly
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
        glColor4f( color.r, color.g, color.b, color.a );

        gl_font(font, textSize );

        double height = (gl_height() / zoom);

        // Cioy text to process it line by line
        txt = text;

        GLboolean result;
        std::size_t pos = txt.find('\n');
        double x = pts[0].x;
        double y = pts[0].y;
        for ( ; pos != std::string::npos; y += height, pos = txt.find('\n') )
        {
            result = setRasterPos( x, y, pos );
            if ( result ) gl_draw(txt.c_str(), pos );
            if ( txt.size() > pos ) txt = txt.substr( pos+1, txt.size() );
        }
        if ( !txt.empty() )
        {
            result = setRasterPos( x, y, txt.size() );
            if ( result ) gl_draw( txt.c_str() );
        }
    }
#endif
    
}

