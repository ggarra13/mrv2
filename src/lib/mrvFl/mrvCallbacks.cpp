// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

#include <tlIO/System.h>

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
#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvSave.h"
#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvStereo3DAux.h"
#include "mrvFl/mrvCallbacks.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvDesktop.h"
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
#include "mrvApp/mrvApp.h"

#include "make_ocio_chooser.h"
#include "mrvSaveImageOptionsUI.h"

#ifdef TLRENDER_FFMPEG
#    include "mrvSaveMovieOptionsUI.h"
#endif

#include "mrvHUDUI.h"
#include "mrvOCIOPresetsUI.h"
#include "mrvHotkeyUI.h"
#include "mrViewer.h"

#include <FL/Fl.H>
#include "mrvFl/mrvIO.h"

#ifdef OPENGL_BACKEND
#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvGLTextEdit.h"
#endif

#ifdef VULKAN_BACKEND
#include "mrvVk/mrvVkShape.h"
#include "mrvVk/mrvVkTextEdit.h"
#endif

namespace
{
    const char* kModule = "cback";
}

namespace mrv
{
    int check_for_changes(ViewerUI* ui)
    {
        if (mrv::App::unsaved_edits || mrv::App::unsaved_annotations)
        {
            
            int choice = fl_choice(
                _("You have unsaved changes. "
                  "Do you want to save the session before closing?"),
                _("Cancel"), _("Save"), _("Don't Save"), NULL, NULL);
            if (choice == 1)
            {
                save_session_cb(nullptr, ui);
                return 1;
            }
            return choice;
        }
        return 1;
    }
    
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

        // void clear_timeline_player(ViewerUI* ui, TimelinePlayer* player)
        // {
        //     ui->uiView->setTimelinePlayer(nullptr);
        //     if (ui->uiSecondary && ui->uiSecondary->window()->visible())
        //         ui->uiSecondary->viewport()->setTimelinePlayer(nullptr);
        //     player->setTimeline(nullptr);
        // }

