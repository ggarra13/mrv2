// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

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

#include <FL/Fl_Pack.H>
#include <FL/Fl_Int_Input.H>
#include <FL/fl_draw.H>
#include <FL/names.h>


#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvImageInfoTool.h"

#include "mrvWidgets/mrvHorSlider.h"
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

    static const int kLineHeight = 20;
    static const int kTextSize = 12;
    static const int kLabelSize = 12;

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
        0x80808000,
        0xa0a0a000,
    };

    static const unsigned int kSizeOfRowColors = ( sizeof(kRowColors) /
                                                   sizeof(Fl_Color) );


    ViewerUI* ImageInfoTool::main() const
    {
        return _p->ui;
    }


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


    static bool regex_match( int row, const std::string& regex,
                             const std::string& text )
    {
        try {
            boost::regex expr{ regex };
            if ( boost::regex_search( text, expr ) )
            {
                ++num_matches;
                if ( match_goal == num_matches )
                {
                    idx = row;
                    return true;
                }
            }
        }
        catch ( const boost::regex_error& e )
        {
        }
        return false;
    }


    static bool process_row( int row, Fl_Widget* w,
                             const std::string& match,
                             MatchType type )
    {
        if ( ( type == kMatchValue || type == kMatchAll ) &&
             dynamic_cast< HorSlider* >( w ) != nullptr )
        {
            HorSlider* input = (HorSlider*) w;
            std::cerr << "procees horslider="
                      << input->uiValue->value() << std::endl;
            if ( regex_match( row, match, input->uiValue->value() ) )
                return true;
        }
        else if ( ( type == kMatchValue || type == kMatchAll ) &&
                  dynamic_cast< Fl_Input* >( w ) != nullptr )
        {
            Fl_Input* input = (Fl_Input*) w; 
            if ( regex_match( row, match, input->value() ) )
                return true;
        }
        else if ( dynamic_cast< Fl_Group* >( w ) != nullptr )
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
            if ( type != kMatchAttribute && type != kMatchAll )
                return false;
            if ( !w->label() ) return false;
            if ( regex_match( row, match, w->label() ) )
                return true;
        }
        return false;
    }

    static int search_table( mrv::Table* t, const std::string& match,
                             MatchType type )
    {
        int rows = t->children()-2;  // we skip the last two as they are scrollbars
        idx = -1;
        for ( int i = 0; i < rows; i += 2 )  // +2 for each column
        {
            Fl_Widget* w = t->child(i);
            if ( !w->visible() ) continue;
            int row = i / 2;
            if ( process_row( row, w, match, type ) )
                break;
            w = t->child(i+1);
            if ( process_row( row, w, match, type ) )
                break;
        }
        return idx;
    }


    static void search_cb( Fl_Widget* o, mrv::ImageInfoTool* info )
    {
        const char* s = info->m_entry->value();

        std::string match = s;
        if ( match.empty() )
        {
            old_match.clear();
            idx = -1;
            info->scroll_to( 0, 0 );
            return;
        }
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
        idx = -1;

        int H  = kLineHeight + 6;
        
        Pack* pack = info->get_pack();
        Pack* p = info->m_image->contents();
        Fl_Button* b = info->m_image->button();
        if ( ! p->children() ) return; // No video/imahr Loaded

        Table* t = (Table*) p->child(0);

        int start = info->m_image->y() - pack->y();
        start += b->h();
        start += info->flex->h();
        
        int idx = search_table( t, match, type );
        if ( idx >= 0 ) {
            info->scroll_to( 0, start + H * idx );
            return;
        }

        p = info->m_video->contents();
        b = info->m_video->button();

        start = info->m_video->y() - pack->y();
        start += b->h();
        start += info->flex->h();
        

        for ( int i = 0; i < p->children(); ++i )
        {
            t = dynamic_cast< Table* >( p->child(i) );
            if ( !t )
            {
                start += p->child(i)->h();
                continue;
            }
            idx = search_table( t, match, type );
            if ( idx >= 0 ) {
                info->scroll_to( 0, start + H * idx );
                return;
            }
            start += t->h();
        }
        
        p = info->m_audio->contents();
        b = info->m_audio->button();

        start = info->m_audio->y() - pack->y();
        start += b->h();
        start += info->flex->h();

        for ( int i = 0; i < p->children(); ++i )
        {
            t = dynamic_cast< Table* >( p->child(i) );
            if ( !t )
            {
                start += p->child(i)->h();
                continue;
            }

            idx = search_table( t, match, type );
            if ( idx >= 0 ) {
                info->scroll_to( 0, start + H * idx );
                return;
            }
            start += t->h();
        }


        p = info->m_subtitle->contents();
        b = info->m_subtitle->button();

        start = info->m_subtitle->y() - pack->y();
        start += b->h();
        start += info->flex->h();

        for ( int i = 0; i < p->children(); ++i )
        {
            t = dynamic_cast< Table* >( p->child(i) );
            if ( !t )
            {
                start += p->child(i)->h();
                continue;
            }
            
            idx = search_table( t, match, type );
            if ( idx >= 0 ) {
                info->scroll_to( 0, start + H * idx );
                return;
            }
            start += t->h();
        }

        p = info->m_attributes->contents();
        b = info->m_attributes->button();

        start = info->m_attributes->y() - pack->y();
        start += b->h();
        start += info->flex->h();
        
        t = (Table*) p->child(0);

        idx = search_table( t, match, type );
        if ( idx >= 0 ) {
            info->scroll_to( 0, start + H * idx );
            return;
        }

        match_goal = 0;

    }
    
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
    ImageInfoTool::scroll_to( int X, int Y )
    {
        Fl_Scroll* scroll = g->get_scroll();
        scroll->scroll_to( X, Y );
    }

    void
    ImageInfoTool::add_controls()
    {
        g->end();

        Fl_Group* controls = g->get_group();

        controls->begin();

        flex = new Fl_Flex( controls->x(), controls->y(),
                            controls->w(), controls->h());
        flex->type( Fl_Flex::HORIZONTAL );
        flex->begin();

        int Y = controls->y();

        Fl_Box* box = new Fl_Box( controls->x(), Y, 80, 30, _("Search") );
        flex->set_size( box, 80 );
        m_entry = new Fl_Input( controls->x()+box->w(), Y, controls->w()-200, 30);
        m_entry->textcolor( FL_BLACK );
        m_entry->color(  (Fl_Color)-1733777408 );
        m_entry->when( FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED |
                       FL_WHEN_ENTER_KEY );
        m_entry->callback( (Fl_Callback*)search_cb, this );

        m_type = new Fl_Choice( m_entry->x()+m_entry->w(), Y, 120, 30 );
        flex->set_size( m_type, 100 );
        m_type->add( _("Both" ) );
        m_type->add( _("Attribute" ) );
        m_type->add( _("Value" ) );
        m_type->value( 0 );

        flex->resizable( m_entry );
        flex->end();

        controls->resizable( flex );
        controls->end();

        
        controls->show();

        
        Y = controls->y() + controls->h();

        Fl_Scroll* scroll = g->get_scroll();
        scroll->position( scroll->x(), Y );

        Pack* pack = g->get_pack();
        pack->position( pack->x(), Y );


        // @todo:
        // menu = new Fl_Menu_Button( 0, 0, 0, 0, _("Attributes Menu") );
        // menu->type( Fl_Menu_Button::POPUP3 );
        g->begin();

        // scrollbar width
        int sw = scroll->scrollbar.visible() ? scroll->scrollbar.w() : 0;
        if ( !g->docked() ) sw = 0;
        int W = g->w() - sw;
        
        // CollapsibleGrop recalcs, we don't care its xyh sizes
        m_image = new mrv::CollapsibleGroup( g->x(), Y, W, 800, _("Main")  );
        m_image->end();

        Y += m_image->h();
        m_video = new mrv::CollapsibleGroup( g->x(), Y, W, 400, _("Video") );
        m_video->end();

        Y += m_video->h();
        m_audio = new mrv::CollapsibleGroup( g->x(), Y, W, 400, _("Audio") );
        m_audio->end();

        Y += m_audio->h();
        m_subtitle = new mrv::CollapsibleGroup( g->x(), Y, W, 400, _("Subtitle") );
        m_subtitle->end();

        Y += m_subtitle->h();
        m_attributes  = new mrv::CollapsibleGroup( g->x(), Y, W, 400, _("Metadata")  );
        m_attributes->end();


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


// Update int slider from int input
    static void update_int_slider( Fl_Int_Input* w )
    {
        Fl_Group* g = w->parent();
        Fl_Slider* s = (Fl_Slider*)g->child(1);
        s->value( atoi( w->value() ) );
    }
    
    void ImageInfoTool::int_slider_cb( Fl_Slider* s, void* data )
    {
        Fl_Int_Input* n = (Fl_Int_Input*) data;
        char buf[64];
        sprintf( buf, "%g", s->value() );
        n->value( buf );
        n->do_callback();
    }

    static bool modify_int( Fl_Int_Input* w, tl::io::Attribute::iterator& i)
    {
        update_int_slider( w );
        return true;
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
        ViewerUI* ui = info->main();
        ui->uiStartFrame->value( w->value() );
        ui->uiTimeline->redraw();
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
        ViewerUI* ui = info->main();
        ui->uiEndFrame->value( w->value() );
        ui->uiTimeline->redraw();
    }



    static void change_fps_cb( HorSlider* w, ImageInfoTool* info )
    {
        float f = w->value();
        const auto player = info->timelinePlayer();
        if (! player ) return;
        player->setSpeed( f );
        info->main()->uiFPS->value( f );
    }



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
        m_curr = nullptr;

        DBG2;
        g->tooltip( _("Load an image or movie file") );
        
        m_image->hide();
        m_video->hide();
        m_audio->hide();
        m_subtitle->hide();
        m_attributes->hide();

        DBG3;
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

        DBG3;
    }


    Table* ImageInfoTool::add_browser( CollapsibleGroup* g, const char* label )
    {
        if (!g) return nullptr;

        X = 0;
        Y = g->y() + kLineHeight;
        
        Fl_Box* box = new Fl_Box( 0, Y, g->w(), 20 );
        g->add( box );

        Table* table = new Table( 0, Y + box->h(), g->w(), 20 );
        table->column_separator(true);
        //table->auto_resize( true );
        // table->labeltype(FL_NO_LABEL);
        static const char* headers[] = { _("Attribute"), _("Value"), 0 };
        table->column_labels( headers );
        table->col_width_all( kMiddle );

        table->align(FL_ALIGN_CENTER | FL_ALIGN_TOP );
        table->end();
        table->copy_label( label );

        g->add( table );

        group = row = 0; // controls line colors

        return table;
    }
    
    Table* ImageInfoTool::add_browser( CollapsibleGroup* g )
    {
        if (!g) return nullptr;

        X = 0;
        Y = g->y() + kLineHeight;

        Table* table = new Table( 0, Y, g->w(), 20 );
        table->column_separator(true);
        //table->auto_resize( true );
        // table->labeltype(FL_NO_LABEL);
        static const char* headers[] = { _("Attribute"), _("Value"), 0 };
        table->column_labels( headers );
        table->col_width_all( kMiddle );

        table->align(FL_ALIGN_CENTER | FL_ALIGN_TOP );
        table->end();

        g->add( table );

        group = row = 0; // controls line colors

        return table;
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



    void ImageInfoTool::add_button( const char* name,
                                    const char* tooltip,
                                    Fl_Callback* callback,
                                    Fl_Callback* callback2 )
    {
        Fl_Color colA = get_title_color();
        Fl_Color colB = get_widget_color();

        int hh = kLineHeight;
        Y += hh;

        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        {
            Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->color( colA );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
            widget->labelsize( kLabelSize );
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

        int hh = kLineHeight;
        Y += hh;

        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        {
            Fl_Box* widget = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->color( colA );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
            widget->labelsize( kLabelSize );
            widget->copy_label( _("1:1") );
            if ( pressed == 0 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
            widget = new Fl_Button( kMiddle+w5, Y, w5, hh );
            widget->tooltip( tooltip );
            widget->labelsize( kLabelSize );
            widget->copy_label( _("1:2") );
            if ( pressed == 1 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
            widget = new Fl_Button( kMiddle+w5*2, Y, w5, hh );
            widget->tooltip( tooltip );
            widget->labelsize( kLabelSize );
            widget->copy_label( _("1:4") );
            if ( pressed == 2 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
            widget = new Fl_Button( kMiddle+w5*3, Y, w5, hh );
            widget->tooltip( tooltip );
            widget->labelsize( kLabelSize );
            widget->copy_label( _("1:8") );
            if ( pressed == 3 ) widget->value(1);
            else widget->value(0);
            if ( callback )
                widget->callback( (Fl_Callback*)callback, (void*)this );
            if ( num_scales > 4 )
            {
                widget = new Fl_Button( kMiddle+w5*4, Y, w5, hh );
                widget->tooltip( tooltip );
                widget->labelsize( kLabelSize );
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
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->color( colA );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
            widget->textsize( kTextSize );
            widget->textcolor( FL_BLACK );
            widget->color( colB );
            widget->tooltip( tooltip ? tooltip : lbl->label() );

            sg->add( widget );

            Fl_Button* pick = new Fl_Button( kMiddle + sg->w()-50, Y, 50, hh,
                                             _("Pick") );
            pick->callback( (Fl_Callback*)attach_ocio_ics_cb, _p->ui->uiView );
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
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->color( colA );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
            widget->copy_label( name );
            g->end();
        }
        m_curr->add( g );

        {
            Fl_Widget* widget = nullptr;
            if ( !editable )
            {
                Fl_Output* o = new Fl_Output( kMiddle, Y, g->w()-kMiddle, hh );
                widget = o;
                o->value( content );
                o->textsize( kTextSize );
                o->textcolor( FL_BLACK );
            }
            else
            {
                Fl_Input* o = new Fl_Input( kMiddle, Y, g->w()-kMiddle, hh );
                widget = o;
                o->value( content );
                o->textsize( kTextSize );
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
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
                sprintf( buf, "% 9d", content );
                widget->value( buf );
                widget->align(FL_ALIGN_LEFT);
		widget->color( (Fl_Color) 0xf98a8a800 );
                widget->deactivate();
                widget->box( FL_FLAT_BOX );
                widget->textsize( kTextSize );
                widget->textcolor( FL_BLACK );
                if ( tooltip ) widget->tooltip( tooltip );
                else widget->tooltip( lbl->label() );
            }
            else
            {
                Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 50, hh );
                sprintf( buf, "% 9d", content );
                widget->value( buf );
                widget->align(FL_ALIGN_LEFT);
		widget->color( (Fl_Color) 0xf98a8a800 );
                widget->textsize( kTextSize );
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
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
            widget->textsize( kTextSize );
            widget->textcolor( FL_BLACK );
            for ( size_t i = 0; i < num; ++i )
            {
                widget->add( _( options[i] ) );
            }
            widget->value( unsigned(content) );
            widget->labelsize( kLabelSize );
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
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->labelsize( kLabelSize );
            widget->copy_label( name );
            widget->color( colA );
            widget->labelcolor( FL_BLACK );
            g->add( widget );
        }
        m_curr->add( g );

        {
            char buf[64];
            Fl_Group* p = new Fl_Group( kMiddle, Y, kMiddle, hh );
            p->begin();
            if ( !editable )
            {
                Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, p->w(), hh );
                sprintf( buf, "% 9d", content );
                widget->value( buf );
                widget->box( FL_FLAT_BOX );
		widget->color( (Fl_Color) 0xf98a8a800 );
                widget->textsize( kTextSize );
                widget->textcolor( FL_BLACK );
                widget->deactivate();
                if ( tooltip ) widget->tooltip( tooltip );
                else widget->tooltip( lbl->label() );
            }
            else
            {
                Fl_Int_Input* widget = new Fl_Int_Input( kMiddle, Y, 60, hh );
                sprintf( buf, "% 9d", content );
                widget->value( buf );
                widget->align(FL_ALIGN_CENTER );
                widget->textsize( kTextSize );
                widget->textcolor( FL_BLACK );
		widget->color( (Fl_Color) 0xf98a8a800 );
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
                                  const otime::RationalTime& content,
                                  const bool editable )
    {
        char buf[128];
        
        int64_t frame = content.to_frames();

        sprintf( buf, _( "Frame %" PRId64 " " ), frame );
        
        std::string text = buf;


        double seconds = content.to_seconds();

        sprintf( buf, _("%.3g seconds "), seconds );
        text += buf;

        text += content.to_timecode();
        
        add_text( name, tooltip, text, false );
    }

    void ImageInfoTool::add_int64( const char* name,
                                   const char* tooltip,
                                   const int64_t content )
    {

        char buf[128];
        sprintf( buf, N_("% 9" PRId64), content );
        add_text( name, tooltip, buf, false );
    }

    void ImageInfoTool::add_rect( const char* name, const char* tooltip,
                                  const tl::math::BBox2i& content,
                                  const bool editable, Fl_Callback* callback )
    {

        Fl_Color colA = get_title_color();
        Fl_Color colB = get_widget_color();

        Fl_Box* lbl;
        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();
        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->labelsize( kLabelSize );
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
            widget->textsize( kTextSize );
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
            widget->textsize( kTextSize );
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
            widget->textsize( kTextSize );
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
            widget->textsize( kTextSize );
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
            widget->textsize( kTextSize );
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
        int hh = kLineHeight;

        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();

        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );

            widget->box( FL_FLAT_BOX );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
            widget->copy_label( name );
            widget->color( colA );
            g->add( widget );
        }
        m_curr->add( g );


        {
            Fl_Group* p = new Fl_Group( kMiddle, Y, kMiddle, hh );
            p->box( FL_FLAT_BOX );
            p->begin();

            HorSlider* widget = new HorSlider( kMiddle, Y, p->w(), hh );
            widget->value( content );
            widget->default_value( content );
            widget->align(FL_ALIGN_LEFT);
            widget->color( colB );
            widget->textsize( kTextSize );
            widget->textcolor( FL_BLACK );
            double maxS = maxV;
            if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
            else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
            else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
            else if ( content > 100 && maxV <= 100 ) maxS = 1000;
            else if ( content > 10 && maxV <= 10 ) maxS = 100;
            else if ( content > maxS ) maxS = content+50;

            widget->range( minV, maxS );
            widget->setEnabled( editable );
            
            if ( tooltip ) widget->tooltip( tooltip );
            else widget->tooltip( lbl->label() );
            
            if ( callback ) widget->callback( callback, this );
            
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

        int hh = kLineHeight;
        Y += hh;
        Fl_Group* g = new Fl_Group( X, Y, kMiddle, hh );
        g->end();

        {
            Fl_Box* widget = lbl = new Fl_Box( X, Y, kMiddle, hh );
            widget->box( FL_FLAT_BOX );
            widget->labelcolor( FL_BLACK );
            widget->labelsize( kLabelSize );
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
            widget->textsize( kTextSize );
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


    void ImageInfoTool::fill_data()
    {
        if ( !player ) return;

        // Refresh the dock size

        kMiddle = g->w() / 2;

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

        const auto& range = player->timeRange();
        const auto& startTime = range.start_time();
        const auto& endTime   = range.end_time_inclusive();
        add_time( _("Start Time"), _("Beginning frame of clip"),
                  startTime, false );
        add_time( _("End Time"), _("Ending frame of clip"),
                  endTime, false );

        const otime::TimeRange& iorange = player->inOutRange();
        int64_t first = iorange.start_time().to_frames();
        int64_t last =  iorange.end_time_inclusive().to_frames();

        add_int( _("First Frame"), _("First frame of clip - User selected"),
                 (int)first, true, true,
                 (Fl_Callback*)change_first_frame_cb, first, last );
        add_int( _("Last Frame"), _("Last frame of clip - User selected"),
                 (int)last, true, true,
                 (Fl_Callback*)change_last_frame_cb, 2, last );

        float   fps  = player->speed();
        add_float( _("Current Speed"), _("Current Speed (Frames Per Second)"), fps, true, true,
                   (Fl_Callback*)change_fps_cb, 1.0f, 60.0f,
                   FL_WHEN_CHANGED );

        ++group;

        m_image->show();


        if ( num_video_streams > 0 )
        {
            for ( int i = 0; i < num_video_streams; ++i )
            {
                char buf[256];

                if ( num_video_streams > 1 )
                {
                    sprintf( buf, _("Video Stream #%d"), i+1 );
                    m_curr = add_browser( m_video, buf );
                }
                else
                {
                    m_curr = add_browser( m_video );
                }

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

                add_text( _("Default Speed"),
                          _("Default Speed in Frames per Second"), buf );

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
        const char* space_type = nullptr;
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

        g->tooltip( nullptr );


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

                if ( num_audio_streams > 1 )
                {
                    sprintf( buf, _("Audio Stream #%d"), i+1 );
                    m_curr = add_browser( m_audio, buf );
                }
                else
                {
                    m_curr = add_browser( m_audio );
                }

                
                // @todo: tlRender handles only one audio track
                const auto& audio = info.audio;


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

                if ( num_subtitle_streams > 1 )
                {
                    sprintf( buf, _("Subtitle Stream #%d"), i+1 );
                    m_curr = add_browser( m_subtitle, buf );
                }
                else
                {
                    m_curr = add_browser( m_subtitle, );
                }

                const Media::subtitle_info_t& s = img->subtitle_info(i);

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

                //    m_curr->layout();
                //     m_curr->parent()->layout();
            }

            m_subtitle->show();
        }
#endif

        // Call g->end() so we refresh the pack/scroll sizes
        g->end();

        if ( g->docked() ) end_group();
    }

} // namespace mrv
