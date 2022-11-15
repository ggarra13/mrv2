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
 * @file   mrvColorInfo.cpp
 * @author gga
 * @date   Wed Nov 08 05:32:32 2006
 *
 * @brief  Color Info text display
 *
 *
 */


#include "mrvCore/mrvI8N.h"
#include <string>
#include <sstream>
#include <limits>
#include <cmath>  // for std::isnan, std::isfinite

using namespace std;

#if defined(WIN32) || defined(WIN64)
# include <float.h>  // for isnan
# define isnan(x) _isnan(x)
# define isfinite(x) _finite(x)
#endif

#include <FL/Fl_Menu.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>


#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvString.h"
#include "mrViewer.h"
#include "mrvGL/mrvGLViewport.h"
#include "mrvColorInfo.h"


namespace
{

void copy_color_cb( void*, mrv::Browser* w )
{
    if ( w->value() < 2 || w->value() > 11 )
        return;

    if ( w->text( w->value() ) == NULL ) return;

    size_t last;
    std::string line( w->text( w->value() ) );
    size_t start = line.find( '\t', 0 );
    line = line.substr( start + 1, line.size()-1 );
    while ( (start = line.find( '@', 0 )) != std::string::npos )
    {
        last = line.find( 'c', start );
        if ( last == std::string::npos ) break;

        if ( start > 0 )
            line = ( line.substr( 0, start-1) +
                     line.substr( last + 1, line.size()-1 ) );
        else
            line = line.substr( last + 1, line.size()-1 );
    }

    std::string copy = " ";
    last = 0;
    while ( (start = line.find_first_not_of( "\t ", last )) !=
            std::string::npos )
    {
        last = line.find_first_of( "\t ", start );
        if ( last == std::string::npos ) last = line.size();

        copy += line.substr( start, last-start ) + " ";
    }

    // Copy text to both the clipboard and to X's XA_PRIMARY
    Fl::copy( copy.c_str(), unsigned( copy.size() ), true );
    Fl::copy( copy.c_str(), unsigned( copy.size() ), false );
}

}

namespace mrv
{
    ViewerUI* ColorInfo::ui = NULL;

    extern std::string float_printf( float x );



class ColorBrowser : public mrv::Browser
{
    int _value;
    ViewerUI*   ui;
public:
    ColorBrowser( int x, int y, int w, int h, const char* l = 0 ) :
    mrv::Browser( x, y, w, h, l ),
    _value( -1 )
    {
    }

    void main( ViewerUI* v ) {
        ui = v;
    }
    ImageView* view() const {
        return uiMain->uiView;
    }

    int mousePush( int x, int y )
    {
        if ( value() < 0 ) return 0;

        Fl_Menu_Button menu(x,y,0,0);

        menu.add( _("Copy/Color"),
                  FL_COMMAND + 'C',
                  (Fl_Callback*)copy_color_cb, (void*)this, 0);

        menu.popup();
        return 1;
    }

    bool valid_value()
    {
        int line = value();
        if (( line < 2 || line > 11 ) ||
            ( line > 5 && line < 8 ))
        {
            value(-1);
            return false;
        }
        _value = line;
        return true;
    }

    void draw()
    {
        value( _value );
        mrv::Browser::draw();
    }

    int handle( int event )
    {
        int ok = 0;
        switch( event )
        {
        case FL_PUSH:
            {
            if ( Fl::event_button() == 3 )
                return mousePush( Fl::event_x(), Fl::event_y() );
            ok = Fl_Browser::handle( event );
            if ( valid_value() ) return 1;
            return ok;
            }
        case FL_ENTER:
            return 1;
        case FL_FOCUS:
            return 1;
        case FL_KEYBOARD:
            ok = view()->handle( event );
            if (!ok) ok = Fl_Browser::handle( event );
            return ok;
        default:
            ok = Fl_Browser::handle( event );
            if ( valid_value() ) return 1;
            return ok;
        }
    }
};


class ColorWidget : public Fl_Box
{
    Fl_Browser* color_browser_;

public:
    ColorWidget( int x, int y, int w, int h, const char* l = 0 ) :
    Fl_Box( x, y, w, h, l )
    {
        box( FL_FRAME_BOX );
    }

