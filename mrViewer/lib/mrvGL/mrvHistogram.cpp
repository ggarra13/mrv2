
#define NOMINMAX


#include <cmath>
#include <limits>



#include <FL/Enumerations.H>
#include <FL/fl_draw.H>


#include "mrvGL/mrvHistogram.h"

#include "mrViewer.h"
#include "mrvCore/mrvI8N.h"

namespace mrv
{


  Histogram::Histogram( int X, int Y, int W, int H, const char* L ) :
    Fl_Widget( X, Y, W, H, L ),
    _channel( kRGB ),
    _histtype( kLog ),
    maxLumma( 0 ),
    maxColor( 0 )
  {
    tooltip( _("Mark an area in the image with the left mouse button") );
  }



  void Histogram::draw()
  {
    fl_rectf(x(), y(), w(), h(), 0, 0, 0 );
    if ( maxLumma > 0 )
      draw_pixels();
  }


  void Histogram::count_pixel( const uint8_t* rgb ) noexcept
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

    maxColor = maxLumma = 0;
    memset( red,   0, sizeof(float) * 256 );
    memset( green, 0, sizeof(float) * 256 );
    memset( blue,  0, sizeof(float) * 256 );
    memset( lumma, 0, sizeof(float) * 256 );
        
    if (!image)
      {
	redraw();
	return;
      }

    int xmin = info.box.min.x;
    int ymin = info.box.min.y;
    int xmax = info.box.max.x;
    int ymax = info.box.max.y;


    imaging::Color4f* pixel;
    uint8_t rgb[3];
    for ( int Y = ymin; Y <= ymax; ++Y )
      {
	for ( int X = xmin; X <= xmax; ++X )
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

  float Histogram::histogram_scale( float val, float maxVal ) const noexcept
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

  void Histogram::draw_pixels() const noexcept
  {

    // Draw the pixel info
    int W = w() - 8 - 3;
    int H = h() - 8;
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

    fl_line_style( FL_SOLID, 4 );
    int Y2 = y() + H;

    for ( int i = 0; i <= W; ++i )
      {
	int X = x() + i;
	
	int maxY = y();

	idx = int( ((float) i / (float) W) * 255 );
	if ( _channel == kLumma )
	  {
	    fl_color( 255, 255, 255 );
	    v = histogram_scale( lumma[idx], maxL );
	    int Y = Y2 - int(H*v);
	    fl_line( X, Y, X, Y2 );
	  }

	if ( _channel == kRed || _channel == kRGB )
	  {
	    fl_color( 255, 0, 0 );
	    v = histogram_scale( red[idx], maxC );
	    int Y = Y2 - int(H*v);
	    if ( Y > maxY ) maxY = Y;
	    fl_line( X, Y, X, Y2 );
	  }

	if ( _channel == kGreen || _channel == kRGB )
	  {
	    fl_color( 0, 255, 0 );
	    v = histogram_scale( green[idx], maxC );
	    int Y = Y2 - int(H*v);
	    if ( Y > maxY ) maxY = Y;
	    fl_line( X, Y, X, Y2 );
	  }

	if ( _channel == kBlue || _channel == kRGB )
	  {
	    fl_color( 0, 0, 255 );
	    v = histogram_scale( blue[idx], maxC );
	    int Y = Y2 - int(H*v);
	    if ( Y > maxY ) maxY = Y;
	    fl_line( X, Y, X, Y2 );
	  }
	
	if ( _channel == kRGB )
	  {
	    // Where all channels overlap, we draw white
	    fl_color( 255, 255, 255 );
	    fl_line( X, maxY, X, Y2 );
	  }
	
      }
	
    fl_line_style(0);
  }

}
