// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <fstream>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvEnvironmentMapOptions.h"
#include "mrvCore/mrvStereo3DOptions.h"

#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvCypher.h"
#include "mrvNetwork/mrvFilesModelItem.h"
#include "mrvNetwork/mrvCompareOptions.h"
#include "mrvNetwork/mrvDisplayOptions.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

#include "mrvSession.h"

namespace
{
    const char* kModule = "mrv2s";
}

namespace mrv
{

    static void toggle_widget(Fl_Widget* w, const bool visible)
    {
        if (visible)
            w->show();
        else
            w->hide();
    }

    std::string currentSession;

    std::string current_session()
    {
        return currentSession;
    }

    void set_current_session(const std::string& file)
    {
        currentSession = file;
        ViewerUI* ui = App::ui;
        ui->uiMain->update_title_bar();
    }

    bool save_session(const std::string& fileName)
    {
        ViewerUI* ui = App::ui;
        App* app = ui->app;
        auto model = app->filesModel();
        auto files_ptrs = model->observeFiles()->get();

        enable_cypher(false);

        Message session;
        session["version"] = 6;

        std::vector< FilesModelItem > files;
        for (const auto file : files_ptrs)
        {
            const std::string fileName = file->path.get();
            if (isTemporaryEDL(fileName))
                continue;
            files.push_back(*file.get());
        }
        session["files"] = files;
        session["Aindex"] = model->observeAIndex()->get();
        session["Bindexes"] = model->observeBIndexes()->get();

        auto player = ui->uiView->getTimelinePlayer();

        Message timeline;
        Message annotation;
        Message time;
        Message playback;

        if (player)
        {
            auto annotations = player->getAllAnnotations();
            std::vector< draw::Annotation > jAnnotations;
            for (const auto& ann : annotations)
            {
                jAnnotations.push_back(*(ann.get()));
            }
            annotation = jAnnotations;
            time = player->currentTime();
            playback = player->playback();
        }

        timeline["annotations"] = annotation;
        timeline["time"] = time;
        timeline["playback"] = playback;

        Message bars = {
            {"menu_bar", (bool)ui->uiMenuGroup->visible()},
            {"top_bar", (bool)ui->uiTopBar->visible()},
            {"pixel_bar", (bool)ui->uiPixelBar->visible()},
            {"bottom_bar", (bool)ui->uiBottomBar->visible()},
            {"status_bar", (bool)ui->uiStatusGroup->visible()},
            {"action_bar", (bool)ui->uiToolsGroup->visible()},
            {"secondary_window", (ui->uiSecondary != nullptr)},
        };

        Message panels = {
            {"Files", (filesPanel != nullptr)},
            {"Color", (colorPanel != nullptr)},
            {"Color Area", (colorAreaPanel != nullptr)},
            {"Compare", (comparePanel != nullptr)},
            {"Playlist", (playlistPanel != nullptr)},
            {"Media Information", (imageInfoPanel != nullptr)},
            {"Annotations", (annotationsPanel != nullptr)},
            {"Devices", (devicesPanel != nullptr)},
            {"Environment Map", (environmentMapPanel != nullptr)},
            {"Settings", (settingsPanel != nullptr)},
#ifdef MRV2_PYBIND11
            {"Python", (pythonPanel != nullptr)},
#endif
#ifdef MRV2_NETWORK
            {"Network", (networkPanel != nullptr)},
#endif
            {"Histogram", (histogramPanel != nullptr)},
            {"Vectorscope", (vectorscopePanel != nullptr)},
            {"Stereo 3D", (stereo3DPanel != nullptr)},
#ifdef TLRENDER_USD
            {"USD", (usdPanel != nullptr)},
#endif
            {"Logs", (logsPanel != nullptr)},
        };

        if (filesPanel)
            filesPanel->save();
        if (colorPanel)
            colorPanel->save();
        if (colorAreaPanel)
            colorAreaPanel->save();
        if (comparePanel)
            comparePanel->save();
        if (playlistPanel)
            playlistPanel->save();
        if (imageInfoPanel)
            imageInfoPanel->save();
        if (annotationsPanel)
            annotationsPanel->save();
        if (environmentMapPanel)
            environmentMapPanel->save();
        if (settingsPanel)
            settingsPanel->save();
#ifdef MRV2_PYBIND11
        if (pythonPanel)
            pythonPanel->save();
#endif
#ifdef MRV2_NETWORK
        if (networkPanel)
            networkPanel->save();
#endif
#ifdef TLRENDER_USD
        if (usdPanel)
            usdPanel->save();
#endif
        if (histogramPanel)
            histogramPanel->save();
        if (vectorscopePanel)
            vectorscopePanel->save();
        if (logsPanel)
            logsPanel->save();

        std::string config = ui->uiPrefs->uiPrefsOCIOConfig->value();
        int ics = ui->uiICS->value();
        int view = ui->OCIOView->value();
        int layer = ui->uiColorChannel->value();

        Message ocio = {
            {"config", config},
            {"ics", ics},
            {"view", view},
        };

        Message settings;

        const auto settingsObject = app->settingsObject();
        const auto keys = settingsObject->keys();
        for (const auto& key : keys)
        {
            std_any value = settingsObject->value(key);
            // // nlohmann::json cannot distinguish floats from doubles
            // try
            // {
            //     double tmpD = std_any_cast< double >(value);
            //     settings[key] = tmpD;
            //     continue;
            // }
            // catch (const std::bad_cast& e)
            // {
            // }
            try
            {
                float tmpF = std::any_cast< float >(value);
                settings[key] = tmpF;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std::any_cast< int >(value);
                settings[key] = tmp;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                bool tmp = std::any_cast< bool >(value);
                settings[key] = tmp;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string& tmpS = std::any_cast< std::string >(value);
                settings[key] = tmpS;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string tmpS = std::any_cast< char* >(value);
                settings[key] = tmpS;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                // If we don't know the type, don't store anything
                continue;
            }
            catch (const std::bad_cast& e)
            {
                LOG_ERROR(
                    "Could not save sesssion ror " << key << " type "
                                                   << value.type().name());
            }
        }

        Message display = app->displayOptions();
        Message compare = model->observeCompareOptions()->get();
        Message stereo = model->observeStereo3DOptions()->get();
        Message environmentMap = ui->uiView->getEnvironmentMapOptions();
        int stereoIndex = model->observeStereoIndex()->get();

        session["ui"] = bars;
        session["panels"] = panels;
        session["timeline"] = timeline;
        session["ocio"] = ocio;
        session["layer"] = layer;
        session["settings"] = settings;
        session["compareOptions"] = compare;
        session["stereo3DOptions"] = stereo;
        session["stereoIndex"] = stereoIndex;
        session["environmentMapOptions"] = environmentMap;
        session["displayOptions"] = display;
        session["editMode"] = editMode;

        std::ofstream ofs(fileName);
        if (!ofs.is_open())
        {
            LOG_ERROR(_("Failed to open the file for writing."));
            return false;
        }

        ofs << std::setw(4) << session << std::endl;

        if (ofs.fail())
        {
            LOG_ERROR(_("Failed to write to the file."));
            return false;
        }
        if (ofs.bad())
        {
            LOG_ERROR(_("The stream is in an unrecoverable error state."));
            return false;
        }
        ofs.close();

        enable_cypher(true);

        std::string msg =
            string::Format(_("Session saved to \"{0}\".")).arg(fileName);
        LOG_INFO(msg);

        return true;
    }

