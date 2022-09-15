#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvFileRequester.h"
#include "mrvFl/mrvCallbacks.h"

#include "mrvPlayApp/mrvFilesModel.h"
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
        ui->uiMain->fill_menu( ui->uiMenuBar );
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

    void change_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        Fl_Menu_Item* item = nullptr;
        Fl_Menu_Item* picked = const_cast< Fl_Menu_Item* >( m->mvalue() );

        int start = m->find_index(_("Compare/Current"));

        // Find submenu's index
        int idx = m->find_index(item) - start - 1;

        App* app = w->app();
        auto model = app->filesModel();

#if 0
        auto compare = model->observeCompareOptions()->get();
        if ( compare.mode == timeline::CompareMode::A )
        {
            std::cerr << "setting A to " << picked->text << std::endl;
            model->setA( idx );
            return;
        }

        if ( compare.mode == timeline::CompareMode::B )
        {
            std::cerr << "setting B to " << picked->text << std::endl;
            model->setB( idx, true );
            return;
        }
#else
        item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/A") ) );
        if ( item->checked() )
        {
            std::cerr << "setting A to " << picked->text << std::endl;
            model->setA( idx );
            return;
        }
        item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/B") ) );;
        if ( item->checked() )
        {
            std::cerr << "setting B to " << picked->text << std::endl;
            model->setB( idx, true );
            return;
        }
#endif
    }

    void wipe_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Wipe;
        std::cerr << "setting Wipe " << std::endl;
        ViewerUI* ui = w->main();
        ui->uiView->setCompareOptions( compare );
    }

    void A_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::A;
        ViewerUI* ui = w->main();
        ui->uiView->setCompareOptions( compare );
    }

    void B_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::B;
        ViewerUI* ui = w->main();
        ui->uiView->setCompareOptions( compare );
    }
}
