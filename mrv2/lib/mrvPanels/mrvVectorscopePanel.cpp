// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.




#include "FL/Fl_Choice.H"

#include "mrvWidgets/mrvVectorscope.h"

#include "mrvFl/mrvColorAreaInfo.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvVectorscopePanel.h"

#include "mrViewer.h"



namespace mrv
{

    struct VectorscopePanel::Private
    {
        Vectorscope*       vectorscope = nullptr;
    };


    VectorscopePanel::VectorscopePanel( ViewerUI* ui ) :
        _r( new Private ),
        PanelWidget( ui )
    {
        add_group( _("Vectorscope") );

        Fl_SVG_Image* svg = load_svg( "Vectorscope.svg" );
        g->image( svg );

        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete vectorscopePanel; vectorscopePanel = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );

    }

    VectorscopePanel::~VectorscopePanel()
    {
    }



    void VectorscopePanel::add_controls()
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

        // Create a square vectorscope
        r.vectorscope = new Vectorscope( X, Y, 270, 270 );
        r.vectorscope->main( p.ui );

        g->resizable(g);
    }

    void VectorscopePanel::update( const area::Info& info )
    {
        _r->vectorscope->update( info );
    }

}
