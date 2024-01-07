// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <fstream>
#include <iomanip>

#include <tlCore/StringFormat.h>

#include <FL/filename.H> // for fl_open_uri()

#include "mrvCore/mrvFileManager.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvMultilineInput.h"
#include "mrvWidgets/mrvPanelGroup.h"
#include "mrvWidgets/mrvSecondaryWindow.h"

#include "mrvFl/mrvSaveOptions.h"
#include "mrvFl/mrvVersioning.h"
#include "mrvFl/mrvFileRequester.h"
#include "mrvFl/mrvSave.h"
#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvStereo3DAux.h"
#include "mrvFl/mrvCallbacks.h"

#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLTextEdit.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvMenus.h"

#include "mrvFlmm/Flmm_ColorA_Chooser.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvEdit/mrvEditUtil.h"

#ifdef MRV2_PDF
#    include "mrvPDF/mrvSavePDF.h"
#endif

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvCypher.h"
#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvTimelineItemOptions.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

#include "make_ocio_chooser.h"
#include "mrvSaveImageOptionsUI.h"

#ifdef TLRENDER_FFMPEG
#    include "mrvSaveMovieOptionsUI.h"
#endif

#include "mrvHUDUI.h"
#include "mrvHotkeyUI.h"
#include "mrViewer.h"

#include <FL/Fl.H>

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "cback";
}

namespace mrv
{
    using namespace panel;

    WindowCallback kWindowCallbacks[] = {
        {_("Annotations"), (Fl_Callback*)annotations_panel_cb},
        {_("Background"), (Fl_Callback*)background_panel_cb},
        {_("Color"), (Fl_Callback*)color_panel_cb},
        {_("Color Area"), (Fl_Callback*)color_area_panel_cb},
        {_("Compare"), (Fl_Callback*)compare_panel_cb},
#ifdef TLRENDER_BMD
        {_("Devices"), (Fl_Callback*)devices_panel_cb},
#endif
        {_("Environment Map"), (Fl_Callback*)environment_map_panel_cb},
        {_("Files"), (Fl_Callback*)files_panel_cb},
        {_("Histogram"), (Fl_Callback*)histogram_panel_cb},
        {_("Logs"), (Fl_Callback*)logs_panel_cb},
        {_("Media Information"), (Fl_Callback*)image_info_panel_cb},
#ifdef MRV2_NETWORK
        {_("Network"), (Fl_Callback*)network_panel_cb},
#endif
#ifdef TLRENDER_NDI
        {_("NDI"), (Fl_Callback*)ndi_panel_cb},
#endif
        {_("Playlist"), (Fl_Callback*)playlist_panel_cb},
#ifdef MRV2_PYBIND11
        {_("Python"), (Fl_Callback*)python_panel_cb},
#endif
        {_("Settings"), (Fl_Callback*)settings_panel_cb},
        {_("Stereo 3D"), (Fl_Callback*)stereo3D_panel_cb},
#ifdef TLRENDER_USD
        {_("USD"), (Fl_Callback*)usd_panel_cb},
#endif
        {_("Vectorscope"), (Fl_Callback*)vectorscope_panel_cb},
        {_("Hotkeys"), (Fl_Callback*)nullptr},
        {_("Preferences"), (Fl_Callback*)nullptr},
        {_("About"), (Fl_Callback*)nullptr},
        {nullptr, nullptr}};

    namespace
    {
        void reset_timeline(ViewerUI* ui)
        {
            if (panel::imageInfoPanel)
                panel::imageInfoPanel->setTimelinePlayer(nullptr);
            ui->uiTimeline->setTimelinePlayer(nullptr);
            ui->uiTimeline->redraw();
            otio::RationalTime start = otio::RationalTime(1, 24);
            otio::RationalTime end = otio::RationalTime(50, 24);
            TimelineClass* c = ui->uiTimeWindow;
            c->uiFrame->setTime(start);
            c->uiStartFrame->setTime(start);
            c->uiEndFrame->setTime(end);

            if (panel::annotationsPanel)
            {
                panel::annotationsPanel->notes->value("");
            }
        }

        void clear_timeline_player(ViewerUI* ui, TimelinePlayer* player)
        {
            std::vector<TimelinePlayer*> players;
            ui->uiView->setTimelinePlayers(players);
            if (ui->uiSecondary && ui->uiSecondary->window()->visible())
                ui->uiSecondary->viewport()->setTimelinePlayers(players, false);
            player->setTimeline(nullptr);
        }

        void set_timeline_players(
            const ViewerUI* ui, const std::vector<TimelinePlayer*>& players,
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline)
        {
            players[0]->setTimeline(timeline);
            ui->uiView->setTimelinePlayers(players);
            if (ui->uiSecondary && ui->uiSecondary->window()->visible())
                ui->uiSecondary->viewport()->setTimelinePlayers(players, false);
        }
    } // namespace

