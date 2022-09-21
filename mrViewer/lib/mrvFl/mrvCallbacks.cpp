#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvSequence.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

    void open_directory_cb( Fl_Widget* w, ViewerUI* ui )
    {
        std::string dir = mrv::open_directory(NULL, ui);
        if (dir.empty()) return;

        stringArray movies, sequences, audios;
        parse_directory( dir, movies, sequences, audios );

        App* app = ui->uiMain->app();
        for ( const auto& movie : movies )
        {
            app->open( movie );
        }
        for ( const auto& sequence : sequences )
        {
            app->open( sequence );
        }
        for ( const auto& audio : audios )
        {
            app->open( audio );
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
        int idx = m->find_index(picked) - start - 1;

        ViewerUI* ui = w->main();
        App* app = w->app();
        auto model = app->filesModel();


        item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/A") ) );
        if ( item->checked() )
        {
            model->setA( idx );
            return;
        }
        item = const_cast< Fl_Menu_Item* >( m->find_item( _("Compare/B") ) );
        if ( item->checked() )
        {
            model->setB( idx, false );
            return;
        }
    }

    void compare_wipe_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Wipe;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void A_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto images = model->observeFiles()->get();


        size_t start = m->find_index(_("Compare/Current")) + 1;

        // Find submenu's index
        size_t num = images.size() + start;
        auto Aindex = model->observeAIndex()->get();
        for ( size_t i = start; i < num; ++i )
        {
            Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
            const char* label = item->label();
            size_t idx = i-start;
            if ( idx == Aindex )
            {
                model->setA( idx );
                item->set();
            }
            else
                item->clear();
        }


        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::A;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void B_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto images = model->observeFiles()->get();


        size_t start = m->find_index(_("Compare/Current")) + 1;

        // Find submenu's index
        size_t num = images.size() + start;
        auto Bindexes = model->observeBIndexes()->get();
        for ( size_t i = start; i < num; ++i )
        {
            Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
            const char* label = item->label();
            size_t idx = i-start;
            if ( Bindexes.size() && idx == Bindexes[0] )
            {
                model->setB( idx, true );
                item->set();
            }
            else
                item->clear();
        }

        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::B;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_overlay_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Overlay;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }
    
    void compare_difference_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Difference;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }
    
    void compare_horizontal_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Horizontal;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_vertical_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Vertical;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }
    
    void compare_tile_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Tile;
        ViewerUI* ui = w->main();
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }
    
    void window_cb( Fl_Menu_* m, ViewerUI* ui )
    {

        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );

        std::string tmp = item->text;
        Fl_Window* w = nullptr;

        unsigned hotkey = 0;
        if ( tmp == _("Reels") )
            w = nullptr;
        else if ( tmp == _("Media Info") )
            w = nullptr;
        else if ( tmp == _("Color Info") )
            w = nullptr;
        else if ( tmp == _("Color Controls") )
            w = nullptr;
        else if ( tmp == _("Action Tools") )
            w = nullptr;
        else if ( tmp == _("Preferences") )
            w = ui->uiPrefs->uiMain;
        else if ( tmp == _("Histogram") )
            w = nullptr;
        else if ( tmp == _("Vectorscope") )
            w = nullptr;
        else if ( tmp == _("Waveform") )
            w = nullptr;
        else if ( tmp == _("Connections") )
            w = nullptr;
        else if ( tmp == _("Hotkeys") )
            w = nullptr;
        else if ( tmp == _("Logs") )
            w = nullptr;
        else if ( tmp == _("About") )
            w = ui->uiAbout->uiMain;
        else
            return; // Unknown window

        if ( w ) w->show();

    }

    bool has_tools_grp = true,
        has_menu_bar = true,
        has_top_bar = true,
        has_bottom_bar = true,
        has_pixel_bar = true;

    void save_ui_state( ViewerUI* ui )
    {
        has_menu_bar   = ui->uiMenuGroup->visible();
        has_top_bar    = ui->uiTopBar->visible();
        has_bottom_bar = ui->uiBottomBar->visible();
        has_pixel_bar  = ui->uiPixelBar->visible();
        has_tools_grp  = ui->uiToolsGroup->visible();

        //@todo: add floating windows too
    }


    void hide_ui_state( ViewerUI* ui )
    {

        int W = ui->uiMain->w();
        int H = ui->uiMain->h();

        if ( has_tools_grp )
        {
            ui->uiToolsGroup->hide();
        }

        if ( has_bottom_bar ) {
            ui->uiBottomBar->hide();
        }
        if ( has_pixel_bar ) {
            ui->uiPixelBar->hide();
        }
        if ( has_top_bar ) {
            ui->uiTopBar->hide();
        }
        if ( has_menu_bar )
        {
            ui->uiMenuGroup->hide();
        }

        ui->uiMain->resize( 0, 0, W, H );
        ui->uiRegion->resize( 0, 0, W, H );
        ui->uiViewGroup->resize( 0, 0, W, H );
        ui->uiView->resize( 0, 0, W, H );

        ui->uiViewGroup->init_sizes();
        ui->uiRegion->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiRegion->redraw();
        ui->uiView->invalidate();
        ui->uiView->redraw();

        //@todo: add hiding of floating windows too
    }

    void toggle_action_tool_bar( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Group* bar = ui->uiToolsGroup;
        bar->size( 45, bar->h() );

        if ( bar->visible() )
            bar->hide();
        else
            bar->show();
        ui->uiViewGroup->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiMain->fill_menu( ui->uiMenuBar );
        bar->redraw();
    }

    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size )
    {
        int W = ui->uiRegion->w();
        int H = ui->uiRegion->h();
        if ( bar->visible() )
        {
            bar->hide();
            H += bar->h();
        }
        else
        {
            bar->size( bar->w(), size );
            bar->show();
            H -= bar->h();
        }
        ui->uiRegion->size( W, H );
        ui->uiRegion->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiRegion->redraw();
        bar->redraw();
    }


    void restore_ui_state( ViewerUI* ui )
    {

        int X = ui->uiRegion->x();
        int W = ui->uiRegion->w();

        int H = ui->uiViewGroup->h();
        int Y = ui->uiViewGroup->y();

        if ( has_menu_bar )
        {
            ui->uiMenuGroup->size( W, int(25) );
            if ( !ui->uiMenuGroup->visible() )    {
                ui->uiMain->fill_menu( ui->uiMenuBar );
                ui->uiMenuGroup->show();
                H -= ui->uiMenuGroup->h();
                Y += ui->uiMenuGroup->h();
            }
        }

        ui->uiTopBar->size( W, int(28) );
        if ( has_top_bar )
        {
            if ( !ui->uiTopBar->visible() )    {
                ui->uiTopBar->show();
                H -= ui->uiTopBar->h();
                Y += ui->uiTopBar->h();
            }
        }

        ui->uiBottomBar->size( W, int(49) );
        if ( has_bottom_bar )
        {
            if ( !ui->uiBottomBar->visible() )  {
                ui->uiBottomBar->show();
                H -= ui->uiBottomBar->h();
            }
        }

        ui->uiPixelBar->size( W, int(30) );
        if ( has_pixel_bar )
        {
            if ( !ui->uiPixelBar->visible() )  {
                ui->uiPixelBar->show();
                H -= ui->uiPixelBar->h();
            }
        }


        ui->uiViewGroup->resize( X, Y, W, H );
        if ( has_tools_grp  && !ui->uiToolsGroup->visible() )
        {
            ui->uiToolsGroup->size( 45, 20 );
            ui->uiToolsGroup->show();
        }

        ui->uiViewGroup->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiRegion->init_sizes();
        ui->uiRegion->redraw();

        //@todo: add showing of floating windows too
    }

    void hud_cb( Fl_Menu_* o, ViewerUI* ui )
    {
        const Fl_Menu_Item* item = o->mvalue();
        GLViewport* view = ui->uiView;

        int i;
        Fl_Group* menu = ui->uiPrefs->uiPrefsHud;
        int num = menu->children();
        for ( i = 0; i < num; ++i )
        {
            const char* fmt = menu->child(i)->label();
            if (!fmt) continue;
            if ( strcmp( fmt, item->label() ) == 0 ) break;
        }

        unsigned int hud = view->getHudDisplay();
        hud ^= ( 1 << i );
        view->setHudDisplay( (HudDisplay) hud );
        view->redraw();
    }

    // Playback callbacks
    void play_forwards_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->playForwards();
    }

    void play_backwards_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->playBackwards();
    }

    void stop_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->stop();
    }

    void toggle_playback_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        ui->uiView->togglePlayback();
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    static void playback_loop_mode( ViewerUI* ui, timeline::Loop mode )
    {
        ui->uiLoopMode->value( (int)mode );
        ui->uiLoopMode->do_callback();
    }

    void playback_loop_cb( Fl_Menu_*, ViewerUI* ui )
    {
        playback_loop_mode( ui, timeline::Loop::Loop );
    }
    void playback_once_cb( Fl_Menu_*, ViewerUI* ui )
    {
        playback_loop_mode( ui, timeline::Loop::Once );
    }
    void playback_ping_pong_cb( Fl_Menu_*, ViewerUI* ui )
    {
        playback_loop_mode( ui, timeline::Loop::PingPong );
    }
}
