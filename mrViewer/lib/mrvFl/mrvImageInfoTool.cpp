

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvSequence.h"

#include <iostream>
using namespace std;

#include <algorithm>

#include <boost/regex.hpp>


#include "mrvCore/mrvMath.h"

#include <FL/Fl_Int_Input.H>
#include <FL/fl_draw.H>
#include <FL/names.h>


#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvTimecode.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvImageInfoTool.h"
#include "mrvWidgets/mrvPack.h"

#include "mrvPreferencesUI.h"
#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace {
const char* kModule = "info";
}

void attach_ocio_ics_cb( Fl_Widget* o, mrv::ImageBrowser* v )
{
}


namespace tl
{

    //! Audio and video I/O.
    namespace io
    {
        typedef std::map< std::string, std::string > Attribute;
    }
}

namespace mrv
{


static const Fl_Color kTitleColors[] = {
    0x608080ff,
    0x808060ff,
    0x606080ff,
    0x608060ff,
    0x806080ff,
};



static const unsigned int kSizeOfTitleColors = ( sizeof(kTitleColors) /
        sizeof(Fl_Color) );

static const Fl_Color kRowColors[] = {
    0x808080ff,
    0xa0a0a0ff,
};

static const unsigned int kSizeOfRowColors = ( sizeof(kRowColors) /
        sizeof(Fl_Color) );

static void change_stereo_image( Fl_Button* w, mrv::ImageInfoTool* info )
{
    // static Media* last = NULL;
    // Media* img = info->get_image();
    // if ( img->is_stereo() && img != last && img->right_eye() )
    // {
    //     last = img;
    //     img = img->right_eye();
    //     info->set_image( img );
    //     w->label( _("Right View") );
    // }
    // else
    // {
    //     if ( last )
    //     {
    //         info->set_image( last );
    //         last = NULL;
    //         w->label( _("Left View") );
    //     }
    // }
    // info->filled = false;
    // info->refresh();
}

mrv::Choice *uiType=(mrv::Choice *)0;

Fl_Input *uiKey=(Fl_Input *)0;

Fl_Input *uiValue=(Fl_Input *)0;

mrv::Choice *uiKeyRemove=(mrv::Choice *)0;

static void cb_OK(Fl_Button*, Fl_Window* v) {
    v->damage( FL_DAMAGE_ALL );  // like fltk2.0's exec return true
    v->hide();
}

static void cb_Cancel(Fl_Button*, Fl_Window* v) {
    v->hide();
}


static void cb_uiType(mrv::Choice* o, void*) {
    std::string type = o->child( o->value() )->label();
    if ( type == _("String") )
        uiValue->value( _("Text") );
    else if ( type == _("Integer") )
        uiValue->value( N_("15") );
    else if ( type == _("Float") || type == _("Double") )
        uiValue->value( _("2.2") );
    else if ( type == N_("Timecode") )
        uiValue->value( _("00:00:00:00") );
    else if ( type == _("Rational") )
        uiValue->value( _("24000/1001") );
    else if ( type == _("Vector2 Integer") )
        uiValue->value( N_("2 5") );
    else if ( type == _("Vector2 Float") || type == _("Vector2 Double") )
        uiValue->value( _("2.2 5.1") );
    else if ( type == _("Vector3 Integer") )
        uiValue->value( N_("2 5 1") );
    else if ( type == _("Vector3 Float") || type == _("Vector3 Double") )
        uiValue->value( _("2.2 5.1 1.4") );
    else if ( type == _("Box2 Integer") )
        uiValue->value( N_("2 5  10 20") );
    else if ( type == _("Box2 Float")  )
        uiValue->value( _("0.2 5.1  10.5 20.2") );
    else if ( type == _("Chromaticities") )
        uiValue->value( _("0.64 0.33  0.3 0.6  0.15 0.06  0.3127 0.3290") );
    else if ( type == _("M33 Float") || type == _("M33 Double")  )
        uiValue->value( _("1.0 0.0 0.0  0.0 1.0 0.0  0.0 0.0 1.0") );
    else if ( type == _("M44 Float") || type == _("M44 Double") )
        uiValue->value( _("1.0 0.0 0.0 0.0  0.0 1.0 0.0 0.0  "
                         "0.0 0.0 1.0 0.0  0.0 0.0 0.0 1.0") );
    else if ( type == _("KeyCode") )
        uiValue->value( _("** multivalue **") );
}

static Fl_Double_Window* make_attribute_add_window() {
    Fl_Double_Window* w;
    {   Fl_Double_Window* o = new Fl_Double_Window(405, 200);
        w = o;
        o->label( _("Add Attribute") );
        o->begin();
        Fl_Group* g = new Fl_Group( 0, 0, 405, 200 );
        g->box( FL_UP_BOX );
        {   mrv::Choice* o = uiType = new mrv::Choice( 10, 30, 390, 25, _("Type") );
            o->align(FL_ALIGN_TOP);
            o->add( _("String") );
            o->add( _("Integer") );
            o->add( _("Float") );
            o->add( _("Double") );
            o->add( _("Rational") );
            o->add( _("M33 Float") );
            o->add( _("M44 Float") );
            o->add( _("M33 Double") );
            o->add( _("M44 Double") );
            o->add( N_("Timecode") );
            o->add( _("Box2 Integer") );
            o->add( _("Box2 Float") );
            o->add( _("Vector2 Integer") );
            o->add( _("Vector2 Float") );
            o->add( _("Vector2 Double") );
            o->add( _("Vector3 Integer") );
            o->add( _("Vector3 Float") );
            o->add( _("Vector3 Double") );
            o->add( _("Chromaticities") );
            o->add( _("KeyCode") );
            o->callback( (Fl_Callback*) cb_uiType, (void*)w );
            o->value( 9 );
        }
        {   Fl_Input* o = uiKey = new Fl_Input(10, 75, 390, 25, _("Keyword"));
            o->value( N_("timecode") );
            o->align(FL_ALIGN_TOP);
        }
        {   Fl_Input* o = uiValue = new Fl_Input(10, 120, 390, 25, _("Value"));
            o->value( N_("00:00:00:00") );
            o->align(FL_ALIGN_TOP);
        }
        {   Fl_Button* o = new Fl_Button(115, 150, 86, 41, _("OK"));
            o->callback((Fl_Callback*)cb_OK, (void*)(w));
        }
        {   Fl_Button* o = new Fl_Button(224, 150, 93, 41, _("Cancel"));
            o->callback((Fl_Callback*)cb_Cancel, (void*)(w));
        }
        g->end();
        o->end();
        o->set_modal();
        o->resizable(o);
    }
    return  w;
}


// static void add_attribute_cb( Fl_Box* widget, ImageInfoTool* info )
// {
//     Media* img = info->get_image();
//     if (!img) return;

//     Fl_Group::current(0);
//     Fl_Double_Window* w = make_attribute_add_window();
//     w->show();
//     while ( w->visible() )
//         Fl::check();

//     if ( ! (w->damage() & FL_DAMAGE_ALL) )
//         return;

//     std::string key = uiKey->value();
//     std::string value = uiValue->value();

//     tl::io::Attribute& attrs = img->attributes();
//     add_attribute( attrs, img );
//     info->filled = false;
//     info->refresh();
//     ViewerUI* ui = info->main();
//     ui->uiView->redraw();
//     delete w;
// }


// static void remove_attribute_cb( Fl_Box* widget, ImageInfoTool* info )
// {
//     Media* img = info->get_image();
//     if (!img) return;

//     tl::io::Attribute& attrs = img->attributes();

//     Fl_Group::current(0);
//     Fl_Window* w = make_remove_window( attrs );
//     w->set_modal();
//     w->show();
//     while ( w->visible() )
//         Fl::check();


//     if ( ! ( w->damage() & FL_DAMAGE_ALL ) ) return;

//     char picked[1024];
//     int ok = uiKeyRemove->item_pathname( picked, sizeof(picked)-1 );
//     if ( ok < 0 )
//     {
//         LOG_ERROR( _("item_pathname returned ") << ok );
//         return;
//     }

//     std::string key = "/";
//     if ( strlen(picked) > 0 ) key = picked;

//     tl::io::Attribute::iterator i = attrs.find( key );
//     if ( i == attrs.end() && key[0] == '/' ) {
//         key = key.substr( 1, key.size() );
//         i = attrs.find( key );
//     }
//     if ( i == attrs.end() )
//     {
//         char buf[128];
//         sprintf( buf, _("No attribute named '%s'"), key.c_str() );
//         mrvALERT( buf );
//         return;
//     }
//     if ( key.find( N_("timecode") ) != std::string::npos )
//     {
//         img->timecode( 0 );
//         img->image_damage( img->image_damage() | Media::kDamageTimecode );
//     }
//     attrs.erase( i );
//     info->filled = false;
//     info->refresh();
// }




ViewerUI* ImageInfoTool::main() const
{
    return _p->ui;
}


GLViewport* ImageInfoTool::view() const
{
    return _p->ui->uiView;
}

}  // namespace mrv

