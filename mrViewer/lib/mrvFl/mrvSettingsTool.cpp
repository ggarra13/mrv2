
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>

#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvSettingsTool.h"
#include "mrvFl/mrvToolGroup.h"
#include "mrvFl/mrvHorSlider.h"
#include "mrvFl/mrvCollapsibleGroup.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvFunctional.h"

#include <mrvPlayApp/mrvFilesModel.h>

#include "mrViewer.h"

namespace mrv
{

    
    SettingsTool::SettingsTool( ViewerUI* ui ) :
        ToolWidget( ui )
    {
        add_group( "A/V" );
    }

    
    void SettingsTool::add_controls()
    {
        TLRENDER_P();
        
        auto cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, "Cache" );
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Check_Button* c;
        HorSlider* s;
        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                           "Read Ahead" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 5.0f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadAhead->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadAhead->value( w->value() );
            size_t active = p.app->filesModel()->observeActive()->get().size();
            auto players = p.ui->uiView->getTimelinePlayers();
            for ( auto& player : players )
            {
                auto value = w->value() / static_cast<double>( active );
                player->setCacheReadAhead( otio::RationalTime( value, 1.0 ) );
            }
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      "Read Behind" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 0.1f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadBehind->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadBehind->value( w->value() );
            size_t active = p.app->filesModel()->observeActive()->get().size();
            auto players = p.ui->uiView->getTimelinePlayers();
            for ( auto& player : players )
            {
                auto value = w->value() / static_cast<double>( active );
                player->setCacheReadBehind( otio::RationalTime( value, 1.0 ) );
            }
        } );
        
        cg->end();

        
        cg = new CollapsibleGroup( g->x(), 110, g->w(), 20, "File Sequences" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Group* bg = new Fl_Group( g->x(), 130, g->w(), 80 );
        auto mW = new Widget< Fl_Choice >( g->x()+100, 130, g->w()-100, 20,
                                           "Audio" );
        Fl_Choice* m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( "None" );
        m->add( "Base Name" );
        m->add( "File Name" );
        m->add( "Directory" );
        m->value(1);
        mW->callback([=]( auto o ) {
            // @todo:
        });

        
        Fl_Input* i;
        auto iW = new Widget<Fl_Input>( g->x()+100, 150, g->w()-g->x()-120, 20,
                                        "Audio file name" );
        i = iW;
        i->labelsize(12);
        iW->callback([=]( auto o ) {
            std::string file = o->value();
            // @todo:
        });
        
        iW = new Widget<Fl_Input>( g->x()+100, 170, g->w()-g->x()-120, 20,
                                   "Audio directory" );
        i = iW;
        i->labelsize(12);
        iW->callback([=]( auto o ) {
            std::string file = o->value();
            // @todo:
        });
        
        auto inW = new Widget<Fl_Int_Input>( g->x()+100, 190,
                                             g->w()-g->x()-120, 20,
                                             "Maximum Digits" );
        i = inW;
        i->labelsize(12);
        i->value("9");
        inW->callback([=]( auto o ) {
            int digits = atoi( o->value() );
            // @todo:
        });
        bg->end();
        
        cg->end();

        
        cg = new CollapsibleGroup( g->x(), 210, g->w(), 20, "File Sequences" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        
        cg->begin();
        

        bg = new Fl_Group( g->x(), 230, g->w(), 160 );
        
        Fl_Box* box = new Fl_Box( g->x(), 230, g->w(), 40,
                                  "Changes are applied to\n"
                                  "newly opened files." );
    
        mW = new Widget< Fl_Choice >( g->x()+100, 250, g->w()-100, 20,
                                      "Timer mode" );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( "System" );
        m->add( "Audio" );
        m->value(0);
        mW->callback([=]( auto o ) {
            // @todo:
        });
    
        mW = new Widget< Fl_Choice >( g->x()+100, 270, g->w()-100, 20,
                                      "Audio buffer frames" );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( "16" );
        m->add( "32" );
        m->add( "64" );
        m->add( "128" );
        m->add( "256" );
        m->add( "512" );
        m->add( "1024" );
        m->value(4);
        mW->callback([=]( auto o ) {
            // @todo:
        });

        inW = new Widget<Fl_Int_Input>( g->x()+100, 290, g->w()-g->x()-120, 20,
                                        "Video Requests" );
        i = inW;
        i->labelsize(12);
        i->value( "48" );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            // @todo:
        });

        inW = new Widget<Fl_Int_Input>( g->x()+100, 310, g->w()-g->x()-120, 20,
                                        "Audio Requests" );
        i = inW;
        i->labelsize(12);
        i->value( "16" );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            // @todo:
        });
        
        inW = new Widget<Fl_Int_Input>( g->x()+100, 330, g->w()-g->x()-120, 20,
                                        "Sequence I/O threads" );
        i = inW;
        i->labelsize(12);
        i->value( "0" );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            // @todo:
        });
    
        auto cV = new Widget< Fl_Check_Button >( g->x()+90, 350,
                                                 g->w(), 20, "FFmpeg YUV to RGB conversion" );
        c = cV;
        c->labelsize(12);
        cV->callback( [=]( auto w ) {
            // @todo:
        } );
        
        inW = new Widget<Fl_Int_Input>( g->x()+100, 370, g->w()-g->x()-120, 20,
                                        "FFmpeg I/O threads" );
        i = inW;
        i->labelsize(12);
        i->value( "0" );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            // @todo:
        });
        bg->end();
        
        cg->end();
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ToolGroup* t = (ToolGroup*) d;
            ToolGroup::cb_dismiss( NULL, t );
            delete settingsTool; settingsTool = nullptr;
        }, g );
    }

}