    void open_files_cb(const std::vector< std::string >& files, ViewerUI* ui)
    {
        for (const auto& file : files)
        {
            ui->app->open(file);
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void open_cb(Fl_Widget* w, ViewerUI* ui)
    {
        const std::vector<std::string>& files = open_image_file(NULL, true);
        open_files_cb(files, ui);
    }

    void open_recent_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(w->mvalue());
        if (!item || !item->label())
            return;
        std::vector<std::string> files;
        std::string file = item->label();
        files.push_back(file);
        open_files_cb(files, ui);
    }

    void open_separate_audio_cb(Fl_Widget* w, ViewerUI* ui)
    {
        ui->app->openSeparateAudioDialog();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void open_directory_cb(Fl_Widget* w, ViewerUI* ui)
    {
        std::string dir = open_directory(NULL);
        if (dir.empty())
            return;

        if (!file::isDirectory(dir))
            return;

        std::vector<std::string> movies, sequences, audios;
        parse_directory(dir, movies, sequences, audios);

        for (const auto& movie : movies)
        {
            ui->app->open(movie);
        }
        for (const auto& sequence : sequences)
        {
            ui->app->open(sequence);
        }
        for (const auto& audio : audios)
        {
            ui->app->open(audio);
        }

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void previous_file_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        model->prev();
    }

    void next_file_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        model->next();
    }

    static std::string lastSavedFile;
    static mrv::SaveOptions lastSavedOptions;

    void save_single_frame_renaming(Fl_Menu_* w, ViewerUI* ui, bool rename)
    {
        const std::string& file = save_single_image();
        if (file.empty())
            return;

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        std::string extension = tl::file::Path(file).getExtension();
        extension = string::toLower(extension);
        if (extension.empty())
        {
            LOG_ERROR(_("File extension cannot be empty."));
            return;
        }

        bool valid_for_exr = false;
        // Sanity check - make sure the video pixel for the current
        // layerId type is float/half
        if (extension == ".exr")
        {
            auto info = player->ioInfo();
            unsigned layerId = ui->uiColorChannel->value();
            auto video = info.video[layerId];
            if (video.pixelType == image::PixelType::RGBA_F16 ||
                video.pixelType == image::PixelType::RGBA_F32 ||
                video.pixelType == image::PixelType::RGB_F16 ||
                video.pixelType == image::PixelType::RGB_F32 ||
                video.pixelType == image::PixelType::LA_F16 ||
                video.pixelType == image::PixelType::LA_F32 ||
                video.pixelType == image::PixelType::L_F16 ||
                video.pixelType == image::PixelType::L_F32)
            {
                valid_for_exr = true;
            }
        }

        SaveImageOptionsUI saveOptions(extension, valid_for_exr);
        if (saveOptions.cancel)
            return;

        mrv::SaveOptions options;
        options.annotations =
            static_cast<bool>(saveOptions.Annotations->value());

        int value;

#ifdef TLRENDER_EXR
        value = saveOptions.PixelType->value();
        if (value == 0)
            options.exrPixelType = tl::image::PixelType::RGBA_F16;
        if (value == 1)
            options.exrPixelType = tl::image::PixelType::RGBA_F32;
#endif

#ifdef TLRENDER_EXR
        value = saveOptions.Compression->value();
        options.exrCompression = static_cast<tl::exr::Compression>(value);
        options.zipCompressionLevel =
            static_cast<int>(saveOptions.ZipCompressionLevel->value());
        options.dwaCompressionLevel = saveOptions.DWACompressionLevel->value();
#endif

        options.noRename = !rename;

        if (save_single_frame(file, ui, options) == 0)
        {
            lastSavedFile = file;
            lastSavedOptions = options;
            lastSavedOptions.noRename = true;
        }
    }

    void save_single_frame_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        save_single_frame_renaming(w, ui, true);
    }

    void save_single_frame_to_folder_cb(Fl_Menu_* w, ViewerUI* ui)
    {

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (lastSavedFile.empty())
            return save_single_frame_renaming(w, ui, false);

        file::Path path(lastSavedFile);
        auto currentTime = player->currentTime();
        std::string file = path.get(currentTime.value());

        save_single_frame(file, ui, lastSavedOptions);
    }

    void save_movie_cb(Fl_Menu_* w, ViewerUI* ui)
    {

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& ioInfo = player->ioInfo();
        bool audioOnly = ioInfo.video.empty();

        std::string file;
        if (audioOnly)
            file = save_audio_file();
        else
            file = save_movie_or_sequence_file();
        if (file.empty())
            return;

        std::string extension = tl::file::Path(file).getExtension();
        extension = string::toLower(extension);
        if (extension.empty())
        {
            LOG_ERROR(_("File extension cannot be empty."));
            return;
        }

        if (extension == ".otio")
        {
            save_timeline_to_disk(file);
            return;
        }

        mrv::SaveOptions options;

#ifdef TLRENDER_FFMPEG
        if (file::isMovie(extension) || file::isAudio(extension))
        {
            bool hasAudio = false;
            if (ioInfo.audio.isValid())
                hasAudio = true;

            bool hasVideo = !ioInfo.video.empty();

            if (!hasAudio && file::isAudio(extension))
            {
                LOG_ERROR(
                    _("Saving audio but current clip does not have audio."));
                return;
            }

            if (hasVideo && file::isAudio(extension))
            {
                LOG_ERROR(_("Saving video but with an audio extension."));
                return;
            }

            SaveMovieOptionsUI saveOptions(hasAudio, audioOnly);
            if (saveOptions.cancel)
                return;

            options.annotations =
                static_cast<bool>(saveOptions.Annotations->value());

            int value;
            value = saveOptions.Profile->value();
            options.ffmpegProfile = static_cast<tl::ffmpeg::Profile>(value);

            value = saveOptions.AudioCodec->value();
            options.ffmpegAudioCodec =
                static_cast<tl::ffmpeg::AudioCodec>(value);
        }
        else
#endif
        {
            bool valid_for_exr = false;
            // Sanity check - make sure the video pixel for the current
            // layerId type is float/half
            if (extension == ".exr")
            {
                unsigned layerId = ui->uiColorChannel->value();
                auto video = ioInfo.video[layerId];
                if (video.pixelType == image::PixelType::RGBA_F16 ||
                    video.pixelType == image::PixelType::RGBA_F32 ||
                    video.pixelType == image::PixelType::RGB_F16 ||
                    video.pixelType == image::PixelType::RGB_F32 ||
                    video.pixelType == image::PixelType::LA_F16 ||
                    video.pixelType == image::PixelType::LA_F32 ||
                    video.pixelType == image::PixelType::L_F16 ||
                    video.pixelType == image::PixelType::L_F32)
                {
                    valid_for_exr = true;
                }
            }

            SaveImageOptionsUI saveOptions(extension, valid_for_exr);
            if (saveOptions.cancel)
                return;

            options.annotations =
                static_cast<bool>(saveOptions.Annotations->value());

            int value;

#ifdef TLRENDER_EXR
            value = saveOptions.PixelType->value();
            if (value == 0)
                options.exrPixelType = tl::image::PixelType::RGBA_F16;
            if (value == 1)
                options.exrPixelType = tl::image::PixelType::RGBA_F32;

            value = saveOptions.Compression->value();
            options.exrCompression = static_cast<tl::exr::Compression>(value);

            options.zipCompressionLevel =
                static_cast<int>(saveOptions.ZipCompressionLevel->value());
            options.dwaCompressionLevel =
                saveOptions.DWACompressionLevel->value();
#endif
        }

        save_movie(file, ui, options);
    }

