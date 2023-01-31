// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvUtil.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvVersioning.h"

#include "mrvWidgets/mrvMainWindow.h"

#include "mrvTools/mrvToolsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrvPreferencesUI.h"

#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl.H>

#include "mrvFl/mrvIO.h"

#ifdef __linux__
#  undef None  // X11 defines None as a macro
#endif

namespace {
    const char* kModule = "menus";
}

//#define OCIO_MENU     1

namespace mrv
{


    float kCrops[] = {
        0.00f, 1.00f, 1.19f, 1.37f, 1.50f, 1.56f, 1.66f, 1.77f, 1.85f, 2.00f,
            2.10f, 2.20f, 2.35f, 2.39f, 4.00f
    };


    void MainWindow::fill_menu( Fl_Menu_* menu )
    {
        Fl_Menu_Item* item = nullptr;
        int mode = 0;
        char buf[256];

        const auto& model = ui->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();

        menu->clear();

        int idx;

        DBG3;
        idx = menu->add( _("File/Open/Movie or Sequence"),
                         kOpenImage.hotkey(),
                         (Fl_Callback*)open_cb, ui );

#if 0

        idx = menu->add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
                         (Fl_Callback*)open_single_cb, ui );
#endif

        idx = menu->add( _("File/Open/With Separate Audio"), kOpenSeparateAudio.hotkey(),
                         (Fl_Callback*)open_separate_audio_cb, ui );

        idx = menu->add( _("File/Open/Directory"), kOpenDirectory.hotkey(),
                         (Fl_Callback*)open_directory_cb, ui );


        mode = 0;
        if ( numFiles == 0 ) mode = FL_MENU_INACTIVE;

        menu->add( _("File/Save/Movie"),
                   kSaveSequence.hotkey(),
                   (Fl_Callback*)save_movie_cb, ui, mode );
#if 0
        menu->add( _("File/Save/OTIO As"), kSaveReel.hotkey(),
                   (Fl_Callback*)save_otio_cb, ui );
        menu->add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                   (Fl_Callback*)save_cb, ui );
        idx += 2;
