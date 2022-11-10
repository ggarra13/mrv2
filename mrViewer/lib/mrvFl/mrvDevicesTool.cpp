
#include <FL/Fl_Spinner.H>


#include "mrvPlayApp/mrvDevicesModel.h"

#include "mrvToolsCallbacks.h"
#include "mrvHorSlider.h"
#include "mrvFunctional.h"

#include "mrvDevicesTool.h"

#include "mrViewer.h"

namespace mrv
{

    struct DevicesTool::Private
    {
        App* app = nullptr;
        std::shared_ptr<observer::ValueObserver<DevicesModelData> > dataObserver;
        HorSlider* maxCLLSlider = nullptr;
        HorSlider* maxFALLSlider = nullptr;
    };

    
    DevicesTool::DevicesTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        add_group( "Devices" );
        
        Fl_SVG_Image* svg = load_svg( "Devices.svg" );
        g->image( svg );
        
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

        HorSlider* s;
        auto sW = new Widget< HorSlider >( g->x(), g->y()+20, g->w(), 20 );
        s = _r->maxCLLSlider = sW;
        s->range( 0.F, 10000.F );
        
        sW = new Widget< HorSlider >( g->x(), g->y()+40, g->w(), 20 );
        s = _r->maxFALLSlider = sW;
        s->range( 0.F, 10000.F );
        

        
        g->end();

        
    }
    

}