enum MatchType
{
    kMatchAll,
    kMatchAttribute,
    kMatchValue
};

int idx = -1;
std::string old_match;
MatchType old_type = kMatchAll;
int num_matches = 0;
int match_goal = 1;


static bool regex_match( float i, const std::string& regex, std::string text )
{
    try {
        boost::regex expr{ regex };
        if ( boost::regex_search( text, expr ) ) {
            ++num_matches;
            if ( match_goal == num_matches )
            {
                idx = int(i - 0.5f);
                return true;
            }
        }
    }
    catch ( const boost::regex_error& e )
    {
    }
  return false;
}


static bool process_row( float row, Fl_Widget* w, const std::string& match,
                         MatchType type )
{
  if ( ( type == kMatchValue || type == kMatchAll ) &&
       dynamic_cast< Fl_Input* >( w ) != NULL )
  {
    Fl_Input* input = (Fl_Input*) w;
    if ( regex_match( row, match, input->value() ) )
      return true;
  }
  else if ( ( type == kMatchValue || type == kMatchAll ) &&
            dynamic_cast< Fl_Output* >( w ) != NULL )
  {
    Fl_Output* output = (Fl_Output*) w;
    if ( regex_match( row, match, output->value() ) )
      return true;
  }
  else if ( dynamic_cast< Fl_Group* >( w ) != NULL )
  {
    Fl_Group* g = (Fl_Group*) w;
    for ( int c = 0; c < g->children(); ++c )
    {
      w = (Fl_Widget*) g->child(c);
      bool ok = process_row( row, w, match, type );
      if ( ok ) return ok;
    }
  }
  else
  {
    if ( type != kMatchAttribute && type != kMatchAll ) return false;
    if ( !w->label() ) return false;
    if ( regex_match( row, match, w->label() ) )
      return true;
  }
  return false;
}

static int search_table( mrv::Table* t, float& row, const std::string& match,
                         MatchType type )
{
  int rows = t->children();
  for ( int i = 0; i < rows; ++i )
    {
      if ( ! t->parent()->visible() ) continue;

      row += 0.5;
      Fl_Widget* w = t->child(i);
      if ( process_row( row, w, match, type ) )
        break;
    }

  return idx;
}

void search_cb( Fl_Widget* o, mrv::ImageInfoTool* info )
{
  std::string match = info->m_entry->value();
  MatchType type = (MatchType) info->m_type->value();
  num_matches = 0;

  if ( match == old_match && type == old_type )
    {
      ++match_goal;
    }
  else
    {
      match_goal = 1;
    }

  old_match = match;
  old_type = type;

  if ( match.empty() )
  {
    info->scroll_to( 0, 0 );
    return;
  }

  idx = -1;

  int H = info->line_height() + 1;
  int H2 = 56 - info->line_height();
  int H3 = 12 + info->line_height();

  mrv::Pack* p = (mrv::Pack*) info->m_image->child(1);
  if ( ! p->children() ) return;

  mrv::Table* t = (mrv::Table*) p->child(0);

  float row = 0;
  int pos = info->line_height();
  if ( p->visible() ) pos += 32;

  int idx = search_table( t, row, match, type );
  if ( idx >= 0 ) {
    info->scroll_to( 0, pos + idx*H );
    return;
  }

  p = (mrv::Pack*) info->m_video->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match, type );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  p = (mrv::Pack*) info->m_audio->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match, type );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->scroll_to( 0, pos + idx*H );
        return;
      }
    }


  p = (mrv::Pack*) info->m_subtitle->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match, type );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  p = (mrv::Pack*) info->m_attributes->child(1);
  for ( int i = 0; i < p->children(); ++i )
    {
      t = (mrv::Table*) p->child(i);

      if ( i == 0 ) pos += H3;
      idx = search_table( t, row, match, type );
      if ( p->visible() ) pos += H2;
      if ( idx >= 0 ) {
        info->scroll_to( 0, pos + idx*H );
        return;
      }
    }

  match_goal = 0;

}

namespace mrv {

    ImageInfoTool::ImageInfoTool( ViewerUI* ui ) :
        ToolWidget( ui )
    {
        add_group( _("Media Information") );
        
        Fl_SVG_Image* svg = load_svg( "Info.svg" );
        g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete imageInfoTool; imageInfoTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
    }


