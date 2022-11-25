


#include <cmath>
#include <limits>

#include <tlCore/Mesh.h>


#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/gl.H>


#include "mrvGL/mrvHistogram.h"
#include "mrvGL/mrvGLViewport.h"

#include "mrvFl/mrvIO.h"
#include "mrViewer.h"

namespace mrv
{


    Histogram::Histogram( int X, int Y, int W, int H, const char* L ) :
        Fl_Gl_Window( X, Y, W, H, L ),
        _channel( kRGB ),
        _histtype( kLog ),
        maxLumma( 0 ),
        maxColor( 0 )
    {
        mode( FL_RGB | FL_DOUBLE | FL_ALPHA );
    }


    void Histogram::draw_grid(const math::BBox2i& r)
    {
        glDisable( GL_BLEND );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        glClearColor( 0, 0, 0, 0 );
        glClear( GL_COLOR_BUFFER_BIT );
    }

    void Histogram::draw()
    {
        if ( !valid() )
        {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport( 0, 0, pixel_w(), pixel_h() );
            glOrtho( 0, w(), 0, h(), -1, 1 );

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            glShadeModel( GL_FLAT );
            glDisable( GL_LIGHTING );
            
            valid(1);
        }

        math::BBox2i r( 0, 0, w(), h() );
        draw_grid(r);
        draw_pixels(r);
    }


    void Histogram::count_pixel( const uint8_t* rgb )
    {
        uint8_t r = rgb[0];
        uint8_t g = rgb[1];
        uint8_t b = rgb[2];

        ++red[ r ];
        if ( red[r] > maxColor ) maxColor = red[r];

        ++green[ g ];
        if ( green[g] > maxColor ) maxColor = green[g];

        ++blue[ b ];
        if ( blue[b] > maxColor ) maxColor = blue[b];

        unsigned int lum = unsigned(r * 0.30f + g * 0.59f + b * 0.11f);
        lumma[ lum ] += 1;
        if ( lumma[lum] > maxLumma ) maxLumma = lumma[lum];
    }


    
    
    void Histogram::update( const area::Info& info )
    {
        GLViewport* view = ui->uiView;
        auto renderSize = view->getRenderSize();
        const imaging::Color4f* image = view->image();
        if (!image) {
            tooltip( _("Mark an area in the image with the left mouse button") );
            return;
        }


        maxColor = maxLumma = 0;
        memset( red,   0, sizeof(float) * 256 );
        memset( green, 0, sizeof(float) * 256 );
        memset( blue,  0, sizeof(float) * 256 );
        memset( lumma, 0, sizeof(float) * 256 );

        int xmin = info.box.min.x;
        int ymin = info.box.min.y;
        int xmax = info.box.max.x;
        int ymax = info.box.max.y;


        unsigned int stepY = (ymax - ymin + 1) / w();
        unsigned int stepX = (xmax - xmin + 1) / h();
        if ( stepX < 1 ) stepX = 1;
        if ( stepY < 1 ) stepY = 1;


        imaging::Color4f* pixel;
        uint8_t rgb[3];
        for ( int Y = ymin; Y <= ymax; Y += stepY )
        {
            for ( int X = xmin; X <= xmax; X += stepX )
            {
                const auto& pixel = image[ X + Y * renderSize.w];
                rgb[0] = (uint8_t)Imath::clamp(pixel.b * 255.0f, 0.f, 255.f);
                rgb[1] = (uint8_t)Imath::clamp(pixel.g * 255.0f, 0.f, 255.f);
                rgb[2] = (uint8_t)Imath::clamp(pixel.r * 255.0f, 0.f, 255.f);
                count_pixel( rgb );
            }
        }

        redraw();
    }

    float Histogram::histogram_scale( float val, float maxVal )
    {
        switch( _histtype )
        {
        case kLinear:
            return (val/maxVal);
        case kSqrt:
            return (sqrtf(1+val)/maxVal);
        case kLog:
        default:
            return (logf(1+val)/maxVal);
        }
    }

    void Histogram::draw_pixels( const math::BBox2i& r )
    {

        // Draw the pixel info
        int W = r.w() - 8;
        int H = r.h() - 4;
        int HH = H - 4;
        int idx;
        float v;

        float maxC, maxL;

        switch( _histtype )
        {
        case kLog:
            maxL = logf( 1+maxLumma );
            maxC = logf( 1+maxColor );
            break;
        case kSqrt:
            maxL = sqrtf( 1+maxLumma );
            maxC = sqrtf( 1+maxColor );
            break;
        default:
            maxL = maxLumma;
            maxC = maxColor;
            break;
        }

        glEnable( GL_BLEND );
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glLineWidth( 4 );
        glBegin( GL_LINES );

        for ( int i = 0; i <= W; ++i )
        {
            int x = i + 4;

            idx = int( ((float) i / (float) W) * 255 );
            if ( _channel == kLumma )
            {
                glColor3f( 1.0, 1.0, 1.0 );
                v = histogram_scale( lumma[idx], maxL );
                int y = int(HH*v);
                glVertex2i( x, y );
                glVertex2i( x, 0 );
                glVertex2i( x+3, 0 );
                glVertex2i( x+3, y );
            }

            if ( _channel == kRed || _channel == kRGB )
            {
                glColor3f( 1.0f, 0.0f, 0.0f );
                v = histogram_scale( red[idx], maxC );
                int y = int(HH*v);
                glVertex2i( x, y );
                glVertex2i( x, 0 );
                glVertex2i( x+3, 0 );
                glVertex2i( x+3, y );
            }

            if ( _channel == kGreen || _channel == kRGB )
            {
                glColor3f( 0.0f, 1.0f, 0.0f );
                v = histogram_scale( green[idx], maxC );
                int y = int(HH*v);
                glVertex2i( x, y );
                glVertex2i( x, 0 );
                glVertex2i( x+3, 0 );
                glVertex2i( x+3, y );
            }

            if ( _channel == kBlue || _channel == kRGB )
            {
                glColor3f( 0.0f, 0.0f, 1.0f );
                v = histogram_scale( blue[idx], maxC );
                int y = int(HH*v);
                glVertex2i( x, y );
                glVertex2i( x, 0 );
                glVertex2i( x+3, 0 );
                glVertex2i( x+3, y );
            }

        }

        glEnd();
        
    }

}
