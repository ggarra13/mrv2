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

    void wipe_cb( Fl_Menu_* m, MainWindow* w )
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

    void fullscreen_ui_off( ViewerUI* ui )
    {
#if 1
        std::cerr << " FULLSCREEN_UI_OFF " << std::endl;
        std::cerr << "-------------------" << std::endl;
        std::cerr << "START   uiMain=" << ui->uiMain->w()
                  << "x" << ui->uiMain->h() << std::endl;
        std::cerr << "START uiRegion=" << ui->uiRegion->w()
                  << "x" << ui->uiRegion->h() << std::endl;
        std::cerr << "START fullscreen="
                  << ui->uiMain->fullscreen_active()
                  << std::endl;
#endif
        ui->uiMain->fullscreen_off();
#if 1
        std::cerr << " FULLSCREEN_UI_OFF " << std::endl;
        std::cerr << "-------------------" << std::endl;
        std::cerr << "END     uiMain=" << ui->uiMain->w()
                  << "x" << ui->uiMain->h() << std::endl;
        std::cerr << "END   uiRegion=" << ui->uiRegion->w()
                  << "x" << ui->uiRegion->h()
                  << std::endl;
        std::cerr << "END fullscreen="
                  << ui->uiMain->fullscreen_active()
                  << std::endl;
#endif
        restore_ui_state( ui );
    }


    void hide_ui_state( ViewerUI* ui )
    {

        int W = ui->uiRegion->w();
        int H = ui->uiRegion->h();

        if ( has_tools_grp )
        {
            W += ui->uiToolsGroup->w();
            ui->uiToolsGroup->hide();
        }

        if ( has_bottom_bar ) {
            H += ui->uiBottomBar->h();
            ui->uiBottomBar->hide();
        }
        if ( has_pixel_bar ) {
            H += ui->uiPixelBar->h();
            ui->uiPixelBar->hide();
        }
        if ( has_top_bar ) {
            H += ui->uiTopBar->h();
            ui->uiTopBar->hide();
        }
        if ( has_menu_bar )
        {
            H += ui->uiMenuGroup->h();
            ui->uiMenuGroup->hide();
        }


        ui->uiRegion->size( W, H );
        ui->uiRegion->init_sizes();
        ui->uiRegion->layout();
        ui->uiRegion->redraw();

#if 0
        std::cerr << "  HIDE UI STATE " << std::endl;
        std::cerr << "------------------" << std::endl;
        std::cerr << "uiMain=" << ui->uiMain->w()
                  << "x" << ui->uiMain->h() << std::endl;
        std::cerr << "uiRegion=" << ui->uiRegion->w()
                  << "x" << ui->uiRegion->h() << std::endl;
#endif
    }

    void toggle_action_tool_bar( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Group* bar = ui->uiToolsGroup;
        bar->size( 45, 433 );

#if 0
        int RX = ui->uiRegion->x();
        int RY = ui->uiRegion->y();
        int RW = ui->uiRegion->w();
        int RH = ui->uiRegion->h();
        int X = ui->uiViewGroup->x();
        int Y = ui->uiViewGroup->y();
        int W = ui->uiViewGroup->w();
        int H = ui->uiViewGroup->h();
        int VW = ui->uiView->w();
        int VH = ui->uiView->h();
        int BW = bar->w();
        int BH = bar->h();

        std::cerr << "START visible? " << bar->visible() << std::endl
                  << "region    W=" << RW << " H=" << RH << std::endl
                  << "viewgroup W=" << W << " H=" << H << std::endl
                  << "     view W=" << VW << " H=" << VH << std::endl
                  << "      bar W=" << BW << " H=" << BH << std::endl;
#endif

        if ( bar->visible() )
            bar->hide();
        else
            bar->show();
        ui->uiViewGroup->layout();
        ui->uiViewGroup->redraw();


#if 0
        RW = ui->uiRegion->w();
        RH = ui->uiRegion->h();
        W = ui->uiViewGroup->w();
        H = ui->uiViewGroup->h();
        VW = ui->uiView->w();
        VH = ui->uiView->h();
        BW = bar->w();
        BH = bar->h();

        std::cerr << "END visible? " << bar->visible() << std::endl
                  << "region    W=" << RW << " H=" << RH << std::endl
                  << "viewgroup W=" << W << " H=" << H << std::endl
                  << "     view W=" << VW << " H=" << VH << std::endl
                  << "      bar W=" << BW << " H=" << BH << std::endl;
#endif
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
    }


    void restore_ui_state( ViewerUI* ui )
    {
#if 1
        std::cerr << " RESTORE UI STATE " << std::endl;
        std::cerr << "------------------" << std::endl;
        std::cerr << "START   uiMain=" << ui->uiMain->w()
                  << "x" << ui->uiMain->h() << std::endl;
        std::cerr << "START uiRegion=" << ui->uiRegion->w()
                  << "x" << ui->uiRegion->h()
                  << std::endl;
#endif
        if ( has_tools_grp ) {
            ui->uiToolsGroup->size( 45, 433 );
            ui->uiToolsGroup->show();
        }

        int W = ui->uiRegion->w();

        if ( has_menu_bar && !ui->uiMenuGroup->visible() )    {
            ui->uiMenuGroup->size( W, int(25) );
            ui->uiMain->fill_menu( ui->uiMenuBar );
            ui->uiMenuGroup->show();
        }
        if ( has_top_bar )    {
            int w = ui->uiTopBar->w();
            ui->uiTopBar->size( w, int(28) );
            ui->uiTopBar->show();
        }
        if ( has_bottom_bar)  {
            ui->uiBottomBar->size( W, int(49) );
            ui->uiBottomBar->show();
        }
        if ( has_pixel_bar )  {
            ui->uiPixelBar->size( W, int(30) );
            ui->uiPixelBar->show();
        }

        ui->uiRegion->size( W, ui->uiMain->h() );
        ui->uiRegion->init_sizes();
        ui->uiRegion->layout();
        ui->uiRegion->redraw();
#if 1
        std::cerr << "END     uiMain=" << ui->uiMain->w()
                  << "x" << ui->uiMain->h() << std::endl;
        std::cerr << "END   uiRegion=" << ui->uiRegion->w()
                  << "x" << ui->uiRegion->h()
                  << std::endl;
#endif

        //@todo: add floating windows too
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
