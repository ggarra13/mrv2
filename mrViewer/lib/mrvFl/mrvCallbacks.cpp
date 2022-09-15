#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvFileRequester.h"
#include "mrvPlayApp/App.h"

#include "mrViewer.h"

namespace mrv
{

void open_cb( Fl_Widget* w, ViewerUI* ui )
{
    const stringArray& files = open_image_file( NULL, true, ui );
    for ( const auto& file : files )
    {
        ui->uiMain->app()->open( file );
    }
}

void exit_cb( Fl_Widget* w, ViewerUI* ui )
{
    delete ui;
    exit(0);
}

    void display_options_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        view->updateDisplayOptions();
    }
}