    bool load_session(const std::string& fileName)
    {
        ViewerUI* ui = App::ui;
        App* app = ui->app;
        auto view = ui->uiView;
        auto model = app->filesModel();

        std::ifstream ifs(fileName);
        if (!ifs.is_open())
        {
            LOG_ERROR(_("Failed to open the file for reading."));
            return false;
        }

        Message session;
        enable_cypher(false);

        ifs >> session;

        if (ifs.fail())
        {
            LOG_ERROR(_("Failed to write to the file."));
            return false;
        }
        if (ifs.bad())
        {
            LOG_ERROR(_("The stream is in an unrecoverable error state."));
            return false;
        }
        ifs.close();

        // Get session version
        int version = session["version"];

        // Decode session file
        close_all_cb(nullptr, ui);

        for (const auto& j : session["files"])
        {
            FilesModelItem item;
            j.get_to(item);

            std::string path = item.path.get();
            std::string audioPath = item.audioPath.get();

            replace_path(path);
            if (!audioPath.empty())
                replace_path(audioPath);

            app->open(path, audioPath);

            // Copy annotations to both item and player
            auto Aitem = model->observeA()->get();
            Aitem->annotations = item.annotations;
            Aitem->videoLayer = item.videoLayer;
            Aitem->currentTime = item.currentTime;

            ui->uiColorChannel->value(item.videoLayer);
            ui->uiColorChannel->do_callback();

            auto player = view->getTimelinePlayer();
            if (player)
            {
                player->setAllAnnotations(item.annotations);
                player->seek(Aitem->currentTime);
            }
        }

        if (version >= 2)
        {
            int Aindex = session["Aindex"];
            model->setA(Aindex);

            std::vector<int> Bindexes = session["Bindexes"];
            model->clearB();
            for (auto i : Bindexes)
                model->setB(i, true);

            Message j = session["timeline"];

            auto tmp = j["annotations"];

            std::vector< std::shared_ptr<draw::Annotation> > annotations;
            for (const auto& value : tmp)
            {
                auto annotation = draw::messageToAnnotation(value);
                annotations.push_back(annotation);
            }

            auto player = view->getTimelinePlayer();
            if (player)
            {
                player->setAllAnnotations(annotations);

                if (version >= 3)
                {
                    timeline::Playback playback;
                    if (j["playback"].type() == nlohmann::json::value_t::string)
                    {
                        j.at("playback").get_to(playback);
                    }
                    else
                    {
                        int v;
                        j.at("playback").get_to(v);
                        playback = static_cast<timeline::Playback>(v);
                    }
                    player->setPlayback(playback);
                }

                otime::RationalTime time;
                j["time"].get_to(time);
                player->seek(time);
            }

            ui->uiTimeline->redraw();
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }

        Message j = session["ui"];

        // Decode bars
        toggle_widget(ui->uiMenuGroup, j["menu_bar"]);
        toggle_widget(ui->uiTopBar, j["top_bar"]);
        toggle_widget(ui->uiPixelBar, j["pixel_bar"]);
        toggle_widget(ui->uiBottomBar, j["bottom_bar"]);
        toggle_widget(ui->uiStatusBar, j["status_bar"]);
        toggle_widget(ui->uiToolsGroup, j["action_bar"]);

        if ((j["secondary_window"] && !ui->uiSecondary) ||
            (!j["secondary_window"] && ui->uiSecondary))
        {
            toggle_secondary_cb(nullptr, ui);
        }

        // Decode ICS
        if (version >= 2)
        {
            //
            // Handle color channel layer
            //
            int layer = session["layer"];
            ui->uiColorChannel->value(layer);
            ui->uiColorChannel->do_callback();

            //
            // Handle OCIO
            //
            j = session["ocio"];
            std::string config = j["config"];

            replace_path(config);

            ui->uiPrefs->uiPrefsOCIOConfig->value(config.c_str());

            Preferences::OCIO(ui);

            int value = j["ics"];
            ui->uiICS->value(value);
            value = j["view"];
            ui->OCIOView->value(value);
            ui->uiView->updateColorConfigOptions();

            // Hide Panels and Windows
            removePanels(ui);
            removeWindows(ui);

            j = session["settings"];

            auto settingsObject = app->settingsObject();

            for (const auto& item : j.items())
            {
                const std::string& key = item.key();
                Message value = item.value();
                Message::value_t type = value.type();
                switch (type)
                {
                case Message::value_t::boolean:
                    settingsObject->setValue(key, value.get<bool>());
                    continue;
                case Message::value_t::number_unsigned:
                {
                    int v = value.get<unsigned>();
                    settingsObject->setValue(key, v);
                    continue;
                }
                case Message::value_t::number_integer:
                    settingsObject->setValue(key, value.get<int>());
                    continue;
                case Message::value_t::number_float:
                {
                    std_any val = settingsObject->value(key);
                    if (!std_any_empty(val))
                    {
                        try
                        {
                            double v = std::any_cast<double>(val);
                            settingsObject->setValue(key, value.get<double>());
                        }
                        catch (const std::bad_cast& e)
                        {
                            settingsObject->setValue(key, value.get<float>());
                        }
                    }
                    else
                    {
                        settingsObject->setValue(key, value.get<float>());
                    }
                    continue;
                }
                case Message::value_t::string:
                    settingsObject->setValue(key, value.get<std::string>());
                    continue;
                default:
                    LOG_ERROR("Unknown Message::value_t for key " << key);
                }
            }
        }

        // Decode panels
        j = session["panels"];
        const WindowCallback* wc = kWindowCallbacks;
        for (; wc->name; ++wc)
        {
            Message value = j[wc->name];
            bool shown = false;
            if (!value.is_null())
                shown = value;
            if (shown)
            {
                show_window_cb(_(wc->name), ui);
            }
        }

        if (version >= 3)
        {
            EnvironmentMapOptions env = session["environmentMapOptions"];
            view->setEnvironmentMapOptions(env);

            timeline::CompareOptions compare = session["compareOptions"];
            model->setCompareOptions(compare);

            int stereoIndex = session["stereoIndex"];
            model->setStereo(stereoIndex);

            Stereo3DOptions stereo = session["stereo3DOptions"];
            model->setStereo3DOptions(stereo);
        }

        if (version >= 4)
        {
            timeline::DisplayOptions display = session["displayOptions"];
            app->setDisplayOptions(display);
        }

        if (version >= 6)
        {
            EditMode editMode = session["editMode"];
            set_edit_mode_cb(editMode, ui);
        }

        enable_cypher(true);
        ui->uiMain->fill_menu(ui->uiMenuBar);

        // Change current session filename.
        set_current_session(fileName);

        return true;
    }

} // namespace mrv