    void
    ImageInfoTool::add_controls()
    {
        int sw = Fl::scrollbar_size();                // scrollbar width

        tl::math::BBox2i r( g->x(), g->y(), g->w(), g->h());

        // menu = new Fl_Menu_Button( 0, 0, 0, 0, _("Attributes Menu") );
        // menu->type( Fl_Menu_Button::POPUP3 );



        // CollapsibleGrop recalcs, we don't care its xyh sizes
        m_image = new mrv::CollapsibleGroup( 0, r.y()+70, r.w()-sw,
                                             800, _("Main")  );
        m_image->end();

        m_video = new mrv::CollapsibleGroup( r.x(), r.y()+870,
                                             r.w()-sw, 400, _("Video") );
        m_video->end();

        m_audio = new mrv::CollapsibleGroup( r.x(), r.y()+1270,
                                             r.w()-sw, 400, _("Audio") );
        m_audio->end();

        m_subtitle = new mrv::CollapsibleGroup( r.x(), r.y()+1670,
                                                r.w()-sw, 400, _("Subtitle") );
        m_subtitle->end();

        m_attributes  = new mrv::CollapsibleGroup( r.x(), r.y()+2070,
                                                   r.w()-sw, 400,
                                                   _("Metadata")  );
        m_attributes->end();


        hide_tabs();

    }



struct aspectName_t
{
    double     ratio;
    const char* name;
};

static const aspectName_t kAspectRatioNames[] =
{
    { 640.0/480.0, _("Video") },
    { 680.0/550.0, _("PAL Video") },
    { 720.0/576.0, _("PAL Video") },
    { 768.0/576.0, _("PAL Video") },
    { 720.0/486.0, _("NTSC Video") },
    { 720.0/540.0, _("NTSC Video") },
    { 1.5,  _("NTSC Video") },
    { 1.37, _("35mm Academy") },
    { 1.56, _("Widescreen (HDTV + STV)") },
    { 1.66, _("35mm European Widescreen") },
    { 1.75, _("Early 35mm") },
    { 1.777778, _("HDTV / Widescreen 16:9") },
    { 1.85, _("35mm Flat") },
    { 2.2,  _("70mm") },
    { 2.35, _("35mm Anamorphic") },
    { 2.39, _("35mm Panavision") },
    { 2.55, _("Cinemascope") },
    { 2.76, _("MGM Camera 65") },
};

void ImageInfoTool::enum_cb( mrv::PopupMenu* m, ImageInfoTool* v )
{
    m->label( m->child( m->value() )->label() );
}

// static void timecode_cb( Fl_Input* w, ImageInfoTool* info )
// {
//     Media* img = dynamic_cast<Media*>( info->get_image() );
//     if ( !img ) return;

//     tl::io::Attribute& attrs = img->attributes();
//     tl::io::Attribute::iterator i = attrs.begin();
//     tl::io::Attribute::iterator e = attrs.end();
//     for ( ; i != e; ++i )
//     {
//         if ( i->first == N_("timecode") ||
//              i->first == N_("Video timecode") ||
//              i->first == N_("Timecode") )
//         {
//             const Imf::TimeCode& t = Media::str2timecode( w->value() );
//             img->process_timecode( t );
//             Imf::TimeCodeAttribute attr( t );
//             i->second = attr.copy();
//         }
//     }

//     img->image_damage( img->image_damage() | Media::kDamageTimecode );
//     ViewerUI* ui = info->main();
//     ui->uiView->redraw();
// }

// Update int slider from int input
static void update_int_slider( Fl_Int_Input* w )
{
    Fl_Group* g = w->parent();
    Fl_Slider* s = (Fl_Slider*)g->child(1);
    s->value( atoi( w->value() ) );
}

// Update float slider from float input
static void update_float_slider( Fl_Float_Input* w )
{
    Fl_Group* g = w->parent();
    Fl_Slider* s = (Fl_Slider*)g->child(1);
    s->value( atof( w->value() ) );
}

void ImageInfoTool::float_slider_cb( Fl_Slider* s, void* data )
{
    Fl_Float_Input* n = (Fl_Float_Input*) data;
    char buf[64];
    sprintf( buf, "%g", s->value() );
    n->value( buf );
    n->do_callback();
}

void ImageInfoTool::int_slider_cb( Fl_Slider* s, void* data )
{
    Fl_Int_Input* n = (Fl_Int_Input*) data;
    char buf[64];
    sprintf( buf, "%g", s->value() );
    n->value( buf );
    n->do_callback();
}

#if 0
static bool modify_string( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    Imf::StringAttribute attr( w->value() );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2i( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    int x, y;
    int num = sscanf( w->value(), "%d %d", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two integers for vector ") << i->first );
        return false;
    }

    Imath::V2i val( x, y );
    Imf::V2iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2f( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    float x, y;
    int num = sscanf( w->value(), "%g %g", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two floats for vector ") << i->first );
        return false;
    }

    Imath::V2f val( x, y );
    Imf::V2fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2d( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    double x, y;
    int num = sscanf( w->value(), "%lg %lg", &x, &y );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two doubles for vector ") << i->first );
        return false;
    }

    Imath::V2d val( x, y );
    Imf::V2dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3i( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    int x, y, z;
    int num = sscanf( w->value(), "%d %d %d", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three integers for vector ") << i->first );
        return false;
    }

    Imath::V3i val( x, y, z );
    Imf::V3iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3f( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    float x, y, z;
    int num = sscanf( w->value(), "%g %g %g", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three floats for vector ") << i->first );
        return false;
    }

    Imath::V3f val( x, y, z );
    Imf::V3fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3d( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    double x, y, z;
    int num = sscanf( w->value(), "%lg %lg %lg", &x, &y, &z );
    if ( num != 3 ) {
        mrvALERT( _("Could not find three doubles for vector ") << i->first );
        return false;
    }

    Imath::V3d val( x, y, z );
    Imf::V3dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_chromaticities( Fl_Input* w,
                                   tl::io::Attribute::iterator& i)
{
    float rx, ry, gx, gy, bx, by, wx, wy;
    int num = sscanf( w->value(), "%g %g  %g %g  %g %g  %g %g",
                      &rx, &ry, &gx, &gy, &bx, &by, &wx, &wy );
    if ( num != 8 ) {
        mrvALERT( _("Could not find eight floats for chromaticities ")
                  << i->first );
        return false;
    }

    Imf::Chromaticities val( Imath::V2f( rx, ry ),
                             Imath::V2f( gx, gy ),
                             Imath::V2f( bx, by ),
                             Imath::V2f( wx, wy ) );
    Imf::ChromaticitiesAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33f( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    float m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->value(),
                      "%g %g %g  %g %g %g  %g %g %g",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) {
        mrvALERT( _("Could not find nine floats for matrix 3x3 ")
                  << i->first );
        return false;
    }

    Imath::M33f val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33d( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    double m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->value(),
                      "%lg %lg %lg  %lg %lg %lg  %lg %lg %lg",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) {
        mrvALERT( _("Could not find nine doubles for matrix 3x3 ")
                  << i->first );
        return false;
    }

    Imath::M33d val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44f( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    float m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->value(),
                      "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                      &m00, &m01, &m02, &m03,
                      &m10, &m11, &m12, &m13,
                      &m20, &m21, &m22, &m23,
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) {
        mrvALERT( _("Could not find sixteen floats for matrix 4x4 ")
                  << i->first );
        return false;
    }

    Imath::M44f val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44d( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    double m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->value(),
                      "%lg %lg %lg %lg  %lg %lg %lg %lg  "
                      "%lg %lg %lg %lg  %lg %lg %lg %lg",
                      &m00, &m01, &m02, &m03,
                      &m10, &m11, &m12, &m13,
                      &m20, &m21, &m22, &m23,
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) {
        mrvALERT( _("Could not find sixteen doubles for matrix 4x4 ")
                  << i->first );
        return false;
    }

    Imath::M44d val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}


static bool modify_box2i( Fl_Input* widget, tl::io::Attribute::iterator& i)
{
    int x, y, w, h;
    int num = sscanf( widget->value(),
                      "%d %d  %d %d", &x, &y, &w, &h );
    if ( num != 4 ) {
        mrvALERT( _("Could not find four integers for box ")
                  << i->first );
        return false;
    }

    Imath::Box2i val( Imath::V2i( x, y ), Imath::V2i( w, h ) );
    Imf::Box2iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}
static bool modify_box2f( Fl_Input* widget, tl::io::Attribute::iterator& i)
{
    float x, y, w, h;
    int num = sscanf( widget->value(),
                      "%g %g  %g %g", &x, &y, &w, &h );
    if ( num != 4 ) {
        mrvALERT( _("Could not find four floats for box ")
                  << i->first );
        return false;
    }

    Imath::Box2f val( Imath::V2f( x, y ), Imath::V2f( w, h ) );
    Imf::Box2fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_rational( Fl_Input* widget,
                             tl::io::Attribute::iterator& i)
{
    int n, d;
    int num = sscanf( widget->value(),
                      "%d / %d", &n, &d );
    if ( num != 2 ) {
        mrvALERT( _("Could not find two integers for rational ")
                  << i->first );
        return false;
    }

    Imf::Rational val( n, d );
    Imf::RationalAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_value( Fl_Input* w, tl::io::Attribute::iterator& i)
{
    if ( dynamic_cast< Imf::StringAttribute* >( i->second ) != NULL )
        return modify_string( w, i );
    else if ( dynamic_cast< Imf::M44dAttribute* >( i->second ) != NULL )
        return modify_m44d( w, i );
    else if ( dynamic_cast< Imf::M44fAttribute* >( i->second ) != NULL )
        return modify_m44f( w, i );
    else if ( dynamic_cast< Imf::M33dAttribute* >( i->second ) != NULL )
        return modify_m33d( w, i );
    else if ( dynamic_cast< Imf::M33fAttribute* >( i->second ) != NULL )
        return modify_m33f( w, i );
    else if ( dynamic_cast< Imf::ChromaticitiesAttribute* >( i->second ) )
        return modify_chromaticities( w, i );
    else if ( dynamic_cast< Imf::Box2iAttribute* >( i->second ) != NULL )
        return modify_box2i( w, i );
    else if ( dynamic_cast< Imf::Box2fAttribute* >( i->second ) != NULL )
        return modify_box2f( w, i );
    else if ( dynamic_cast< Imf::V3dAttribute* >( i->second ) != NULL )
        return modify_v3d( w, i );
    else if ( dynamic_cast< Imf::V3fAttribute* >( i->second ) != NULL )
        return modify_v3f( w, i );
    else if ( dynamic_cast< Imf::V3iAttribute* >( i->second ) != NULL )
        return modify_v3i( w, i );
    else if ( dynamic_cast< Imf::V2dAttribute* >( i->second ) != NULL )
        return modify_v2d( w, i );
    else if ( dynamic_cast< Imf::V2fAttribute* >( i->second ) != NULL )
        return modify_v2f( w, i );
    else if ( dynamic_cast< Imf::V2iAttribute* >( i->second ) != NULL )
        return modify_v2i( w, i );
    else if ( dynamic_cast< Imf::RationalAttribute* >( i->second ) != NULL )
        return modify_rational( w, i );
    else
        LOG_ERROR( _("Unknown attribute to convert from string") );
    return false;
}

static bool modify_keycode( Fl_Int_Input* w,
                            tl::io::Attribute::iterator& i,
                            const std::string& subattr )
{
    Imf::KeyCodeAttribute* attr(
        dynamic_cast<Imf::KeyCodeAttribute*>(i->second ) );
    if ( !attr ) return false;

    Imf::KeyCode t = attr->value();
    try
    {
        if ( subattr == "filmMfcCode" )
        {
            t.setFilmMfcCode( atoi( w->value() ) );
        }
        else if ( subattr == "filmType" )
        {
            t.setFilmType( atoi( w->value() ) );
        }
        else if ( subattr == "prefix" )
        {
            t.setPrefix( atoi( w->value() ) );
        }
        else if ( subattr == "count" )
        {
            t.setCount( atoi( w->value() ) );
        }
        else if ( subattr == "perfOffset" )
        {
            t.setPerfOffset( atoi( w->value() ) );
        }
        else if ( subattr == "perfsPerFrame" )
        {
            t.setPerfsPerFrame( atoi( w->value() ) );
        }
        else if ( subattr == "perfsPerCount" )
        {
            t.setPerfsPerCount( atoi( w->value() ) );
        }
        else
        {
            mrvALERT( _("Unknown KeyCode subattr") );
            return false;
        }
        Imf::KeyCodeAttribute nattr( t );
        delete i->second;
        i->second = nattr.copy();
        update_int_slider( w );
    } catch ( const std::exception& e )
    {
        mrvALERT( e.what() );
    }

    return true;
}
#endif

static bool modify_int( Fl_Int_Input* w, tl::io::Attribute::iterator& i)
{
    update_int_slider( w );
    return true;
}

static bool modify_float( Fl_Float_Input* w, tl::io::Attribute::iterator& i)
{
    update_float_slider( w );
    return true;
}

static void change_float_cb( Fl_Float_Input* w, ImageInfoTool* info )
{
    // Media* img = dynamic_cast<Media*>( info->get_image() );
    // if ( !img ) return;

    // Fl_Group* g = (Fl_Group*)w->parent()->parent();

    // for ( int j = 0; j < g->children(); ++j )
    // {
    //     Fl_Group* sg = dynamic_cast< Fl_Group* >( g->child(j) );
    //     if ( !sg || sg->children() == 0 ) continue;

    //     Fl_Widget* widget = sg->child(0);  // Fl_Box Label

    //     Fl_Group* tg = dynamic_cast< Fl_Group* >( g->child(j+1) );
    //     if ( !tg || tg->children() == 0 ) continue;
    //     Fl_Widget* sw = tg->child(0);  // Fl_Float_Input Value

    //     if ( !widget->label() || sw != w ) continue;
    //     std::string key = widget->label();

    //     tl::io::Attribute& attributes; //= img->attributes();
    //     tl::io::Attribute::iterator i = attributes.find( key );
    //     if ( i != attributes.end() )
    //     {
    //         if ( key == "rotate" || key == "Video rotate" ||
    //              key == _("Video rotate") )
    //         {

    //             Imf::FloatAttribute attr( atof( w->value() ) );
    //             attributes[key] = attr.copy();

    //             img->image_damage( img->image_damage() |
    //                                Media::kDamageContents );
    //             info->view()->redraw();
    //         }
    //         modify_float( w, i );
    //         return;
    //     }
    // }



    // Fl_Widget* widget = g->child(0);
    // if ( !widget->label() ) return;

}

static void change_string_cb( Fl_Input* w, ImageInfoTool* info )
{
    // Media* img = dynamic_cast<Media*>( info->get_image() );
    // if ( !img ) {
    //     LOG_ERROR( "Image is invalid" );
    //     return;
    // }


    // mrv::Table* t = dynamic_cast< mrv::Table* >( w->parent()->parent() );
    // if ( t == NULL )
    // {
    //     return;
    // }

    // Fl_Widget* box;
    // bool found = false;
    // for ( int r = 0; r < t->rows(); ++r )
    // {
    //     int i = 2 * r;
    //     if ( i >= t->children() ) break;
    //     Fl_Group* sg = dynamic_cast< Fl_Group* >( t->child(i) );
    //     if ( !sg ) {
    //         break;
    //     }
    //     box = (Fl_Widget*)sg->child(0);
    //     Fl_Widget* widget = t->child(i+1);
    //     if ( widget == w ) {
    //         found = true;
    //         break;
    //     }
    // }

    // if (!found )
    // {
    //     LOG_ERROR( _("Could not find attribute \"") << w->label() << "\"");
    //     return;
    // }


    // if ( !box->label() ) {
    //     LOG_ERROR( _("Widget has no label") );
    //     return;
    // }

    // std::string key = box->label();
    // tl::io::Attribute& attributes = img->attributes();
    // tl::io::Attribute::iterator i = attributes.find( key );
    // if ( i != attributes.end() )
    // {
    //     bool ok = modify_value( w, i );
    //     if (!ok) {
    //         info->filled = false;
    //         info->refresh();
    //         toggle_modify_attribute( key, info );
    //     }
    //     return;
    // }
}

static void change_int_cb( Fl_Int_Input* w, ImageInfoTool* info )
{
    // Media* img = dynamic_cast<Media*>( info->get_image() );
    // if ( !img ) return;

    // Fl_Group* g = (Fl_Group*)w->parent()->parent();
    // Fl_Widget* widget = g->child(0);
    // if ( !widget->label() ) return;

    // std::string key = widget->label();
    // tl::io::Attribute& attributes = img->attributes();
    // tl::io::Attribute::iterator i = attributes.find( key );
    // if ( i != attributes.end() )
    // {
    //     modify_int( w, i );
    //     return;
    // }
}

static void change_keycode_cb( Fl_Int_Input* w, ImageInfoTool* info )
{

    // Fl_Group* g = (Fl_Group*)w->parent()->parent();
    // Fl_Widget* widget = g->child(0);
    // if ( !widget->label() ) return;

    // std::string key = widget->label();
    // std::string subattr;
    // size_t p = key.rfind( '.' );
    // if ( p != std::string::npos )
    // {
    //     subattr = key.substr( p+1, key.size() );
    //     key  = key.substr( 0, p );
    // }
    // tl::io::Attribute& attributes = img->attributes();
    // tl::io::Attribute::iterator i = attributes.find( key );
    // if ( i != attributes.end() )
    // {
    //     modify_keycode( w, i, subattr );
    //     return;
    // }
}

static void change_first_frame_cb( Fl_Int_Input* w, ImageInfoTool* info )
{
    int  first  = atoi( w->value() );
    const auto& player = info->timelinePlayer();
    auto range = player->inOutRange();
    auto start = range.start_time();
    const auto& end_time = range.end_time_inclusive();
    start = otime::RationalTime( first, start.rate() );
    range = otime::TimeRange::range_from_start_end_time_inclusive( start,
                                                                   end_time );
    player->setInOutRange( range );
    info->main()->uiTimeline->redraw();
}

static void change_last_frame_cb( Fl_Int_Input* w,
                                  ImageInfoTool* info )
{
    int  last  = atoi( w->value() );
    const auto& player = info->timelinePlayer();
    auto range = player->inOutRange();
    const auto& start_time = range.start_time();
    auto end = range.end_time_inclusive();
    end = otime::RationalTime( last, end.rate() );
    range = otime::TimeRange::range_from_start_end_time_inclusive( start_time,
                                                                   end );
    player->setInOutRange( range );
    info->main()->uiTimeline->redraw();
}



static void change_fps_cb( Fl_Float_Input* w, ImageInfoTool* info )
{
    float f = (float) atof( w->value() );
    const auto player = info->timelinePlayer();
    if (! player ) return;
    player->setSpeed( f );
    info->main()->uiFPS->value( f );
    update_float_slider( w );
}


static void change_scale_x_cb( Fl_Float_Input* w, ImageInfoTool* info )
{

    // img->scale_x( atof( w->value() ) );
    // update_float_slider( w );

    // info->main()->uiView->redraw();
}

static void change_scale_y_cb( Fl_Float_Input* w, ImageInfoTool* info )
{
    // Media* img = info->get_image();

    // img->scale_y( atof( w->value() ) );
    // update_float_slider( w );

    // info->main()->uiView->redraw();
}

static void change_x_cb( Fl_Float_Input* w, ImageInfoTool* info )
{
    // Media* img = info->get_image();

    // img->x( atof( w->value() ) );
    // update_float_slider( w );

    // info->main()->uiView->redraw();
}

static void change_y_cb( Fl_Float_Input* w, ImageInfoTool* info )
{
    // Media* img = info->get_image();

    // img->y( atof( w->value() ) );
    // update_float_slider( w );

    // info->main()->uiView->redraw();
}


// static void r3d_camera_cb( Fl_Button* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->load_camera_settings();
//     img->image_damage( mrv::Media::kDamageAll );
//     info->refresh();
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_sidecar_cb( Fl_Button* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage* >( info->get_image() );
//     if ( !img ) return;

//     img->load_rmd_sidecar();
//     img->image_damage( mrv::Media::kDamageAll );
//     info->refresh();
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }


// static void change_gain_blue_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->GainBlue( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_gain_green_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->GainGreen( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_gain_red_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->GainRed( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_exposure_compensation_cb( Fl_Float_Input* w,
//                                              ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->ExposureCompensation( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_exposure_adjust_cb( Fl_Float_Input* w,
//                                        ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->ExposureAdjust( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_brightness_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Brightness( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_contrast_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Contrast( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_flut_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Flut( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_kelvin_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Kelvin( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_tint_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Tint( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_saturation_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Saturation( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_shadow_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Shadow( (float) atof( w->value() ) );
//     update_float_slider( w );

//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }


// static void change_blend_cb( mrv::PopupMenu* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     unsigned idx = w->value();
//     img->hdr_mode( (R3DSDK::HdrMode) idx );
//     w->copy_label( w->child( idx )->label() );
//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_trackno_cb( Fl_Int_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->trackNo( atoi( w->value() ) );
//     img->image_damage( mrv::Media::kDamageAll );
//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_bias_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     img->Bias( (float) atof( w->value() ) );
//     update_float_slider( w );


//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_sharpness_cb( mrv::PopupMenu* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;


//     int v = w->value();
//     w->copy_label( w->child(v)->label() );
//     img->Sharpness( (R3DSDK::ImageOLPFCompensation) v );
//     img->refetch();

//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_denoise_cb( mrv::PopupMenu* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     int v = w->value();
//     w->copy_label( w->child(v)->label() );
//     img->Denoise( (R3DSDK::ImageDenoise) v );
//     img->refetch();

//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }

// static void change_detail_cb( mrv::PopupMenu* w, ImageInfoTool* info )
// {
//     R3dImage* img = dynamic_cast< R3dImage*>( info->get_image() );
//     if ( !img ) return;

//     int v = w->value();
//     w->copy_label( w->child(v)->label() );
//     img->Detail( (R3DSDK::ImageDetail) v );
//     img->refetch();

//     mrv::GLViewport* view = info->main()->uiView;
//     view->redraw();
// }



// static void change_gamma_cb( Fl_Float_Input* w, ImageInfoTool* info )
// {
//     Media* img = info->get_image();

//     float g = (float) atof( w->value() );
//     img->gamma( g );
//     update_float_slider( w );


//     mrv::GLViewport* view = info->main()->uiView;
//     view->gamma( g );
//     view->redraw();
// }

double ImageInfoTool::to_memory( long double value,
                                    const char*& extension )
{
    if ( value >= 1099511627776 )
    {
        value /= 1099511627776;
        extension = N_("Tb");
    }
    else if ( value >= 1073741824 )
    {
        value /= 1073741824;
        extension = N_("Gb");
    }
    else if ( value >= 1048576 )
    {
        value /= 1048576;
        extension = N_("Mb");
    }
    else if ( value >= 1024 )
    {
        value /= 1024;
        extension = N_("Kb");
    }
    else
    {
        extension = N_("bytes");
    }
    return value;
}

ImageInfoTool::~ImageInfoTool()
{

}

TimelinePlayer* ImageInfoTool::timelinePlayer() const
{
    return player;
}

void ImageInfoTool::setTimelinePlayer( TimelinePlayer* timelinePlayer )
{
    player = timelinePlayer;
    refresh();
}

void ImageInfoTool::hide_tabs()
{

    DBG2;
    g->tooltip( _("Load an image or movie file") );

    m_image->hide();
    m_video->hide();
    m_audio->hide();
    m_subtitle->hide();
    m_attributes->hide();

    m_curr = nullptr;
    DBG3;
}


void ImageInfoTool::fill_data()
{
    if ( !player ) return;

    kMiddle = g->w() / 2;;
    
    char buf[1024];
    m_curr = add_browser(m_image);

    DBGM1( "m_curr=" << m_curr );
    const auto tplayer = player->timelinePlayer();
    if ( !tplayer ) return;
    
    const auto info = tplayer->getIOInfo();

    const auto path   = player->path();
    const auto directory = path.getDirectory();

    const auto audioPath   = player->audioPath();
    const otime::RationalTime& time = player->currentTime();

    std::string fullname = createStringFromPathAndTime( path, time );

    add_text( _("Directory"), _("Directory where clip resides"), directory );


    add_text( _("Filename"), _("Filename of the clip"), fullname );

    if ( !audioPath.isEmpty() && path != audioPath )
    {
        add_text( _("Audio Directory"), _("Directory where audio clip resides"),
                  audioPath.getDirectory() );

        add_text( _("Audio Filename"), _("Filename of the audio clip"),
                  audioPath.get( -1, false ) );

    }

    ++group;

    DBGM1( "m_curr=" << m_curr );



    unsigned num_video_streams = info.video.size();
    // @todo: tlRender does not handle multiple audio tracks
    unsigned num_audio_streams = info.audio.isValid();
    // @todo: tlRender does not handle subtitle tracks
    unsigned num_subtitle_streams = 0;


    add_int( _("Video Streams"), _("Number of video streams in file"),
             num_video_streams );
    add_int( _("Audio Streams"), _("Number of audio streams in file"),
             num_audio_streams );
        // add_int( _("Subtitle Streams"),
        //          _("Number of subtitle streams in file"),
        //          num_subtitle_streams );
    
    const otime::TimeRange& range = player->timeRange();
    int64_t first= range.start_time().to_frames();
    int64_t last = range.end_time_inclusive().to_frames();
    add_int( _("Start Frame"), _("Beginning frame of clip"),
             (int)first, false );
    add_int( _("End Frame"), _("Ending frame of clip"),
             (int)last, false );

    const otime::TimeRange& iorange = player->inOutRange();
    first = iorange.start_time().to_frames();
    last =  iorange.end_time_inclusive().to_frames();

    add_int( _("First Frame"), _("First frame of clip - User selected"),
             (int)first, true, true,
             (Fl_Callback*)change_first_frame_cb, first, last );
    add_int( _("Last Frame"), _("Last frame of clip - User selected"),
             (int)last, true, true,
             (Fl_Callback*)change_last_frame_cb, 2, last );

    float   fps  = player->speed();
    add_float( _("FPS"), _("Frames Per Second"), fps, true, true,
               (Fl_Callback*)change_fps_cb, 1.0f, 60.0f,
               FL_WHEN_CHANGED );

    ++group;

    m_image->show();

    
    if ( num_video_streams > 0 )
    {
        for ( int i = 0; i < num_video_streams; ++i )
        {

            char buf[256];
            sprintf( buf, _("Video Stream #%d"), i+1 );

            m_curr = add_browser( m_video );

            m_curr->copy_label( buf );

            add_bool( _("Known Codec"), _("mrViewer knows codec used"),
                      true );

            const auto& video = info.video[i];
            const auto& size = video.size;

            add_int( _("Width"), _("Width of clip"), (unsigned)size.w,
                     false );
            add_int( _("Height"), _("Height of clip"), (unsigned)size.h,
                     false );


            double aspect_ratio = (double)size.w / (double) size.h;


            const char* name = _("Unknown");
            int num = sizeof( kAspectRatioNames ) / sizeof(aspectName_t);
            constexpr double fuzz = 0.001;
            for ( int i = 0; i < num; ++i )
            {
                if ( mrv::is_equal( aspect_ratio, kAspectRatioNames[i].ratio,
                                    fuzz ) )          {
                    name = _( kAspectRatioNames[i].name );
                    break;
                }
            }


            sprintf( buf, N_("%g (%s)"), aspect_ratio, name );
            add_text( _("Aspect Ratio"), _("Aspect ratio of clip"), buf );


            add_float( _("Pixel Ratio"), _("Pixel ratio of clip"),
                       size.pixelAspectRatio, false, true );

            ++group;



            tl::imaging::PixelType pixelType = video.pixelType;
            uint8_t   pixelDepth = tl::imaging::getBitDepth( pixelType );
            uint8_t channelCount = tl::imaging::getChannelCount( pixelType );

            const char* depth;
            switch( pixelDepth )
            {
            case 8:
                depth = _("unsigned byte (8-bits per channel)");
                break;
            case 12:
                depth = _("(12-bits per channel)");
                break;
            case 16:
                depth = _("unsigned short (16-bits per channel)");
                if ( pixelType == tl::imaging::PixelType::RGB_F16 ||
                     pixelType == tl::imaging::PixelType::RGBA_F16 )
                    depth = _("half float (16-bits per channel)");
                break;
            case 32:
                depth = _("unsigned int (32-bits per channel)");
                if ( pixelType == tl::imaging::PixelType::RGB_F32 ||
                     pixelType == tl::imaging::PixelType::RGBA_F32 )
                    depth = _("float (32-bits per channel)");
                break;
            default:
                depth = _("Unknown bit depth");
                break;
            }

            add_text( _("Depth"), _("Bit depth of clip"), depth );

            add_int( _("Image Channels"), _("Number of channels in clip"),
                     channelCount, false );


            name = "";
            double fps = player->defaultSpeed();
            
            if      ( is_equal( fps, 29.97 ) )     name = "(NTSC)";
            else if ( is_equal( fps, 30.0 ) )      name = "(60hz HDTV)";
            else if ( is_equal( fps, 25.0 ) )      name = "(PAL)";
            else if ( is_equal( fps, 24.0 ) )      name = "(Film)";
            else if ( is_equal( fps, 50.0 ) )      name = _("(PAL Fields)");
            else if ( is_equal( fps, 59.940059 ) ) name = _("(NTSC Fields)");


            sprintf( buf, "%g %s", fps, name );

            add_text( _("FPS"), _("Frames per Second"), buf );
            
            std::vector< std::string > yuvCoeffs =
                tl::imaging::getYUVCoefficientsLabels();
            add_enum( _("YUV Coefficients"),
                      _("YUV Coefficients used for video conversion"),
                      getLabel( video.yuvCoefficients ), yuvCoeffs, false );

            std::vector< std::string > videoLevels =
                tl::imaging::getVideoLevelsLabels();
            add_enum( _("Video Levels"), _("Video Levels"),
                      getLabel( video.videoLevels ), videoLevels, false );


            ++group;

            std::string format = tl::imaging::getLabel( pixelType );

            add_text( _("Render Pixel Format"), _("Render Pixel Format"),
                      format.c_str() );

        }

        m_video->show();
        
#if 0
            add_ocio_ics( _("Input Color Space"),
                          _("OCIO Input Color Space"),
                          img->ocio_input_color_space().c_str() );

            DBG3;
            ++group;
#endif


#if 0

    DBG3;
    if ( !img->has_video() )
    {
        add_text( _("Line Order"), _("Line order in file"),
                  img->line_order() );
    }



    if ( !img->has_video() )
    {
        ++group;

    DBG3;

        add_text( _("Compression"), _("Clip Compression"), img->compression() );


    }

    ++group;
#endif

    }

#if 0


    DBG3;
    const char* space_type = NULL;
    double memory_space = double( to_memory( (long double)img->memory(),
                                  space_type ) );
    sprintf( buf, N_("%.3f %s"), memory_space, space_type );
    add_text( _("Memory"), _("Memory without Compression"), buf );


    DBG3;
    if ( img->disk_space() >= 0 )
    {

        double disk_space = double( to_memory( (long double)img->disk_space(),
                                               space_type ) );

        double pct   = double( 100.0 * ( (long double) img->disk_space() /
                                         (long double) img->memory() ) );


    DBG3;
        sprintf( buf, N_("%.3f %s  (%.2f %% of memory size)"),
                 disk_space, space_type, pct );

        add_text( _("Disk space"), _("Disk space"), buf );



        if ( !img->has_video() )
        {
            double ratio = 100.0 - double(pct);
            sprintf( buf, _("%4.8g %%"), ratio );

            add_text( _("Compression Ratio"), _("Compression Ratio"), buf );
        }

    }


    DBG3;

    ++group;
    add_text( _("Creation Date"), _("Creation Date"), img->creation_date() );


    DBG3;


    DBG3;
#endif



    DBG3;

    g->tooltip( NULL );


    auto& attributes = info.tags;
    {
        m_curr = add_browser( m_attributes );

        for ( const auto& attribute : attributes )
        {
            add_text( _(attribute.first.c_str()), "", attribute.second );
        }

        m_attributes->show();
    }


    if ( num_audio_streams > 0 )
    {
        for ( int i = 0; i < num_audio_streams; ++i )
        {
            char buf[256];

            m_curr = add_browser( m_audio );
            sprintf( buf, _("Audio Stream #%d"), i+1 );
            m_curr->copy_label( buf );

            // @todo: tlRender handles only one audio track
            const auto& audio = info.audio;



            add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      audio.isValid() );
#if 0
            add_text( _("Codec"), _("Codec Name"), s.codec_name );
            add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
#endif
            ++group;

            const char* channels = "Stereo";
            switch( audio.channelCount )
            {
            case 1:
                channels = "Mono"; break;
            case 6:
                channels = "5:1"; break;
            case 8:
                channels = "7:1"; break;
            default:
                sprintf( buf, N_("%d"), audio.channelCount );
                channels = buf;
                break;
            }


            add_text( _("Channels"), _("Number of audio channels"), channels );

            add_text( _("Format"), _("Format"), getLabel( audio.dataType ) );
            sprintf( buf, _("%d Hz."), audio.sampleRate );
            add_text( _("Frequency"), _("Frequency of audio"), buf );

#if 0
            sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
            add_text( _("Max. Bitrate"), _("Max. Bitrate"), buf );
#endif

            ++group;

            add_text( _("Language"), _("Language if known"), audio.name );
            ++group;

#if 0
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition);
            ++group;

            add_time( _("Start"), _("Start of Audio"), s.start, fps );
            add_time( _("Duration"), _("Duration of Audio"),
                      s.duration, fps );
#endif

        }


        m_audio->show();

    }

#if 0
    if ( num_subtitle_streams > 0 )
    {
        for ( int i = 0; i < num_subtitle_streams; ++i )
        {
            char buf[256];

            m_curr = add_browser( m_subtitle );
            sprintf( buf, _("Subtitle Stream #%d"), i+1 );
            m_curr->copy_label( buf );

            const Media::subtitle_info_t& s = img->subtitle_info(i);

            add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      s.has_codec );
            add_text( _("Codec"), _("Codec name"), s.codec_name );
            add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
            add_bool( _("Closed Captions"), _("Video has Closed Captions"),
                      s.closed_captions );
            ++group;

            sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
            add_text( _("Avg. Bitrate"), _("Avg. Bitrate"), buf );

            ++group;
            add_text( _("Language"), _("Language if known"), s.language );
            ++group;
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition );

            ++group;
            add_time( _("Start"), _("Start of Subtitle"), s.start, img->fps() );
            add_time( _("Duration"), _("Duration of Subtitle"),
                      s.duration, img->fps() );

            //    m_curr->relayout();
            //     m_curr->parent()->relayout();
        }

        m_subtitle->show();
    }
#endif

    
}

void ImageInfoTool::refresh()
{
    TLRENDER_P();
    
    hide_tabs();

    m_image->clear();
    m_video->clear();
    m_audio->clear();
    m_subtitle->clear();
    m_attributes->clear();

    fill_data();


    m_image->end();
    m_attributes->end();
    m_video->end();
    m_audio->end();
    m_subtitle->end();

    Pack*       pack  = g->get_pack();
    pack->layout();
    p.ui->uiDock->pack->layout();
    int sw = Fl::scrollbar_size();                // scrollbar width
    g->size( g->w()-sw, pack->h() + 20 );
    p.ui->uiResizableBar->HandleDrag(0);

    DBG3;
}


mrv::Table* ImageInfoTool::add_browser( mrv::CollapsibleGroup* g )
{
    if (!g) return nullptr;

    X = 0;
    Y = g->y() + line_height();

    mrv::Table* table = new mrv::Table( 0, Y, g->w(), 20 );
    table->column_separator(true);
    //table->auto_resize( true );
    table->labeltype(FL_NO_LABEL);
    static const char* headers[] = { _("Attribute"), _("Value"), 0 };
    table->column_labels( headers );
    table->col_width( 0, kMiddle );
    table->col_width( 1, kMiddle );

    table->align(FL_ALIGN_CENTER);
    table->end();

    g->add( table );

    group = row = 0; // controls line colors

    return table;
}

int ImageInfoTool::line_height()
{
    return 24;
}

Fl_Color ImageInfoTool::get_title_color()
{
    return kTitleColors[ group % kSizeOfTitleColors ];
}

Fl_Color ImageInfoTool::get_widget_color()
{
    Fl_Color col = kRowColors[ row % kSizeOfRowColors ];
    ++row;
    return col;
}



void ImageInfoTool::compression_cb( mrv::PopupMenu* t, ImageInfoTool* v )
{
    // unsigned   idx = t->value();
    // Media* img = v->get_image();
    // img->compression( idx );
    // t->label( t->child(idx)->label() );
    // v->filled = false;
    // v->refresh();
}


void ImageInfoTool::add_button( const char* name,
                                const char* tooltip,
                                Fl_Callback* callback,
                                Fl_Callback* callback2 )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    int hh = line_height();
    Y += hh;

    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    g = new Fl_Group( kMiddle, Y, g->w() - kMiddle, hh );
    {
        int w2 = g->w() - kMiddle;
        w2 /= 2;
        Fl_Button* widget = new Fl_Button( kMiddle, Y, w2, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("Load") );
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w2, Y, w2, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("Reset") );
        if ( callback )
            widget->callback( (Fl_Callback*)callback2, (void*)this );
        g->end();
    }
    m_curr->add( g );

    m_curr->end();
}


void ImageInfoTool::add_scale( const char* name,
                                  const char* tooltip,
                                  int pressed,
                                  int num_scales,
                                  Fl_Callback* callback )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    int hh = line_height();
    Y += hh;

    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    g = new Fl_Group( kMiddle, Y, g->w() - kMiddle, hh );
    {
        int w5 = g->w() - kMiddle;
        w5 /= num_scales;
        Fl_Button* widget = new Fl_Button( kMiddle, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:1") );
        if ( pressed == 0 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:2") );
        if ( pressed == 1 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5*2, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:4") );
        if ( pressed == 2 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        widget = new Fl_Button( kMiddle+w5*3, Y, w5, hh );
        widget->tooltip( tooltip );
        widget->copy_label( _("1:8") );
        if ( pressed == 3 ) widget->value(1);
        else widget->value(0);
        if ( callback )
            widget->callback( (Fl_Callback*)callback, (void*)this );
        if ( num_scales > 4 )
        {
            widget = new Fl_Button( kMiddle+w5*4, Y, w5, hh );
            widget->tooltip( tooltip );
            widget->copy_label( _("1:16") );
            if ( pressed == 4 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
        }
        g->end();
    }
    m_curr->add( g );

    m_curr->end();
}



void ImageInfoTool::add_ocio_ics( const char* name,
                                     const char* tooltip,
                                     const char* content,
                                     const bool editable,
                                     Fl_Callback* callback )
{
    if ( !editable )
        return add_text( name, tooltip, content );

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Group* sg = new Fl_Group( kMiddle, Y, kMiddle, hh );
        sg->end();

        Fl_Input* widget = new Fl_Input( kMiddle, Y, sg->w()-50, hh );
        widget->value( content );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        widget->tooltip( tooltip ? tooltip : lbl->label() );

        sg->add( widget );

        Fl_Button* pick = new Fl_Button( kMiddle + sg->w()-50, Y, 50, hh,
                                         _("Pick") );
        pick->callback( (Fl_Callback*)attach_ocio_ics_cb, view() );
        sg->add( pick );

        m_curr->add( sg );
    }
    m_curr->end();
}

void ImageInfoTool::add_text( const char* name,
                              const char* tooltip,
                              const char* content,
                              const bool editable,
                              const bool active,
                              Fl_Callback* callback )
{

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        g->end();
    }
    m_curr->add( g );

    {
        Fl_Widget* widget = NULL;
        if ( !editable )
        {
            Fl_Output* o = new Fl_Output( kMiddle, Y, g->w()-kMiddle, hh );
            widget = o;
            o->value( content );
            o->textcolor( FL_BLACK );
        }
        else
        {
            Fl_Input* o = new Fl_Input( kMiddle, Y, g->w()-kMiddle, hh );
            widget = o;
            o->value( content );
            o->textcolor( FL_BLACK );
        }
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( !editable )
        {
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        if (!active) widget->deactivate();
        m_curr->add( widget );
    }
    m_curr->end();
}


void ImageInfoTool::add_text( const char* name,
                                 const char* tooltip,
                                 const std::string& content,
                                 const bool editable,
                                 const bool active,
                                 Fl_Callback* callback )
{
    add_text( name, tooltip, content.c_str(), editable, active, callback );
}

void ImageInfoTool::add_int( const char* name, const char* tooltip,
                             const int content, const bool editable,
                             const bool active,
                             Fl_Callback* callback,
                             const int minV, const int maxV,
                             const int when )
{

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, kMiddle, hh );
        p->end();
        p->box( FL_FLAT_BOX );
        // p->set_horizontal();
        p->begin();

        if ( !editable )
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "   %d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 50, hh );
            sprintf( buf, "   %d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            Fl_Slider* slider = new Fl_Slider( kMiddle+50, Y, p->w()-40, hh );
            // slider->type(Fl_Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );
            int maxS = maxV;

            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > maxS ) maxS = content+50;
            slider->maximum( maxS );

            slider->value( content );
            slider->step( 1.0 );
            // slider->slider_size(10);
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->when( when );
            slider->callback( (Fl_Callback*)int_slider_cb, widget );

            p->resizable(slider);
        }
        p->end();
        m_curr->add( p );
        if ( !active )
        {
            p->deactivate();
        }
    }
    m_curr->end();
}

void ImageInfoTool::add_enum( const char* name,
                                 const char* tooltip,
                                 const size_t content,
                                 const char* const* options,
                                 const size_t num,
                                 const bool editable,
                                 Fl_Callback* callback
                               )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        mrv::PopupMenu* widget = new mrv::PopupMenu( kMiddle, Y,
                                                     g->w()-kMiddle, hh );
        widget->type( 0 );
        widget->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
        widget->color( colB );
        widget->labelcolor( FL_BLACK );
        widget->textcolor( FL_BLACK );
        for ( size_t i = 0; i < num; ++i )
        {
            widget->add( _( options[i] ) );
        }
        widget->value( unsigned(content) );
        widget->copy_label( _( options[content] ) );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        widget->menu_end();

        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        m_curr->add( widget );
    }
    m_curr->end();
}


void ImageInfoTool::add_enum( const char* name,
                                 const char* tooltip,
                                 const std::string& content,
                                 stringArray& options,
                                 const bool editable,
                                 Fl_Callback* callback
                               )
{
    size_t index;
    stringArray::iterator it = std::find( options.begin(), options.end(),
                                          content );
    if ( it != options.end() )
    {
        index = std::distance( options.begin(), it );
    }
    else
    {
        index = options.size();
        options.push_back( content );
    }

    size_t num = options.size();
    const char** opts = new const char*[num];
    for ( size_t i = 0; i < num; ++i )
        opts[i] = options[i].c_str();

    add_enum( name, tooltip,index, opts, num, editable, callback );

    delete [] opts;
}




void ImageInfoTool::add_int( const char* name,
                             const char* tooltip,
                             const unsigned int content,
                             const bool editable,
                             const bool active,
                             Fl_Callback* callback,
                             const unsigned int minV,
                             const unsigned int maxV )
{

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->copy_label( name );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        g->add( widget );
    }
    m_curr->add( g );

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, kMiddle, hh );
        // p->set_horizontal();
        p->begin();
        if ( !editable )
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "   %d", content );
            widget->value( buf );
            widget->box( FL_FLAT_BOX );
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            widget->deactivate();
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 60, hh );
            sprintf( buf, "   %d", content );
            widget->value( buf );
            widget->align(FL_ALIGN_CENTER );
            widget->textcolor( FL_BLACK );
            widget->color( colB );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            mrv::Slider* slider = new mrv::Slider( kMiddle+60, Y,
                                                   p->w()-60, hh );
            slider->type(mrv::Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );

            unsigned maxS = maxV;
            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > 10 && maxV <= 10 ) maxS = 100;
            else if ( content > maxS ) maxS = content+50;


            slider->maximum( maxS );
            slider->value( content );
            slider->step( 1.0 );
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->slider_size(10);
            slider->when( FL_WHEN_RELEASE );
            slider->callback( (Fl_Callback*)int_slider_cb, widget );

            p->resizable(slider);
        }
        
        p->end();
        m_curr->add( p );
        if ( !active ) {
            p->deactivate();
        }
    }

    m_curr->end();
}


