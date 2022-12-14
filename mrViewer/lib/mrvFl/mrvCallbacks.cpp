// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvSequence.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <iomanip>

#include "mrvWidgets/mrvToolGroup.h"
#include "mrvWidgets/mrvSecondaryWindow.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvMenus.h"
#include "mrvFl/mrvVersioning.h"
#include "mrvFl/mrvFileRequester.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "make_ocio_chooser.h"
#include "mrvHotkeyUI.h"
#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace {
  const char* kModule = "cback";
}

namespace mrv
{

    WindowCallback kWindowCallbacks[] =
    {
        {_("Files"), (Fl_Callback*)files_tool_grp },
        {_("Media Information"), (Fl_Callback*)image_info_tool_grp },
        {_("Color"), (Fl_Callback*)color_tool_grp },
        {_("Color Area"), (Fl_Callback*)color_area_tool_grp },
        {_("Compare"), (Fl_Callback*)compare_tool_grp },
        {_("Histogram"), (Fl_Callback*)histogram_tool_grp },
        {_("Annotations"), (Fl_Callback*)annotations_tool_grp },
        {_("Devices"), (Fl_Callback*)devices_tool_grp },
        {_("Settings"), (Fl_Callback*)settings_tool_grp },
        {_("Vectorscope"), (Fl_Callback*)vectorscope_tool_grp },
        {_("Preferences"), (Fl_Callback*)nullptr },
        {_("Hotkeys"), (Fl_Callback*)nullptr },
        {_("Logs"), (Fl_Callback*)logs_tool_grp },
        {_("About"), (Fl_Callback*)nullptr },
        { nullptr, nullptr }
    };

    static void refresh_tool_grp()
    {
        if ( filesTool )     filesTool->refresh();
        if ( compareTool ) compareTool->refresh();
        if ( imageInfoTool ) imageInfoTool->refresh();
    }

    static void reset_timeline( ViewerUI* ui )
    {
        if ( imageInfoTool ) imageInfoTool->setTimelinePlayer( nullptr );
        TimelineClass* c = ui->uiTimeWindow;
        c->uiTimeline->setTimelinePlayer( nullptr );
        otio::RationalTime start = otio::RationalTime( 1, 24 );
        otio::RationalTime end   = otio::RationalTime( 50, 24 );
        c->uiFrame->setTime( start );
        c->uiStartFrame->setTime( start );
        c->uiEndFrame->setTime( end );
    }

    static void printIndices( const std::vector< int >& Bindexes )
    {
        std::cerr << "Indices now: " << std::endl;
        for ( auto& i : Bindexes )
        {
            std::cerr << i << " ";
        }
        std::cerr << std::endl;
    }

    void open_files_cb( const std::vector< std::string >& files, ViewerUI* ui )
    {
        for ( const auto& file : files )
        {
            ui->app->open( file );
        }
        ui->uiMain->fill_menu( ui->uiMenuBar );
        refresh_tool_grp();
    }

    void open_cb( Fl_Widget* w, ViewerUI* ui )
    {
        const stringArray& files = open_image_file( NULL, true, ui );
        open_files_cb( files, ui );
    }

