// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlCore/StringFormat.h>

#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>

#include "mrViewer.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvSettingsPanel.h"

#include "mrvFl/mrvIO.h"


#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvSettingsObject.h"



namespace mrv
{

    static const char* kModule = "settings";

    SettingsPanel::SettingsPanel( ViewerUI* ui ) :
        PanelWidget( ui )
    {
        add_group( "Settings" );

        Fl_SVG_Image* svg = load_svg( "Settings.svg" );
        g->image( svg );

        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete settingsPanel; settingsPanel = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
    }


    void SettingsPanel::add_controls()
    {
        TLRENDER_P();

        SettingsObject* settingsObject = p.ui->app->settingsObject();

        DBGM0( "g->w() = " << g->w() );


        auto cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, _("Cache") );
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Check_Button* c;
        HorSlider* s;
        int digits;

        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                           _("Read Ahead") );
        s = sV;
        s->tooltip( _("Read Ahead in seconds") );
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 5.0f );
        s->value( std_any_cast<double>( settingsObject->value( "Cache/ReadAhead" ) ) );
        sV->callback( [=]( auto w ) {
          settingsObject->setValue( "Cache/ReadAhead", (double) w->value() );
          p.ui->app->_cacheUpdate();
        } );


        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      _("Read Behind") );
        s = sV;
        s->tooltip( _("Read Behind in seconds") );
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 0.1f );
        s->value( std_any_cast<double>( settingsObject->value( "Cache/ReadBehind" ) ) );
        sV->callback( [=]( auto w ) {
          settingsObject->setValue( "Cache/ReadBehind", (double) w->value() );
          p.ui->app->_cacheUpdate();
        } );

        cg->end();


        cg = new CollapsibleGroup( g->x(), 110, g->w(), 20,
                                   _("File Sequences") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Group* bg = new Fl_Group( g->x(), 130, g->w(), 80 );
        bg->box( FL_NO_BOX );
        bg->begin();
        auto mW = new Widget< Fl_Choice >( g->x()+130, 130, g->w()-130, 20,
                                           _("Audio") );
        Fl_Choice* m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( _("None") );
        m->add( _("Base Name") );
        m->add( _("File Name") );
        m->add( _("Directory") );
        m->value( std_any_cast<int>( settingsObject->value( "FileSequence/Audio" ) ) );
        mW->callback([=]( auto o ) {
            int v = o->value();
            settingsObject->setValue( "FileSequence/Audio", v );
        });




        Fl_Input* i;
        auto iW = new Widget<Fl_Input>( g->x()+130, 150, g->w()-130, 20,
                                        _("Audio file name") );
        i = iW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        std::string value = std_any_cast<std::string>(
                                                      settingsObject->value( "FileSequence/AudioFileName" ) );

        i->value( value.c_str() );
        iW->callback([=]( auto o ) {
            std::string file = o->value();
            settingsObject->setValue( "FileSequence/AudioFileName", file );
        });


        iW = new Widget<Fl_Input>( g->x()+130, 170, g->w()-130, 20,
                                   "Audio directory" );
        i = iW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        i->value( std_any_cast<std::string>(
                      settingsObject->value( "FileSequence/AudioDirectory" ) ).c_str() );
        iW->callback([=]( auto o ) {
            std::string dir = o->value();
            settingsObject->setValue( "FileSequence/AudioDirectory", dir );
        });


        DBG;
        auto inW = new Widget<Fl_Int_Input>( g->x()+130, 190,
                                             g->w()-130, 20,
                                             _("Maximum Digits") );
        i = inW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        digits = std_any_cast< int >( settingsObject->value("Misc/MaxFileSequenceDigits") );
        std::string text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int digits = atoi( o->value() );
            settingsObject->setValue( "Misc/MaxFileSequenceDigits", digits );
        });



        bg->end();

        cg->end();


        cg = new CollapsibleGroup( g->x(), 210, g->w(), 20, _("Performance") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();


        bg = new Fl_Group( g->x(), 230, g->w(), 140 );
        bg->box( FL_NO_BOX );
        bg->begin();

        Fl_Box* box = new Fl_Box( g->x(), 230, g->w(), 40,
                                  "Changes are applied to "
                                  "newly opened files." );
        box->labelsize(12);
        box->align( FL_ALIGN_WRAP );



        DBG;
        mW = new Widget< Fl_Choice >( g->x()+130, 270, g->w()-130, 20,
                                      _("Timer mode") );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        for (const auto& i : timeline::getTimerModeLabels())
        {
            m->add( i.c_str() );
        }

        m->value( std_any_cast<int>( settingsObject->value( "Performance/TimerMode") ) );

        mW->callback([=]( auto o ) {
            int v = o->value();
            settingsObject->setValue( "Performance/TimerMode", v );
        });

        DBG;
        mW = new Widget< Fl_Choice >( g->x()+130, 290, g->w()-130, 20,
                                      _("Audio buffer frames") );
        m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        for (const auto& i : timeline::getAudioBufferFrameCountLabels())
        {
            m->add( i.c_str() );
        }
        m->value( std_any_cast<int>(
                      settingsObject->value( "Performance/AudioBufferFrameCount") ) );

        mW->callback([=]( auto o ) {
            int v = o->value();
            settingsObject->setValue( "Performance/AudioBufferFrameCount", v );
        });

        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+160, 310, g->w()-160, 20,
                                        _("Video Requests") );
        i = inW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            settingsObject->value("Performance/VideoRequestCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );

        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            settingsObject->setValue( "Performance/VideoRequestCount",
                                     requests );
            p.ui->app->_cacheUpdate();
        });

        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+160, 330, g->w()-160, 20,
                                        _("Audio Requests") );
        i = inW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            settingsObject->value("Performance/AudioRequestCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            settingsObject->setValue( "Performance/AudioRequestCount", requests );
            p.ui->app->_cacheUpdate();
        });


        DBG;
        inW = new Widget<Fl_Int_Input>( g->x()+160, 350, g->w()-160, 20,
                                        _("Sequence I/O threads") );
        i = inW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        // i->range( 1, 64 );
        digits = std_any_cast< int >(
            settingsObject->value( "Performance/SequenceThreadCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            settingsObject->setValue( "Performance/SequenceThreadCount",
                                     requests );
            p.ui->app->_cacheUpdate();
        });

        bg->end();

        auto cV = new Widget< Fl_Check_Button >( g->x()+90, 370,
                                                 g->w(), 20,
                                                 _("FFmpeg YUV to RGB "
                                                   "conversion") );
        c = cV;
        c->labelsize(12);
        c->value( std_any_cast<int>(
                      settingsObject->value( "Performance/FFmpegYUVToRGBConversion" ) ) );

        cV->callback( [=]( auto w ) {
            int v = w->value();
            settingsObject->setValue( "Performance/FFmpegYUVToRGBConversion", v );
        } );

        bg = new Fl_Group( g->x(), 390, g->w(), 30 );
        bg->box( FL_NO_BOX );
        bg->begin();

        inW = new Widget<Fl_Int_Input>( g->x()+160, 390, g->w()-160, 20,
                                        _("FFmpeg I/O threads") );
        i = inW;
        i->labelsize(12);
        i->color( (Fl_Color)-1733777408 );
        i->textcolor( FL_BLACK );
        digits = std_any_cast< int >(
            settingsObject->value( "Performance/FFmpegThreadCount") );
        text = string::Format( "{0}" ).arg(digits);
        i->value( text.c_str() );
        inW->callback([=]( auto o ) {
            int requests = atoi( o->value() );
            settingsObject->setValue( "Performance/FFmpegThreadCount",
                                     requests );
            p.ui->app->_cacheUpdate();
        });

        bg->end();


        cg->end();

        // This does not work properly, and it is counter intuitive as
        // it hides the tool docks.
        auto bW = new Widget< Fl_Button >( g->x(), g->y(), g->w(), 20,
                                           _("Default Settings") );
        b = bW;
        b->box( FL_UP_BOX );
        bW->callback([=]( auto o ) {
            settingsObject->reset();
            save();
            refresh();
        });

        // To refresh the timeline caching bars after a reset of settings.
        p.ui->uiTimeWindow->uiTimeline->redraw();
    }


    void SettingsPanel::refresh()
    {
        begin_group();
        add_controls();
        end_group();
    }
}
