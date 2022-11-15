
#include "mrvGL/mrvGLViewport.h"

#include "mrvColorAreaInfo.h"
#include "mrvWidgets/mrvColorInfo.h"

#include "mrvToolsCallbacks.h"
#include "mrvHorSlider.h"
#include "mrvFunctional.h"

#include "mrvColorAreaTool.h"

#include "mrViewer.h"

namespace mrv
{

    struct ColorAreaTool::Private
    {
    };

    
    ColorAreaTool::ColorAreaTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( "Color Area" );
        
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

        Fl_Box* box;
        Pack*    sg;
        Fl_Group* bg;
        CollapsibleGroup* cg;
        Fl_Button* b;
        Fl_Choice* m;
            
        g->clear();
        g->begin();

        int X = g->x();
        int Y = g->y() + 20;

        auto tW = new Widget< ColorInfo >( X, Y, g->w(), 200 );
        
        g->end();
    }
    

}