    int mousePush( int x, int y )
    {
        color_browser_->value( 4 );

        Fl_Menu_Button menu(x,y,0,0);

        menu.add( _("Copy/Color"),
                  FL_COMMAND + 'C',
                  (Fl_Callback*)copy_color_cb, (void*)color_browser_, 0);

        menu.popup();
        return 1;
    }

    void color_browser( Fl_Browser* b ) {
        color_browser_ = b;
    }

    int handle( int event )
    {
        switch( event )
        {
        case FL_PUSH:
            if ( Fl::event_button() == FL_RIGHT_MOUSE )
                return mousePush( Fl::event_x(), Fl::event_y() );
        default:
            return Fl_Box::handle( event );
        }
    }
};


ColorInfo::ColorInfo( int x, int y, int w, int h, const char* l ) :
    Fl_Group( x, y, w, h, l )
{
    tooltip( _("Mark an area in the image with SHIFT + the left mouse button") );
    area = new Fl_Box( 0, 0, w, 50 );
    area->box( FL_FLAT_BOX );
    area->align( FL_ALIGN_CENTER | FL_ALIGN_INSIDE );

    dcol = new ColorWidget( 16, 10, 32, 32 );


    int w4 = w / 4;
    int w5 = w / 5;
    static int col_widths[] = { w4, w5, w5, w5, w5, 0 };
    browser = new ColorBrowser( 0, area->h(), w, h - area->h() );
    browser->column_widths( col_widths );
    browser->showcolsep( 1 );
    browser->type(FL_HOLD_BROWSER);
    browser->resizable(browser);
    resizable(this);

    dcol->color_browser( browser );

}

void ColorInfo::main( ViewerUI* m ) {
    uiMain = m;
    browser->main(m);
}

ImageView* ColorInfo::view() const
{
    return uiMain->uiView;
}

int  ColorInfo::handle( int event )
{
    return Fl_Group::handle( event );
}

void ColorInfo::update()
{
    if ( ! uiMain->uiView ) return;

    mrv::media fg = uiMain->uiView->foreground();
    if (!fg) return;

    CMedia* img = fg->image();
    mrv::Rectd selection = uiMain->uiView->selection();
    update( img, selection );
    redraw();
}

void ColorInfo::selection_to_coord( const CMedia* img,
                                    const mrv::Rectd& selection,
                                    int& xmin, int& ymin, int& xmax,
                                    int& ymax, int off[2],
                                    bool& right, bool& bottom )
{
    const mrv::Recti& dpw = img->display_window();
    const mrv::Recti& daw = img->data_window();
    const mrv::Recti& dpw2 = img->display_window2();
    const mrv::Recti& daw2 = img->data_window2();
    unsigned W = dpw.w();
    unsigned H = dpw.h();
    if ( W == 0 ) W = img->width();
    if ( H == 0 ) H = img->height();

    unsigned wt = W;
    unsigned ht = H;
    xmin = (int) selection.x();
    ymin = (int) selection.y();


    CMedia::StereoOutput output = uiMain->uiView->stereo_output();
    CMedia::StereoInput  input = uiMain->uiView->stereo_input();

    if ( output == CMedia::kStereoRight )
    {
        W = dpw2.w();
        H = dpw2.h();
        xmin -= off[0] = daw2.x();
        ymin -= off[1] = daw2.y();
        right = true;
    }
    else if ( selection.x() >= W &&
              ( output & CMedia::kStereoSideBySide ) )
    {
        W = dpw2.w();
        H = dpw2.h();
        if ( output & CMedia::kStereoRight )
        {
            xmin -= off[0] = daw.x();
            ymin -= off[1] = daw.y();
            if ( input & CMedia::kTopBottomStereoInput )
                ymin -= ht;
        }
        else
        {
            xmin -= off[0] = daw2.x();
            ymin -= off[1] = daw2.y();
        }
        xmin -= wt;
        right = true;
    }
    else if ( selection.y() >= H &&
              (output & CMedia::kStereoTopBottom) )
    {
        W = dpw2.w();
        H = dpw2.h();
        if ( output & CMedia::kStereoRight )
        {
            xmin -= off[0] = daw.x();
            ymin -= off[1] = daw.y();
            if ( input & CMedia::kLeftRightStereoInput )
                xmin -= wt;
        }
        else
        {
            xmin -= off[0] = daw2.x();
            ymin -= off[1] = daw2.y();
        }
        ymin -= ht;
        bottom = true;
    }
    else
    {
        if ( output & CMedia::kStereoRight )
        {
            xmin -= off[0] = daw2.x();
            ymin -= off[1] = daw2.y();
        }
        else
        {
            xmin -= off[0] = daw.x();
            ymin -= off[1] = daw.y();
        }
    }

    if ( input == CMedia::kTopBottomStereoInput &&
            ( ( output & CMedia::kStereoRight ) || right ) )
    {
        ymin += ht;
    }
    else if ( input == CMedia::kLeftRightStereoInput &&
              ( ( output & CMedia::kStereoRight ) || bottom ) )
    {
        xmin += wt;
    }

    if ( selection.w() > 0 ) W = (int)selection.w();
    if ( selection.h() > 0 ) H = (int)selection.h();

    if ( img->flipX() ) xmin = wt - xmin;
    if ( img->flipY() ) ymin = ht - ymin;

    if ( img->flipX() )
        xmax = xmin - W + 1;
    else
        xmax = xmin + W - 1;

    if ( img->flipY() )
        ymax = ymin - H + 1;
    else
        ymax = ymin + H - 1;

    if ( xmax < xmin )
    {
        int tmp = xmax;
        xmax = xmin;
        xmin = tmp;
    }

    if ( ymax < ymin )
    {
        int tmp = ymax;
        ymax = ymin;
        ymin = tmp;
    }


    if ( xmin < 0 ) xmin = 0;
    if ( ymin < 0 ) ymin = 0;

    if ( xmax < 0 ) xmax = 0;
    if ( ymax < 0 ) ymax = 0;


}


void ColorInfo::update( const CMedia* img,
                        const mrv::Rectd& selection )
{
    if ( !visible_r() ) return;


    area->label( "" );

    std::ostringstream text;
    if ( img && (selection.w() > 0 || selection.h() > 0) )
    {
        CMedia::Pixel hmin( std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max() );

        CMedia::Pixel pmin( std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max() );
        CMedia::Pixel pmax( std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min() );
        CMedia::Pixel hmax( std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::min() );
        CMedia::Pixel pmean( 0, 0, 0, 0 );
        CMedia::Pixel hmean( 0, 0, 0, 0 );



        mrv::image_type_ptr pic = img->left();
        if (!pic) return;

        unsigned count = 0;

        int off[2];
        int xmin, ymin, xmax, ymax;
        bool right = false;
        bool bottom = false;
        selection_to_coord( img, selection, xmin, ymin, xmax, ymax, off,
                            right, bottom );

        CMedia::StereoOutput stereo_output = uiMain->uiView->stereo_output();
        if ( right )
        {
            if ( stereo_output == CMedia::kStereoCrossed )
                pic = img->left();
            else
                pic = img->right();
            if (!pic) return;
        }
        else if ( stereo_output & CMedia::kStereoSideBySide )
        {
            if ( stereo_output & CMedia::kStereoRight )
                pic = img->right();
            else
                pic = img->left();
        }
        else if ( bottom )
        {
            if ( stereo_output == CMedia::kStereoBottomTop )
                pic = img->left();
            else if ( stereo_output & CMedia::kStereoTopBottom )
                pic = img->right();
            if (!pic) return;
        }
        else if ( stereo_output & CMedia::kStereoTopBottom )
        {
            if ( stereo_output & CMedia::kStereoRight )
                pic = img->right();
            else
                pic = img->left();
        }
        else
        {
            pic = img->left();
        }
        if ( xmin >= (int) pic->width() ) xmin = (int) pic->width()-1;
        if ( ymin >= (int) pic->height() ) ymin = (int) pic->height()-1;

        if ( xmax >= (int) pic->width() ) xmax = (int) pic->width()-1;
        if ( ymax >= (int) pic->height() ) ymax = (int) pic->height()-1;

        if ( xmax < xmin ) {
            int tmp = xmax;
            xmax = xmin;
            xmin = tmp;
        }

        int H = img->data_window().h();
        if ( H == 0 )
        {
            H = img->display_window().h();
            if ( H == 0 ) H = (int) pic->height();
        }


        if ( ymax < ymin ) {
            int tmp = ymax;
            ymax = ymin;
            ymin = tmp;
        }

        unsigned spanX = xmax-xmin+1;
        unsigned spanY = ymax-ymin+1;
        unsigned numPixels = spanX * spanY;



        text << std::endl
             << _("Area") << ": (" << ( xmin + off[0]  )
             << ", " << ( H - ymax - 1 + off[1] )
             << ") - (" << ( xmax + off[0] )
             << ", " << ( H - ymin - 1 + off[1] ) << ")" << std::endl
             << _("Size") << ": [ " << spanX << "x" << spanY << " ] = "
             << numPixels << " "
             << ( numPixels == 1 ? _("pixel") : _("pixels") )
             << std::endl;
        area->copy_label( text.str().c_str() );

        mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                              uiMain->uiLType->value();


        float gain  = uiMain->uiView->gain();
        float gamma = uiMain->uiView->gamma();
        float one_gamma = 1.0f / gamma;

        mrv::ImageView* view = uiMain->uiView;
        mrv::DrawEngine* engine = view->engine();

        ImageView::PixelValue v = (ImageView::PixelValue)
                                  uiMain->uiPixelValue->value();

        CMedia::Pixel rp;

        for ( int y = ymin; y <= ymax; ++y )
        {
            for ( int x = xmin; x <= xmax; ++x, ++count )
            {

                if ( stereo_output == CMedia::kStereoInterlaced )
                {
                    if ( y % 2 == 1 ) pic = img->right();
                    else pic = img->left();
                }
                else if ( stereo_output == CMedia::kStereoInterlacedColumns )
                {
                    if ( x % 2 == 1 ) pic = img->right();
                    else pic = img->left();
                }
                else if ( stereo_output == CMedia::kStereoCheckerboard )
                {
                    if ( (x + y) % 2 == 0 ) pic = img->right();
                    else pic = img->left();
                }

                if ( x >= (int)pic->width() || y >= (int)pic->height() )
                {
                    --count;
                    continue;
                }

                CMedia::Pixel op = pic->pixel( x, y );

                if ( view->normalize() )
                {
                    view->normalize( op );
                }

                op.r *= gain;
                op.g *= gain;
                op.b *= gain;

                ColorControlsUI* cc = uiMain->uiColorControls;
                if ( cc->uiActive->value() )
                {
                    const Imath::M44f& m = colorMatrix(cc);
                    Imath::V3f* iop = (Imath::V3f*)&op;
                    *iop *= m;
                }

                if ( view->use_lut() && v == ImageView::kRGBA_Full )
                {
                    Imath::V3f* iop = (Imath::V3f*)&op;
                    Imath::V3f* irp = (Imath::V3f*)&rp;
                    engine->evaluate( img, *iop, *irp );
                    rp.a = op.a;

                }
                else
                {
                    rp = op;
                }

                if ( v != ImageView::kRGBA_Original )
                {
                    // The code below is same as
                    //   rp.r = powf(rp.r, one_gamma);
                    // but faster
                    if ( rp.r > 0.0f && isfinite(rp.r) )
                        rp.r = expf( logf(rp.r) * one_gamma );
                    if ( rp.g > 0.0f && isfinite(rp.g) )
                        rp.g = expf( logf(rp.g) * one_gamma );
                    if ( rp.b > 0.0f && isfinite(rp.b) )
                        rp.b = expf( logf(rp.b) * one_gamma );
                }

                if ( rp.r < pmin.r ) pmin.r = rp.r;
                if ( rp.g < pmin.g ) pmin.g = rp.g;
                if ( rp.b < pmin.b ) pmin.b = rp.b;
                if ( rp.a < pmin.a ) pmin.a = rp.a;

                if ( rp.r > pmax.r ) pmax.r = rp.r;
                if ( rp.g > pmax.g ) pmax.g = rp.g;
                if ( rp.b > pmax.b ) pmax.b = rp.b;
                if ( rp.a > pmax.a ) pmax.a = rp.a;

                pmean.r += rp.r;
                pmean.g += rp.g;
                pmean.b += rp.b;
                pmean.a += rp.a;

                CMedia::Pixel hsv;

                switch( uiMain->uiBColorType->value()+1 )
                {
                case color::kITU_709:
                    hsv = color::rgb::to_ITU709( rp );
                    break;
                case color::kITU_601:
                    hsv = color::rgb::to_ITU601( rp );
                    break;
                case color::kYDbDr:
                    hsv = color::rgb::to_YDbDr( rp );
                    break;
                case color::kYIQ:
                    hsv = color::rgb::to_yiq( rp );
                    break;
                case color::kYUV:
                    hsv = color::rgb::to_yuv( rp );
                    break;
                case color::kCIE_Luv:
                    hsv = color::rgb::to_luv( rp );
                    break;
                case color::kCIE_Lab:
                    hsv = color::rgb::to_lab( rp );
                    break;
                case color::kCIE_xyY:
                    hsv = color::rgb::to_xyY( rp );
                    break;
                case color::kCIE_XYZ:
                    hsv = color::rgb::to_xyz( rp );
                    break;
                case color::kHSL:
                    hsv = color::rgb::to_hsl( rp );
                    break;
                default:
                case color::kHSV:
                    hsv = color::rgb::to_hsv( rp );
                    break;
                }

                hsv.a = calculate_brightness( rp, brightness_type );

                if ( hsv.r < hmin.r ) hmin.r = hsv.r;
                if ( hsv.g < hmin.g ) hmin.g = hsv.g;
                if ( hsv.b < hmin.b ) hmin.b = hsv.b;
                if ( hsv.a < hmin.a ) hmin.a = hsv.a;

                if ( hsv.r > hmax.r ) hmax.r = hsv.r;
                if ( hsv.g > hmax.g ) hmax.g = hsv.g;
                if ( hsv.b > hmax.b ) hmax.b = hsv.b;
                if ( hsv.a > hmax.a ) hmax.a = hsv.a;

                hmean.r += hsv.r;
                hmean.g += hsv.g;
                hmean.b += hsv.b;
                hmean.a += hsv.a;
            }
        }




        float c = float(count);

        pmean.r /= c;
        pmean.g /= c;
        pmean.b /= c;
        pmean.a /= c;

        hmean.r /= c;
        hmean.g /= c;
        hmean.b /= c;
        hmean.a /= c;

        static const char* kR = "@C4286611456@c";
        static const char* kG = "@C1623228416@c";
        static const char* kB = "@C2155937536@c";
        static const char* kA = "@C2964369408@c";

        static const char* kH = "@C2964324352@c";
        static const char* kS = "@C2964324352@c";
        static const char* kV = "@C2964324352@c";
        static const char* kL = "@C2964324352@c";




        Fl_Color col;

        {
            float r = pmean.r;
            float g = pmean.g;
            float b = pmean.b;

            if ( r < 0.f ) r = 0.0f;
            else if ( r > 1.f ) r = 1.0f;

            if ( g < 0.f ) g = 0.0f;
            else if ( g > 1.f ) g = 1.0f;

            if ( b < 0.f ) b = 0.0f;
            else if ( b > 1.f ) b = 1.0f;

            if ( r <= 0.01f && g <= 0.01f && b <= 0.01f )
                col = FL_BLACK;
            else
            {
                col = fl_rgb_color((uchar)(r*255),
                               (uchar)(g*255),
                               (uchar)(b*255));
            }
        }

        dcol->color( col );
        dcol->redraw();


        text.str("");
        text.str().reserve(1024);
        text << "@b\t"
             << kR
             << N_("R") << "\t"
             << kG
             << N_("G") << "\t"
             << kB
             << N_("B") << "\t"
             << kA
             << N_("A")
             << std::endl
             << _("Maximum") << ":\t@c"
             << float_printf( pmax.r ) << "\t@c"
             << float_printf( pmax.g ) << "\t@c"
             << float_printf( pmax.b ) << "\t@c"
             << float_printf( pmax.a ) << std::endl
             << _("Minimum") << ":\t@c"
             << float_printf( pmin.r ) << "\t@c"
             << float_printf( pmin.g ) << "\t@c"
             << float_printf( pmin.b ) << "\t@c"
             << float_printf( pmin.a ) << std::endl;

        CMedia::Pixel r(pmax);
        r.r -= pmin.r;
        r.g -= pmin.g;
        r.b -= pmin.b;
        r.a -= pmin.a;

        text << _("Range") << ":\t@c"
             << float_printf( r.r) << "\t@c"
             << float_printf( r.g) << "\t@c"
             << float_printf( r.b) << "\t@c"
             << float_printf( r.a) << std::endl
             << "@b" << _("Mean") << ":\t@c"
             << kR
             << float_printf( pmean.r) << "\t@c"
             << kG
             << float_printf( pmean.g) << "\t@c"
             << kB
             << float_printf( pmean.b) << "\t@c"
             << kA
             << float_printf( pmean.a) << std::endl
             << std::endl
             << "@b\t";

        switch( uiMain->uiBColorType->value()+1 )
        {
        case color::kITU_709:
            text << kH << N_("7") << "\t@c"
                 << kS << N_("0") << "\t@c"
                 << kL << N_("9");
            break;
        case color::kITU_601:
            text << kH << N_("6") << "\t@c"
                 << kS << N_("0") << "\t@c"
                 << kL << N_("1");
            break;
        case color::kYIQ:
            text << kH << N_("Y") << "\t@c"
                 << kS << N_("I") << "\t@c"
                 << kL << N_("Q");
            break;
        case color::kYDbDr:
            text << kH << N_("Y") << "\t@c"
                 << kS << N_("Db") << "\t@c"
                 << kL << N_("Dr");
            break;
        case color::kYUV:
            text << kH << N_("Y") << "\t@c"
                 << kS << N_("U") << "\t@c"
                 << kL << N_("V");
            break;
        case color::kCIE_Luv:
            text << kH << N_("L") << "\t@c"
                 << kS << N_("u") << "\t@c"
                 << kL << N_("v");
            break;
        case color::kCIE_Lab:
            text << kH << N_("L") << "\t@c"
                 << kS << N_("a") << "\t@c"
                 << kL << N_("b");
            break;
        case color::kCIE_xyY:
            text << kH << N_("x") << "\t@c"
                 << kS << N_("y") << "\t@c"
                 << kL << N_("Y");
            break;
        case color::kCIE_XYZ:
            text << kH << N_("X") << "\t@c"
                 << kS << N_("Y") << "\t@c"
                 << kL << N_("Z");
            break;
        case color::kHSL:
            text << kH << N_("H") << "\t@c"
                 << kS << N_("S") << "\t@c"
                 << kL << N_("L");
            break;
        case color::kHSV:
        default:
            text << kH << N_("H") << "\t@c"
                 << kS << N_("S") << "\t@c"
                 << kV << N_("V");
            break;
        }

        text << "\t" << kL;

        switch( brightness_type )
        {
        case kAsLuminance:
            text << N_("Y");
            break;
        case kAsLumma:
            text << N_("Y'");
            break;
        case kAsLightness:
            text << N_("L");
            break;
        }

        text << std::endl
             << _("Maximum") << ":\t@c"
             << float_printf( hmax.r) << "\t@c"
             << float_printf( hmax.g) << "\t@c"
             << float_printf( hmax.b) << "\t@c"
             << float_printf( hmax.a) << std::endl
             << _("Minimum") << ":\t@c"
             << float_printf( hmin.r) << "\t@c"
             << float_printf( hmin.g) << "\t@c"
             << float_printf( hmin.b) << "\t@c"
             << float_printf( hmin.a) << std::endl;

        r = hmax;
        r.r -= hmin.r;
        r.g -= hmin.g;
        r.b -= hmin.b;
        r.a -= hmin.a;

        text << _("Range") << ":\t@c"
             << float_printf( r.r) << "\t@c"
             << float_printf( r.g) << "\t@c"
             << float_printf( r.b) << "\t@c"
             << float_printf( r.a) << std::endl
             << "@b" << _("Mean") << ":\t@c"
             << kH
             << float_printf( hmean.r) << "\t@c"
             << kS
             << float_printf( hmean.g) << "\t@c"
             << kV
             << float_printf( hmean.b) << "\t@c"
             << kL
             << float_printf( hmean.a);
    }

    stringArray lines;
    mrv::split_string( lines, text.str(), "\n" );
    stringArray::iterator i = lines.begin();
    stringArray::iterator e = lines.end();
    area->redraw_label();
    browser->clear();
    int idx = 0;
    for ( ; i != e; ++i, ++idx )
    {
        browser->add( (*i).c_str() );
        //child(idx)->align( FL_ALIGN_CENTER );
    }
    browser->redraw();
}

}  // namespace mrv
