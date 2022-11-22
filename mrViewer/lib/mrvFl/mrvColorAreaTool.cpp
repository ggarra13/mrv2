
#include "mrvGL/mrvGLViewport.h"

#include "mrvColorAreaInfo.h"
#include "mrvWidgets/mrvColorInfo.h"

#include "mrvToolsCallbacks.h"
#include "mrvHorSlider.h"
#include "mrvFunctional.h"

#include "mrvColorAreaTool.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "carea";
}


namespace mrv
{

    struct ColorAreaTool::Private
    {
        ColorInfo*       colorInfo = nullptr;
    };

    
    ColorAreaTool::ColorAreaTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( _("Color Area") );
        
        // Fl_SVG_Image* svg = load_svg( "ColorArea.svg" );
        // g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete colorAreaTool; colorAreaTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    ColorAreaTool::~ColorAreaTool()
    {
    }



    void ColorAreaTool::add_controls()
    {
        TLRENDER_P();
        TLRENDER_R();
            
        g->clear();
        g->begin();

        int X = g->x();
        int Y = g->y() + 20;

        r.colorInfo = new ColorInfo( X, Y, g->w(), 280 );
        r.colorInfo->main( p.ui );
        
        g->end();
    }
    
    void ColorAreaTool::update( const area::Info& info )
    {
        TLRENDER_R();
        DBG;
        DBGM1( "r.colorInfo= " << r.colorInfo << " info=" << &info );
        r.colorInfo->update( info );
        DBG;
    }

}
