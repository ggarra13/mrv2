


#include "mrvToolsCallbacks.h"
#include "mrvFunctional.h"

#include "mrvDevicesTool.h"

#include "mrViewer.h"

namespace mrv
{

    struct DevicesTool::Private
    {
    };

    
    DevicesTool::DevicesTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( "Devices" );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete devicesTool; devicesTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
        
    }

    DevicesTool::~DevicesTool()
    {
    }



    void DevicesTool::add_controls()
    {
        TLRENDER_P();
	
        
        g->clear();
    
        g->begin();

        g->end();

        
    }
    

}
