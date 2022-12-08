// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvColorAreaInfo.h"

#include "FL/Fl_Choice.H"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHistogram.h"

#include "mrvGL/mrvGLViewport.h"

#include "mrvToolsCallbacks.h"

#include "mrvHistogramTool.h"

#include "mrViewer.h"



namespace mrv
{

    struct HistogramTool::Private
    {
        Histogram*       histogram = nullptr;
    };

    
    HistogramTool::HistogramTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( _("Histogram") );
        
        // Fl_SVG_Image* svg = load_svg( "ColorArea.svg" );
        // g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete histogramTool; histogramTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    HistogramTool::~HistogramTool()
    {
        TLRENDER_R();
	delete r.histogram; r.histogram = nullptr;
    }



    void HistogramTool::add_controls()
    {
        TLRENDER_P();
        TLRENDER_R();

	Pack* pack = g->get_pack();
	pack->spacing( 5 );

        g->clear();
        g->begin();

        int X = g->x();
        int Y = g->y();
        int W = g->w()-3;
        int H = g->h();


        Fl_Group* cg;
        Fl_Box*   b;
        Fl_Choice* c;

        cg = new Fl_Group( X, Y, W, 20 );
        cg->begin();
        b = new Fl_Box( X, Y, 120, 20, _("Type") );
        auto cW = new Widget< Fl_Choice >( X + b->w(), Y, W - b->w(), 20 );
        c = cW;
        c->add( _("Linear") );
        c->add( _("Logarithmic") );
        c->add( _("Square Root") );
        c->value(1);
        cW->callback( [=] ( auto o )
            {
                Histogram::Type c = (Histogram::Type) o->value();
                _r->histogram->histogram_type( c );
            }
            );
        cg->end();

        
        cg = new Fl_Group( X, Y, W, 20 );
        cg->begin();
        b = new Fl_Box( X, Y, 120, 20, _("Channel") );
        cW = new Widget< Fl_Choice >( X + b->w(), Y, W - b->w(), 20 );
        c = cW;
        c->add( "RGB" );
        c->add( _("Red") );
        c->add( _("Green") );
        c->add( _("Blue") );
        c->add( _("Lumma") );
        c->value(0);
        cW->callback( [=] ( auto o )
            {
                Histogram::Channel c = (Histogram::Channel) o->value();
                _r->histogram->channel( c );
            }
            );
        
        cg->end();
        
        // Create a square histogram
        r.histogram = new Histogram( X, Y, W, 270 );
        r.histogram->main( p.ui );
        

        g->resizable(g);
    }
    
    void HistogramTool::update( const area::Info& info )
    {
        _r->histogram->update( info );
    }

}