        // void set_timeline_player(
        //     const ViewerUI* ui, TimelinePlayer* player,
        //     const otio::SerializableObject::Retainer<otio::Timeline>&
        //     timeline)
        // {
        //     player->setTimeline(timeline);
        //     ui->uiView->setTimelinePlayer(player);
        //     if (ui->uiSecondary && ui->uiSecondary->window()->visible())
        //         ui->uiSecondary->viewport()->setTimelinePlayer(player);
        // }
    } // namespace

    void open_files_cb(const std::vector< std::string >& files, ViewerUI* ui)
    {
        for (const auto& file : files)
        {
            ui->app->open(file);
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void
    open_single_files_cb(const std::vector< std::string >& files, ViewerUI* ui)
    {
        auto settings = ui->app->settings();
        int savedDigits = settings->getValue<int>("Misc/MaxFileSequenceDigits");
        settings->setValue("Misc/MaxFileSequenceDigits", 0);

        for (const auto& file : files)
        {
            ui->app->open(file);
        }

        settings->setValue("Misc/MaxFileSequenceDigits", savedDigits);

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void open_file_cb(const std::string& file, ViewerUI* ui)
    {
        ui->app->open(file);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void open_cb(Fl_Widget* w, ViewerUI* ui)
    {
        const std::vector<std::string>& files = open_image_file(NULL, true);
        open_files_cb(files, ui);
    }

    void open_single_image_cb(Fl_Widget* w, ViewerUI* ui)
    {
        const std::vector<std::string>& files = open_image_file(NULL, false);
        open_single_files_cb(files, ui);
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
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void next_file_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        model->next();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void previous_file_limited_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        auto Aindex = model->observeAIndex()->get();
        if (Aindex <= 0)
            return;
        model->prev();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void next_file_limited_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = ui->app->filesModel();
        auto numFiles = model->observeFiles()->getSize();
        auto Aindex = model->observeAIndex()->get();
        if (Aindex + 1 >= numFiles)
            return;
        model->next();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void goto_file_cb(Fl_Widget* w, void* data)
    {
        ViewerUI* ui = App::ui;
        size_t Aindex = (size_t)data;
        auto model = App::app->filesModel();
        auto numFiles = model->observeFiles()->getSize();
        if (Aindex < 0 || Aindex >= numFiles)
            return;
        model->setA(Aindex);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void select_Bfile_cb(Fl_Widget* w, void* data)
    {
        ViewerUI* ui = App::ui;
        size_t Bindex = (size_t)data;
        auto model = App::app->filesModel();
        auto numFiles = model->observeFiles()->getSize();
        if (Bindex < 0 || Bindex >= numFiles)
            return;

        const auto bIndexes = model->observeBIndexes()->get();
        const auto i = std::find(bIndexes.begin(), bIndexes.end(), Bindex);
        model->setB(Bindex, i == bIndexes.end());
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_a_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::A;
        model->setCompareOptions(o);

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_b_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::B;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_wipe_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Wipe;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_overlay_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Overlay;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_difference_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Difference;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_horizontal_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Horizontal;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_vertical_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Vertical;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_tile_cb(Fl_Widget* w, ViewerUI* ui)
    {
        auto model = App::app->filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = timeline::CompareMode::Tile;
        model->setCompareOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    static std::string lastSavedFile;
    static mrv::SaveOptions lastSavedOptions;

    void save_single_frame_renaming(Fl_Menu_* w, ViewerUI* ui, bool rename)
    {
        auto model = ui->app->filesModel();
        auto Afile = model->observeA()->get();
        if (!Afile)
            return;

        const std::string& file = save_single_image(Afile->path.get().c_str());
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
        if (extension == ".exr")
        {
            valid_for_exr = true;
        }

        SaveImageOptionsUI saveOptions(extension, valid_for_exr);
        if (saveOptions.cancel)
            return;

        mrv::SaveOptions options;
        options.annotations =
            static_cast<bool>(saveOptions.Annotations->value());
        options.resolution =
            static_cast<SaveResolution>(saveOptions.Resolution->value());

        int value;

#ifdef TLRENDER_EXR
        value = saveOptions.PixelType->value();
        if (value == 0)
            options.exrPixelType = tl::image::PixelType::RGBA_F16;
        else if (value == 1)
            options.exrPixelType = tl::image::PixelType::RGBA_F32;
        value = saveOptions.Compression->value();
        options.exrCompression = static_cast<Imf::Compression>(value);
        value = saveOptions.Contents->value();
        options.exrSaveContents = static_cast<mrv::SaveContents>(value);
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

    void save_audio_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        const auto& ioInfo = player->ioInfo();
        if (!ioInfo.audio.isValid())
            return;

        std::string file = save_audio_file();
        if (file.empty())
            return;

        std::string extension = tl::file::Path(file).getExtension();
        extension = string::toLower(extension);
        if (!file::isAudio(extension))
        {
            LOG_ERROR(_("Saving audio but not with an audio extension."));
            return;
        }

        mrv::SaveOptions options;
        bool hasAudio = true;
        bool hasVideo = false;
        bool audioOnly = true;

#ifdef TLRENDER_FFMPEG
        SaveMovieOptionsUI saveOptions(hasAudio, audioOnly);
        if (saveOptions.cancel)
            return;

        options.video = options.saveVideo = false;
        int value = saveOptions.AudioCodec->value();
        options.ffmpegAudioCodec = static_cast<tl::ffmpeg::AudioCodec>(value);
#endif

        try
        {
            save_movie(file, ui, options);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }
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
        bool annotations_frames_only = false;

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
            options.resolution =
                static_cast<SaveResolution>(saveOptions.Resolution->value());

            int value;
            value = saveOptions.Profile->value();

            const Fl_Menu_Item* item = &saveOptions.Profile->menu()[value];

            // We need to iterate through all the profiles, as some profiles
            // may be hidden from the UI due to FFmpeg being compiled as
            // LGPL.
            int index = 0;
            auto entries = tl::ffmpeg::getProfileLabels();
            for (auto entry : entries)
            {
                if (entry == item->label())
                {
                    options.ffmpegProfile =
                        static_cast<tl::ffmpeg::Profile>(index);
                }
                ++index;
            }

            std::string preset;
            value = saveOptions.Preset->value();
            if (value >= 0)
            {
                const Fl_Menu_Item* item = &saveOptions.Preset->menu()[value];
                if (item->label())
                {
                    auto entries = tl::ffmpeg::getProfileLabels();
                    std::string profileName =
                        entries[(int)options.ffmpegProfile];
                    preset = tl::string::toLower(profileName) + "-" +
                             item->label() + ".pst";
                    options.ffmpegPreset = presetspath() + preset;
                    if (!file::isReadable(options.ffmpegPreset))
                    {
                        options.ffmpegPreset = "";
                    }
                }
            }

            std::string pixelFormat;
            value = saveOptions.PixelFormat->value();
            if (value >= 0)
            {
                const Fl_Menu_Item* item =
                    &saveOptions.PixelFormat->menu()[value];
                if (item->label())
                {
                    options.ffmpegPixelFormat = item->label();
                }
            }
            value = saveOptions.AudioCodec->value();
            options.ffmpegAudioCodec =
                static_cast<tl::ffmpeg::AudioCodec>(value);

            options.ffmpegHardwareEncode = saveOptions.Hardware->value();
            options.ffmpegOverride = saveOptions.Override->value();
            if (options.ffmpegOverride)
            {
                const Fl_Menu_Item* item;

                item = &saveOptions.ColorRange
                            ->menu()[saveOptions.ColorRange->value()];
                options.ffmpegColorRange = item->label();

                item = &saveOptions.ColorSpace
                            ->menu()[saveOptions.ColorSpace->value()];
                options.ffmpegColorSpace = item->label();

                item = &saveOptions.ColorPrimaries
                            ->menu()[saveOptions.ColorPrimaries->value()];
                options.ffmpegColorPrimaries = item->label();

                item = &saveOptions.ColorTRC
                            ->menu()[saveOptions.ColorTRC->value()];
                options.ffmpegColorTRC = item->label();
            }
        }
        else
#endif
        {
            bool valid_for_exr = false;
            if (extension == ".exr")
            {
                valid_for_exr = true;
            }

            SaveImageOptionsUI saveOptions(extension, valid_for_exr);
            if (saveOptions.cancel)
                return;

            options.annotations =
                static_cast<bool>(saveOptions.Annotations->value());
            options.resolution =
                static_cast<SaveResolution>(saveOptions.Resolution->value());
            annotations_frames_only =
                static_cast<bool>(saveOptions.AnnotationFramesOnly->value());

            int value;

#ifdef TLRENDER_EXR
            value = saveOptions.PixelType->value();
            if (value == 0)
                options.exrPixelType = tl::image::PixelType::RGBA_F16;
            if (value == 1)
                options.exrPixelType = tl::image::PixelType::RGBA_F32;

            value = saveOptions.Compression->value();
            options.exrCompression = static_cast<Imf::Compression>(value);

            options.zipCompressionLevel =
                static_cast<int>(saveOptions.ZipCompressionLevel->value());
            options.dwaCompressionLevel =
                saveOptions.DWACompressionLevel->value();
#endif
        }

        if (annotations_frames_only)
        {
            auto times = player->getAnnotationTimes();
            save_multiple_frames(file, times, ui, options);
        }
        else
        {
            try
            {
                save_movie(file, ui, options);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }
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

    void save_annotations_only_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return;

        const auto& ioInfo = player->ioInfo();

        const auto& annotations = player->getAllAnnotations();
        if (annotations.empty())
            return;

        const std::string& file = save_movie_or_sequence_file();
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
            LOG_ERROR(_("Cannot save annotations to .otio file"));
            return;
        }

        mrv::SaveOptions options;
        bool annotations_frames_only = false;

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

            SaveMovieOptionsUI saveOptions(hasAudio, false);
            if (saveOptions.cancel)
                return;

            options.annotations =
                static_cast<bool>(saveOptions.Annotations->value());
            options.resolution =
                static_cast<SaveResolution>(saveOptions.Resolution->value());

            int value;
            value = saveOptions.Profile->value();

            const Fl_Menu_Item* item = &saveOptions.Profile->menu()[value];

            // We need to iterate through all the profiles, as some profiles
            // may be hidden from the UI due to FFmpeg being compiled as
            // LGPL.
            int index = 0;
            auto entries = tl::ffmpeg::getProfileLabels();
            for (auto entry : entries)
            {
                if (entry == item->label())
                {
                    options.ffmpegProfile =
                        static_cast<tl::ffmpeg::Profile>(index);
                }
                ++index;
            }

            std::string preset;
            value = saveOptions.Preset->value();
            if (value >= 0)
            {
                const Fl_Menu_Item* item = &saveOptions.Preset->menu()[value];
                if (item->label())
                {
                    auto entries = tl::ffmpeg::getProfileLabels();
                    std::string profileName =
                        entries[(int)options.ffmpegProfile];
                    preset = tl::string::toLower(profileName) + "-" +
                             item->label() + ".pst";
                    options.ffmpegPreset = presetspath() + preset;
                    if (!file::isReadable(options.ffmpegPreset))
                    {
                        options.ffmpegPreset = "";
                    }
                }
            }

            std::string pixelFormat;
            value = saveOptions.PixelFormat->value();
            if (value >= 0)
            {
                const Fl_Menu_Item* item =
                    &saveOptions.PixelFormat->menu()[value];
                if (item->label())
                {
                    options.ffmpegPixelFormat = item->label();
                }
            }
            value = saveOptions.AudioCodec->value();
            options.ffmpegAudioCodec =
                static_cast<tl::ffmpeg::AudioCodec>(value);

            options.ffmpegHardwareEncode = saveOptions.Hardware->value();
            options.ffmpegOverride = saveOptions.Override->value();
            if (options.ffmpegOverride)
            {
                const Fl_Menu_Item* item;

                item = &saveOptions.ColorRange
                            ->menu()[saveOptions.ColorRange->value()];
                options.ffmpegColorRange = item->label();

                item = &saveOptions.ColorSpace
                            ->menu()[saveOptions.ColorSpace->value()];
                options.ffmpegColorSpace = item->label();

                item = &saveOptions.ColorPrimaries
                            ->menu()[saveOptions.ColorPrimaries->value()];
                options.ffmpegColorPrimaries = item->label();

                item = &saveOptions.ColorTRC
                            ->menu()[saveOptions.ColorTRC->value()];
                options.ffmpegColorTRC = item->label();
            }
        }
        else
#endif
        {
            bool valid_for_exr = false;
            if (extension == ".exr")
            {
                valid_for_exr = true;
            }

            SaveImageOptionsUI saveOptions(extension, valid_for_exr);
            if (saveOptions.cancel)
                return;

            options.annotations =
                static_cast<bool>(saveOptions.Annotations->value());
            annotations_frames_only =
                static_cast<bool>(saveOptions.AnnotationFramesOnly->value());

            int value;

#ifdef TLRENDER_EXR
            value = saveOptions.PixelType->value();
            if (value == 0)
                options.exrPixelType = tl::image::PixelType::RGBA_F16;
            if (value == 1)
                options.exrPixelType = tl::image::PixelType::RGBA_F32;

            value = saveOptions.Compression->value();
            options.exrCompression = static_cast<Imf::Compression>(value);

            options.zipCompressionLevel =
                static_cast<int>(saveOptions.ZipCompressionLevel->value());
            options.dwaCompressionLevel =
                saveOptions.DWACompressionLevel->value();
#endif
        }

        options.annotations = true;
        options.video = false;

        if (annotations_frames_only)
        {
            auto times = player->getAnnotationTimes();
            save_multiple_annotation_frames(file, times, ui, options);
        }
        else
        {
            try
            {
                save_movie(file, ui, options);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }
    }

    void save_annotations_as_json_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        auto view = ui->uiView;
        auto player = view->getTimelinePlayer();
        if (!player)
            return;

        const auto& annotations = player->getAllAnnotations();
        if (annotations.empty())
            return;

        const std::string& file = save_annotations();
        if (file.empty())
            return;

        Message j;
        j["render_size"] = view->getRenderSize();

        std::vector< draw::Annotation > flatAnnotations;
        for (const auto& annotation : annotations)
        {
            flatAnnotations.push_back(*annotation.get());
        }
        j["annotations"] = flatAnnotations;

        std::ofstream f(file);
        f << j;
    }

    void close_current_cb(Fl_Widget* w, ViewerUI* ui)
    {
        int ok = check_for_changes(ui);
        if (ok == 0)
            return;

        App::unsaved_edits = false;
        App::unsaved_annotations = false;
        
        // Must come before model->close().
        if (ui->uiPrefs->SendMedia->value())
            tcp->pushMessage("closeCurrent", 0);

        auto model = ui->app->filesModel();
        model->close();

        ui->uiMain->update_title_bar();
        ui->uiMain->fill_menu(ui->uiMenuBar);

        auto images = model->observeFiles()->get();
        if (images.empty())
            reset_timeline(ui);
    }

    void close_all_cb(Fl_Widget* w, ViewerUI* ui)
    {
        int ok = check_for_changes(ui);
        if (ok == 0)
            return;

        App::unsaved_edits = false;
        App::unsaved_annotations = false;

        
        if (ui->uiPrefs->SendMedia->value())
            tcp->pushMessage("closeAll", 0);

        auto model = ui->app->filesModel();
        model->closeAll();

        ui->uiMain->update_title_bar();
        ui->uiMain->fill_menu(ui->uiMenuBar);

        reset_timeline(ui);
    }

    void exit_cb(Fl_Widget* w, ViewerUI* ui)
    {
        int ok = check_for_changes(ui);
        if (ok == 0)
            return;

        App::unsaved_edits = false;
        App::unsaved_annotations = false;
        
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
#ifdef TLRENDER_NDI
        if (panel::ndiPanel)
            panel::ndiPanel->save();
#endif
#ifdef MRV2_NETWORK
        if (panel::networkPanel)
            panel::networkPanel->save();
#endif
#ifdef TLRENDER_USD
        if (panel::usdPanel)
            panel::usdPanel->save();
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
#ifdef TLRENDER_NDI
        delete panel::ndiPanel;
        panel::ndiPanel = nullptr;
#endif

        // Close all files
        close_all_cb(w, ui);

        // Remove thumbnail system.
#ifdef VULKAN_BACKEND
        auto context = App::app->getContext();
        auto system  = context->getSystem<timelineui_vk::ThumbnailSystem>();
        context->removeSystem(system);
#endif

        // Delete Color Chooser
        delete colorChooser;

        // Remove any temporary EDLs in tmppath
        if (ui->uiPrefs->uiPrefsRemoveEDLs->value())
            removeTemporaryEDLs(ui);

        Fl::hide_all_windows();
        
        tcp->unlock();
    }

    void previous_channel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        int value = ui->uiColorChannel->value();
        const int size = ui->uiColorChannel->size() - 1;
        --value;
        if (value < 0)
            value = size - 1;
        ui->uiColorChannel->value(value);
        ui->uiColorChannel->do_callback();
    }

    void next_channel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        int value = App::ui->uiColorChannel->value();
        const int size = ui->uiColorChannel->size() - 1;
        ++value;
        if (value >= size)
            value = 0;
        ui->uiColorChannel->value(value);
        ui->uiColorChannel->do_callback();
    }

    void minify_nearest_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.minify = timeline::ImageFilter::Nearest;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void minify_linear_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.minify = timeline::ImageFilter::Linear;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void magnify_nearest_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.magnify = timeline::ImageFilter::Nearest;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void magnify_linear_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.imageFilters.magnify = timeline::ImageFilter::Linear;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void mirror_x_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.mirror.x ^= 1;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->updateCoords();
        ui->uiView->redrawWindows();
    }

    void mirror_y_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.mirror.y ^= 1;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->updateCoords();
        ui->uiView->redrawWindows();
    }

    void hdr_data_from_file_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.hdrInfo = timeline::HDRInformation::FromFile;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void hdr_data_inactive_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.hdrInfo = timeline::HDRInformation::Inactive;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void hdr_data_active_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.hdrInfo = timeline::HDRInformation::Active;
        ui->app->setDisplayOptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
        ui->uiView->redrawWindows();
    }

    void rotate_plus_90_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        float r = ui->uiView->getRotation();
        r += 90.F;
        if (r == 270.F)
            r = -90.F;
        ui->uiView->setRotation(r);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void rotate_minus_90_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        float r = ui->uiView->getRotation();
        r -= 90.F;
        if (r == -270.F)
            r = 90.F;
        ui->uiView->setRotation(r);
        ui->uiMain->fill_menu(ui->uiMenuBar);
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

    void toggle_lumma_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Lumma;
        toggle_channel(ui, channel);
    }

    void toggle_color_channel_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        const timeline::Channels channel = timeline::Channels::Color;
        toggle_channel(ui, channel);
    }

    void toggle_normalize_image_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.normalize.enabled = !o.normalize.enabled;
        ui->app->setDisplayOptions(o);
        refresh_media_cb(m, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_ignore_chromaticities_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.ignoreChromaticities = !o.ignoreChromaticities;
        ui->app->setDisplayOptions(o);
        refresh_media_cb(m, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_invalid_values_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::DisplayOptions o = ui->app->displayOptions();
        o.invalidValues ^= 1;
        ui->app->setDisplayOptions(o);
        refresh_media_cb(m, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_hdr_tonemap_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        timeline::HDROptions o = ui->uiView->getHDROptions();
        o.tonemap ^= 1;
        ui->uiView->setHDROptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }
    
    void select_hdr_tonemap_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const Fl_Menu_Item* item = m->mvalue();
        const std::string algorithm = item->label();

        int idx = 0;
        for (const auto& entry : timeline::getHDRTonemapAlgorithmLabels())
        {
            if (entry == algorithm)
                break;
            ++idx;
        }

        timeline::HDROptions o = ui->uiView->getHDROptions();
        o.algorithm = static_cast<timeline::HDRTonemapAlgorithm>(idx);
        ui->uiView->setHDROptions(o);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    // \@bug: Under Wayland and Vulkan, FLTK sends a key repeat after
    //        a change in fullscreen.  We use this constant to avoid
    //        the repetition of the key.
    constexpr int kVulkanWaylandKeyRepeat = 1500;
    
    auto last_fullscreen_active = std::chrono::high_resolution_clock::now();
    auto last_presentation_active = std::chrono::high_resolution_clock::now();
    
    void toggle_fullscreen_cb(Fl_Menu_* m, ViewerUI* ui)
    {        
        MyViewport* view = ui->uiView;
        
#ifdef VULKAN_BACKEND
        if (desktop::Wayland())
        {
            auto now = std::chrono::high_resolution_clock::now();
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - last_fullscreen_active).count();
            last_fullscreen_active = now;
            if (elapsed < kVulkanWaylandKeyRepeat)
            {
                return;
            }
        }
#endif
            
        bool active = !view->getFullScreenMode();
        ui->uiView->setFullScreenMode(active);

        
        // These are needed to clean the resources and avoid
        // OpenGL flickering.
        ui->uiView->refresh();
        ui->uiView->valid(0);
        ui->uiTimeline->refresh();
        ui->uiTimeline->valid(0);

        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Fullscreen", active);
        
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_presentation_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        MyViewport* view = ui->uiView;
        
#ifdef VULKAN_BACKEND
        if (desktop::Wayland())
        {
            auto now = std::chrono::high_resolution_clock::now();
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - last_presentation_active).count();
            last_presentation_active = now;
            if (elapsed < kVulkanWaylandKeyRepeat)
            {
                return;
            }
        }
#endif

        bool presentation = !view->getPresentationMode();
        view->setPresentationMode(presentation);

        
        // These are needed to clean the resources and avoid
        // OpenGL flickering.
        ui->uiView->refresh();
        ui->uiView->valid(0);
        ui->uiTimeline->refresh();
        ui->uiTimeline->valid(0);

        bool send = ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("Presentation", !presentation);

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_float_on_top_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool active = true;
        const Fl_Menu_Item* item = m->mvalue();
        if (!item->value())
            active = false;
        ui->uiMain->always_on_top(active);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_secondary_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        MainWindow* window;
        MyViewport* view;

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

            // These are needed to clean the resources and avoid
            // OpenGL flickering.
            ui->uiView->refresh();
            ui->uiView->valid(0);
            ui->uiTimeline->refresh();
            ui->uiTimeline->valid(0);
        }
        else
        {
            App* app = ui->app;
            view->setContext(app->getContext());
            view->setOCIOOptions(ui->uiView->getOCIOOptions());
            view->setLUTOptions(app->lutOptions());
            timeline::ImageOptions imageOptions = app->imageOptions();
            timeline::DisplayOptions displayOptions = app->displayOptions();
            view->setImageOptions({imageOptions});
            view->setDisplayOptions({displayOptions});
            auto model = app->filesModel();
            view->setCompareOptions(model->observeCompareOptions()->get());
            view->setTimelinePlayer(ui->uiView->getTimelinePlayer());
            window->show();

            bool value = ui->uiPrefs->uiPrefsSecondaryOnTop->value();
            window->always_on_top(value);

            view->frameView();
            view->redraw();
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
        if (!item->value())
            active = false;
        ui->uiSecondary->window()->always_on_top(active);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_click_through(Fl_Menu_* w, ViewerUI* ui)
    {
        bool value = !ui->uiMain->get_click_through();
        ui->uiMain->set_click_through(value);
    }

    void more_ui_transparency(Fl_Menu_* w, ViewerUI* ui)
    {
        int alpha = ui->uiMain->get_alpha();
        alpha -= 5;
        ui->uiMain->set_alpha(alpha);
    }

    void less_ui_transparency(Fl_Menu_* w, ViewerUI* ui)
    {
        int alpha = ui->uiMain->get_alpha();
        alpha += 5;
        ui->uiMain->set_alpha(alpha);
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
        if (ui->uiView->getPresentationMode())
            return;

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
        save_edit_mode_state(ui);

        if (ui->uiView->getPresentationMode())
            return;

        has_menu_bar = ui->uiMenuGroup->visible();
        has_top_bar = ui->uiTopBar->visible();
        has_bottom_bar = ui->uiBottomBar->visible();
        has_pixel_bar = ui->uiPixelBar->visible();
        has_status_bar = ui->uiStatusGroup->visible();
        has_tools_grp = ui->uiToolsGroup->visible();
        has_dock_grp = ui->uiDockGroup->visible();

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

    void toggle_timeline_bar(Fl_Menu_*, ViewerUI* ui)
    {
        toggle_ui_bar(ui, ui->uiBottomBar);
        save_ui_state(ui, ui->uiBottomBar);
        if (ui->uiBottomBar->visible())
        {
            set_edit_mode_cb(previousEditMode, ui);
        }
        else
        {
            set_edit_mode_cb(EditMode::kNone, ui);
        }
        
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
                set_edit_mode_cb(EditMode::kSaved, ui);
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

        ui->uiView->frameView();

        PanelGroup::show_all();
    }

    HUDUI* hudClass = nullptr;

    void hud_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        if (!hudClass)
        {
            MyViewport* view = ui->uiView;
            Fl_Group::current(view);
            hudClass = new HUDUI(view);
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void frame_view_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto view = ui->uiView;
        view->setFrameView(item->value());
        if (ui->uiSecondary && ui->uiSecondary->viewport())
        {
            view = ui->uiSecondary->viewport();
            view->setFrameView(item->value());
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_safe_areas_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool checked = !ui->uiView->getSafeAreas();
        ui->uiView->setSafeAreas(checked);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_data_window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool checked = !ui->uiView->getDataWindow();
        ui->uiView->setDataWindow(checked);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_ignore_display_window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool checked = !ui->uiView->getIgnoreDisplayWindow();
        ui->uiView->setIgnoreDisplayWindow(checked);
        refresh_media_cb(m, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_display_window_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool checked = !ui->uiView->getDisplayWindow();
        ui->uiView->setDisplayWindow(checked);
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

    OCIOPresetsUI* OCIOPresetsClass = nullptr;

    void ocio_presets_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        if (!OCIOPresetsClass)
        {
            MyViewport* view = ui->uiView;
            Fl_Group::current(view);
            OCIOPresetsClass = new OCIOPresetsUI();
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_ocio_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        timeline::OCIOOptions options = ui->uiView->getOCIOOptions();
        options.enabled = !options.enabled;
        ui->uiView->setOCIOOptions(options);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_ocio_topbar_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        if (ui->uiOCIO->visible())
        {
            ui->uiOCIO->hide();
            ui->uiCOLORS->show();
        }
        else
        {
            ui->uiOCIO->show();
            ui->uiCOLORS->hide();
        }
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void current_ocio_ics_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const Fl_Menu_Item* selected = m->mvalue();
        char pathname[1024];
        int ret = m->item_pathname(pathname, 1024, selected);
        if (ret != 0)
            return;

        const std::string& colorSpace = _("OCIO/     Input Color Space");
        std::string ics = pathname;
        size_t pos = ics.find(colorSpace);
        if (pos != std::string::npos)
        {
            ics = ics.substr(pos + colorSpace.size() + 1, ics.size());
            ocio::setIcs(ics);
        }
    }

    void current_ocio_look_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        const Fl_Menu_Item* selected = m->mvalue();
        char pathname[1024];
        int ret = m->item_pathname(pathname, 1024, selected);
        if (ret != 0)
            return;

        const std::string& colorSpace = _("OCIO/     Look");
        std::string look = pathname;
        size_t pos = look.find(colorSpace);
        if (pos != std::string::npos)
        {
            look = look.substr(pos + colorSpace.size() + 1, look.size());
            ocio::setLook(look);
        }
    }

    void monitor_ocio_view_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        int monitorId = -1;
        const Fl_Menu_Item* selected = m->mvalue();
        char pathname[1024];
        int ret = m->item_pathname(pathname, 1024, selected);
        if (ret != 0)
            return;

        // Remove monitor name from menu pathname.
        int numMonitors = Fl::screen_count();
        std::string combined = pathname;
        for (int i = 0; i < numMonitors; ++i)
        {
            const std::string& monitorName = desktop::monitorName(i);

            int idx = combined.find(monitorName);
            if (idx == std::string::npos)
                continue;

            monitorId = i;
            combined =
                combined.substr(idx + monitorName.size() + 1, combined.size());
            break;
        }

        // Split combined display/view into separate parts.
        timeline::OCIOOptions o;
        ocio::splitView(combined, o.display, o.view);
        if (numMonitors == 1)
        {
            // If only one monitor, update main UI.
            ocio::setView(combined);
        }
        else
        {
            // Set the display and view for this monitor.
            ui->uiView->setOCIOOptions(monitorId, o);
        }

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void all_monitors_ocio_view_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        // Get item selected
        const Fl_Menu_Item* selected = m->mvalue();
        char pathname[1024];
        int ret = m->item_pathname(pathname, 1024, selected);
        if (ret != 0)
            return;

        // Change all monitors
        static std::string allMonitors = _("All Monitors");
        int numMonitors = Fl::screen_count();
        std::string combined = pathname;

        int idx = combined.find(allMonitors);
        if (idx == std::string::npos)
            return;

        combined =
            combined.substr(idx + allMonitors.size() + 1, combined.size());

        for (int i = 0; i < numMonitors; ++i)
        {
            // Split combined display/view into separate parts.
            timeline::OCIOOptions o;
            ocio::splitView(combined, o.display, o.view);
            ui->uiView->setOCIOOptions(i, o);
        }

        ui->uiMain->fill_menu(ui->uiMenuBar);
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
        o.alphaBlend = timeline::AlphaBlend::kNone;
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

    void toggle_otio_clip_in_out_cb(Fl_Menu_*, ViewerUI* ui)
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

        const auto& fullRange = player->timeRange();
        const auto& inOutRange = player->inOutRange();
        auto range = item->trimmed_range_in_parent().value();
        if (range == inOutRange)
        {
            range = fullRange;
        }

        auto rate = track->trimmed_range().end_time_exclusive().rate();
        range = otime::TimeRange::range_from_start_end_time(
            range.start_time().rescaled_to(rate).round(),
            range.end_time_exclusive().rescaled_to(rate).round());
        player->setInOutRange(range);

        TimelineClass* c = ui->uiTimeWindow;
        c->uiStartButton->value(!c->uiStartButton->value());
        c->uiEndButton->value(!c->uiEndButton->value());
        c->uiStartFrame->setTime(range.start_time());
        c->uiEndFrame->setTime(range.end_time_exclusive());

        ui->uiTimeline->redraw();
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
        const auto& view = ui->uiView;
        const auto& player = view->getTimelinePlayer();
        if (!player)
            return;
        auto currentTime = player->currentTime().round();
        std::vector< otime::RationalTime > times = player->getAnnotationTimes();
        std::sort(
            times.begin(), times.end(), std::greater<otime::RationalTime>());
        for (const auto& time : times)
        {
            const auto& roundedTime = time.round();
            if (roundedTime < currentTime)
            {
                view->stop();
                player->seek(time);
                return;
            }
        }
        // If no next annotation was found, look to the first annotation
        if (!times.empty())
        {
            view->stop();
            player->seek(times[0]);
        }
    }

    void next_annotation_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto& view = ui->uiView;
        const auto& player = view->getTimelinePlayer();
        if (!player)
            return;
        const auto& currentTime = player->currentTime().round();
        std::vector< otime::RationalTime > times = player->getAnnotationTimes();
        std::sort(times.begin(), times.end());
        for (const auto& time : times)
        {
            const auto& roundedTime = time.round();
            if (roundedTime > currentTime)
            {
                view->stop();
                player->seek(time);
                return;
            }
        }
        // If no next annotation was found, look to the first annotation
        if (!times.empty())
        {
            view->stop();
            player->seek(times[0]);
        }
    }

    void toggle_visible_annotation_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        bool value = !ui->uiView->getShowAnnotations();
        ui->uiView->setShowAnnotations(value);
        ui->uiView->redrawWindows();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void annotation_clear_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;
        if (ui->uiPrefs->SendAnnotations->value())
            tcp->pushMessage("Clear Frame Annotations", 0);
        player->clearFrameAnnotation();
        ui->uiView->updateUndoRedoButtons();
        ui->uiTimeline->redraw();
        ui->uiView->redrawWindows();
    }

    void annotation_clear_all_cb(Fl_Menu_*, ViewerUI* ui)
    {
        const auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        int ok = fl_choice(
            _("You are about to clear all annotations.  You can still undo "
              "the operation right after. "
              "Do you want to continue?"),
            _("No"), _("Yes"), NULL, NULL);
        if (!ok)
            return;
        
        if (ui->uiPrefs->SendAnnotations->value())
            tcp->pushMessage("Clear All Annotations", 0);
        player->clearAllAnnotations();
        ui->uiView->updateUndoRedoButtons();
        ui->uiTimeline->redraw();
        ui->uiView->redrawWindows();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_none_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getDisplayOptions();
        options.thumbnails = false;
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_editable_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        bool editable = (bool)item->value();
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
        options.editAssociatedClips = item->value();
        Message msg;
        msg["command"] = "setTimelineItemOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setItemOptions(options);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_frame_view_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Message msg;
        msg["command"] = "Timeline/FrameView";
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->frameView();
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_scroll_to_current_frame_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        bool value = item->value();
        auto settings = ui->app->settings();
        Message msg;
        msg["command"] = "Timeline/ScrollToCurrentFrame";
        msg["value"] = value;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        settings->setValue("Timeline/ScrollToCurrentFrame", value);
        ui->uiTimeline->setScrollToCurrentFrame(value);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_track_info_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getDisplayOptions();
        options.trackInfo = item->value();
        auto settings = ui->app->settings();
        settings->setValue("Timeline/TrackInfo", options.trackInfo);
        Message msg;
        msg["command"] = "setTimelineDisplayptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_clip_info_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getDisplayOptions();
        options.clipInfo = item->value();
        auto settings = ui->app->settings();
        settings->setValue("Timeline/ClipInfo", options.clipInfo);
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_markers_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getDisplayOptions();
        options.markers = item->value();
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_active_track_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        std::string track = item->label();

        size_t startIndex = track.find("#");
        if (startIndex == std::string::npos)
            return;

        size_t endIndex = track.find("-");
        size_t len = endIndex - startIndex - 2;
        unsigned trackIndex = std::stoul(track.substr(startIndex + 1, len)) - 1;
        toggleTrack(trackIndex, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void toggle_timeline_transitions_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        Fl_Menu_Item* item = const_cast< Fl_Menu_Item* >(m->mvalue());
        auto options = ui->uiTimeline->getDisplayOptions();
        options.transitions = item->value();
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_small_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getDisplayOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 100;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_medium_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getDisplayOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 200;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
        if (editMode != EditMode::kTimeline)
            set_edit_mode_cb(EditMode::kFull, ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void timeline_thumbnails_large_cb(Fl_Menu_* m, ViewerUI* ui)
    {
        auto options = ui->uiTimeline->getDisplayOptions();
        options.thumbnails = true;
        options.thumbnailHeight = 300;
        options.waveformHeight = options.thumbnailHeight / 2;
        Message msg;
        msg["command"] = "setTimelineDisplayOptions";
        msg["value"] = options;
        if (ui->uiPrefs->SendUI->value())
            tcp->pushMessage(msg);
        ui->uiTimeline->setDisplayOptions(options);
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
#ifdef OPENGL_BACKEND
        w->textcolor(c);
        w->redraw();
#endif
#ifdef VULKAN_BACKEND
        w->color = from_fltk_color(c);
        ui->uiView->redrawWindows();
#endif
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
        uint8_t r, g, b, a = 255;
        Fl::get_color(c, r, g, b);
        image::Color4f color = image::Color4f(r / 255.F, g / 255.F, b / 255.F);

        if (!flmm_color_a_chooser(_("Pick Color"), r, g, b, a))
            return color;

        color = image::Color4f(r / 255.F, g / 255.F, b / 255.F, a / 255.F);
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
        ViewerUI* ui, const int sum, const bool first_or_last = false)
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

        tl::file::Path otioPath = media->path;
        auto clipPath = media->path;
        if (string::compare(
                otioPath.getExtension(), ".otio",
                string::Compare::CaseInsensitive))
        {
            const auto& tags = ui->uiView->getTags();
            auto i = tags.find("otioClipName");
            if (i != tags.end())
            {
                clipPath = tl::file::Path(i->second);
            }
        }

        const std::string& fileName =
            media_version(ui, clipPath, sum, first_or_last, otioPath);
        if (fileName.empty())
            return;

        if (otioPath != clipPath)
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
        item->playback = player->playback();
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
        const std::string& docs = docspath();
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
            ui->uiPrefs->SendMedia->value(item->value());
        }
        else if (label == _("UI"))
        {
            ui->uiPrefs->SendUI->value(item->value());
        }
        else if (label == _("Pan And Zoom"))
        {
            ui->uiPrefs->SendPanAndZoom->value(item->value());
        }
        else if (label == _("Color"))
        {
            ui->uiPrefs->SendColor->value(item->value());
        }
        else if (label == _("Timeline"))
        {
            ui->uiPrefs->SendTimeline->value(item->value());
        }
        else if (label == _("Annotations"))
        {
            ui->uiPrefs->SendAnnotations->value(item->value());
        }
        else if (label == _("Audio"))
        {
            ui->uiPrefs->SendAudio->value(item->value());
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
            ui->uiPrefs->ReceiveMedia->value(item->value());
        }
        else if (label == _("UI"))
        {
            ui->uiPrefs->ReceiveUI->value(item->value());
        }
        else if (label == _("Pan And Zoom"))
        {
            ui->uiPrefs->ReceivePanAndZoom->value(item->value());
        }
        else if (label == _("Color"))
        {
            ui->uiPrefs->ReceiveColor->value(item->value());
        }
        else if (label == _("Timeline"))
        {
            ui->uiPrefs->ReceiveTimeline->value(item->value());
        }
        else if (label == _("Annotations"))
        {
            ui->uiPrefs->ReceiveAnnotations->value(item->value());
        }
        else if (label == _("Audio"))
        {
            ui->uiPrefs->ReceiveAudio->value(item->value());
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
            if (file::isTemporaryEDL(path))
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
        MyViewport* view = ui->uiView;
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
                break;
            }
        }

        if (shapes.empty())
        {
            player->clearFrameAnnotation();
        }
    }

    void add_note_annotation_cb(ViewerUI* ui, const std::string& text)
    {
        MyViewport* view = ui->uiView;
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
#ifdef OPENGL_BACKEND
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
#endif
            
#ifdef VULKAN_BACKEND
            if (auto s = dynamic_cast<VKTextShape*>(shape.get()))
            {
                window.add(s->text);
            }
#endif
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

    void refresh_media_cb(Fl_Menu_* m, void* d)
    {
        auto app = App::app;
        auto model = app->filesModel();
        auto files = model->observeFiles()->get();
        if (files.size() < 1)
            return;

        auto player = app->ui->uiView->getTimelinePlayer();
        const auto& time = player->currentTime();

        auto origIndex = model->observeAIndex()->get();
        const auto& media = files[origIndex];

        auto item = std::make_shared<FilesModelItem>();
        item->init = true;
        item->path = media->path;
        item->audioPath = media->audioPath;
        item->inOutRange = media->inOutRange;
        item->ioInfo = media->ioInfo;
        item->speed = media->speed;
        item->audioOffset = media->audioOffset;
        item->videoLayer = media->videoLayer;
        item->loop = media->loop;
        item->playback = player->playback();
        item->currentTime = time;
        model->replace(origIndex, item);

        auto newIndex = model->observeAIndex()->get();
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

    void update_video_frame_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        player->updateVideoCache(player->currentTime());
    }

    void refresh_file_cache_cb(Fl_Menu_* m, void* d)
    {
        auto ui = App::ui;
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return;

        auto app = App::app;

        // Update the I/O cache.
        auto ioSystem = app->getContext()->getSystem<io::System>();
        ioSystem->getCache()->clear();

        player->clearCache();
    }

    void refresh_movie_cb(Fl_Menu_* m, void* d)
    {
        auto app = App::app;
        auto model = app->filesModel();
        if (model->observeFiles()->getSize() < 1)
            return;

        // Check if item is a movie.
        auto item = model->observeA()->get();
        auto path = item->path;
        if (!file::isMovie(path))
            return;

        refresh_media_cb(m, d);
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
