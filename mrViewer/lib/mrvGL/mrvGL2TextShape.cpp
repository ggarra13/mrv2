


#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/fl_draw.H>


#include "mrvGLShape.h"


namespace {
const char* kModule = N_("gl2text");
}




namespace mrv {


void GL2TextShape::draw( 
    const std::shared_ptr<timeline::IRender>& render )
{
    //Turn on Color Buffer and Depth Buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    //Only write to the Stencil Buffer where 1 is not set
    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);

    //Keep the content of the Stencil Buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_DITHER );
    glDisable( GL_LIGHTING );
    glEnable( GL_BLEND );

    glActiveTexture( GL_TEXTURE0 );

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glColor4f( color.r, color.g, color.b, color.a );


    // int textsize = int( size() * z );
    // if ( textsize < 1 ) return;


    gl_font(font(), textsize );

    double height = (gl_height() / z);

    std::string txt = text();

    GLboolean result;
    std::size_t pos = txt.find('\n');
    double x = double( pts[0].x );
    double y = double( pts[0].y );
    for ( ; pos != std::string::npos; y -= height, pos = txt.find('\n') )
    {
        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double xMove = gl_width( txt.c_str(), pos ) / z;
            double yMove = height;
            double bxMove = -xMove * m * z;
            double byMove = -yMove * m * z;
            glRasterPos2d( x + xMove, y + yMove );
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
        if ( result == GL_GL_TRUE )
            gl_draw(txt.c_str(), pos );
        if ( txt.size() > pos )
            txt = txt.substr( pos+1, txt.size() );
    }
    if ( !txt.empty() )
    {
        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double xMove = gl_width( txt.c_str() ) / z;
            double yMove = height;
            double bxMove = -xMove * m * z;
            double byMove = -yMove * m * z;
            glRasterPos2d( x + xMove, y + yMove );
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
        if ( result == GL_TRUE )
            gl_draw( txt.c_str() );
    }

}


} // namespace mrv