    void open_recent_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( w->mvalue() );
        if ( !item || !item->label() ) return;
        stringArray files;
        std::string file = item->label();
        files.push_back( file );
        open_files_cb( files, ui );
    }

    void open_separate_audio_cb( Fl_Widget* w, ViewerUI* ui )
    {
        ui->app->openSeparateAudioDialog();
        ui->uiMain->fill_menu( ui->uiMenuBar );
        refresh_tool_grp();
    }

    void open_directory_cb( Fl_Widget* w, ViewerUI* ui )
    {
        std::string dir = open_directory(NULL, ui);
        if (dir.empty()) return;

        if ( !fs::is_directory( dir ) ) return;

        stringArray movies, sequences, audios;
        parse_directory( dir, movies, sequences, audios );

        for ( const auto& movie : movies )
        {
            ui->app->open( movie );
        }
        for ( const auto& sequence : sequences )
        {
            ui->app->open( sequence );
        }
        for ( const auto& audio : audios )
        {
            ui->app->open( audio );
        }

        ui->uiMain->fill_menu( ui->uiMenuBar );
        refresh_tool_grp();
    }

    void previous_file_cb( Fl_Widget* w, ViewerUI* ui )
    {
        auto model = ui->app->filesModel();
        model->prev();
    }

    void next_file_cb( Fl_Widget* w, ViewerUI* ui )
    {
        auto model = ui->app->filesModel();
        model->next();
    }


    void save_movie_cb( Fl_Menu_* w, ViewerUI* ui )
    {
      try
	{
	  save_movie_file( ui );
	}
      catch( std::exception& e )
	{
	  LOG_ERROR( e.what() );
	}
    }

    void close_current_cb( Fl_Widget* w, ViewerUI* ui )
    {
        auto model = ui->app->filesModel();
        model->close();

        ui->uiMain->fill_menu( ui->uiMenuBar );

        auto images = model->observeFiles()->get();
        if ( images.empty() ) reset_timeline( ui );

        refresh_tool_grp();
    }

    void close_all_cb( Fl_Widget* w, ViewerUI* ui )
    {
        auto model = ui->app->filesModel();
        model->closeAll();

        ui->uiMain->fill_menu( ui->uiMenuBar );

        reset_timeline( ui );

        refresh_tool_grp();
    }

    void exit_cb( Fl_Widget* w, ViewerUI* ui )
    {
        ui->uiView->stop();

        //! Close all files
        close_all_cb( w, ui );

        // Store window preferences
        if ( colorTool )         colorTool->save();
        if ( filesTool )         filesTool->save();
        if ( colorAreaTool ) colorAreaTool->save();
        if ( compareTool )     compareTool->save();
        if ( settingsTool )   settingsTool->save();
        if ( logsTool )           logsTool->save();
        if ( devicesTool )     devicesTool->save();
        if ( annotationsTool ) annotationsTool->save();
        if ( imageInfoTool )   imageInfoTool->save();
        if ( histogramTool )   histogramTool->save();
        if ( vectorscopeTool ) vectorscopeTool->save();
        if ( ui->uiSecondary ) ui->uiSecondary->save();

        // Save preferences
        Preferences::save();

        // Delete all windows which will close all threads.
        delete ui->uiSecondary; ui->uiSecondary = nullptr;
        ui->uiMain->hide(); // Wayland seems to segfault if we just delete it.
        delete ui->uiPrefs; ui->uiPrefs = nullptr;
        delete ui->uiAbout; ui->uiAbout = nullptr;
        delete ui->uiHotkey; ui->uiHotkey = nullptr;

        // Hide all ToolGroup windows
        ToolGroup::hide_all();

        // The program should exit cleanly from the Fl::run loop now
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

    void toggle_color_channel_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        const timeline::Channels channel = timeline::Channels::Color;
        ui->uiView->toggleDisplayChannel( channel );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void toggle_fullscreen_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        bool active = true;
        const Fl_Menu_Item* item = m->mvalue();
        if ( !item->checked() ) active = false;
        ui->uiView->setFullScreenMode( active );
    }

    void toggle_float_on_top_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        bool active = true;
        const Fl_Menu_Item* item = m->mvalue();
        if ( !item->checked() ) active = false;
        ui->uiMain->always_on_top( active );
    }

    void toggle_secondary_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        MainWindow* window;
        Viewport* view;

        if ( ui->uiSecondary )
        {
            window = ui->uiSecondary->window();
        }
        else
        {
            ui->uiSecondary = new SecondaryWindow( ui );
            window = ui->uiSecondary->window();
        }

        view = ui->uiSecondary->viewport();
        if ( window->visible() )
        {
            window->hide();
            // This hiding and showing of uiView is needed
            // or else we would get flickering on Windows and Linux.
            ui->uiView->hide();
            ui->uiView->show();
            ui->uiView->take_focus();
        }
        else
        {
            view->setContext( ui->app->getContext() );
            view->setColorConfigOptions( ui->uiView->getColorConfigOptions() );
            view->setLUTOptions( ui->uiView->lutOptions() );
            view->setImageOptions( ui->uiView->getImageOptions() );
            view->setDisplayOptions(  ui->uiView->getDisplayOptions() );
            view->setCompareOptions( ui->uiView->getCompareOptions() );
            view->setTimelinePlayers( ui->uiView->getTimelinePlayers(), false );
            window->show();
            view->frameView();
        }
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void toggle_secondary_float_on_top_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );
        if ( ! ui->uiSecondary || ! ui->uiSecondary->window()->visible() ) {
            item->clear();
            return;
        }

        bool active = true;
        if ( !item->checked() ) active = false;
        ui->uiSecondary->window()->always_on_top( active );
    }


    void show_window_cb( const std::string& label, ViewerUI* ui )
    {

        Fl_Window* w = nullptr;

        const WindowCallback* wc = kWindowCallbacks;
        for ( ; wc->name; ++wc )
        {
            if ( label == _( wc->name ) )
            {
                if ( wc->callback )
                {
                    wc->callback( nullptr, ui );
                    return;
                }
            }
        }

        if ( label == _("Preferences") )
            w = ui->uiPrefs->uiMain;
        else if ( label == _("Waveform") )
            w = nullptr;
        else if ( label == _("Hotkeys") )
            w = ui->uiHotkey->uiMain;
        else if ( label == _("About") )
            w = ui->uiAbout->uiMain;
        else
        {
            LOG_ERROR( "Callbacks: Unknown window " << label );
            return; // Unknown window
        }

        if ( !w || w->visible() ) return;

        w->show();
        w->callback( []( Fl_Widget* o, void* data )
            {
                ViewerUI* ui = static_cast< ViewerUI* >( data );
                o->hide();
                ui->uiMain->fill_menu( ui->uiMenuBar );
            }, ui );
    }

    void window_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );
        std::string label = item->text;
        show_window_cb( label, ui );
    }


    bool has_tools_grp = true,
        has_menu_bar = true,
        has_top_bar = true,
        has_bottom_bar = true,
        has_pixel_bar = true,
        has_dock_grp = true,
        has_preferences_window = false,
        has_hotkeys_window = false,
        has_about_window = false;

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
        else if ( bar == ui->uiDockGroup )
            has_dock_grp  = ui->uiDockGroup->visible();

    }
    void save_ui_state( ViewerUI* ui )
    {
        has_menu_bar   = ui->uiMenuGroup->visible();
        has_top_bar    = ui->uiTopBar->visible();
        has_bottom_bar = ui->uiBottomBar->visible();
        has_pixel_bar  = ui->uiPixelBar->visible();
        has_tools_grp  = ui->uiToolsGroup->visible();
        has_dock_grp = ui->uiDockGroup->visible();

        has_preferences_window = ui->uiPrefs->uiMain->visible();
        has_hotkeys_window = ui->uiHotkey->uiMain->visible();
        has_about_window = ui->uiAbout->uiMain->visible();
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
        if ( has_dock_grp )
        {
            ui->uiDockGroup->hide();
        }

        if ( has_preferences_window ) ui->uiPrefs->uiMain->hide();
        if ( has_hotkeys_window )     ui->uiHotkey->uiMain->hide();
        if ( has_about_window )       ui->uiAbout->uiMain->hide();

        ToolGroup::hide_all();

        ui->uiRegion->layout();
    }

    void toggle_action_tool_bar( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Group* bar = ui->uiToolsGroup;

        if ( bar->visible() )
            bar->hide();
        else
            bar->show();

        ui->uiViewGroup->layout();
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size )
    {
        if ( bar->visible() )
        {
            bar->hide();
        }
        else
        {
            bar->show();
        }

        ui->uiRegion->layout();
    }


    void restore_ui_state( ViewerUI* ui )
    {

        if ( has_menu_bar )
        {
            if ( !ui->uiMenuGroup->visible() )    {
                ui->uiMain->fill_menu( ui->uiMenuBar );
                ui->uiMenuGroup->show();
            }
        }

        if ( has_top_bar )
        {
            if ( !ui->uiTopBar->visible() )    {
                ui->uiTopBar->show();
            }
        }

        if ( has_bottom_bar )
        {
            if ( !ui->uiBottomBar->visible() )  {
                ui->uiBottomBar->show();
            }
        }

        if ( has_pixel_bar )
        {
            if ( !ui->uiPixelBar->visible() )  {
                ui->uiPixelBar->show();
            }
        }


        ui->uiViewGroup->layout();

        if ( has_tools_grp )
        {
            if ( !ui->uiToolsGroup->visible() )
            {
                ui->uiToolsGroup->show();
            }
        }

        if ( has_dock_grp )
        {
            if ( !ui->uiDockGroup->visible() )
            {
                ui->uiDockGroup->show();
            }
        }

        ui->uiRegion->layout();

        if ( has_preferences_window ) ui->uiPrefs->uiMain->show();
        if ( has_hotkeys_window )     ui->uiHotkey->uiMain->show();
        if ( has_about_window )       ui->uiAbout->uiMain->show();

        ToolGroup::show_all();
    }

    void hud_toggle_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );
        Viewport* view = ui->uiView;
        view->setHudActive( item->checked() );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }


    void safe_areas_cb( Fl_Menu_* m, ViewerUI* ui )
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >( m->mvalue() );
        ui->uiView->setSafeAreas( item->checked() );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }


    void masking_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        // Find offset of View/Mask submenu
        int offset = w->find_index( _("View/Mask") ) + 1;

        int idx = w->value() - offset;
        float mask = kCrops[idx];
        ui->uiView->setMask( mask );
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

        Viewport* view = ui->uiView;
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
    }

    static void playback_loop_mode( ViewerUI* ui, timeline::Loop mode )
    {
        TimelineClass* c = ui->uiTimeWindow;
        c->uiLoopMode->value( (int)mode );
        c->uiLoopMode->do_callback();
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

    // OCIO callbacks
    void attach_ocio_ics_cb( Fl_Menu_*, ViewerUI* ui )
    {
        mrv::PopupMenu* w = ui->uiICS;
        std::string ret = make_ocio_chooser( w->label(),
                                             OCIOBrowser::kInputColorSpace );
        if ( ret.empty() ) return;

        for ( size_t i = 0; i < w->children(); ++i )
        {
            const Fl_Menu_Item* o = w->child(i);
            if ( !o || !o->label() ) continue;

            if ( ret == o->label() )
            {
                w->copy_label( o->label() );
                w->value(i);
                w->do_callback();
                ui->uiView->redraw();
                break;
            }
        }
    }

    void attach_ocio_display_cb( Fl_Menu_*, ViewerUI* ui )
    {
        std::string ret = make_ocio_chooser( Preferences::OCIO_Display,
                                             OCIOBrowser::kDisplay );
        if ( ret.empty() ) return;
        Preferences::OCIO_Display = ret;
        ui->uiView->redraw();
    }

    void attach_ocio_view_cb( Fl_Menu_*, ViewerUI* ui )
    {
        std::string ret = make_ocio_chooser( Preferences::OCIO_View,
                                             OCIOBrowser::kView );
        if ( ret.empty() ) return;
        Preferences::OCIO_View = ret;
        Fl_Menu_Button* m = ui->OCIOView;
        for ( int i = 0; i < m->size(); ++i )
        {
            const char* lbl = m->menu()[i].label();
            if ( !lbl ) continue;
            if ( ret == lbl )
            {
                m->value(i);
                break;
            }
        }
        m->copy_label( _(ret.c_str()) );
        m->redraw();
        ui->uiView->redraw();
    }

    void video_levels_from_file_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.videoLevels = timeline::InputVideoLevels::FromFile;
        ui->uiView->redraw();
    }

    void video_levels_legal_range_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.videoLevels = timeline::InputVideoLevels::LegalRange;
        ui->uiView->redraw();
    }

    void video_levels_full_range_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.videoLevels = timeline::InputVideoLevels::FullRange;
        ui->uiView->redraw();
    }

    void alpha_blend_none_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.alphaBlend = timeline::AlphaBlend::None;
        ui->uiView->redraw();
    }

    void alpha_blend_straight_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.alphaBlend = timeline::AlphaBlend::Straight;
        ui->uiView->redraw();
    }

    void alpha_blend_premultiplied_cb( Fl_Menu_*, ViewerUI* ui )
    {
        timeline::ImageOptions& o = ui->uiView->getImageOptions(-1);
        o.alphaBlend = timeline::AlphaBlend::Premultiplied;
        ui->uiView->redraw();
    }

    void start_frame_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->startFrame();
    }

    void end_frame_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->endFrame();
    }

    void next_frame_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->frameNext();
    }

    void previous_frame_cb( Fl_Menu_*, ViewerUI* ui )
    {
        ui->uiView->framePrev();
    }

    void previous_annotation_cb( Fl_Menu_*, ViewerUI* ui )
    {
        const auto& player = ui->uiView->getTimelinePlayer();
        if ( !player ) return;
        otio::RationalTime currentTime = player->currentTime();
        int64_t        currentFrame    = currentTime.to_frames();
        std::vector< int64_t > frames = player->getAnnotationFrames();
        std::sort( frames.begin(), frames.end(), std::greater<int64_t>() );
        const auto& range = player->timeRange();
        const auto& duration = range.end_time_inclusive() -
                               range.start_time();
        for ( auto frame : frames )
        {
            if ( frame < currentFrame )
            {
                otio::RationalTime time( frame, duration.rate() );
                player->seek( time );
                return;
            }
        }
    }

    void next_annotation_cb( Fl_Menu_*, ViewerUI* ui )
    {
        const auto& player = ui->uiView->getTimelinePlayer();
        if ( !player ) return;
        otio::RationalTime currentTime = player->currentTime();
        int64_t        currentFrame = currentTime.to_frames();
        std::vector< int64_t > frames = player->getAnnotationFrames();
        std::sort( frames.begin(), frames.end() );
        const auto& range = player->timeRange();
        const auto& duration = range.end_time_inclusive() -
                               range.start_time();
        for ( auto frame : frames )
        {
            if ( frame > currentFrame )
            {
                otio::RationalTime time( frame, duration.rate() );
                player->seek( time );
                return;
            }
        }
    }

    void set_pen_color_cb( Fl_Button* o, ViewerUI* ui )
    {
        uint8_t r, g, b; Fl_Color c = o->color();
        Fl::get_color(c,r,g,b);
        if (!fl_color_chooser(_("Pick Draw Color"), r,g,b)) return;
        Fl::set_color(c,r,g,b);
        ui->uiPenColor->color( o->color() );
        ui->uiPenColor->redraw();

        SettingsObject* settingsObject = ui->app->settingsObject();
        std_any value;
        value = (int)r;
        settingsObject->setValue( kPenColorR, (int)r );
        value = (int)g;
        settingsObject->setValue( kPenColorG, (int)g );
        value = (int)b;
        settingsObject->setValue( kPenColorB, (int)b );

        if ( annotationsTool ) annotationsTool->redraw();

        auto w = ui->uiView->getMultilineInput();
        if (!w) return;
        w->textcolor( c );
        w->redraw();

    }

    static void image_version_cb( const ViewerUI* ui, const int sum,
                                  const bool first_or_last = false )
    {
        auto player = ui->uiView->getTimelinePlayer();
        if ( !player ) return;

        const auto& annotations = player->getAllAnnotations();
        
        const auto& time  = player->currentTime();
        const auto& model = ui->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        if ( numFiles == 0 ) return;
        
        auto Aindex = model->observeAIndex()->get();
        const auto& media = files->getItem( Aindex );

        const std::string& fileName = media_version( ui, media->path, sum,
                                                     first_or_last );
        if ( fileName.empty() ) return;
        
        
        auto item = std::make_shared<FilesModelItem>();
        item->init = true;
        item->path = file::Path(fileName);
        item->audioPath   = media->audioPath;
        item->inOutRange  = media->inOutRange;
        item->speed       = media->speed;
        item->audioOffset = media->audioOffset;
        item->videoLayer  = media->videoLayer;
        item->loop        = media->loop;
        item->playback    = media->playback;
        item->currentTime = time;
        model->replace(Aindex, item);
        
        player = ui->uiView->getTimelinePlayer();
        if ( !player ) return;

        player->setAllAnnotations( annotations );
        ui->uiView->redrawWindows();

        refresh_tool_grp();
    }

    // Versioning
    void first_image_version_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        image_version_cb( ui, -1, true );
    }
    
    void previous_image_version_cb( Fl_Menu_* w,  ViewerUI* ui )
    {
        image_version_cb( ui, -1, false );
    }
    
    void next_image_version_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        image_version_cb( ui, 1, false );
    }
    
    void last_image_version_cb( Fl_Menu_* w, ViewerUI* ui )
    {
        image_version_cb( ui, 1, true );
    }
}