    void save_pdf_cb(Fl_Menu_* w, ViewerUI* ui)
    {
#ifdef MRV2_PDF
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& annotations = player->getAllAnnotations();
        if (annotations.empty())
            return;

        const std::string& file = save_pdf();
        if (file.empty())
            return;

        save_pdf(file, ui);
#endif
    }

    void close_current_cb(Fl_Widget* w, ViewerUI* ui)
    {
        // Must come before model->close().
        if (ui->uiPrefs->SendMedia->value())
            tcp->pushMessage("closeCurrent", 0);

        auto model = ui->app->filesModel();
        model->close();

        ui->uiMain->fill_menu(ui->uiMenuBar);

        auto images = model->observeFiles()->get();
        if (images.empty())
            reset_timeline(ui);
    }

    void close_all_cb(Fl_Widget* w, ViewerUI* ui)
    {
        if (ui->uiPrefs->SendMedia->value())
            tcp->pushMessage("closeAll", 0);

        auto model = ui->app->filesModel();
        model->closeAll();

        ui->uiMain->fill_menu(ui->uiMenuBar);

        reset_timeline(ui);
    }

    void exit_cb(Fl_Widget* w, ViewerUI* ui)
    {

        tcp->lock();

        ui->uiView->stop();

        // Store window preferences
        if (panel::colorPanel)
            panel::colorPanel->save();
        if (panel::filesPanel)
            panel::filesPanel->save();
        if (panel::colorAreaPanel)
            panel::colorAreaPanel->save();
        if (panel::comparePanel)
            panel::comparePanel->save();
        if (panel::playlistPanel)
            panel::playlistPanel->save();
        if (panel::settingsPanel)
            panel::settingsPanel->save();
        if (panel::logsPanel)
            panel::logsPanel->save();
        if (panel::devicesPanel)
            panel::devicesPanel->save();
        if (panel::annotationsPanel)
            panel::annotationsPanel->save();
        if (panel::imageInfoPanel)
            panel::imageInfoPanel->save();
        if (panel::histogramPanel)
            panel::histogramPanel->save();
        if (panel::vectorscopePanel)
            panel::vectorscopePanel->save();
        if (panel::environmentMapPanel)
            panel::environmentMapPanel->save();
#ifdef MRV2_PYBIND11
        if (panel::pythonPanel)
            panel::pythonPanel->save();
#endif
#ifdef MRV2_NETWORK
        if (panel::networkPanel)
            panel::networkPanel->save();
#endif
#ifdef TLRENDER_USD
        if (panel::usdPanel)
            panel::usdPanel->save();
#endif
#ifdef TLRENDER_NDI
        if (panel::ndiPanel)
            panel::ndiPanel->save();
#endif
        if (panel::stereo3DPanel)
            panel::stereo3DPanel->save();

        if (panel::backgroundPanel)
            panel::backgroundPanel->save();

        if (ui->uiSecondary)
            ui->uiSecondary->save();

        // Save preferences
        Preferences::save();

        // Delete all windows which will close all threads.
        delete ui->uiSecondary;
        ui->uiSecondary = nullptr;
        delete ui->uiAbout;
        ui->uiAbout = nullptr;
        delete ui->uiHotkey;
        ui->uiHotkey = nullptr;

        // Hide all PanelGroup windows
        PanelGroup::hide_all();

        // Delete all panels with images or threads
        delete panel::stereo3DPanel;
        panel::stereo3DPanel = nullptr;
        delete panel::filesPanel;
        panel::filesPanel = nullptr;
        delete panel::comparePanel;
        panel::comparePanel = nullptr;
        delete panel::playlistPanel;
        panel::playlistPanel = nullptr;
#ifdef MRV2_NETWORK
        delete panel::networkPanel;
        panel::networkPanel = nullptr;
#endif

        // Close all files
        close_all_cb(w, ui);

        // Hide any GL Window (needed in Windows)
        Fl_Window* pw = Fl::first_window();
        while (pw)
        {
            pw->hide();
            pw = Fl::first_window();
        }

        // Delete Color Chooser
        delete colorChooser;

        // Remove any temporary EDLS in tmppath
        if (ui->uiPrefs->uiPrefsRemoveEDLs->value())
            removeTemporaryEDLs(ui);

        tcp->unlock();
    }

