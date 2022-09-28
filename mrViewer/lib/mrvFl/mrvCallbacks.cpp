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

    void open_files_cb( const std::vector< std::string >& files, ViewerUI* ui )
    {
        for ( const auto& file : files )
        {
            ui->uiMain->app()->open( file );
        }
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void open_cb( Fl_Widget* w, ViewerUI* ui )
    {
        const stringArray& files = open_image_file( NULL, true, ui );
        open_files_cb( files, ui );
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

    void _reset_timeline( ViewerUI* ui )
    {
        ui->uiTimeline->setTimelinePlayer( nullptr );
        otio::RationalTime start = otio::RationalTime( 1, 24 );
        otio::RationalTime end   = otio::RationalTime( 50, 24 );
        ui->uiFrame->setTime( start );
        ui->uiStartFrame->setTime( start );
        ui->uiEndFrame->setTime( end );
    }

    void close_current_cb( Fl_Widget* w, ViewerUI* ui )
    {
        App* app = ui->uiMain->app();
        auto model = app->filesModel();
        model->close();
        ui->uiMain->fill_menu( ui->uiMenuBar );
        auto images = model->observeFiles()->get();
        if ( images.empty() ) _reset_timeline( ui );
    }

    void close_all_cb( Fl_Widget* w, ViewerUI* ui )
    {
        App* app = ui->uiMain->app();
        auto model = app->filesModel();
        model->closeAll();
        ui->uiMain->fill_menu( ui->uiMenuBar );
        _reset_timeline( ui );
    }

    void exit_cb( Fl_Widget* w, ViewerUI* ui )
    {
        // Delete the viewport so that the timeline is terminated
        // and so are all the threads.
        App* app = ui->uiMain->app();
        delete app;
        exit(0);
    }


    void minify_nearest_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
        o.imageFilters.minify = timeline::ImageFilter::Nearest;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    void minify_linear_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
        o.imageFilters.minify = timeline::ImageFilter::Linear;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    void magnify_nearest_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
        o.imageFilters.magnify = timeline::ImageFilter::Nearest;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    void magnify_linear_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        timeline::DisplayOptions& o = ui->uiView->getDisplayOptions(-1);
        o.imageFilters.magnify = timeline::ImageFilter::Linear;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    void mirror_x_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        timeline::DisplayOptions& d = ui->uiView->getDisplayOptions(-1);
        d.mirror.x ^= 1;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    void mirror_y_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        timeline::DisplayOptions& d = ui->uiView->getDisplayOptions(-1);
        d.mirror.y ^= 1;
        ui->uiMain->fill_menu( ui->uiMenuBar );
        ui->uiView->redraw();
    }

    static void toggle_channel( Fl_Menu_Item* item,
                                TimelineViewport* view,
                                const timeline::Channels channel )
    {
        const timeline::DisplayOptions& d = view->getDisplayOptions(-1);
        if ( d.channels == channel ) item->uncheck();

        view->toggleDisplayChannel( channel );
    }

    void toggle_red_channel_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Red;
        toggle_channel( item, ui->uiView, channel );
        ui->uiMain->fill_menu( ui->uiMenuBar );

    }

    void toggle_green_channel_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Green;
        toggle_channel( item, ui->uiView, channel );
        ui->uiMain->fill_menu( ui->uiMenuBar );

    }

    void toggle_blue_channel_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Blue;
        toggle_channel( item, ui->uiView, channel );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void toggle_alpha_channel_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        const timeline::Channels channel = timeline::Channels::Alpha;
        toggle_channel( item, ui->uiView, channel );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void _printActive( App* app )
    {
        auto model = app->filesModel();
        auto activeList = model->observeActive()->get();

        std::cerr << "------------------------------------ active list"
                  << std::endl;
        for ( const auto& active : activeList )
        {
            std::cerr << active->path.get() << std::endl;
        }
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
        ui->uiView->redraw();
    }

    void A_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto images = model->observeFiles()->get();
        if ( images.empty() ) return;

        auto Aindex = model->observeAIndex()->get();

        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::A;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );


        size_t start = m->find_index(_("Compare/Current")) + 1;

        // Find submenu's index
        size_t num = images.size() + start;
        for ( size_t i = start; i < num; ++i )
        {
            Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
            const char* label = item->label();
            size_t idx = i-start;
            if ( idx == Aindex )
            {
                item->set();
            }
            else
                item->clear();
        }
    }

    void B_media_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto images = model->observeFiles()->get();
        if ( images.empty() ) return;

        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::B;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );


        size_t start = m->find_index(_("Compare/Current")) + 1;

        // Find submenu's index
        size_t num = images.size() + start;
        auto Bindexes = model->observeBIndexes()->get();


        for ( size_t i = start; i < num; ++i )
        {
            Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( &(m->menu()[i]) );
            const char* label = item->label();
            size_t idx = i-start;
            if ( !Bindexes.empty() && idx == Bindexes[0] )
            {
                item->set();
            }
            else
                item->clear();
        }

    }

    void compare_overlay_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Overlay;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_difference_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Difference;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_horizontal_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Horizontal;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_vertical_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Vertical;
        model->setCompareOptions( compare );
        ui->uiView->setCompareOptions( compare );
    }

    void compare_tile_cb( Fl_Menu_* m, MainWindow* w )
    {
        App* app = w->app();
        ViewerUI* ui = w->main();
        auto model = app->filesModel();
        auto compare = model->observeCompareOptions()->get();
        compare.mode = timeline::CompareMode::Tile;
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

    void save_ui_state( ViewerUI* ui, Fl_Group* bar )
    {
        if ( bar == ui->uiMenuGroup )
            has_menu_bar   = ui->uiMenuGroup->visible();
        else if ( bar == ui->uiTopBar )
            has_top_bar    = ui->uiTopBar->visible();
        else if ( bar == ui->uiBottomBar )
            has_bottom_bar = ui->uiBottomBar->visible();
        else if ( bar == ui->uiPixelBar )
            has_pixel_bar  = ui->uiPixelBar->visible();
        else if ( bar == ui->uiToolsGroup )
            has_tools_grp  = ui->uiToolsGroup->visible();

        //@todo: add floating windows too
    }
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

        // Do not remove this resizes
        ui->uiMain->resize( 0, 0, W, H );
        ui->uiRegion->resize( 0, 0, W, H );
        ui->uiViewGroup->resize( 0, 0, W, H );
        ui->uiView->resize( 0, 0, W, H );

        // Do not remove this init_sizes/redraws.
        ui->uiViewGroup->init_sizes();
        ui->uiRegion->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiRegion->redraw();
        ui->uiView->invalidate();
        ui->uiView->redraw();
    }

    void toggle_action_tool_bar( Fl_Menu_* m, ViewerUI* ui )
    {
        int W = ui->uiRegion->w();
        int H = ui->uiRegion->h();
        Fl_Group* bar = ui->uiToolsGroup;
        bar->size( 45, bar->h() );

        if ( bar->visible() )
            bar->hide();
        else
            bar->show();
        ui->uiViewGroup->init_sizes();
        ui->uiViewGroup->layout();
        ui->uiViewGroup->redraw();
        ui->uiMain->fill_menu( ui->uiMenuBar );
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
        ui->uiRegion->layout();
        ui->uiRegion->redraw();
        ui->uiViewGroup->redraw();
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
        
        if ( has_tools_grp )
        {
            if ( !ui->uiToolsGroup->visible() )
            {
                ui->uiToolsGroup->show();
            }
        }
        
        ui->uiViewGroup->init_sizes();
        ui->uiViewGroup->redraw();
        ui->uiRegion->init_sizes();
        ui->uiRegion->redraw();

        //@todo: add showing of floating windows too
    }

    void hud_toggle_cb( Fl_Menu_* o, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( o->mvalue() );
        GLViewport* view = ui->uiView;
        view->setHudActive( item->checked() );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void hud_cb( Fl_Menu_* o, ViewerUI* ui )
    {
        const Fl_Menu_Item* item = o->mvalue();

        int i;
        Fl_Group* menu = ui->uiPrefs->uiPrefsHud;
        int num = menu->children();
        for ( i = 0; i < num; ++i )
        {
            const char* fmt = menu->child(i)->label();
            if (!fmt) continue;
            if ( strcmp( fmt, item->label() ) == 0 ) break;
        }

        GLViewport* view = ui->uiView;
        unsigned int hud = view->getHudDisplay();
        hud ^= ( 1 << i );
        view->setHudDisplay( (HudDisplay) hud );
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