void ImageInfoTool::add_time( const char* name, const char* tooltip,
                                 const double content,
                                 const double fps, const bool editable )
{
    int64_t seconds = (int64_t) content;
    int ms = (int) ((content - (double) seconds) * 1000);

    int64_t frame = int64_t( content * fps );


    char buf[128];

    sprintf( buf, _( "Frame %" PRId64 " " ), frame );
    std::string text = buf;

    sprintf( buf, _("%" PRId64 " seconds %d ms."), seconds, ms );
    text += buf;

    if ( content > 60.0 )
    {
        int64_t hours, minutes;
        hours    = seconds / 3600;
        seconds -= hours * 3600;
        minutes  = seconds / 60;
        seconds -= minutes * 60;

        sprintf( buf,
                 _(" ( %02" PRId64 ":%02" PRId64 ":%02" PRId64 "  %d ms. )"),
                 hours, minutes, seconds, ms );
        text += buf;
    }

    add_text( name, tooltip, text, false );
}

void ImageInfoTool::add_int64( const char* name,
                                  const char* tooltip,
                                  const int64_t content )
{

    char buf[128];
    sprintf( buf, N_("%" PRId64), content );
    add_text( name, tooltip, buf, false );
}

void ImageInfoTool::add_rect( const char* name, const char* tooltip,
                                 const tl::math::BBox2i& content,
                                 const bool editable, Fl_Callback* callback )
{

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->copy_label( name );
        widget->color( colA );
        widget->labelcolor( FL_BLACK );
        g->add( widget );
    }
    m_curr->add( g );

    char buf[64];
    unsigned dw = (g->w() - kMiddle) / 6;
    Fl_Group* g2 = new Fl_Group( kMiddle, Y, g->w()-kMiddle, hh );
    g2->end();
    if ( tooltip ) g2->tooltip( tooltip );
    else g2->tooltip( lbl->label() );
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, dw, hh );
        sprintf( buf, "%d", content.min.x );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->color( colB );
        widget->textcolor( FL_BLACK );
        widget->box( FL_FLAT_BOX );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw, Y, dw, hh );
        sprintf( buf, "%d", content.min.y );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*2, Y, dw, hh );
        sprintf( buf, "%d", content.max.x );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*3, Y, dw, hh );
        sprintf( buf, "%d", content.max.y );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle+dw*4, Y, dw,
                                                 hh, "W:" );
        sprintf( buf, "%d", content.w() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->color( colB );
        widget->labelcolor( FL_LIGHT3 );
        widget->textcolor( FL_BLACK );
        widget->deactivate();
        g2->add( widget );
    }
    {
        Fl_Int_Input* widget = new Fl_Int_Input( kMiddle + dw*5, Y, dw,
                                                 hh, "H:" );
        sprintf( buf, "%d", content.h() );
        widget->value( buf );
        widget->align(FL_ALIGN_LEFT);
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_LIGHT3 );
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        widget->deactivate();
        g2->add( widget );
    }
    m_curr->add( g2 );
    m_curr->end();
}

