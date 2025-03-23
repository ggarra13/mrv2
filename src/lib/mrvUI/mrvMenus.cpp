// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <locale>
#include <string> // Add this include for string-related functionality

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvString.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvVersioning.h"

#include "mrvUI/mrvDesktop.h"
#include "mrvUI/mrvMenus.h"

#include "mrvWidgets/mrvMainWindow.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvDummyClient.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvApp.h"

#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"

// The FLTK includes have to come last as they conflict with Windows' includes
#include <FL/fl_utf8.h>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl.H>
#ifdef __APPLE__
#    include <FL/platform.H> // Needed for Fl_Mac_App_Menu
#endif

namespace
{
    const char* kModule = "menus";
}

namespace mrv
{
#ifdef MRV2_PYBIND11
    OrderedMap<std::string, py::handle > pythonMenus;
#endif

    float kCrops[] = {0.00f, 1.00f, 1.19f, 1.37f, 1.50f, 1.56f, 1.66f, 1.77f,
                      1.85f, 2.00f, 2.10f, 2.20f, 2.35f, 2.39f, 4.00f};

    void MainWindow::fill_menu(Fl_Menu_* menu)
    {
        using namespace panel;

        Fl_Menu_Item* item = nullptr;
        int mode = 0;
        char buf[1024];

        ViewerUI* ui = App::ui;
        const auto model = ui->app->filesModel();
        const size_t numFiles = model->observeFiles()->getSize();

        bool isOtio = false;
        std::shared_ptr<FilesModelItem> Aitem;
        if (numFiles > 0)
        {
            Aitem = model->observeA()->get();

            if (Aitem && string::compare(
                             Aitem->path.getExtension(), ".otio",
                             string::Compare::CaseInsensitive))
                isOtio = true;
        }
        menu->clear();

        int idx;

        menu->add(
            _("File/Open/Movie or Sequence"), kOpenImage.hotkey(),
            (Fl_Callback*)open_cb, ui);

        menu->add(
            _("File/Open/With Separate Audio"), kOpenSeparateAudio.hotkey(),
            (Fl_Callback*)open_separate_audio_cb, ui);

        menu->add(
            _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
            (Fl_Callback*)open_single_image_cb, ui);

        menu->add(
            _("File/Open/Directory"), kOpenDirectory.hotkey(),
            (Fl_Callback*)open_directory_cb, ui, FL_MENU_DIVIDER);

        menu->add(
            _("File/Open/Session"), kOpenSession.hotkey(),
            (Fl_Callback*)load_session_cb, ui);
        

        mode = 0;
        if (numFiles == 0)
            mode = FL_MENU_INACTIVE;

        menu->add(
            _("File/Save/Movie or Sequence"), kSaveSequence.hotkey(),
            (Fl_Callback*)save_movie_cb, ui, mode);
        menu->add(
            _("File/Save/Audio"), kSaveAudio.hotkey(),
            (Fl_Callback*)save_audio_cb, ui, mode);
        menu->add(
            _("File/Save/Single Frame"), kSaveImage.hotkey(),
            (Fl_Callback*)save_single_frame_cb, ui, mode | FL_MENU_DIVIDER);
        menu->add(
            _("File/Save/Frames to Folder"), kSaveImageToFolder.hotkey(),
            (Fl_Callback*)save_single_frame_to_folder_cb, ui,
            mode | FL_MENU_DIVIDER);
        menu->add(
            _("File/Save/OTIO EDL Timeline"), kSaveOTIOEDL.hotkey(),
            (Fl_Callback*)save_timeline_to_disk_cb, ui, mode | FL_MENU_DIVIDER);

        auto player = ui->uiView->getTimelinePlayer();
        mode = 0;
        if (!player || !player->hasAnnotations())
            mode = FL_MENU_INACTIVE;

        menu->add(
            _("File/Save/Annotations Only"), kSaveAnnotationsOnly.hotkey(),
            (Fl_Callback*)save_annotations_only_cb, ui, mode);

        menu->add(
            _("File/Save/Annotations as JSON"), kSaveAnnotationsAsJson.hotkey(),
            (Fl_Callback*)save_annotations_as_json_cb, ui,
            mode | FL_MENU_DIVIDER);

        mode = 0;
        if (numFiles == 0)
            mode = FL_MENU_INACTIVE;
        menu->add(
            _("File/Save/PDF Document"), kSavePDF.hotkey(),
            (Fl_Callback*)save_pdf_cb, ui, FL_MENU_DIVIDER | mode);
        menu->add(
            _("File/Save/Session"), kSaveSession.hotkey(),
            (Fl_Callback*)save_session_cb, ui);
        menu->add(
            _("File/Save/Session As"), kSaveSessionAs.hotkey(),
            (Fl_Callback*)save_session_as_cb, ui);

        menu->add(
            _("File/Close Current"), kCloseCurrent.hotkey(),
            (Fl_Callback*)close_current_cb, ui, mode);

        menu->add(
            _("File/Close All"), kCloseAll.hotkey(), (Fl_Callback*)close_all_cb,
            ui, mode);

        // std_any value;
        SettingsObject* settings = ui->app->settings();
        const std::vector< std::string >& recentFiles = settings->recentFiles();

        // Add files to Recent menu quoting the / to avoid splitting the menu
        for (auto file : recentFiles)
        {
            file = string::commentCharacter(file, '\\');
            file = string::commentCharacter(file, '/');
            snprintf(buf, 256, _("File/Recent/%s"), file.c_str());
            menu->add(buf, 0, (Fl_Callback*)open_recent_cb, ui);
        }

        menu->add(
            _("File/Quit"), kQuitProgram.hotkey(), (Fl_Callback*)exit_cb, ui);

        idx = menu->add(
            _("Window/Presentation"), kTogglePresentation.hotkey(),
            (Fl_Callback*)toggle_presentation_cb, ui, FL_MENU_TOGGLE);

        
        const Viewport* uiView = ui->uiView;
        const Viewport* uiView2 = nullptr;
        if (ui->uiSecondary && ui->uiSecondary->window()->visible())
            uiView2 = ui->uiSecondary->viewport();

        item = (Fl_Menu_Item*)&menu->menu()[idx];
        if (uiView->getPresentationMode() ||
            (uiView2 && uiView2->getPresentationMode()))
            item->set();
        else
            item->clear();

        idx = menu->add(
            _("Window/Full Screen"), kFullScreen.hotkey(),
            (Fl_Callback*)toggle_fullscreen_cb, ui, FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&menu->menu()[idx];

        if ((uiView->getFullScreenMode() && !uiView->getPresentationMode()) ||
            (uiView2 && uiView2->getFullScreenMode() &&
             !uiView2->getPresentationMode()))
            item->set();
        else
            item->clear();

        //! @todo: how to support float on top on Wayland
        idx = menu->add(
            _("Window/Float On Top"), kToggleFloatOnTop.hotkey(),
            (Fl_Callback*)toggle_float_on_top_cb, ui,
            FL_MENU_TOGGLE | FL_MENU_DIVIDER);
        item = (Fl_Menu_Item*)&menu->menu()[idx];
        if (ui->uiMain->is_on_top())
            item->set();
        else
            item->clear();

        idx = menu->add(
            _("Window/Secondary"), kToggleSecondary.hotkey(),
            (Fl_Callback*)toggle_secondary_cb, ui, FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&menu->menu()[idx];
        if (ui->uiSecondary && ui->uiSecondary->window()->visible())
            item->set();
        else
            item->clear();

        //! @todo: how to support float on top on Wayland
        idx = menu->add(
            _("Window/Secondary Float On Top"),
            kToggleSecondaryFloatOnTop.hotkey(),
            (Fl_Callback*)toggle_secondary_float_on_top_cb, ui,
            FL_MENU_TOGGLE | FL_MENU_DIVIDER);
        item = (Fl_Menu_Item*)&menu->menu()[idx];
        if (ui->uiSecondary && ui->uiSecondary->window()->is_on_top())
            item->set();
        else
            item->clear();

        idx = menu->add(
            _("Window/Toggle Click Through"), kToggleClickThrough.hotkey(),
            (Fl_Callback*)toggle_click_through, ui);
        idx = menu->add(
            _("Window/More UI Transparency"), kUITransparencyMore.hotkey(),
            (Fl_Callback*)more_ui_transparency, ui);
        idx = menu->add(
            _("Window/Less UI Transparency"), kUITransparencyMore.hotkey(),
            (Fl_Callback*)less_ui_transparency, ui);

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Menu Bar"));
        idx = menu->add(
            buf, kToggleMenuBar.hotkey(), (Fl_Callback*)toggle_menu_bar, ui,
            FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiMenuGroup->visible())
            item->set();

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Top Bar"));
        idx = menu->add(
            buf, kToggleTopBar.hotkey(), (Fl_Callback*)toggle_top_bar, ui,
            FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiTopBar->visible())
            item->set();

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Pixel Bar"));
        idx = menu->add(
            buf, kTogglePixelBar.hotkey(), (Fl_Callback*)toggle_pixel_bar, ui,
            FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiPixelBar->visible())
            item->set();

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Timeline Bar"));
        idx = menu->add(
            buf, kToggleTimeline.hotkey(), (Fl_Callback*)toggle_bottom_bar, ui,
            FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiBottomBar->visible())
            item->set();

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Status Bar"));
        idx = menu->add(
            buf, kToggleStatusBar.hotkey(), (Fl_Callback*)toggle_status_bar, ui,
            FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiStatusGroup->visible())
            item->set();

        snprintf(buf, 256, "%s", _("View/Tool Bars/Toggle Action Dock"));
        idx = menu->add(
            buf, kToggleToolBar.hotkey(), (Fl_Callback*)toggle_action_tool_bar,
            ui, FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiToolsGroup->visible())
            item->set();

        
        mode = FL_MENU_TOGGLE;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;
        idx = menu->add(
            _("View/Auto Frame"), kFrameView.hotkey(),
            (Fl_Callback*)frame_view_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (uiView->hasFrameView())
            item->set();
        

        idx = menu->add(
            _("View/Safe Areas"), kSafeAreas.hotkey(),
            (Fl_Callback*)toggle_safe_areas_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (uiView->getSafeAreas())
            item->set();

        idx = menu->add(
            _("View/OpenEXR/Data Window"), kDataWindow.hotkey(),
            (Fl_Callback*)toggle_data_window_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (uiView->getDataWindow())
            item->set();
        

        idx = menu->add(
            _("View/OpenEXR/Display Window"), kDisplayWindow.hotkey(),
            (Fl_Callback*)toggle_display_window_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (uiView->getDisplayWindow())
            item->set();

        idx = menu->add(
            _("View/OpenEXR/Ignore Display Window"),
            kIgnoreDisplayWindow.hotkey(),
            (Fl_Callback*)toggle_ignore_display_window_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (uiView->getIgnoreDisplayWindow())
            item->set();

        

        idx = menu->add(
            _("Panel/One Panel Only"), kToggleOnePanelOnly.hotkey(),
            (Fl_Callback*)toggle_one_panel_only_cb, ui,
            FL_MENU_TOGGLE | FL_MENU_DIVIDER);
        item = (Fl_Menu_Item*)&menu->menu()[idx];
        if (panel::onlyOne())
            item->set();
        else
            item->clear();

        

        std::unordered_map<std::string, std::string > panelsMap;
        const WindowCallback* wc = kWindowCallbacks;
        for (; wc->name; ++wc)
        {
            panelsMap.insert(std::make_pair(_(wc->name), wc->name));
        }

        // Copy key-value pairs from the map to a vector
        std::vector<std::pair<std::string, std::string>> vec(
            panelsMap.begin(), panelsMap.end());

        
        // Sort the vector in ascending order based on the keys
        std::locale loc;
        std::sort(
            vec.begin(), vec.end(),
            [&loc](const auto& a, const auto& b)
            {
                return std::lexicographical_compare(
                    a.first.begin(), a.first.end(), b.first.begin(),
                    b.first.end(),
                    [&loc](const auto& ch1, const auto& ch2) {
                        return std::tolower(ch1, loc) < std::tolower(ch2, loc);
                    });
            });

        const std::string menu_panel_root = _("Panel/");
        const std::string menu_window_root = _("Window/");

        for (const auto& pair : vec)
        {
            std::string tmp = pair.second;
            std::string menu_root = menu_panel_root;

            mode = FL_MENU_TOGGLE;
            unsigned hotkey = 0;
            if (tmp == "Files")
                hotkey = kToggleReel.hotkey();
            else if (tmp == "Media Information")
                hotkey = kToggleMediaInfo.hotkey();
            else if (tmp == "Color Info")
                hotkey = kToggleColorInfo.hotkey();
            else if (tmp == "Color")
                hotkey = kToggleColorControls.hotkey();
            else if (tmp == "Color Area")
                hotkey = kToggleColorInfo.hotkey();
            else if (tmp == "Compare")
                hotkey = kToggleCompare.hotkey();
            else if (tmp == "Playlist")
                hotkey = kTogglePlaylist.hotkey();
            else if (tmp == "Devices")
                hotkey = kToggleDevices.hotkey();
            else if (tmp == "Settings")
                hotkey = kToggleSettings.hotkey();
            else if (tmp == "Annotations")
                hotkey = kToggleAnnotation.hotkey();
            else if (tmp == "Histogram")
                hotkey = kToggleHistogram.hotkey();
            else if (tmp == "Vectorscope")
                hotkey = kToggleVectorscope.hotkey();
            else if (tmp == "Environment Map")
                hotkey = kToggleEnvironmentMap.hotkey();
            else if (tmp == "Waveform")
                hotkey = kToggleWaveform.hotkey();
            else if (tmp == "NDI")
                hotkey = kToggleNDI.hotkey();
            else if (tmp == "Network")
                hotkey = kToggleNetwork.hotkey();
            else if (tmp == "USD")
                hotkey = kToggleUSD.hotkey();
            else if (tmp == "Stereo 3D")
                hotkey = kToggleStereo3D.hotkey();
            else if (tmp == "Background")
                hotkey = kToggleBackground.hotkey();
            else if (tmp == "Python")
                hotkey = kTogglePythonConsole.hotkey();
            else if (tmp == "Logs")
                hotkey = kToggleLogs.hotkey();
            else if (tmp == "Hotkeys")
            {
                menu_root = menu_window_root;
                hotkey = kToggleHotkeys.hotkey();
                mode = 0;
            }
            else if (tmp == "Preferences")
            {
                menu_root = menu_window_root;
                hotkey = kTogglePreferences.hotkey();
                mode = 0;
            }
            else if (tmp == "About")
            {
                continue;
            }
            else
            {
                std::cerr << "Menus: Unknown panel " << tmp << std::endl;
                continue; // Unknown window check
            }

            
            tmp = pair.first;
            std::string menu_name = menu_root + tmp + "\t";
            int idx = menu->add(
                menu_name.c_str(), hotkey, (Fl_Callback*)window_cb, ui, mode);
            item = const_cast<Fl_Menu_Item*>(&menu->menu()[idx]);
            if (tmp == _("Files"))
            {
                if (filesPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Color"))
            {
                if (colorPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Color Area"))
            {
                if (colorAreaPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Histogram"))
            {
                if (histogramPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Vectorscope"))
            {
                if (vectorscopePanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Compare"))
            {
                if (comparePanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Playlist"))
            {
                if (playlistPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Devices"))
            {
                if (devicesPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Annotations"))
            {
                if (annotationsPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Settings"))
            {
                if (settingsPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Logs"))
            {
                if (logsPanel)
                    item->set();
                else
                    item->clear();
            }
#ifdef MRV2_PYBIND11
            else if (tmp == _("Python"))
            {
                if (pythonPanel)
                    item->set();
                else
                    item->clear();
            }
#endif
            else if (tmp == _("NDI"))
            {
#ifdef TLRENDER_NDI
                if (ndiPanel)
                    item->set();
                else
                    item->clear();
#endif
            }
            else if (tmp == _("Network"))
            {
#ifdef MRV2_NETWORK
                if (networkPanel)
                    item->set();
                else
                    item->clear();
#endif
            }
            else if (tmp == _("USD"))
            {
#ifdef TLRENDER_USD
                if (usdPanel)
                    item->set();
                else
                    item->clear();
#endif
            }
            else if (tmp == _("Stereo 3D"))
            {
                if (stereo3DPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Media Information"))
            {
                if (imageInfoPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Environment Map"))
            {
                if (environmentMapPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (tmp == _("Background"))
            {
                if (backgroundPanel)
                    item->set();
                else
                    item->clear();
            }
            else if (
                tmp == _("Hotkeys") || tmp == _("Preferences") ||
                tmp == _("About"))
            {
                continue;
            }
            else
            {
                LOG_ERROR(_("Unknown menu entry ") << tmp);
            }
        }

        // Make sure to sync panels remotely.
        syncPanels();

        
        {
            const timeline::DisplayOptions& displayOptions =
                ui->app->displayOptions();
            const timeline::ImageOptions& imageOptions =
                ui->app->imageOptions();
            const timeline::BackgroundOptions& backgroundOptions =
                uiView->getBackgroundOptions();

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Color)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Color Channel"), kColorChannel.hotkey(),
                (Fl_Callback*)toggle_color_channel_cb, ui, mode);

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Red)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Red Channel"), kRedChannel.hotkey(),
                (Fl_Callback*)toggle_red_channel_cb, ui, mode);

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Green)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Green Channel "), kGreenChannel.hotkey(),
                (Fl_Callback*)toggle_green_channel_cb, ui, mode);

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Blue)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Blue Channel"), kBlueChannel.hotkey(),
                (Fl_Callback*)toggle_blue_channel_cb, ui, mode);

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Alpha)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Alpha Channel"), kAlphaChannel.hotkey(),
                (Fl_Callback*)toggle_alpha_channel_cb, ui, mode);

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            if (displayOptions.channels == timeline::Channels::Lumma)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Lumma"), kLummaChannel.hotkey(),
                (Fl_Callback*)toggle_lumma_channel_cb, ui,
                FL_MENU_DIVIDER | mode);

            mode = FL_MENU_TOGGLE;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;

            if (displayOptions.mirror.x)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Mirror X"), kFlipX.hotkey(),
                (Fl_Callback*)mirror_x_cb, ui, mode);

            mode = FL_MENU_TOGGLE;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;

            if (displayOptions.mirror.y)
                mode |= FL_MENU_VALUE;
            idx = menu->add(
                _("Render/Mirror Y"), kFlipY.hotkey(),
                (Fl_Callback*)mirror_y_cb, ui, FL_MENU_DIVIDER | mode);

            mode = 0;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            idx = menu->add(
                _("Render/Rotate/-90 Degrees"), kRotateMinus90.hotkey(),
                (Fl_Callback*)rotate_minus_90_cb, ui, mode);

            idx = menu->add(
                _("Render/Rotate/+90 Degrees"), kRotatePlus90.hotkey(),
                (Fl_Callback*)rotate_plus_90_cb, ui, mode);

            mode = FL_MENU_RADIO;

            idx = menu->add(
                _("Render/Video Levels/From File"), kVideoLevelsFile.hotkey(),
                (Fl_Callback*)video_levels_from_file_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.videoLevels ==
                timeline::InputVideoLevels::FromFile)
                item->set();

            idx = menu->add(
                _("Render/Video Levels/Legal Range"),
                kVideoLevelsLegalRange.hotkey(),
                (Fl_Callback*)video_levels_legal_range_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.videoLevels ==
                timeline::InputVideoLevels::LegalRange)
                item->set();

            idx = menu->add(
                _("Render/Video Levels/Full Range"),
                kVideoLevelsFullRange.hotkey(),
                (Fl_Callback*)video_levels_full_range_cb, ui,
                FL_MENU_DIVIDER | mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.videoLevels ==
                timeline::InputVideoLevels::FullRange)
                item->set();

            mode = FL_MENU_RADIO;

            idx = menu->add(
                _("Render/Alpha Blend/None"), kAlphaBlendNone.hotkey(),
                (Fl_Callback*)alpha_blend_none_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.alphaBlend == timeline::AlphaBlend::None)
                item->set();

            idx = menu->add(
                _("Render/Alpha Blend/Straight"), kAlphaBlendStraight.hotkey(),
                (Fl_Callback*)alpha_blend_straight_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.alphaBlend == timeline::AlphaBlend::Straight)
                item->set();

            idx = menu->add(
                _("Render/Alpha Blend/Premultiplied"),
                kAlphaBlendPremultiplied.hotkey(),
                (Fl_Callback*)alpha_blend_premultiplied_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (imageOptions.alphaBlend == timeline::AlphaBlend::Premultiplied)
                item->set();

            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;

            unsigned filtering_linear = 0;
            unsigned filtering_nearest = 0;
            if (displayOptions.imageFilters.minify ==
                timeline::ImageFilter::Nearest)
                filtering_linear = kMinifyTextureFiltering.hotkey();
            else
                filtering_nearest = kMinifyTextureFiltering.hotkey();

            idx = menu->add(
                _("Render/Minify Filter/Nearest"), filtering_nearest,
                (Fl_Callback*)minify_nearest_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.imageFilters.minify ==
                timeline::ImageFilter::Nearest)
                item->set();

            idx = menu->add(
                _("Render/Minify Filter/Linear"), filtering_linear,
                (Fl_Callback*)minify_linear_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.imageFilters.minify ==
                timeline::ImageFilter::Linear)
                item->set();

            filtering_linear = 0;
            filtering_nearest = 0;
            if (displayOptions.imageFilters.magnify ==
                timeline::ImageFilter::Nearest)
                filtering_linear = kMagnifyTextureFiltering.hotkey();
            else
                filtering_nearest = kMagnifyTextureFiltering.hotkey();

            idx = menu->add(
                _("Render/Magnify Filter/Nearest"), filtering_nearest,
                (Fl_Callback*)magnify_nearest_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.imageFilters.magnify ==
                timeline::ImageFilter::Nearest)
                item->set();

            idx = menu->add(
                _("Render/Magnify Filter/Linear"), filtering_linear,
                (Fl_Callback*)magnify_linear_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.imageFilters.magnify ==
                timeline::ImageFilter::Linear)
                item->set();

            idx = menu->add(
                _("Render/HDR/Auto Normalize"), kAutoNormalize.hotkey(),
                (Fl_Callback*)toggle_normalize_image_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.normalize.enabled)
                item->set();

            idx = menu->add(
                _("Render/HDR/Invalid Values"), kInvalidValues.hotkey(),
                (Fl_Callback*)toggle_invalid_values_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.invalidValues)
                item->set();

            idx = menu->add(
                _("Render/HDR/Ignore Chromaticities"),
                kIgnoreChromaticities.hotkey(),
                (Fl_Callback*)toggle_ignore_chromaticities_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (displayOptions.ignoreChromaticities)
                item->set();

            const timeline::HDROptions& hdrOptions = uiView->getHDROptions();
            idx = menu->add(
                _("Render/HDR/Tonemap"), kToggleHDRTonemap.hotkey(),
                (Fl_Callback*)toggle_hdr_tonemap_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (hdrOptions.tonemap)
                item->set();

            int selected = static_cast<int>(hdrOptions.algorithm);
            mode = FL_MENU_RADIO;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            std::string tonemap_root = _("Render/HDR/Tonemap");
            int tonemap = 0;
            for (const auto& algorithm :
                 timeline::getHDRTonemapAlgorithmLabels())
            {
                const std::string entry = tonemap_root + "/" + algorithm;
                idx = menu->add(
                    entry.c_str(), 0, (Fl_Callback*)select_hdr_tonemap_cb, ui,
                    mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (tonemap == (int)selected)
                    item->set();
                ++tonemap;
            }
        }

        timeline::Playback playback = timeline::Playback::Stop;

        mode = FL_MENU_RADIO;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Playback/Stop"), kStop.hotkey(), (Fl_Callback*)stop_cb, ui,
            mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (playback == timeline::Playback::Stop)
            item->set();

        idx = menu->add(
            _("Playback/Forwards"), kPlayFwd.hotkey(),
            (Fl_Callback*)play_forwards_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (playback == timeline::Playback::Forward)
            item->set();

        idx = menu->add(
            _("Playback/Backwards"), kPlayBack.hotkey(),
            (Fl_Callback*)play_backwards_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (playback == timeline::Playback::Reverse)
            item->set();

        mode = 0;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;
        menu->add(
            _("Playback/Toggle Playback"), kPlayDirection.hotkey(),
            (Fl_Callback*)toggle_playback_cb, ui, FL_MENU_DIVIDER | mode);

        // Set In/Out
        TimelineClass* c = ui->uiTimeWindow;

        mode = FL_MENU_TOGGLE;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Playback/Toggle In Point"), kSetInPoint.hotkey(),
            (Fl_Callback*)playback_set_in_point_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (c->uiStartButton->value())
            item->set();
        idx = menu->add(
            _("Playback/Toggle Out Point"), kSetOutPoint.hotkey(),
            (Fl_Callback*)playback_set_out_point_cb, ui,
            FL_MENU_DIVIDER | mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (c->uiEndButton->value())
            item->set();

        if (isOtio)
        {
            menu->add(
                _("Playback/Toggle In\\/Out Otio Clip"),
                kToggleOtioClipInOut.hotkey(),
                (Fl_Callback*)toggle_otio_clip_in_out_cb, ui,
                FL_MENU_DIVIDER | FL_MENU_TOGGLE | mode);
        }

        // Looping
        

        timeline::Loop loop = timeline::Loop::Loop;
        if (player)
            loop = player->loop();

        mode = FL_MENU_RADIO;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Playback/Loop Playback"), kPlaybackLoop.hotkey(),
            (Fl_Callback*)playback_loop_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (loop == timeline::Loop::Loop)
            item->set();
        idx = menu->add(
            _("Playback/Playback Once"), kPlaybackOnce.hotkey(),
            (Fl_Callback*)playback_once_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (loop == timeline::Loop::Once)
            item->set();
        idx = menu->add(
            _("Playback/Playback Ping Pong"), kPlaybackPingPong.hotkey(),
            (Fl_Callback*)playback_ping_pong_cb, ui, FL_MENU_DIVIDER | mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (loop == timeline::Loop::PingPong)
            item->set();

        mode = 0;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        menu->add(
            _("Playback/Go to/Start"), kFirstFrame.hotkey(),
            (Fl_Callback*)start_frame_cb, ui, mode);
        menu->add(
            _("Playback/Go to/End"), kLastFrame.hotkey(),
            (Fl_Callback*)end_frame_cb, ui, FL_MENU_DIVIDER | mode);

        menu->add(
            _("Playback/Go to/Previous Frame"), kFrameStepBack.hotkey(),
            (Fl_Callback*)previous_frame_cb, ui, mode);
        menu->add(
            _("Playback/Go to/Next Frame"), kFrameStepFwd.hotkey(),
            (Fl_Callback*)next_frame_cb, ui, FL_MENU_DIVIDER | mode);

        if (player)
        {
            mode = 0;
            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;

            if (numFiles)
            {
                if (isOtio)
                {
                    menu->add(
                        _("Playback/Go to/Previous Clip"),
                        kPreviousClip.hotkey(), (Fl_Callback*)previous_clip_cb,
                        ui, mode);
                    menu->add(
                        _("Playback/Go to/Next Clip"), kNextClip.hotkey(),
                        (Fl_Callback*)next_clip_cb, ui, FL_MENU_DIVIDER | mode);
                }
            }

            const auto& annotations = player->getAllAnnotations();
            if (!annotations.empty())
            {
                menu->add(
                    _("Playback/Go to/Previous Annotation"),
                    kShapeFrameStepBack.hotkey(),
                    (Fl_Callback*)previous_annotation_cb, ui, mode);
                menu->add(
                    _("Playback/Go to/Next Annotation"),
                    kShapeFrameStepFwd.hotkey(),
                    (Fl_Callback*)next_annotation_cb, ui,
                    FL_MENU_DIVIDER | mode);

                menu->add(
                    _("Playback/Annotation/Clear"), kShapeFrameClear.hotkey(),
                    (Fl_Callback*)annotation_clear_cb, ui);
                menu->add(
                    _("Playback/Annotation/Clear All"),
                    kShapeFrameClearAll.hotkey(),
                    (Fl_Callback*)annotation_clear_all_cb, ui);
            }
        }

        mode = FL_MENU_RADIO;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        const char* tmp;
        size_t num = ui->uiPrefs->uiPrefsCropArea->children();
        for (size_t i = 0; i < num; ++i)
        {
            tmp = ui->uiPrefs->uiPrefsCropArea->child(i)->label();
            if (!tmp)
                continue;
            snprintf(buf, 256, _("View/Mask/%s"), tmp);
            idx = menu->add(buf, 0, (Fl_Callback*)masking_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            float mask = kCrops[i];
            if (mrv::is_equal(mask, uiView->getMask()))
                item->set();
        }

        mode = 0;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        snprintf(buf, 256, "%s", _("View/Hud"));
        idx =
            menu->add(buf, kHudToggle.hotkey(), (Fl_Callback*)hud_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (hudClass)
            item->set();

        snprintf(buf, 256, "%s", _("View/Compare/None"));
        idx = menu->add(
            buf, kCompareNone.hotkey(), (Fl_Callback*)compare_a_cb, ui, mode);

        snprintf(buf, 256, "%s", _("View/Compare/Overlay"));
        idx = menu->add(
            buf, kCompareOverlay.hotkey(), (Fl_Callback*)compare_overlay_cb, ui,
            mode);

        snprintf(buf, 256, "%s", _("View/Compare/Wipe"));
        idx = menu->add(
            buf, kCompareWipe.hotkey(), (Fl_Callback*)compare_wipe_cb, ui,
            mode);

        snprintf(buf, 256, "%s", _("View/Compare/Difference"));
        idx = menu->add(
            buf, kCompareDifference.hotkey(),
            (Fl_Callback*)compare_difference_cb, ui, mode);

        snprintf(buf, 256, "%s", _("View/Compare/Horizontal"));
        idx = menu->add(
            buf, kCompareHorizontal.hotkey(),
            (Fl_Callback*)compare_horizontal_cb, ui, mode);

        snprintf(buf, 256, "%s", _("View/Compare/Vertical"));
        idx = menu->add(
            buf, kCompareVertical.hotkey(), (Fl_Callback*)compare_vertical_cb,
            ui, mode);

        snprintf(buf, 256, "%s", _("View/Compare/Tile"));
        idx = menu->add(
            buf, kCompareTile.hotkey(), (Fl_Callback*)compare_tile_cb, ui,
            mode);

        mode = 0;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        menu->add(
            _("Timeline/Cache/Clear"), kClearCache.hotkey(),
            (Fl_Callback*)refresh_file_cache_cb, ui, mode);

        menu->add(
            _("Timeline/Cache/Update Frame"), kUpdateVideoFrame.hotkey(),
            (Fl_Callback*)update_video_frame_cb, ui, mode);

        mode = FL_MENU_TOGGLE;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Timeline/Editable"), kToggleTimelineEditable.hotkey(),
            (Fl_Callback*)toggle_timeline_editable_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        bool editable = ui->uiTimeline->isEditable();
        if (editable)
            item->set();

        const auto& itemOptions = ui->uiTimeline->getItemOptions();
        const auto& displayOptions = ui->uiTimeline->getDisplayOptions();
        idx = menu->add(
            _("Timeline/Edit Associated Clips"),
            kToggleEditAssociatedClips.hotkey(),
            (Fl_Callback*)toggle_timeline_edit_associated_clips_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (itemOptions.editAssociatedClips)
            item->set();

        mode = 0;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Timeline/Frame View"), kToggleTimelineFrameView.hotkey(),
            (Fl_Callback*)timeline_frame_view_cb, ui, mode);

        mode = FL_MENU_TOGGLE;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        idx = menu->add(
            _("Timeline/Scroll To Current Frame"),
            kToggleTimelineScrollToCurrentFrame.hotkey(),
            (Fl_Callback*)toggle_timeline_scroll_to_current_frame_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        bool boolValue =
            settings->getValue<bool>("Timeline/ScrollToCurrentFrame");
        if (boolValue)
            item->set();

        idx = menu->add(
            _("Timeline/Track Info"), kToggleTimelineTrackInfo.hotkey(),
            (Fl_Callback*)toggle_timeline_track_info_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.trackInfo)
            item->set();

        idx = menu->add(
            _("Timeline/Clip Info"), kToggleTimelineClipInfo.hotkey(),
            (Fl_Callback*)toggle_timeline_clip_info_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.clipInfo)
            item->set();

        
        mode = FL_MENU_RADIO;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;

        int thumbnails_none = 0;
        int thumbnails_small = 0;
        if (displayOptions.thumbnails)
            thumbnails_none = kToggleTimelineThumbnails.hotkey();
        else
            thumbnails_small = kToggleTimelineThumbnails.hotkey();

        idx = menu->add(
            _("Timeline/Thumbnails/None"), thumbnails_none,
            (Fl_Callback*)timeline_thumbnails_none_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (!displayOptions.thumbnails)
            item->set();
        idx = menu->add(
            _("Timeline/Thumbnails/Small"), thumbnails_small,
            (Fl_Callback*)timeline_thumbnails_small_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.thumbnails && displayOptions.thumbnailHeight == 100)
            item->set();
        idx = menu->add(
            _("Timeline/Thumbnails/Medium"), 0,
            (Fl_Callback*)timeline_thumbnails_medium_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.thumbnails && displayOptions.thumbnailHeight == 200)
            item->set();
        idx = menu->add(
            _("Timeline/Thumbnails/Large"), 0,
            (Fl_Callback*)timeline_thumbnails_large_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.thumbnails && displayOptions.thumbnailHeight == 300)
            item->set();

        mode = FL_MENU_TOGGLE;
        if (numFiles == 0)
            mode |= FL_MENU_INACTIVE;
        idx = menu->add(
            _("Timeline/Transitions"), kToggleTimelineTransitions.hotkey(),
            (Fl_Callback*)toggle_timeline_transitions_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.transitions)
            item->set();
        idx = menu->add(
            _("Timeline/Markers"), kToggleTimelineMarkers.hotkey(),
            (Fl_Callback*)toggle_timeline_markers_cb, ui, mode);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (displayOptions.markers)
            item->set();

        if (isOtio)
        {
            std::vector<std::string> tracks;
            std::vector<bool> tracksActive;
            getActiveTracks(tracks, tracksActive, ui);
            unsigned numTracks = tracks.size();
            if (numTracks > 1)
            {
                for (unsigned i = 0; i < tracks.size(); ++i)
                {
                    std::string msg =
                        tl::string::Format(_("Timeline/Visible Tracks/{0}"))
                            .arg(tracks[i]);

                    idx = menu->add(
                        msg.c_str(), 0,
                        (Fl_Callback*)toggle_timeline_active_track_cb, ui,
                        FL_MENU_TOGGLE);
                    item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    if (tracksActive[i])
                        item->set();
                }
            }
        }

        const int aIndex = model->observeAIndex()->get();
        if (numFiles > 0 && aIndex >= 0 && aIndex < numFiles)
        {
            const auto& files = model->observeFiles()->get();
            const auto& path = files[aIndex]->path;
            std::string fileName = path.get(-1);

            if (isOtio)
            {
                const auto& tags = uiView->getTags();
                auto i = tags.find("otioClipName");
                if (i != tags.end())
                {
                    fileName = i->second;
                }
            }

            fileName = mrv::file::normalizePath(fileName);

            const std::regex& regex = version_regex(ui, false);
            bool has_version = regex_match(fileName, regex);

            if (numFiles > 1)
            {
                menu->add(
                    _("Image/Next"), kNextImage.hotkey(),
                    (Fl_Callback*)next_file_cb, ui);
                menu->add(
                    _("Image/Next Limited"), kNextImageLimited.hotkey(),
                    (Fl_Callback*)next_file_limited_cb, ui);
                menu->add(
                    _("Image/Previous"), kPreviousImage.hotkey(),
                    (Fl_Callback*)previous_file_cb, ui);
                menu->add(
                    _("Image/Previous Limited"), kPreviousImageLimited.hotkey(),
                    (Fl_Callback*)previous_file_limited_cb, ui,
                    FL_MENU_DIVIDER);
            }

            if (has_version)
            {
                menu->add(
                    _("Image/Version/First"), kFirstVersionImage.hotkey(),
                    (Fl_Callback*)first_image_version_cb, ui);
                menu->add(
                    _("Image/Version/Last"), kLastVersionImage.hotkey(),
                    (Fl_Callback*)last_image_version_cb, ui, FL_MENU_DIVIDER);
                menu->add(
                    _("Image/Version/Previous"), kPreviousVersionImage.hotkey(),
                    (Fl_Callback*)previous_image_version_cb, ui);
                menu->add(
                    _("Image/Version/Next"), kNextVersionImage.hotkey(),
                    (Fl_Callback*)next_image_version_cb, ui);
            }

            if (numFiles > 1)
            {
                mode = FL_MENU_RADIO;

                auto o = model->observeCompareOptions()->get();
                idx = menu->add(
                    _("Image/Compare Mode/A"), 0, (Fl_Callback*)compare_a_cb,
                    ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::A)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/B"), 0, (Fl_Callback*)compare_b_cb,
                    ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::B)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Wipe"), 0,
                    (Fl_Callback*)compare_wipe_cb, ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Wipe)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Overlay"), 0,
                    (Fl_Callback*)compare_overlay_cb, ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Overlay)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Difference"), 0,
                    (Fl_Callback*)compare_difference_cb, ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Difference)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Horizontal"), 0,
                    (Fl_Callback*)compare_horizontal_cb, ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Horizontal)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Vertical"), 0,
                    (Fl_Callback*)compare_vertical_cb, ui, mode);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Vertical)
                    item->set();

                idx = menu->add(
                    _("Image/Compare Mode/Tile"), 0,
                    (Fl_Callback*)compare_tile_cb, ui, mode | FL_MENU_DIVIDER);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                if (o.mode == timeline::CompareMode::Tile)
                    item->set();

                const auto& Aindex = model->observeAIndex()->get();
                const auto& Bindexes = model->observeBIndexes()->get();

                mode = FL_MENU_TOGGLE;
                for (size_t i = 0; i < numFiles; ++i)
                {

                    const auto& path = files[i]->path;
                    fileName = path.getBaseName() + path.getNumber() +
                               path.getExtension();
                    snprintf(buf, 256, _("Image/Go to/%s"), fileName.c_str());
                    std::uintptr_t ptr = i;
                    idx = menu->add(
                        buf, 0, (Fl_Callback*)goto_file_cb, (void*)ptr, mode);
                    item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    if (i == Aindex)
                        item->set();
                    else
                        item->clear();

                    snprintf(buf, 256, _("Image/Compare/%s"), fileName.c_str());
                    idx = menu->add(
                        buf, 0, (Fl_Callback*)select_Bfile_cb, (void*)ptr,
                        mode);
                    item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    for (size_t j = 0; j < Bindexes.size(); ++j)
                    {
                        if (i == Bindexes[j])
                        {
                            item->set();
                            break;
                        }
                    }
                }
            }

            menu->add(
                _("Edit/Frame/Cut"), kEditCutFrame.hotkey(),
                (Fl_Callback*)edit_cut_frame_cb, ui);
            menu->add(
                _("Edit/Frame/Copy"), kEditCopyFrame.hotkey(),
                (Fl_Callback*)edit_copy_frame_cb, ui);
            menu->add(
                _("Edit/Frame/Paste"), kEditPasteFrame.hotkey(),
                (Fl_Callback*)edit_paste_frame_cb, ui);
            menu->add(
                _("Edit/Frame/Insert"), kEditInsertFrame.hotkey(),
                (Fl_Callback*)edit_insert_frame_cb, ui);

            menu->add(
                _("Edit/Audio Clip/Insert"), kEditInsertAudioClip.hotkey(),
                (Fl_Callback*)insert_audio_clip_cb, ui);
            menu->add(
                _("Edit/Audio Clip/Remove"), kEditRemoveAudioClip.hotkey(),
                (Fl_Callback*)edit_remove_audio_clip_cb, ui);
            menu->add(
                _("Edit/Audio Gap/Insert"), kEditInsertAudioGap.hotkey(),
                (Fl_Callback*)edit_insert_audio_gap_cb, ui);
            menu->add(
                _("Edit/Audio Gap/Remove"), kEditRemoveAudioGap.hotkey(),
                (Fl_Callback*)edit_remove_audio_gap_cb, ui);

            menu->add(
                _("Edit/Slice"), kEditSliceClip.hotkey(),
                (Fl_Callback*)edit_slice_clip_cb, ui);
            menu->add(
                _("Edit/Remove"), kEditRemoveClip.hotkey(),
                (Fl_Callback*)edit_remove_clip_cb, ui);

            menu->add(
                _("Edit/Undo"), kEditUndo.hotkey(), (Fl_Callback*)edit_undo_cb,
                ui);
            menu->add(
                _("Edit/Redo"), kEditRedo.hotkey(), (Fl_Callback*)edit_redo_cb,
                ui);
        }

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
        //         snprintf( buf, 1024, _("Subtitle/Track #%d - %s"), i,
        //                  image->subtitle_info(i).language.c_str() );

        //         idx = menu->add( buf, 0,
        //                          (Fl_Callback*)change_subtitle_cb, ui,
        //                          FL_MENU_RADIO );
        //         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
        //         if ( image->subtitle_stream() == (int)i )
        //             item->set();
        //     }
        // }

        // if ( dynamic_cast< Fl_Menu_Button* >( menu ) )
        // {
        //     menu->add( _("Pixel/Copy RGBA Values to Clipboard"),
        //                kCopyRGBAValues.hotkey(),
        //                (Fl_Callback*)copy_pixel_rgba_cb, (void*)view);
        // }
        

#ifdef TLRENDER_OCIO
        mode = 0;
        snprintf(buf, 256, "%s", _("OCIO/Presets"));
        idx = menu->add(
            buf, kOCIOPresetsToggle.hotkey(), (Fl_Callback*)ocio_presets_cb, ui,
            FL_MENU_DIVIDER);

        idx = menu->add(
            _("OCIO/In Top Bar"), kOCIOInTopBarToggle.hotkey(),
            (Fl_Callback*)toggle_ocio_topbar_cb, ui,
            FL_MENU_DIVIDER | FL_MENU_TOGGLE);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        if (ui->uiOCIO->visible())
            item->set();

        std::string ics = string::commentCharacter(ocio::ics(), '/');
        std::string look = ocio::look();

        menu->add(_("OCIO/Current"), 0, 0, nullptr, FL_MENU_INACTIVE);

        snprintf(buf, 1024, _("OCIO/         ICS: %s"), ics.c_str());
        idx = menu->add(buf, 0, 0, nullptr);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        item->labelcolor(FL_YELLOW);

        int num_screens = Fl::screen_count();
        const timeline::OCIOOptions& o = uiView->getOCIOOptions();
        if (num_screens == 1)
        {
            snprintf(buf, 1024, _("OCIO/     Display: %s"), o.display.c_str());
            menu->add(buf, 0, 0, nullptr);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            item->labelcolor(FL_YELLOW);

            snprintf(buf, 1024, _("OCIO/        View: %s"), o.view.c_str());
            menu->add(buf, 0, 0, nullptr);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            item->labelcolor(FL_YELLOW);
        }
        else
        {
            bool same = true;
            const timeline::OCIOOptions prev = uiView->getOCIOOptions(0);
            int num_screens = Fl::screen_count();
            for (int m = 0; m < num_screens; ++m)
            {
                const timeline::OCIOOptions& o = uiView->getOCIOOptions(m);
                if (o.display != prev.display || o.view != prev.view)
                {
                    same = false;
                    break;
                }
            }

            if (same)
            {
                const timeline::OCIOOptions& o = uiView->getOCIOOptions(0);
                snprintf(
                    buf, 1024, _("OCIO/     Display: %s"), o.display.c_str());
                idx = menu->add(buf, 0, 0, nullptr);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                item->labelcolor(FL_YELLOW);

                snprintf(buf, 1024, _("OCIO/        View: %s"), o.view.c_str());
                idx = menu->add(buf, 0, 0, nullptr);
                item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                item->labelcolor(FL_YELLOW);
            }
            else
            {
                for (int m = 0; m < num_screens; ++m)
                {
                    const timeline::OCIOOptions& o = uiView->getOCIOOptions(m);
                    std::string monitorName = mrv::desktop::monitorName(m);
                    snprintf(
                        buf, 1024, _("OCIO/  Monitor #%d Display: %s"), m,
                        o.display.c_str());
                    idx = menu->add(buf, 0, 0, nullptr);
                    item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    item->labelcolor(FL_YELLOW);

                    snprintf(
                        buf, 1024, _("OCIO/  Monitor #%d    View: %s"), m,
                        o.view.c_str());
                    idx = menu->add(buf, 0, 0, nullptr);
                    item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    item->labelcolor(FL_YELLOW);
                }
            }
        }

        snprintf(buf, 1024, _("OCIO/        Look: %s"), look.c_str());
        idx = menu->add(buf);
        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        item->labelcolor(FL_YELLOW);

        const timeline::LUTOptions& lut = ui->app->lutOptions();
        if (lut.enabled && !lut.fileName.empty())
        {
            file::Path path(lut.fileName);
            snprintf(
                buf, 1024, _("OCIO/        LUT: %s"),
                path.getBaseName().c_str());
            idx = menu->add(buf, 0, 0, nullptr);
        }

        item = (Fl_Menu_Item*)&(menu->menu()[idx]);
        item->labelcolor(FL_YELLOW);
        item->flags |= FL_MENU_DIVIDER;

        menu->add(
            _("OCIO/Change Current File"), 0, 0, nullptr, FL_MENU_INACTIVE);

        idx = ui->uiICS->value();
        if (idx >= 0)
        {
            const Fl_Menu_Item* item = ui->uiICS->child(idx);
            char pathname[1024];
            int ret = ui->uiICS->item_pathname(pathname, 1024, item);
            if (ret == 0)
                ics = pathname;
        }
        for (int i = 0; i < ui->uiICS->children(); ++i)
        {
            std::string colorSpace = _("OCIO/     Input Color Space");
            const Fl_Menu_Item* item = ui->uiICS->child(i);
            if (!item || !item->label() || (item->flags & FL_SUBMENU))
                continue;

            char pathname[1024];
            int ret = ui->uiICS->item_pathname(pathname, 1024, item);
            if (ret != 0)
                continue;

            if (pathname[0] != '/')
                colorSpace += '/';
            colorSpace += pathname;
            idx = menu->add(
                colorSpace.c_str(), 0, (Fl_Callback*)current_ocio_ics_cb, ui,
                FL_MENU_TOGGLE);

            {
                Fl_Menu_Item* item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                size_t pos = colorSpace.find(ics);
                if (pos != std::string::npos &&
                    pos + ics.size() == colorSpace.size())
                {
                    item->set();
                }
            }
        }

        idx = ui->uiOCIOLook->value();
        if (idx >= 0)
        {
            const Fl_Menu_Item* item = ui->uiOCIOLook->child(idx);
            char pathname[1024];
            int ret = ui->uiOCIOLook->item_pathname(pathname, 1024, item);
            if (ret == 0)
                look = pathname;
        }
        for (int i = 0; i < ui->uiOCIOLook->children(); ++i)
        {
            std::string colorSpace = _("OCIO/     Look");
            const Fl_Menu_Item* item = ui->uiOCIOLook->child(i);
            if (!item || !item->label() || (item->flags & FL_SUBMENU))
                continue;

            char pathname[1024];
            int ret = ui->uiOCIOLook->item_pathname(pathname, 1024, item);
            if (ret != 0)
                continue;

            if (pathname[0] != '/')
                colorSpace += '/';
            colorSpace += pathname;
            idx = menu->add(
                colorSpace.c_str(), 0, (Fl_Callback*)current_ocio_look_cb, ui,
                FL_MENU_TOGGLE);

            {
                Fl_Menu_Item* item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                size_t pos = colorSpace.find(look);
                if (pos != std::string::npos &&
                    pos + look.size() == colorSpace.size())
                {
                    item->set();
                }
            }
        }

        menu->add(_("OCIO/Displays"), 0, nullptr, nullptr, FL_MENU_INACTIVE);

        menu->add(_("OCIO/Displays"), 0, nullptr, nullptr, FL_MENU_INACTIVE);

        if (num_screens > 1)
        {
            const timeline::OCIOOptions& o = uiView->getOCIOOptions(0);
            std::string combined = ocio::combineView(o.display, o.view);

            for (int i = 0; i < ui->uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = ui->uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = ui->uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                std::string colorSpace = "OCIO/     All Monitors";

                if (pathname[0] != '/')
                    colorSpace += '/';
                colorSpace += pathname;
                int idx = menu->add(
                    colorSpace.c_str(), 0,
                    (Fl_Callback*)all_monitors_ocio_view_cb, ui,
                    FL_MENU_TOGGLE);
                {
                    Fl_Menu_Item* item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    size_t pos = colorSpace.find(combined);
                    if (pos != std::string::npos &&
                        pos + combined.size() == colorSpace.size())
                    {
                        item->set();
                    }
                }
            }
        }

        for (int m = 0; m < num_screens; ++m)
        {
            const timeline::OCIOOptions& o = uiView->getOCIOOptions(m);
            std::string monitorName = mrv::desktop::monitorName(m);
            std::string combined = ocio::combineView(o.display, o.view);
            combined = monitorName + "/" + combined;

            for (int i = 0; i < ui->uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = ui->uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = ui->uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                std::string colorSpace = "OCIO/     " + monitorName;

                if (pathname[0] != '/')
                    colorSpace += '/';
                colorSpace += pathname;
                int idx = menu->add(
                    colorSpace.c_str(), 0, (Fl_Callback*)monitor_ocio_view_cb,
                    ui, FL_MENU_TOGGLE);
                {
                    Fl_Menu_Item* item = (Fl_Menu_Item*)&(menu->menu()[idx]);
                    size_t pos = colorSpace.find(combined);
                    if (pos != std::string::npos &&
                        pos + combined.size() == colorSpace.size())
                    {
                        item->set();
                    }
                }
            }
        }
#endif

        
        if (dynamic_cast< DummyClient* >(tcp) == nullptr)
        {
            mode = FL_MENU_TOGGLE;

            idx = menu->add(
                _("Sync/Send/Media"), 0, (Fl_Callback*)toggle_sync_send_cb, ui,
                mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendMedia->value())
                item->set();
            else
                item->clear();

            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            idx = menu->add(
                _("Sync/Send/UI"), 0, (Fl_Callback*)toggle_sync_send_cb, ui,
                mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendUI->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Send/Pan And Zoom"), 0,
                (Fl_Callback*)toggle_sync_send_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendPanAndZoom->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Send/Color"), 0, (Fl_Callback*)toggle_sync_send_cb, ui,
                mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendColor->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Send/Timeline"), 0, (Fl_Callback*)toggle_sync_send_cb,
                ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendTimeline->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Send/Annotations"), 0,
                (Fl_Callback*)toggle_sync_send_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendAnnotations->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Send/Audio"), 0, (Fl_Callback*)toggle_sync_send_cb, ui,
                mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->SendAudio->value())
                item->set();
            else
                item->clear();

            /// ACCEPT
            mode = FL_MENU_TOGGLE;

            idx = menu->add(
                _("Sync/Accept/Media"), 0, (Fl_Callback*)toggle_sync_receive_cb,
                ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveMedia->value())
                item->set();
            else
                item->clear();

            if (numFiles == 0)
                mode |= FL_MENU_INACTIVE;
            idx = menu->add(
                _("Sync/Accept/UI"), 0, (Fl_Callback*)toggle_sync_receive_cb,
                ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveUI->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Accept/Pan And Zoom"), 0,
                (Fl_Callback*)toggle_sync_receive_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceivePanAndZoom->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Accept/Color"), 0, (Fl_Callback*)toggle_sync_receive_cb,
                ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveColor->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Accept/Timeline"), 0,
                (Fl_Callback*)toggle_sync_receive_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveTimeline->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Accept/Annotations"), 0,
                (Fl_Callback*)toggle_sync_receive_cb, ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveAnnotations->value())
                item->set();
            else
                item->clear();
            idx = menu->add(
                _("Sync/Accept/Audio"), 0, (Fl_Callback*)toggle_sync_receive_cb,
                ui, mode);
            item = (Fl_Menu_Item*)&(menu->menu()[idx]);
            if (ui->uiPrefs->ReceiveAudio->value())
                item->set();
            else
                item->clear();
        }

#ifdef MRV2_PYBIND11
        for (const auto& entry : pythonMenus)
        {
            int mode = 0;
            const py::handle& obj = pythonMenus.at(entry);
            if (!py::isinstance<py::function>(obj) &&
                !py::isinstance<py::tuple>(obj))
            {
                std::string msg =
                    string::Format(_("In '{0}' expected a function as a value "
                                     "or a tuple containing a Python function "
                                     "and a string with menu options in it."))
                        .arg(entry);
                LOG_ERROR(msg);
                continue;
            }
            if (py::isinstance<py::tuple>(obj))
            {
                // obj is a Python tuple
                py::tuple tup = py::reinterpret_borrow<py::tuple>(obj);
                if (tup.size() == 2 && py::isinstance<py::function>(tup[0]) &&
                    py::isinstance<py::str>(tup[1]))
                {
                    py::str str(tup[1]);
                    const std::string modes = str.cast<std::string>();
                    if (modes.find("__divider__") != std::string::npos)
                    {
                        mode |= FL_MENU_DIVIDER;
                    }
                }
                else
                {
                    std::string msg =
                        string::Format(
                            _("In '{0}' expected a function a tuple "
                              "containing a Python function and a string "
                              "with menu options in it."))
                            .arg(entry);
                    LOG_ERROR(msg);
                    continue;
                }
            }
            idx = menu->add(
                entry.c_str(), 0, (Fl_Callback*)run_python_method_cb,
                (void*)&pythonMenus.at(entry), mode);
        }
#endif

        menu->add(
            _("Help/Documentation"), 0, (Fl_Callback*)help_documentation_cb,
            ui);
        menu->add(
            _("Help/About"), kToggleAbout.hotkey(), (Fl_Callback*)window_cb,
            ui);

        menu->menu_end();

#ifdef __APPLE__
        Fl_Sys_Menu_Bar* smenubar = dynamic_cast< Fl_Sys_Menu_Bar* >(menu);
        if (smenubar)
        {
            Fl_Mac_App_Menu::about = _("About mrv2");
            Fl_Mac_App_Menu::print = "";
            Fl_Mac_App_Menu::hide = _("Hide mrv2");
            Fl_Mac_App_Menu::hide_others = _("Hide Others");
            Fl_Mac_App_Menu::services = _("Services");
            Fl_Mac_App_Menu::quit = _("Quit mrv2");

            Fl_Sys_Menu_Bar::about((Fl_Callback*)about_cb, ui);

            smenubar->update();
        }
        else
        {
            menu->textfont(ui->uiPrefs->uiFontMenus->value());
        }

#else
        menu->textfont(ui->uiPrefs->uiFontMenus->value());
#endif
        

        menu->redraw();
    }

} // namespace mrv
