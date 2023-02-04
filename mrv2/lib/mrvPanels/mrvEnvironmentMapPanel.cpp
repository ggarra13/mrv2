// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Flex.H>

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"

#include "mrvGL/mrvTimelineViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrViewer.h"


namespace mrv
{

    EnvironmentMapPanel::EnvironmentMapPanel( ViewerUI* ui ) :
        PanelWidget( ui )
    {

        add_group( _("Environment Map") );

        Fl_SVG_Image* svg = load_svg( "EnvironmentMap.svg" );
        g->image( svg );

        auto view = ui->uiView;

        view->setActionMode( ActionMode::kRotate );
        
        EnvironmentMapOptions o = view->getEnvironmentMapOptions();
        // o.type = EnvironmentMapOptions::kSpherical;
        o.type = EnvironmentMapOptions::kCubic;
        view->setEnvironmentMapOptions(o);

        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete environmentMapPanel; environmentMapPanel = nullptr;
            auto view = ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type = EnvironmentMapOptions::kNone;
            view->setEnvironmentMapOptions(o);
            view->setActionMode( ActionMode::kScrub );
            view->refreshWindows();
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );

    }

    EnvironmentMapPanel::~EnvironmentMapPanel()
    {
        clear_controls();
    }

    void EnvironmentMapPanel::clear_controls()
    {
        delete hAperture;
        delete vAperture;
        delete focalLength;
        delete rotateX;
        delete rotateY;
    }

    void EnvironmentMapPanel::add_controls()
    {
        TLRENDER_P();

        g->clear();
        g->begin();
        
        Fl_Radio_Round_Button* r;
        HorSlider* s;
        CollapsibleGroup* cg = new CollapsibleGroup( g->x(), 20, g->w(), 20,
                                                     _("Type") );
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        Fl_Flex* flex = new Fl_Flex( g->x(), 20, g->w(), 20 );
        flex->type( Fl_Flex::HORIZONTAL );

        flex->begin();


        auto rB = new Widget< Fl_Radio_Round_Button >( g->x(), 90, g->w(), 20,
                                                       _("None") );
        r = rB;
        r->value(0);
        r->tooltip( _("Turn off image warping.") );
        rB->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type = EnvironmentMapOptions::kNone;
            view->setEnvironmentMapOptions( o );
        } );
        
        rB = new Widget< Fl_Radio_Round_Button >( g->x(), 90, g->w(), 20,
                                                  _("Spherical") );
        r = rB;
        r->value(0);
        r->tooltip( _("Wrap the image or images onto a sphere.") );
        rB->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type = EnvironmentMapOptions::kSpherical;
            view->setEnvironmentMapOptions( o );
        } );


        rB = new Widget< Fl_Radio_Round_Button >( g->x(), 90, g->w(), 20,
                                                  _("Cubic") );
        r = rB;
        r->value(1);
        r->tooltip( _("Wrap the image or images onto a cube.") );
        rB->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type = EnvironmentMapOptions::kCubic;
            view->setEnvironmentMapOptions( o );
        } );
        
        
        flex->end();

        cg->end();

        cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, _("Projection") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                           _("H. Aperture") );
        s = hAperture = sV;
        s->tooltip(
            _( "Horizontal Aperture of the Projection.")
        );
        s->range( 0.001f, 90.0f );
        s->step( 0.01F );
        s->default_value( 24.0f );
        sV->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.horizontalAperture = w->value();
            view->setEnvironmentMapOptions( o );
        } );


        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      _("V. Aperture") );
        s = vAperture = sV;
        s->tooltip(
            _( "Vertical Aperture of the Projection.")
        );
        s->range( 0.f, 90.0f );
        s->step( 0.1F );
        s->default_value( 0.f );
        sV->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.verticalAperture = w->value();
            view->setEnvironmentMapOptions( o );
        } );

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      _("Focal Length") );
        s = focalLength = sV;
        s->tooltip(
            _( "Focal Length of the Projection.")
        );
        s->range( 0.0001f, 180.0f );
        s->step( 0.1F );
        s->default_value( 90.f );
        sV->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.focalLength = w->value();
            view->setEnvironmentMapOptions( o );
        } );

        cg->end();

        cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, _("Rotation") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "X" );
        s = rotateX = sV;
        s->tooltip( _("Rotation in X of the sphere." ) );
        s->range( -90.f, 90.0f );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.rotateX = w->value();
            view->setEnvironmentMapOptions( o );
        } );


        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Y" );
        s = rotateY = sV;
        s->tooltip( _("Rotation in Y of the sphere." ) );
        s->range( -180.f, 180.0f );
        s->default_value( -90.0f );
        sV->callback( [=]( auto w ) {
            auto view = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.rotateY = w->value();
            view->setEnvironmentMapOptions( o );
        } );


        cg->end();


        g->end();

        p.ui->uiView->redrawWindows();

    }


}