void ImageInfoTool::add_float( const char* name,
                                  const char* tooltip,
                                  const float content, const bool editable,
                                  const bool active,
                                  Fl_Callback* callback,
                                  const float minV, float maxV,
                                  const int when,
                                  const mrv::Slider::SliderType type )
{
    

    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;
    int hh = line_height();
    
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();
    
    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
    
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );
    

    {
        char buf[64];
        Fl_Group* p = new Fl_Group( kMiddle, Y, kMiddle, hh );
        p->box( FL_FLAT_BOX );
        // p->set_horizontal();
        p->begin();

        if ( !editable )
        {
            Fl_Float_Input* widget = new Fl_Float_Input( kMiddle, Y, p->w(), hh );
            sprintf( buf, "   %g", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
        }
        else
        {
            Fl_Float_Input* widget = new Fl_Float_Input( kMiddle, Y, 60, hh );
            sprintf( buf, "   %g", content );
            widget->value( buf );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textcolor( FL_BLACK );
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );

            if ( callback ) widget->callback( callback, this );

            mrv::Slider* slider = new mrv::Slider( kMiddle+60, Y,
                                                   p->w()-60, hh );
            slider->ticks(mrv::Slider::TICK_ABOVE);
            // slider->linesize(1);
            slider->type( FL_HORIZONTAL );
            slider->minimum( minV );

            double maxS = maxV;
            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > 10 && maxV <= 10 ) maxS = 100;
            else if ( content > maxS ) maxS = content+50;

            slider->maximum( maxS );
            slider->value( content );
            // slider->slider_size(10);
            if ( tooltip ) slider->tooltip( tooltip );
            else slider->tooltip( lbl->label() );
            slider->slider_type( type );
            slider->when( when );
            slider->callback( (Fl_Callback*)float_slider_cb, widget );

            p->resizable(slider);
        }
        p->end();
        m_curr->add( p );
        if ( !active ) {
            p->deactivate();
        }
    }
    m_curr->end();
}

