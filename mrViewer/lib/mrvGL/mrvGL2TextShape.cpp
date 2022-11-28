// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/fl_draw.H>

#include "mrvGLShape.h"


namespace mrv {

#ifdef USE_OPENGL2

    bool GL2TextShape::setRasterPos( double x, double y, size_t pos )
    {
        GLboolean result = GL_TRUE;

        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double width  = gl_width( txt.c_str(), pos ) / viewZoom;
            double height = (gl_height() / viewZoom);
            double xMove, yMove, bxMove, byMove;
            xMove = width;
            yMove = height;
            bxMove = -xMove * pixels_per_unit * viewZoom;
            byMove = -yMove * pixels_per_unit * viewZoom;
            glRasterPos2d( x + xMove, y + yMove );
            result = GL_TRUE;
            glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            if ( result == GL_FALSE )
            {
                // Probably bottom right corner, don't offset x.
                bxMove = 0;
                glRasterPos2d( x, y + yMove );
                glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            }
            glBitmap( 0, 0, 0, 0, bxMove, byMove, NULL );
        }
        return (bool)result;
    }
    
    void GL2TextShape::draw( 
        const std::shared_ptr<timeline::IRender>& render )
    {
        int textSize = int( fontSize * viewZoom );
        if ( text.empty() || textSize < 1 ) return;
        
        //Turn on Color Buffer and Depth Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        //Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);

        //Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glEnable( GL_BLEND );

        // We need to flip the FLTK projection matrix in Y
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( matrix.e );

        // So compositing works properly
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
        glColor4f( color.r, color.g, color.b, color.a );

        gl_font(font, textSize );

        double height = (gl_height() / viewZoom);

        // Cioy text to process it line by line
        txt = text;

        GLboolean result;
        std::size_t pos = txt.find('\n');
        double x = pts[0].x;
        double y = pts[0].y;
        for ( ; pos != std::string::npos; y -= height, pos = txt.find('\n') )
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