#endif
        mode = 0;
        if ( numFiles == 0 ) mode = FL_MENU_INACTIVE;

        idx = menu->add( _("File/Close Current"),
                         kCloseCurrent.hotkey(),
                         (Fl_Callback*)close_current_cb, ui, mode );

        idx = menu->add( _("File/Close All"),
                         kCloseAll.hotkey(),
                         (Fl_Callback*)close_all_cb, ui, mode );

        SettingsObject* settings = ui->app->settingsObject();
        const std::vector< std::string >& recentFiles = settings->recentFiles();

        // Add files to Recent menu quoting the / to avoid splitting the menu
        for ( auto file : recentFiles )
        {
            size_t pos = 0;
            while ( ( pos = file.find( '/', pos ) ) != std::string::npos )
            {
                file = file.substr( 0, pos ) + "\\" +
                       file.substr( pos, file.size() );
                pos += 2;
            }
            snprintf( buf, 256, _("File/Recent/%s"), file.c_str() );
            menu->add( buf, 0, (Fl_Callback*)open_recent_cb, ui );
        }


        DBG3;
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        item->flags |= FL_MENU_DIVIDER;
        menu->add( _("File/Quit"), kQuitProgram.hotkey(),
                   (Fl_Callback*)exit_cb, ui );

        idx = menu->add( _("Window/Presentation"), kTogglePresentation.hotkey(),
                         (Fl_Callback*)toggle_presentation_cb, ui,
                         FL_MENU_TOGGLE  );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( ui->uiView->getPresentationMode() ) item->set();
        else item->clear();

        idx = menu->add( _("Window/Full Screen"), kFullScreen.hotkey(),
                         (Fl_Callback*)toggle_fullscreen_cb, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( ui->uiMain->fullscreen_active() ) item->set();
        else item->clear();


        idx = menu->add( _("Window/Float On Top"), kToggleFloatOnTop.hotkey(),
                         (Fl_Callback*)toggle_float_on_top_cb, ui,
                         FL_MENU_TOGGLE | FL_MENU_DIVIDER );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( ui->uiMain->is_on_top() )
            item->set();
        else
            item->clear();

        idx = menu->add( _("Window/Secondary"), kToggleSecondary.hotkey(),
                         (Fl_Callback*)toggle_secondary_cb, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( ui->uiSecondary && ui->uiSecondary->window()->visible() )
            item->set();
        else
            item->clear();

        idx = menu->add( _("Window/Secondary Float On Top"),
                         kToggleSecondaryFloatOnTop.hotkey(),
                         (Fl_Callback*)toggle_secondary_float_on_top_cb, ui,
                         FL_MENU_TOGGLE | FL_MENU_DIVIDER );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( ui->uiSecondary && ui->uiSecondary->window()->is_on_top() )
            item->set();
        else
            item->clear();

        idx = menu->add( _("Panel/One Panel Only"),
                         kToggleOnePanelOnly.hotkey(),
                         (Fl_Callback*)toggle_one_panel_only_cb, ui,
                         FL_MENU_TOGGLE | FL_MENU_DIVIDER );
        item = (Fl_Menu_Item*) &menu->menu()[idx];
        if ( onePanelOnly() )
            item->set();
        else
            item->clear();


        std::string menu_panel_root = _("Panel/");
        std::string menu_window_root = _("Window/");
        const WindowCallback* wc = kWindowCallbacks;
        for ( ; wc->name; ++wc )
        {
            std::string tmp = wc->name;
            std::string menu_root = menu_panel_root;

            unsigned hotkey = 0;
            if ( tmp == _("Files") ) hotkey = kToggleReel.hotkey();
            else if ( tmp == _("Media Information") )
                hotkey = kToggleMediaInfo.hotkey();
            else if ( tmp == _("Color Info") )
                hotkey = kToggleColorInfo.hotkey();
            else if ( tmp == _("Color") )
                hotkey = kToggleColorControls.hotkey();
            else if ( tmp == _("Color Area") )
                hotkey = kToggleColorInfo.hotkey();
            else if ( tmp == _("Compare") )
                hotkey = kToggleCompare.hotkey();
            else if ( tmp == _("Playlist") )
                hotkey = kTogglePlaylist.hotkey();
            else if ( tmp == _("Devices") )
                hotkey = kToggleDevices.hotkey();
            else if ( tmp == _("Settings") )
                hotkey = kToggleSettings.hotkey();
            else if ( tmp == _("Annotations") )
                hotkey = kToggleAnnotation.hotkey();
            else if ( tmp == _("Histogram") )
                hotkey = kToggleHistogram.hotkey();
            else if ( tmp == _("Vectorscope") )
                hotkey = kToggleVectorscope.hotkey();
            else if ( tmp == _("Environment Map") )
                hotkey = kToggleLatLong.hotkey();
            else if ( tmp == _("Waveform") )
                hotkey = kToggleWaveform.hotkey();
            else if ( tmp == _("Hotkeys") )
                hotkey = kToggleHotkeys.hotkey();
            else if ( tmp == _("Logs") )
                hotkey = kToggleLogs.hotkey();
            else if ( tmp == _("Preferences") )
            {
                menu_root = menu_window_root;
                hotkey = kTogglePreferences.hotkey();
            }
            else if ( tmp == _("About") )
            {
                menu_root = menu_window_root;
                hotkey = kToggleAbout.hotkey();
            }
            else
            {
                std::cerr << "Menus: Unknown panel " << tmp << std::endl;
                continue; // Unknown window check
            }

            std::string menu_name = menu_root + tmp;
            int idx = menu->add( menu_name.c_str(), hotkey,
                                 (Fl_Callback*)window_cb, ui,
                                 FL_MENU_TOGGLE );
            item = const_cast<Fl_Menu_Item*>( &menu->menu()[idx] );
            if ( tmp == _("Files") )
            {
                if ( filesTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Color") )
            {
                if ( colorTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Color Area") )
            {
                if ( colorAreaTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Histogram") )
            {
                if ( histogramTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Vectorscope") )
            {
                if ( vectorscopeTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Compare") )
            {
                if ( compareTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Playlist") )
            {
                if ( playlistTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Devices") )
            {
                if ( devicesTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Annotations") )
            {
                if ( annotationsTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Settings") )
            {
                if ( settingsTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Logs") )
            {
                if ( logsTool ) item->set();
                else item->clear();
            }
            else if ( tmp == _("Media Information") )
            {
                if ( imageInfoTool ) item->set();
                else item->clear();
            }
        }


        mode = FL_MENU_TOGGLE;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        idx = menu->add( _("View/Safe Areas"), kSafeAreas.hotkey(),
                         (Fl_Callback*)safe_areas_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiView->getSafeAreas() ) item->set();

#if 0
        if ( hasMedia )
        {


            idx = menu->add( _("View/Display Window"), kDisplayWindow.hotkey(),
                             (Fl_Callback*)display_window_cb, ui,
                             FL_MENU_TOGGLE );
            item = (Fl_Menu_Item*) &(menu->menu()[idx]);
            if ( display_window() ) item->set();

            idx = menu->add( _("View/Data Window"), kDataWindow.hotkey(),
                             (Fl_Callback*)data_window_cb, ui, FL_MENU_TOGGLE );
            item = (Fl_Menu_Item*) &(menu->menu()[idx]);
            if ( data_window() ) item->set();
        }
#endif

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;


        const timeline::DisplayOptions& d =
            ui->uiView->getDisplayOptions(-1);
        const timeline::ImageOptions& o =
            ui->uiView->getImageOptions(-1);
        if ( d.channels == timeline::Channels::Color )
            mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Color Channel"), kColorChannel.hotkey(),
                         (Fl_Callback*)toggle_color_channel_cb, ui,
                         mode );

        mode = FL_MENU_RADIO;
        if ( d.channels == timeline::Channels::Red )
            mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Red Channel"), kRedChannel.hotkey(),
                         (Fl_Callback*)toggle_red_channel_cb, ui,
                         mode );

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        if ( d.channels == timeline::Channels::Green )
            mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Green Channel "), kGreenChannel.hotkey(),
                         (Fl_Callback*)toggle_green_channel_cb, ui,
                         mode );

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        if ( d.channels == timeline::Channels::Blue )
            mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Blue Channel"), kBlueChannel.hotkey(),
                         (Fl_Callback*)toggle_blue_channel_cb, ui,
                         mode );

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        if ( d.channels == timeline::Channels::Alpha )
            mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Alpha Channel"), kAlphaChannel.hotkey(),
                         (Fl_Callback*)toggle_alpha_channel_cb, ui,
                         FL_MENU_DIVIDER | mode );

        mode = FL_MENU_TOGGLE;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        if ( d.mirror.x ) mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Mirror X"),
                         kFlipX.hotkey(), (Fl_Callback*)mirror_x_cb,
                         ui, mode );


        mode = FL_MENU_TOGGLE;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        if ( d.mirror.y ) mode |= FL_MENU_VALUE;
        idx = menu->add( _("Render/Mirror Y"),
                         kFlipY.hotkey(), (Fl_Callback*)mirror_y_cb,
                         ui, FL_MENU_DIVIDER | mode );



        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;


        idx = menu->add( _("Render/Video Levels/From File"),
                         0, (Fl_Callback*)video_levels_from_file_cb,
                         ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.videoLevels == timeline::InputVideoLevels::FromFile )
            item->set();

        idx = menu->add( _("Render/Video Levels/Legal Range"),
                         0, (Fl_Callback*)video_levels_legal_range_cb,
                         ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.videoLevels == timeline::InputVideoLevels::LegalRange )
            item->set();

        idx = menu->add( _("Render/Video Levels/Full Range"),
                         0, (Fl_Callback*)video_levels_full_range_cb,
                         ui, FL_MENU_DIVIDER | mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.videoLevels == timeline::InputVideoLevels::FullRange )
            item->set();

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        idx = menu->add( _("Render/Alpha Blend/None"),
                         0, (Fl_Callback*)alpha_blend_none_cb,
                         ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.alphaBlend == timeline::AlphaBlend::None )
            item->set();

        idx = menu->add( _("Render/Alpha Blend/Straight"),
                         0, (Fl_Callback*)alpha_blend_straight_cb,
                         ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.alphaBlend == timeline::AlphaBlend::Straight )
            item->set();

        idx = menu->add( _("Render/Alpha Blend/Premultiplied"),
                         0, (Fl_Callback*)alpha_blend_premultiplied_cb,
                         ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( o.alphaBlend == timeline::AlphaBlend::Premultiplied )
            item->set();

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        idx = menu->add( _("Render/Minify Filter/Nearest"),
                         0, (Fl_Callback*)minify_nearest_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( d.imageFilters.minify == timeline::ImageFilter::Nearest )
            item->set();

        idx = menu->add( _("Render/Minify Filter/Linear"),
                         0, (Fl_Callback*)minify_linear_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( d.imageFilters.minify == timeline::ImageFilter::Linear )
            item->set();

        idx = menu->add( _("Render/Magnify Filter/Nearest"),
                         0, (Fl_Callback*)magnify_nearest_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( d.imageFilters.magnify == timeline::ImageFilter::Nearest )
            item->set();

        idx = menu->add( _("Render/Magnify Filter/Linear"),
                         kTextureFiltering.hotkey(),
                         (Fl_Callback*)magnify_linear_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( d.imageFilters.magnify == timeline::ImageFilter::Linear )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Menu bar") );
        idx = menu->add( buf, kToggleMenuBar.hotkey(),
                         (Fl_Callback*)toggle_menu_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiMenuBar->visible() )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Top bar") );
        idx = menu->add( buf, kToggleTopBar.hotkey(),
                         (Fl_Callback*)toggle_top_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiTopBar->visible() )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Pixel bar") );
        idx = menu->add( buf, kTogglePixelBar.hotkey(),
                         (Fl_Callback*)toggle_pixel_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiPixelBar->visible() )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Timeline") );
        idx = menu->add( buf, kToggleTimeline.hotkey(),
                         (Fl_Callback*)toggle_bottom_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiBottomBar->visible() )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Status Bar") );
        idx = menu->add( buf, kToggleStatusBar.hotkey(),
                         (Fl_Callback*)toggle_status_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiStatusGroup->visible() )
            item->set();

        snprintf( buf, 256, "%s", _("View/Toggle Action Dock") );
        idx = menu->add( buf, kToggleToolBar.hotkey(),
                         (Fl_Callback*)toggle_action_tool_bar, ui,
                         FL_MENU_TOGGLE );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( ui->uiToolsGroup->visible() )
            item->set();

        timeline::Playback playback = timeline::Playback::Stop;

        auto player = ui->uiView->getTimelinePlayer();
        if ( player ) playback = player->playback();

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        idx = menu->add( _("Playback/Stop"), kStop.hotkey(),
                         (Fl_Callback*)stop_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( playback == timeline::Playback::Stop )
            item->set();

        idx = menu->add( _("Playback/Forwards"), kPlayFwd.hotkey(),
                         (Fl_Callback*)play_forwards_cb, ui,
                         mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( playback == timeline::Playback::Forward )
            item->set();

        idx = menu->add( _("Playback/Backwards"), kPlayBack.hotkey(),
                         (Fl_Callback*)play_backwards_cb, ui,
                         mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( playback == timeline::Playback::Reverse )
            item->set();

        mode = 0;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        menu->add( _("Playback/Toggle Playback"), kPlayDirection.hotkey(),
                   (Fl_Callback*)toggle_playback_cb, ui,
                   FL_MENU_DIVIDER | mode );

        // Set In/Out
        TimelineClass* c = ui->uiTimeWindow;
        mode = FL_MENU_TOGGLE;
        idx = menu->add( _("Playback/Toggle In Point"), kSetInPoint.hotkey(),
                         (Fl_Callback*)playback_set_in_point_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( c->uiStartButton->value() )
            item->set();
        idx = menu->add( _("Playback/Toggle Out Point"), kSetOutPoint.hotkey(),
                         (Fl_Callback*)playback_set_out_point_cb, ui,
                         FL_MENU_DIVIDER | mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( c->uiEndButton->value() )
            item->set();



        // Looping


        timeline::Loop loop = timeline::Loop::Loop;
        if ( player ) loop = player->loop();

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        idx = menu->add( _("Playback/Loop Playback"),
                         kPlaybackLoop.hotkey(),
                         (Fl_Callback*)playback_loop_cb, ui,
                         mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( loop == timeline::Loop::Loop )
            item->set();
        idx = menu->add( _("Playback/Playback Once"),
                         kPlaybackOnce.hotkey(),
                         (Fl_Callback*)playback_once_cb, ui,
                         mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( loop == timeline::Loop::Once )
            item->set();
        idx = menu->add( _("Playback/Playback Ping Pong"),
                         kPlaybackPingPong.hotkey(),
                         (Fl_Callback*)playback_ping_pong_cb, ui,
                         FL_MENU_DIVIDER | mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( loop == timeline::Loop::PingPong )
            item->set();

        mode = 0;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        menu->add( _("Playback/Go to/Start"), 0,
                   (Fl_Callback*)start_frame_cb, ui, mode );
        menu->add( _("Playback/Go to/End"), 0,
                   (Fl_Callback*)end_frame_cb, ui,
                   FL_MENU_DIVIDER | mode );

        menu->add( _("Playback/Go to/Previous Frame"),
                   kFrameStepBack.hotkey(),
                   (Fl_Callback*)previous_frame_cb, ui,
                   mode );
        menu->add( _("Playback/Go to/Next Frame"),
                   kFrameStepFwd.hotkey(),
                   (Fl_Callback*)next_frame_cb, ui,  FL_MENU_DIVIDER | mode );

        if ( player )
        {
            const auto& annotations = player->getAllAnnotations();
            if ( ! annotations.empty() )
            {
                menu->add( _("Playback/Go to/Previous Annotation"),
                           kShapeFrameStepBack.hotkey(),
                           (Fl_Callback*)previous_annotation_cb, ui, mode );
                menu->add( _("Playback/Go to/Next Annotation"),
                           kShapeFrameStepFwd.hotkey(),
                           (Fl_Callback*)next_annotation_cb, ui,
                           FL_MENU_DIVIDER | mode );
                menu->add( _("Playback/Annotation/Clear"),
                           kShapeFrameClear.hotkey(),
                           (Fl_Callback*)annotation_clear_cb, ui );
                menu->add( _("Playback/Annotation/Clear All"),
                           kShapeFrameClearAll.hotkey(),
                           (Fl_Callback*)annotation_clear_all_cb, ui );
            }
        }

        mode = FL_MENU_RADIO;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        const char* tmp;
        size_t num = ui->uiPrefs->uiPrefsCropArea->children();
        for ( size_t i = 0; i < num; ++i )
        {
            tmp = ui->uiPrefs->uiPrefsCropArea->child(i)->label();
            if ( !tmp ) continue;
            snprintf( buf, 256, _("View/Mask/%s"), tmp );
            idx = menu->add( buf, 0, (Fl_Callback*)masking_cb, ui,
                             mode );
            item = (Fl_Menu_Item*) &(menu->menu()[idx]);
            float mask = kCrops[i];
            if ( mrv::is_equal( mask, ui->uiView->getMask() ) )
                item->set();
        }


        mode = FL_MENU_TOGGLE;
        if ( numFiles == 0 ) mode |= FL_MENU_INACTIVE;
        Viewport* view = ui->uiView;

        snprintf( buf, 256, "%s", _("View/Hud/Active") );
        idx = menu->add( buf, kHudToggle.hotkey(),
                         (Fl_Callback*) hud_toggle_cb, ui, mode );
        item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        if ( view->getHudActive() ) item->set();
        else item->clear();

        mode = FL_MENU_TOGGLE;
        if ( !item->checked() || numFiles == 0 ) mode |= FL_MENU_INACTIVE;

        num = ui->uiPrefs->uiPrefsHud->children();
        for ( size_t i = 0; i < num; ++i )
        {
            const char* tmp = ui->uiPrefs->uiPrefsHud->child(i)->label();
            snprintf( buf, 256, _("View/Hud/%s"), tmp );
            idx = menu->add( buf, 0, (Fl_Callback*)hud_cb, ui, mode );
            item = (Fl_Menu_Item*) &(menu->menu()[idx]);
            if ( view->getHudDisplay() & (1 << i) ) item->set();
        }



        const int aIndex = ui->app->filesModel()->observeAIndex()->get();
        std::string fileName;
        if (numFiles > 0 && aIndex >= 0 && aIndex < numFiles)
          {
            const auto& files = ui->app->filesModel()->observeFiles()->get();
            fileName = files[aIndex]->path.get(-1, false);

            const auto& ioInfo = files[aIndex]->ioInfo;
            std::stringstream ss;
            ss.precision(2);
            if (!ioInfo.video.empty())
              {
                {
                  ss << "V:" <<
                    ioInfo.video[0].size.w << "x" <<
                    ioInfo.video[0].size.h << ":" <<
                    std::fixed << ioInfo.video[0].size.getAspect() << " " <<
                    ioInfo.video[0].pixelType;
                }
              }
            if (ioInfo.audio.isValid())
              {
                if ( !ss.str().empty() ) ss << ", ";
                ss << "A: " <<
                  static_cast<size_t>(ioInfo.audio.channelCount) << " " <<
                  ioInfo.audio.dataType << " " <<
                  ioInfo.audio.sampleRate;
              }
            char buf[256];
            snprintf( buf, 256, "mrv2 - %s  %s",
                      fileName.c_str(), ss.str().c_str() );
            ui->uiMain->copy_label( buf );
          }
        else
        {
            ui->uiMain->copy_label( "mrv2" );
        }


        if ( numFiles > 0 )
        {

            const boost::regex& regex = version_regex( ui );
            bool has_version = regex_match( fileName, regex );

            if ( has_version )
            {
                menu->add( _("Image/Version/First"),
                           kFirstVersionImage.hotkey(),
                           (Fl_Callback*)first_image_version_cb, ui);
                menu->add( _("Image/Version/Last"),
                           kLastVersionImage.hotkey(),
                           (Fl_Callback*)last_image_version_cb, ui,
                           FL_MENU_DIVIDER);
                menu->add( _("Image/Version/Previous"),
                           kPreviousVersionImage.hotkey(),
                           (Fl_Callback*)previous_image_version_cb, ui);
                menu->add( _("Image/Version/Next"),
                           kNextVersionImage.hotkey(),
                           (Fl_Callback*)next_image_version_cb, ui);
            }

        }

#if 0


        // size_t num = image->number_of_video_streams();
        // if ( num > 1 )
        // {


        //     for ( unsigned i = 0; i < num; ++i )
        //     {
        //         char buf[256];
        //         snprintf( buf, 256, _("Video/Track #%d - %s"), i,
        //                  image->video_info(i).language.c_str() );

        //         idx = menu->add( buf, 0,
        //                          (Fl_Callback*)change_video_cb, ui,
        //                          FL_MENU_RADIO );
        //         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        //         if ( image->video_stream() == (int) i )
        //             item->set();
        //     }
        // }

        // num = image->number_of_subtitle_streams();

        // if ( dynamic_cast< aviImage* >( image ) != NULL )
        // {
        //     menu->add( _("Subtitle/Load"), 0,
        //                (Fl_Callback*)load_subtitle_cb, ui );
        // }



        // if ( num > 0 )
        // {
        //     idx = menu->add( _("Subtitle/No Subtitle"), 0,
        //                      (Fl_Callback*)change_subtitle_cb, ui,
        //                      FL_MENU_TOGGLE  );
        //     Fl_Menu_Item* item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        //     if ( image->subtitle_stream() == -1 )
        //         item->set();
        //     for ( unsigned i = 0; i < num; ++i )
        //     {
        //         char buf[256];
        //         snprintf( buf, 256, _("Subtitle/Track #%d - %s"), i,
        //                  image->subtitle_info(i).language.c_str() );

        //         idx = menu->add( buf, 0,
        //                          (Fl_Callback*)change_subtitle_cb, ui, FL_MENU_RADIO );
        //         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        //         if ( image->subtitle_stream() == (int)i )
        //             item->set();
        //     }
        // }



        // if ( 1 )
        // {

        //     menu->add( _("Audio/Attach Audio File"), kAttachAudio.hotkey(),
        //                (Fl_Callback*)attach_audio_cb, ui );
        //     menu->add( _("Audio/Edit Audio Frame Offset"),
        //                kEditAudio.hotkey(),
        //                (Fl_Callback*)edit_audio_cb, ui );
        //     menu->add( _("Audio/Detach Audio File"), kDetachAudio.hotkey(),
        //                (Fl_Callback*)detach_audio_cb, ui );
        // }



        // if ( dynamic_cast< Fl_Menu_Button* >( menu ) )
        // {
        //     menu->add( _("Pixel/Copy RGBA Values to Clipboard"),
        //                kCopyRGBAValues.hotkey(),
        //                (Fl_Callback*)copy_pixel_rgba_cb, (void*)ui->uiView);
        // }



#endif



        menu->menu_end();

#ifdef __APPLE__
        Fl_Sys_Menu_Bar* smenubar = dynamic_cast< Fl_Sys_Menu_Bar* >( menu );
        if ( smenubar )
        {
            Fl_Mac_App_Menu::about = _("About mrViewer");
            Fl_Mac_App_Menu::print = "";
            Fl_Mac_App_Menu::hide = _("Hide mrViewer");
            Fl_Mac_App_Menu::hide_others = _("Hide Others");
            Fl_Mac_App_Menu::services = _("Services");
            Fl_Mac_App_Menu::quit = _("Quit mrViewer");

            smenubar->update();
        }
#endif

        menu->redraw();
        DBG3;

    }

} // namespace mrv
