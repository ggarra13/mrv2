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

    void mirror_x_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        timeline::DisplayOptions& d = view->getDisplayOptions();
        d.mirror.x ^= 1;
    }

    void mirror_y_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        timeline::DisplayOptions& d = view->getDisplayOptions();
        d.mirror.y ^= 1;
    }

    static void toggle_channel( Fl_Menu_Item* item,
                                TimelineViewport* view,
                                const timeline::Channels channel )
    {
        const timeline::DisplayOptions& d = view->getDisplayOptions();
        if ( d.channels == channel ) item->uncheck();

        view->toggleDisplayChannel( channel );
    }

    void toggle_red_channel_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Red;
        toggle_channel( item, view, channel );

    }

    void toggle_green_channel_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Green;
        toggle_channel( item, view, channel );

    }

    void toggle_blue_channel_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Blue;
        toggle_channel( item, view, channel );
    }

    void toggle_alpha_channel_cb( Fl_Menu_* w, TimelineViewport* view )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Alpha;
        toggle_channel( item, view, channel );
    }
}