void ImageInfoTool::add_bool( const char* name,
                              const char* tooltip,
                              const bool content,
                              const bool editable,
                              Fl_Callback* callback )
{
    Fl_Color colA = get_title_color();
    Fl_Color colB = get_widget_color();

    Fl_Box* lbl;

    int hh = line_height();
    Y += hh;
    Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
    g->end();

    {
        Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
        widget->box( FL_FLAT_BOX );
        widget->labelcolor( FL_BLACK );
        widget->copy_label( name );
        widget->color( colA );
        g->add( widget );
    }
    m_curr->add( g );

    {
        Fl_Input* widget = new Fl_Input( kMiddle, Y, g->w()-kMiddle, 20 );
        widget->value( content? _("Yes") : _("No") );
        widget->box( FL_FLAT_BOX );
        widget->align(FL_ALIGN_LEFT);
        widget->textcolor( FL_BLACK );
        widget->color( colB );
        if ( tooltip ) widget->tooltip( tooltip );
        else widget->tooltip( lbl->label() );
        if ( !editable )
        {
            widget->deactivate();
            widget->box( FL_FLAT_BOX );
        }
        else
        {
            if ( callback )
                widget->callback( callback, this );
        }
        m_curr->add( widget );
    }
    m_curr->end();

}


} // namespace mrv
