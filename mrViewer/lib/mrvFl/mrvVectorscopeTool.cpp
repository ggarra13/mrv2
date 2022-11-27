

#include "mrvColorAreaInfo.h"

#include "FL/Fl_Choice.H"

#include "mrvWidgets/mrvVectorscope.h"

#include "mrvToolsCallbacks.h"

#include "mrvVectorscopeTool.h"

#include "mrViewer.h"



namespace mrv
{

    struct VectorscopeTool::Private
    {
        Vectorscope*       vectorscope = nullptr;
    };

    
    VectorscopeTool::VectorscopeTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( _("Vectorscope") );
        
        // Fl_SVG_Image* svg = load_svg( "Vectorscope.svg" );
        // g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete vectorscopeTool; vectorscopeTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    VectorscopeTool::~VectorscopeTool()
    {
    }



    void VectorscopeTool::add_controls()
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
        r.vectorscope = new Vectorscope( X, Y, W, W );
        r.vectorscope->main( p.ui );

        g->resizable(g);
    }
    
    void VectorscopeTool::update( const area::Info& info )
    {
        _r->vectorscope->update( info );
    }

}