    void minify_nearest_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.minify = timeline::ImageFilter::Nearest;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    void minify_linear_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.minify = timeline::ImageFilter::Linear;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    void magnify_nearest_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.magnify = timeline::ImageFilter::Nearest;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    void magnify_linear_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.magnify = timeline::ImageFilter::Linear;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    void mirror_x_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.mirror.x ^= 1;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    void mirror_y_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.mirror.y ^= 1;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redraw();
    }

    static void toggle_channel(ViewerUI* ui, const timeline::Channels channel)
    {
        App* app = ui->app;
        timeline::DisplayOptions o = app->displayOptions();
        if (o.channels == channel)
        {
            o.channels = timeline::Channels::Color;
        }
        else
        {
            o.channels = channel;
        }
        app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_red_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Red;
        toggle_channel(ui, channel);
    }

    void toggle_green_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Green;
        toggle_channel(ui, channel);
    }

    void toggle_blue_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Blue;
        toggle_channel(ui, channel);
    }

    void toggle_alpha_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Alpha;
        toggle_channel(ui, channel);
    }

    void toggle_color_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Color;
        toggle_channel(ui, channel);
    }

    void toggle_fullscreen_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool active = true;
        const Fl_Menu_Item* item = m->mvalue();
        if (!item->checked())
            active = false;
        ui->uiView->setFullScreenMode(active);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Fullscreen", active);
    }

    void toggle_presentation_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Viewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        view->setPresentationMode(!presentation);

        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Presentation", !presentation);

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_float_on_top_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool active = true;
        const Fl_Menu_Item* item = m->mvalue();
        if (!item->checked())
            active = false;
        ui->uiMain->always_on_top(active);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_secondary_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        MainWindow* window;
        Viewport* view;

        if (ui->uiSecondary)
        {
            window = ui->uiSecondary->window();
        }
        else
        {
            ui->uiSecondary = new SecondaryWindow(ui);
            window = ui->uiSecondary->window();
        }

        view = ui->uiSecondary->viewport();
        if (window->visible())
        {
            window->hide();
            ui->uiView->take_focus();
        }
        else
        {
            App* app = ui->app;
            view->setContext(app->getContext());
            view->setOCIOOptions(ui->uiView->getOCIOOptions());
            view->setLUTOptions(app->lutOptions());
            std::vector< timeline::ImageOptions > imageOptions;
            std::vector< timeline::DisplayOptions > displayOptions;
            const auto& players = ui->uiView->getTimelinePlayers();
            for (const auto& p : players)
            {
                imageOptions.push_back(app->imageOptions());
                displayOptions.push_back(app->displayOptions());
            }
            view->setImageOptions(imageOptions);
            view->setDisplayOptions(displayOptions);
            auto model = app->filesModel();
            view->setCompareOptions(model->observeCompareOptions()->get());
            view->setTimelinePlayers(players, false);
            window->show();

            bool value = ui->uiPrefs->uiPrefsSecondaryOnTop->value();
            window->always_on_top(value);

            view->frameView();
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_secondary_float_on_top_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        if (!ui->uiSecondary || !ui->uiSecondary->window()->visible())
        {
            item->clear();
            return;
        }

        bool active = true;
        if (!item->checked())
            active = false;
        ui->uiSecondary->window()->always_on_top(active);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_one_panel_only_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        panel::onlyOne(!panel::onlyOne());
    }

    void show_window_cb(const std::string& label, ViewerUI* ui)
    {
        Fl_Window* w = nullptr;

        const WindowCallback* wc = kWindowCallbacks;
        for (; wc->name; ++wc)
        {
            if (label == wc->name || label == _(wc->name))
            {
                if (wc->callback)
                {
                    wc->callback(nullptr, ui);
                    return;
                }
            }
        }

        if (label == _("Preferences"))
            w = ui->uiPrefs->uiMain;
        else if (label == _("Hotkeys"))
            w = ui->uiHotkey->uiMain;
        else if (label == _("About"))
            w = ui->uiAbout->uiMain;
        else
        {
            LOG_ERROR("Callbacks: Unknown window " << label);
            return; // Unknown window
        }

        if (!w || w->visible())
            return;

        w->show();
        w->callback(
            [](Fl_Widget* o, void* data)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(data);
                o->hide();
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    void window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        std::string label_with_tab = item->text;
        auto label = label_with_tab;
        if (label_with_tab[label_with_tab.size() - 1] == '\t')
            label = label_with_tab.substr(0, label_with_tab.size() - 1);
        show_window_cb(label, ui);
    }

    void about_cb(Fl_Widget* w, ViewerUI* ui)
    {
        show_window_cb(_("About"), ui);
    }

    bool has_tools_grp = true, has_menu_bar = true, has_top_bar = true,
         has_bottom_bar = true, has_pixel_bar = true, has_status_bar = true,
         has_dock_grp = false, has_preferences_window = false,
         has_hotkeys_window = false, has_about_window = false;

    void save_ui_state(ViewerUI* ui, Fl_Group* bar)
    {
        if (bar == ui->uiMenuGroup)
            has_menu_bar = ui->uiMenuGroup->visible();
        else if (bar == ui->uiTopBar)
            has_top_bar = ui->uiTopBar->visible();
        else if (bar == ui->uiBottomBar)
            has_bottom_bar = ui->uiBottomBar->visible();
        else if (bar == ui->uiPixelBar)
            has_pixel_bar = ui->uiPixelBar->visible();
        else if (bar == ui->uiStatusGroup)
            has_status_bar = ui->uiStatusGroup->visible();
        else if (bar == ui->uiToolsGroup)
            has_tools_grp = ui->uiToolsGroup->visible();
        else if (bar == ui->uiDockGroup)
            has_dock_grp = ui->uiDockGroup->visible();
    }

    void save_ui_state(ViewerUI* ui)
    {
        has_menu_bar = ui->uiMenuGroup->visible();
        has_top_bar = ui->uiTopBar->visible();
        has_bottom_bar = ui->uiBottomBar->visible();
        has_pixel_bar = ui->uiPixelBar->visible();
        has_status_bar = ui->uiStatusGroup->visible();
        has_tools_grp = ui->uiToolsGroup->visible();
        has_dock_grp = ui->uiDockGroup->visible();

        save_edit_mode_state(ui);

        has_preferences_window = ui->uiPrefs->uiMain->visible();
        has_hotkeys_window = ui->uiHotkey->uiMain->visible();
        has_about_window = ui->uiAbout->uiMain->visible();
    }

    void hide_ui_state(ViewerUI* ui)
    {
        int W = ui->uiMain->w();
        int H = ui->uiMain->h();

        if (has_tools_grp)
        {
            ui->uiToolsGroup->hide();
        }

        if (has_bottom_bar)
        {
            ui->uiBottomBar->hide();
            set_edit_mode_cb(EditMode::kNone, ui);
        }
        if (has_pixel_bar)
        {
            ui->uiPixelBar->hide();
        }
        if (has_top_bar)
        {
            ui->uiTopBar->hide();
        }
        if (has_menu_bar)
        {
            ui->uiMenuGroup->hide();
        }
        if (has_status_bar)
        {
            ui->uiStatusGroup->hide();
        }
        if (has_dock_grp)
        {
            ui->uiDockGroup->hide();
        }

        if (has_preferences_window)
            ui->uiPrefs->uiMain->hide();
        if (has_hotkeys_window)
            ui->uiHotkey->uiMain->hide();
        if (has_about_window)
            ui->uiAbout->uiMain->hide();

        PanelGroup::hide_all();

        ui->uiRegion->layout();
        ui->uiViewGroup->layout();
    }

    void toggle_action_tool_bar(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Group* bar = ui->uiToolsGroup;

        if (bar->visible())
            bar->hide();
        else
            bar->show();

        save_ui_state(ui, bar);

        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Action Bar", (bool)bar->visible());

        ui->uiViewGroup->layout();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_ui_bar(ViewerUI* ui, Fl_Group* const bar, const int size)
    {
        if (bar->visible())
        {
            bar->hide();
        }
        else
        {
            bar->show();
        }

        ui->uiRegion->layout();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_menu_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiMenuGroup);
        save_ui_state(ui, ui->uiMenuGroup);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Menu Bar", (bool)ui->uiMenuGroup->visible());
    }

    void toggle_top_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiTopBar);
        save_ui_state(ui, ui->uiTopBar);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Top Bar", (bool)ui->uiTopBar->visible());
    }

    void toggle_pixel_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiPixelBar);
        save_ui_state(ui, ui->uiPixelBar);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Pixel Bar", (bool)ui->uiPixelBar->visible());
    }

    void toggle_bottom_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiBottomBar);
        save_ui_state(ui, ui->uiBottomBar);
        if (ui->uiBottomBar->visible())
            set_edit_mode_cb(editMode, ui);
        else
            set_edit_mode_cb(EditMode::kNone, ui);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Bottom Bar", (bool)ui->uiBottomBar->visible());
    }

    void toggle_status_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiStatusGroup);
        save_ui_state(ui, ui->uiStatusGroup);
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Status Bar", (bool)ui->uiStatusGroup->visible());
    }

    void restore_ui_state(ViewerUI* ui)
    {
        if (has_menu_bar)
        {
            if (!ui->uiMenuGroup->visible())
            {
                ui->uiMain->fill_menu(ui->uiMenuBar);
                ui->uiMenuGroup->show();
            }
        }

        if (has_top_bar)
        {
            if (!ui->uiTopBar->visible())
            {
                ui->uiTopBar->show();
            }
        }

        if (has_bottom_bar)
        {
            if (!ui->uiBottomBar->visible())
            {
                ui->uiBottomBar->show();
                set_edit_mode_cb(editMode, ui);
            }
        }

        if (has_pixel_bar)
        {
            if (!ui->uiPixelBar->visible())
            {
                ui->uiPixelBar->show();
            }
        }

        if (has_status_bar)
        {
            if (!ui->uiStatusGroup->visible())
            {
                ui->uiStatusGroup->show();
            }
        }

        if (has_tools_grp)
        {
            if (!ui->uiToolsGroup->visible())
            {
                ui->uiToolsGroup->show();
            }
        }

        if (has_dock_grp)
        {
            if (!ui->uiDockGroup->visible())
            {
                ui->uiDockGroup->show();
            }
        }

        ui->uiRegion->layout();
        ui->uiViewGroup->layout();

        if (has_bottom_bar)
        {
            set_edit_mode_cb(editMode, ui);
        }

        if (has_preferences_window)
            ui->uiPrefs->uiMain->show();
        if (has_hotkeys_window)
            ui->uiHotkey->uiMain->show();
        if (has_about_window)
            ui->uiAbout->uiMain->show();

        PanelGroup::show_all();
    }

    HUDUI* hud = nullptr;

    void hud_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Viewport* view = ui->uiView;
        if (!hud)
        {
            Fl_Group::current(view);
            hud = new HUDUI(view);
        }
    }

    void safe_areas_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        ui->uiView->setSafeAreas(item->checked());
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void data_window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        ui->uiView->setDataWindow(item->checked());
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void display_window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        ui->uiView->setDisplayWindow(item->checked());
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void masking_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        // Find offset of View/Mask submenu
        int offset = w->find_index(_("View/Mask")) + 1;

        int idx = w->value() - offset;
        float mask = kCrops[idx];
        ui->uiView->setMask(mask);
    }

    // Playback callbacks
    void play_forwards_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->playForwards();
    }

    void play_backwards_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->playBackwards();
    }

    void stop_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->stop();
    }

    void toggle_playback_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        ui->uiView->togglePlayback();
    }

    // In/Out point callbacks
    void playback_set_in_point_cb(Fl_Menu_*, ViewerUI* ui)
    {
        TimelineClass* c = ui->uiTimeWindow;
        c->uiStartButton->value(!c->uiStartButton->value());
        c->uiStartButton->do_callback();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void playback_set_out_point_cb(Fl_Menu_*, ViewerUI* ui)
    {
        TimelineClass* c = ui->uiTimeWindow;
        c->uiEndButton->value(!c->uiEndButton->value());
        c->uiEndButton->do_callback();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    static void playback_loop_mode(ViewerUI* ui, timeline::Loop mode)
    {
        TimelineClass* c = ui->uiTimeWindow;
        c->uiLoopMode->value((int)mode);
        c->uiLoopMode->do_callback();
    }

    void playback_loop_cb(Fl_Menu_*, ViewerUI* ui)
    {
        playback_loop_mode(ui, timeline::Loop::Loop);
    }
    void playback_once_cb(Fl_Menu_*, ViewerUI* ui)
    {
        playback_loop_mode(ui, timeline::Loop::Once);
    }
    void playback_ping_pong_cb(Fl_Menu_*, ViewerUI* ui)
    {
        playback_loop_mode(ui, timeline::Loop::PingPong);
    }

    // OCIO callbacks
    void attach_ocio_ics_cb(Fl_Menu_*, ViewerUI* ui)
    {
        mrv::PopupMenu* w = ui->uiICS;
        std::string ret =
            make_ocio_chooser(w->label(), OCIOBrowser::kInputColorSpace);
        if (ret.empty())
            return;

        for (size_t i = 0; i < w->children(); ++i)
        {
            const Fl_Menu_Item* o = w->child(i);
            if (!o || !o->label())
                continue;

            if (ret == o->label())
            {
                w->copy_label(o->label());
                w->value(i);
                w->do_callback();
                ui->uiView->redraw();
                break;
            }
        }
    }

    void attach_ocio_display_cb(Fl_Menu_*, ViewerUI* ui)
    {
        std::string ret =
            make_ocio_chooser(Preferences::OCIO_Display, OCIOBrowser::kDisplay);
        if (ret.empty())
            return;
        Preferences::OCIO_Display = ret;
        ui->uiView->redraw();
    }

    void attach_ocio_view_cb(Fl_Menu_*, ViewerUI* ui)
    {
        std::string ret =
            make_ocio_chooser(Preferences::OCIO_View, OCIOBrowser::kView);
        if (ret.empty())
            return;
        Preferences::OCIO_View = ret;
        Fl_Menu_Button* m = ui->OCIOView;
        for (int i = 0; i < m->size(); ++i)
        {
            const char* lbl = m->menu()[i].label();
            if (!lbl)
                continue;
            if (ret == lbl)
            {
                m->value(i);
                break;
            }
        }
        m->copy_label(_(ret.c_str()));
        m->redraw();
        ui->uiView->redraw();
    }

    void video_levels_from_file_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.videoLevels = timeline::InputVideoLevels::FromFile;
        app->setImageOptions(o);
    }

    void video_levels_legal_range_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.videoLevels = timeline::InputVideoLevels::LegalRange;
        app->setImageOptions(o);
    }

    void video_levels_full_range_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.videoLevels = timeline::InputVideoLevels::FullRange;
        app->setImageOptions(o);
    }

    void alpha_blend_none_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.alphaBlend = timeline::AlphaBlend::None;
        app->setImageOptions(o);
    }

    void alpha_blend_straight_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.alphaBlend = timeline::AlphaBlend::Straight;
        app->setImageOptions(o);
    }

    void alpha_blend_premultiplied_cb(Fl_Menu_*, ViewerUI* ui)
    {
        App* app = ui->app;
        timeline::ImageOptions o = app->imageOptions();
        o.alphaBlend = timeline::AlphaBlend::Premultiplied;
        app->setImageOptions(o);
    }

    void start_frame_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->startFrame();
    }

    void end_frame_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->endFrame();
    }

    void next_frame_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->frameNext();
    }

    void previous_frame_cb(Fl_Menu_*, ViewerUI* ui)
    {
        ui->uiView->framePrev();
    }

    void next_clip_cb(Fl_Menu_*, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto time = player->currentTime();
        const auto timeline = player->getTimeline();
        const auto tracks = timeline->video_tracks();
        const auto track = tracks[0];

        const auto item =
            otio::dynamic_retainer_cast<otio::Item>(track->child_at_time(time));
        if (!item)
            return;

        int index = track->index_of_child(item) + 1;
        if (index >= track->children().size())
            index = 0;
        const auto child = track->children()[index];
        const auto next_item = otio::dynamic_retainer_cast<otio::Item>(child);
        if (!next_item)
            return;
        const auto range = next_item->trimmed_range_in_parent().value();
        const auto next_time = range.start_time();
        player->seek(next_time);
    }

    void previous_clip_cb(Fl_Menu_*, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto time = player->currentTime();
        const auto timeline = player->getTimeline();
        const auto tracks = timeline->video_tracks();
        const auto track = tracks[0];

        const auto item =
            otio::dynamic_retainer_cast<otio::Item>(track->child_at_time(time));
        if (!item)
            return;

        int index = track->index_of_child(item) - 1;
        if (index < 0)
            index = track->children().size() - 1;
        const auto child = track->children()[index];
        const auto prev_item = otio::dynamic_retainer_cast<otio::Item>(child);
        if (!prev_item)
            return;
        const auto range = prev_item->trimmed_range_in_parent().value();
        const auto prev_time = range.start_time();
        player->seek(prev_time);
    }

    void previous_annotation_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto& player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        auto currentTime = player->currentTime();
        std::vector< otime::RationalTime > times = player->getAnnotationTimes();
        std::sort(
            times.begin(), times.end(), std::greater<otime::RationalTime>());
        for (const auto& time : times)
        {
            if (time::floor(time) < time::floor(currentTime))
            {
                player->seek(time);
                return;
            }
        }
    }

    void next_annotation_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto& player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        auto currentTime = player->currentTime();
        std::vector< otime::RationalTime > times = player->getAnnotationTimes();
        std::sort(times.begin(), times.end());
        for (const auto& time : times)
        {
            if (time::floor(time) > time::floor(currentTime))
            {
                player->seek(time);
                return;
            }
        }
    }

    void annotation_clear_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        if (ui->uiPrefs->SendAnnotations->value())
            tcp->pushMessage("Clear Frame Annotations", 0);
        player->clearFrameAnnotation();
        ui->uiTimeline->redraw();
        ui->uiView->redrawWindows();
    }

    void annotation_clear_all_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        if (ui->uiPrefs->SendAnnotations->value())
            tcp->pushMessage("Clear All Annotations", 0);
        player->clearAllAnnotations();
        ui->uiTimeline->redraw();
        ui->uiView->redrawWindows();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_none_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getItemOptions();
        options.thumbnails = false;
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_editable_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        bool editable = (bool)item->checked();
        ui->uiTimeline->setEditable(editable);
        Message msg;
        msg["command"] = "setTimelineEditable";
        msg["value"] = editable;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_edit_associated_clips_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getItemOptions();
        options.editAssociatedClips = item->checked();
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_markers_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getItemOptions();
        options.showMarkers = item->checked();
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_transitions_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getItemOptions();
        options.showTransitions = item->checked();
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_small_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getItemOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 100;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_medium_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getItemOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 200;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_large_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getItemOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 300;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void update_pen_color(
        Fl_Color c, uint8_t r, uint8_t g, uint8_t b, uint8_t a, ViewerUI* ui)
    {
        SettingsObject* settings = ui->app->settings();
        settings->setValue(kPenColorR, (int)r);
        settings->setValue(kPenColorG, (int)g);
        settings->setValue(kPenColorB, (int)b);
        settings->setValue(kPenColorA, (int)a);

        if (annotationsPanel)
            annotationsPanel->redraw();

        auto w = ui->uiView->getMultilineInput();
        if (!w)
            return;
        w->textcolor(c);
        w->redraw();
    }

    image::Color4f from_fltk_color(const Fl_Color& c)
    {
        uint8_t r, g, b;
        Fl::get_color(c, r, g, b);
        image::Color4f color = image::Color4f(r / 255.F, g / 255.F, b / 255.F);
        return color;
    }

    Fl_Color to_fltk_color(const image::Color4f& color)
    {
        Fl_Color c = fl_rgb_color(color.r * 255, color.g * 255, color.b * 255);
        return c;
    }

    image::Color4f get_color_cb(Fl_Color c, ViewerUI* ui)
    {
        uint8_t r, g, b;
        Fl::get_color(c, r, g, b);
        image::Color4f color = image::Color4f(r / 255.F, g / 255.F, b / 255.F);

        if (!fl_color_chooser(_("Pick Color"), r, g, b))
            return color;

        color = image::Color4f(r / 255.F, g / 255.F, b / 255.F);
        return color;
    }

    void set_pen_color_cb(Fl_Button* o, ViewerUI* ui)
    {
        uint8_t r, g, b;
        Fl_Color c = o->color();
        Fl::get_color(c, r, g, b);
        float opacity = ui->uiPenOpacity->value();
        uchar a = 255 * opacity;
        if (!flmm_color_a_chooser(_("Pick Draw Color and Alpha"), r, g, b, a))
            return;
        Fl::set_color(c, r, g, b);
        ui->uiPenColor->color(o->color());
        ui->uiPenOpacity->value(a / 255.0F);
        ui->uiPenColor->redraw();
        ui->uiPenOpacity->redraw();

        update_pen_color(c, r, g, b, a, ui);
    }

    void flip_pen_color_cb(Fl_Button* o, ViewerUI* ui)
    {
        uint8_t r, g, b;
        Fl_Color c = ui->uiOldPenColor->color();
        Fl::get_color(c, r, g, b);
        Fl_Color saved = ui->uiPenColor->color();
        ui->uiPenColor->color(c);
        ui->uiPenColor->redraw();
        ui->uiOldPenColor->color(saved);
        ui->uiOldPenColor->redraw();

        float opacity = ui->uiPenOpacity->value();
        uchar a = 255 * opacity;
        update_pen_color(c, r, g, b, a, ui);

        Fl::get_color(saved, r, g, b);
        SettingsObject* settings = ui->app->settings();
        settings->setValue(kOldPenColorR, (int)r);
        settings->setValue(kOldPenColorG, (int)g);
        settings->setValue(kOldPenColorB, (int)b);
        settings->setValue(kOldPenColorA, (int)a);
    }

    static void image_version_cb(
        const ViewerUI* ui, const int sum, const bool first_or_last = false)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& annotations = player->getAllAnnotations();
        int layer = ui->uiColorChannel->value();

        const auto& time = player->currentTime();
        const auto& model = ui->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        if (numFiles == 0)
            return;

        auto Aindex = model->observeAIndex()->get();
        const auto& media = files->getItem(Aindex);

        const std::string& fileName =
            media_version(ui, media->path, sum, first_or_last);
        if (fileName.empty())
            return;

        auto item = std::make_shared<FilesModelItem>();
        item->init = true;
        item->path = file::Path(fileName);
        item->audioPath = media->audioPath;
        item->inOutRange = media->inOutRange;
        item->speed = media->speed;
        item->audioOffset = media->audioOffset;
        item->videoLayer = media->videoLayer;
        item->loop = media->loop;
        item->playback = media->playback;
        item->currentTime = time;
        model->replace(Aindex, item);

        player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        player->setVideoLayer(layer);
        player->setAllAnnotations(annotations);
        ui->uiView->redrawWindows();
    }

    // Versioning
    void first_image_version_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        image_version_cb(ui, -1, true);
    }

    void previous_image_version_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        image_version_cb(ui, -1, false);
    }

    void next_image_version_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        image_version_cb(ui, 1, false);
    }

    void last_image_version_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        image_version_cb(ui, 1, true);
    }

    void help_documentation_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const char* language = getenv("LANGUAGE");
        if (!language || strlen(language) == 0)
            language = "en";

        std::string code = language;
        code = code.substr(0, 2);

        std::string docs =
            "file://" + mrv::rootpath() + "/docs/" + code + "/index.html";
        fl_open_uri(docs.c_str());
    }

    void toggle_annotation_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool value = ui->uiView->getShowAnnotations();
        ui->uiView->setShowAnnotations(!value);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_sync_send_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const Fl_Menu_Item* item = m->mvalue();
        std::string label = item->label();
        if (label == _("Media"))
        {
            ui->uiPrefs->SendMedia->value(item->checked());
        }
        else if (label == _("UI"))
        {
            ui->uiPrefs->SendUI->value(item->checked());
        }
        else if (label == _("Pan And Zoom"))
        {
            ui->uiPrefs->SendPanAndZoom->value(item->checked());
        }
        else if (label == _("Color"))
        {
            ui->uiPrefs->SendColor->value(item->checked());
        }
        else if (label == _("Timeline"))
        {
            ui->uiPrefs->SendTimeline->value(item->checked());
        }
        else if (label == _("Annotations"))
        {
            ui->uiPrefs->SendAnnotations->value(item->checked());
        }
        else if (label == _("Audio"))
        {
            ui->uiPrefs->SendAudio->value(item->checked());
        }
        else
        {
            LOG_ERROR("Unknown Sync/Send item " << label);
        }
    }

    void toggle_sync_receive_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const Fl_Menu_Item* item = m->mvalue();
        std::string label = item->label();
        if (label == _("Media"))
        {
            ui->uiPrefs->ReceiveMedia->value(item->checked());
        }
        else if (label == _("UI"))
        {
            ui->uiPrefs->ReceiveUI->value(item->checked());
        }
        else if (label == _("Pan And Zoom"))
        {
            ui->uiPrefs->ReceivePanAndZoom->value(item->checked());
        }
        else if (label == _("Color"))
        {
            ui->uiPrefs->ReceiveColor->value(item->checked());
        }
        else if (label == _("Timeline"))
        {
            ui->uiPrefs->ReceiveTimeline->value(item->checked());
        }
        else if (label == _("Annotations"))
        {
            ui->uiPrefs->ReceiveAnnotations->value(item->checked());
        }
        else if (label == _("Audio"))
        {
            ui->uiPrefs->ReceiveAudio->value(item->checked());
        }
        else
        {
            LOG_ERROR("Unknown Sync/Receive item " << label);
        }
    }

    static void save_session_impl(const std::string& file, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        auto files = model->observeFiles()->get();

        bool hasEDLs = false;
        for (const auto& file : files)
        {
            const file::Path path = file->path;
            if (isTemporaryEDL(path))
            {
                hasEDLs = true;
                break;
            }
        }

        if (hasEDLs)
        {
            int ok = fl_choice(
                _("You have EDLs in the current session.  These "
                  "will not be saved in the session file.  "
                  "Do you want to continue?"),
                _("No"), _("Yes"), NULL, NULL);
            if (!ok)
                return;
        }

        if (session::save(file))
        {
            auto settings = ui->app->settings();
            settings->addRecentFile(file);
        }

        ui->uiMain->update_title_bar();
    }

    void save_session_as_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const std::string& file = save_session_file();
        if (file.empty())
            return;

        save_session_impl(file, ui);

        session::setCurrent(file);
    }

    void save_session_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const std::string file = session::current();
        if (file.empty())
            return save_session_as_cb(m, ui);

        save_session_impl(file, ui);
    }

    void load_session_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const std::string& file = open_session_file();
        if (file.empty())
            return;

        if (session::load(file))
        {
            auto settings = ui->app->settings();
            settings->addRecentFile(file);
        }

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void clear_note_annotation_cb(ViewerUI* ui)
    {
        Viewport* view = ui->uiView;
        if (!view)
            return;
        auto player = view->getTimelinePlayer();
        if (!player)
            return;
        auto annotation = player->getAnnotation();
        if (!annotation)
            return;
        std::shared_ptr< draw::Shape > s;
        auto shapes = annotation->shapes;
        for (auto it = shapes.begin(); it != shapes.end(); ++it)
        {
            if (dynamic_cast< draw::NoteShape* >((*it).get()))
            {
                it = shapes.erase(it);
                if (ui->uiPrefs->SendAnnotations->value())
                    tcp->pushMessage("Clear Note Annotation", "");

                if (it == shapes.end())
                    player->clearFrameAnnotation();
                break;
            }
        }
    }

    void add_note_annotation_cb(ViewerUI* ui, const std::string& text)
    {
        Viewport* view = ui->uiView;
        if (!view)
            return;
        auto player = view->getTimelinePlayer();
        if (!player)
            return;
        auto annotation = player->getAnnotation();
        if (!annotation)
        {
            annotation = player->createAnnotation(false);
            if (!annotation)
                return;
        }

        std::shared_ptr< draw::Shape > s;
        for (const auto& shape : annotation->shapes)
        {
            if (dynamic_cast< draw::NoteShape* >(shape.get()))
            {
                s = shape;
                break;
            }
        }
        if (!s)
        {
            s = std::make_shared< draw::NoteShape >();
            annotation->push_back(s);
        }
        auto shape = dynamic_cast< draw::NoteShape* >(s.get());
        if (!shape)
            return;
        shape->text = text;

        if (ui->uiPrefs->SendAnnotations->value())
            tcp->pushMessage("Create Note Annotation", text);
    }

    void edit_text_shape_cb(ViewerUI* ui)
    {
        TextEdit window(400, 190, _("Edit Text Annotation"));

        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        if (ui->uiView->children() > 0 ||
            ui->uiSecondary && ui->uiSecondary->viewport()->children() > 0)
            return;

        auto annotation = player->getAnnotation();
        if (!annotation)
            return;

        for (const auto& shape : annotation->shapes)
        {
#ifdef USE_OPENGL2
            if (auto s = dynamic_cast<GL2TextShape*>(shape.get()))
            {
                window.add(s->text);
            }
#endif
            if (auto s = dynamic_cast<GLTextShape*>(shape.get()))
            {
                window.add(s->text);
            }
        }
        window.textAnnotations.menu_end();

        if (window.textAnnotations.size() <= 1)
            return;

        window.set_modal();
        window.show();
        while (window.shown())
        {
            Fl::check();
        }
    }

    void clone_file_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto app = ui->app;
        auto model = app->filesModel();
        if (model->observeFiles()->getSize() < 1)
            return;

        auto item = model->observeA()->get();
        int layer = ui->uiColorChannel->value();

        auto player = ui->uiView->getTimelinePlayer();
        timeline::Playback playback = player->playback();
        auto currentTime = player->currentTime();
        auto inOutRange = player->inOutRange();

        app->open(item->path.get(), item->audioPath.get());

        auto newItem = model->observeA()->get();
        newItem->inOutRange = inOutRange;
        newItem->speed = item->speed;
        newItem->audioOffset = item->audioOffset;
        newItem->loop = item->loop;
        newItem->playback = playback;
        newItem->currentTime = currentTime;
        newItem->annotations = item->annotations;
        ui->uiColorChannel->value(layer);
        ui->uiColorChannel->do_callback();

        player = ui->uiView->getTimelinePlayer();
        player->setAllAnnotations(newItem->annotations);
        player->setPlayback(playback);
        player->seek(currentTime);
        panel::redrawThumbnails();
    }

    void clone_and_replace_cb(Fl_Menu_* m, void* d)
    {
        auto app = App::app;
        auto model = app->filesModel();
        if (model->observeFiles()->getSize() < 1)
            return;

        auto origIndex = model->observeAIndex()->get();

        clone_file_cb(m, d);

        auto newIndex = model->observeAIndex()->get();
        model->setA(origIndex);
        model->close();
        model->setA(newIndex);
    }

    void set_stereo_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto app = ui->app;
        auto model = app->filesModel();
        size_t numFiles = model->observeFiles()->getSize();
        if (numFiles < 1)
            return;

        auto stereoIndex = model->observeStereoIndex()->get();
        if (stereoIndex >= 0)
            return;

        auto Aindex = model->observeAIndex()->get();
        auto Aitem = model->observeA()->get();
        int layerId = Aitem->videoLayer;
        size_t numLayers = ui->uiColorChannel->children();
        if (layerId < 0 || layerId >= numLayers)
            return;

        std::string layer = ui->uiColorChannel->child(layerId)->label();
        std::string matchedLayer = getMatchingLayer(layer, ui);
        if (layer == matchedLayer)
            return;

        size_t i;
        for (i = 0; i < numLayers; ++i)
        {
            layer = ui->uiColorChannel->child(i)->label();
            if (layer == matchedLayer)
                break;
        }

        if (i >= numLayers)
            return;

        clone_file_cb(nullptr, nullptr);

        ui->uiColorChannel->value(i);
        ui->uiColorChannel->do_callback();

        stereoIndex = model->observeAIndex()->get();
        model->setStereo(stereoIndex);

        model->setA(Aindex);

        auto o = model->observeStereo3DOptions()->get();
        o.input = Stereo3DInput::Image;
        model->setStereo3DOptions(o);
    }

    void refresh_file_cache_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        player->clearCache();
    }

    void copy_filename_cb(Fl_Menu_* m, void* d)
    {
        auto app = App::app;
        auto model = app->filesModel();
        if (model->observeFiles()->getSize() < 1)
            return;

        auto item = model->observeA()->get();
        auto path = item->path.get();

        Fl::copy(path.c_str(), path.size(), 1);
    }

    void file_manager_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto app = ui->app;
        auto model = app->filesModel();

        auto item = model->observeA()->get();
        if (!item)
        {
            LOG_ERROR(_("No item selected"));
        }
        auto path = item->path.get();

        file_manager::show_uri(path);
    }

} // namespace mrv
