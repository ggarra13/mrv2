

#include <tlCore/StringFormat.h>

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
#include "mrvFl/mrvIO.h"

#include "mrvFl/mrvFunctional.h"

#include <mrvPlayApp/mrvFilesModel.h>
#include <mrvPlayApp/mrvSettingsObject.h>

#include "mrViewer.h"



namespace mrv
{

    static const char* kModule = "settings";
    
    SettingsTool::SettingsTool( ViewerUI* ui ) :
        ToolWidget( ui )
    {
        add_group( "Audio/Video" );
    }

    
    void SettingsTool::add_controls()
    {
        TLRENDER_P();
	
        SettingsObject* st = p.ui->app->settingsObject();
        
        auto cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, "Cache" );
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Check_Button* c;
        HorSlider* s;
        int digits;
        std::string text;
        DBG;
        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                           "Read Ahead" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 5.0f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadAhead->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadAhead->value( w->value() );
            size_t active = p.ui->app->filesModel()->observeActive()->get().size();
            auto players = p.ui->uiView->getTimelinePlayers();
            for ( auto& player : players )
            {
                auto value = w->value() / static_cast<double>( active );
                player->setCacheReadAhead( otio::RationalTime( value, 1.0 ) );
            }
        } );
        
        DBG;
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      "Read Behind" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 0.1f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadBehind->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadBehind->value( w->value() );
            size_t active = p.ui->app->filesModel()->observeActive()->get().size();
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
        DBG;
        auto mW = new Widget< Fl_Choice >( g->x()+100, 130, g->w()-100, 20,
                                           "Audio" );
        Fl_Choice* m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( "None" );
        m->add( "Base Name" );
        m->add( "File Name" );
        m->add( "Directory" );
        m->value( std_any_cast<int>( st->value( "FileSequence/Audio" ) ) );
        mW->callback([=]( auto o ) {
            int v = o->value();
            st->setValue( "FileSequence/Audio", v );
        });

        

        
        Fl_Input* i;
        DBG;
        auto iW = new Widget<Fl_Input>( g->x()+100, 150, g->w()-g->x()-120, 20,
                                        "Audio file name" );
        i = iW;
        i->labelsize(12);
	std::string value = std_any_cast<std::string>(
						      st->value( "FileSequence/AudioFileName" ) );
	
        i->value( value.c_str() );
        iW->callback([=]( auto o ) {
            std::string file = o->value();
            st->setValue( "FileSequence/AudioFileName", file );
        });
        
        
        DBG;
        iW = new Widget<Fl_Input>( g->x()+100, 170, g->w()-g->x()-120, 20,
                                   "Audio directory" );
        i = iW;
        i->labelsize(12);
        i->value( std_any_cast<std::string>(
                      st->value( "FileSequence/AudioDirectory" ) ).c_str() );
        iW->callback([=]( auto o ) {
            std::string dir = o->value();
            st->setValue( "FileSequence/AudioDirectory", dir );
        });
        
        
        DBG;
        auto inW = new Widget<Fl_Int_Input>( g->x()+100, 190,
                                             g->w()-g->x()-120, 20,
                                             "Maximum Digits" );
        i = inW;
        i->labelsize(12);
        digits = std_any_cast< int >(
            st->value("Misc/MaxFileSequenceDigits") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int digits = atoi( o->value() );
            st->setValue( "Misc/MaxFileSequenceDigits", digits );
        });
        

        
        bg->end();
        
        cg->end();

        
        cg = new CollapsibleGroup( g->x(), 210, g->w(), 20, "Performance" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        
        cg->begin();
        

        bg = new Fl_Group( g->x(), 230, g->w(), 140 );
        
        Fl_Box* box = new Fl_Box( g->x(), 230, g->w(), 40,
                                  "Changes are applied to "
                                  "newly opened files." );
        box->labelsize(12);
        box->align( FL_ALIGN_WRAP );


        
        DBG;
        mW = new Widget< Fl_Choice >( g->x()+130, 270, g->w()-g->x()-130, 20,
                                      "Timer mode" );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        for (const auto& i : timeline::getTimerModeLabels())
        {
            m->add( i.c_str() );
        }
            
        m->value( std_any_cast<int>( st->value( "Performance/TimerMode") ) );
        
        mW->callback([=]( auto o ) {
            int v = o->value();
            st->setValue( "Performance/TimerMode", v );
        });
    
        DBG;
        mW = new Widget< Fl_Choice >( g->x()+130, 290, g->w()-g->x()-130, 20,
                                      "Audio buffer frames" );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        for (const auto& i : timeline::getAudioBufferFrameCountLabels())
        {
            m->add( i.c_str() );
        }
        m->value( std_any_cast<int>(
                      st->value( "Performance/AudioBufferFrameCount") ) );
        
        mW->callback([=]( auto o ) {
            int v = o->value();
            st->setValue( "Performance/AudioBufferFrameCount", v );
        });

        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+130, 310, g->w()-g->x()-130, 20,
                                        "Video Requests" );
        i = inW;
        i->labelsize(12);
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            st->value("Performance/VideoRequestCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            st->setValue( "Performance/VideoRequestCount", requests );
        });

        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+130, 330, g->w()-g->x()-130, 20,
                                        "Audio Requests" );
        i = inW;
        i->labelsize(12);
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            st->value("Performance/AudioRequestCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            st->setValue( "Performance/AudioRequestCount", requests );
        });
        
        
        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+130, 350, g->w()-g->x()-130, 20,
                                        "Sequence I/O threads" );
        i = inW;
        i->labelsize(12);
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            st->value( "Performance/SequenceThreadCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            st->setValue( "Performance/SequenceThreadCount", requests );
        });
        bg->end();
    
        auto cV = new Widget< Fl_Check_Button >( g->x()+90, 370,
                                                 g->w(), 20, "FFmpeg YUV to RGB conversion" );
        c = cV;
        c->labelsize(12);
        c->value( std_any_cast<int>(
                      st->value( "Performance/FFmpegYUVToRGBConversion" ) ) );
        
        cV->callback( [=]( auto w ) {
            int v = w->value();
            st->setValue( "Performance/FFmpegYUVToRGBConversion", v );
        } );
        
        bg = new Fl_Group( g->x(), 390, g->w(), 30 );
        
        inW = new Widget<Fl_Int_Input>( g->x()+130, 390, g->w()-g->x()-130, 20,
                                        "FFmpeg I/O threads" );
        i = inW;
        i->labelsize(12);
        digits = std_any_cast< int >(
            st->value( "Performance/FFmpegThreadCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        // i->range( 1, 64 );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            st->setValue( "Performance/FFmpegThreadCount", requests );
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
