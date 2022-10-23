

#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvReelTool.h"
#include "mrvFl/mrvToolGroup.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvFunctional.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvPlayApp/App.h"

#include "mrViewer.h"

namespace mrv
{

   

    struct ReelTool::Private
    {
        std::weak_ptr<system::Context> context;
        std::unique_ptr<mrv::ThumbnailCreator> thumbnailCreator;
    };

    
    ReelTool::ReelTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {

        _r->context = ui->uiMain->app()->getContext();

        add_group( "Reel" );
        
    }

    void ReelTool::add_controls()
    {
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ToolGroup* t = (ToolGroup*) d;
            ToolGroup::cb_dismiss( NULL, t );
            delete reelTool; reelTool = nullptr;
        }, g );
    }

}
