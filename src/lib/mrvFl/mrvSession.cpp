// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include <fstream>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvString.h"

#include "mrvOptions/mrvEnvironmentMapOptions.h"
#include "mrvOptions/mrvStereo3DOptions.h"

#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvCypher.h"
#include "mrvNetwork/mrvFilesModelItem.h"

#include "mrvOptions/mrvCompareOptions.h"
#include "mrvOptions/mrvDisplayOptions.h"
#include "mrvOptions/mrvImageOptions.h"
#include "mrvOptions/mrvTimelineItemOptions.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvApp.h"

#include "mrvFl/mrvIO.h"

#include "mrvSession.h"

#include "mrvFl/mrvOCIO.h"

namespace
{
    const char* kModule = "mrv2s";
    const int kSessionVersion = 17;
} // namespace

namespace
{
    void toggle_widget(Fl_Widget* w, const bool visible)
    {
        if (visible)
            w->show();
        else
            w->hide();
        mrv::App::ui->uiRegion->layout();
        mrv::App::ui->uiViewGroup->layout();
    }
} // namespace

namespace mrv
{

    namespace session
    {

        std::map<std::string, std::string> sessionMetadata;
        std::string currentSession;

        std::string current()
        {
            return currentSession;
        }

        void setCurrent(const std::string& file)
        {
            currentSession = file;
            ViewerUI* ui = App::ui;
            ui->uiMain->update_title_bar();
        }

        bool save(const std::string& fileName)
        {
            using namespace panel;

            ViewerUI* ui = App::ui;
            App* app = App::app;
            auto model = app->filesModel();
            auto files_ptrs = model->observeFiles()->get();

            enable_cypher(false);

            Message session;
            session["version"] = kSessionVersion;

            std::vector< FilesModelItem > files;
            for (const auto file_ptr : files_ptrs)
            {
                FilesModelItem file = *file_ptr;
                file::Path path = file.path;
                if (file::isTemporaryEDL(path))
                    continue;
                file::Path audioPath = file.audioPath;
                const fs::path sessionPath(fileName);
                file.path = getRelativePath(path, sessionPath);
                if (!audioPath.isEmpty())
                    file.audioPath = getRelativePath(audioPath, sessionPath);
                files.push_back(file);
            }
            session["files"] = files;
            session["Aindex"] = model->observeAIndex()->get();
            session["Bindexes"] = model->observeBIndexes()->get();

            auto view   = ui->uiView;
            auto player = view->getTimelinePlayer();

            Message timeline;
            Message annotation;
            Message voiceAnnotation;
            Message time;
            Message playback;
            Message inOutRange;

            if (player)
            {
                auto annotations = player->getAllAnnotations();
                auto voannotations = player->getAllVoiceAnnotations();
                std::vector< draw::Annotation > jAnnotations;
                std::vector< voice::Annotation > voAnnotations;
                for (const auto& ann : annotations)
                {
                    jAnnotations.push_back(*(ann.get()));
                }
                for (const auto& ann : voannotations)
                {
                    voAnnotations.push_back(*(ann.get()));
                }
                annotation = jAnnotations;
                voiceAnnotation = voAnnotations;
                time = player->currentTime();
                playback = player->playback();
                inOutRange = player->inOutRange();
            }

            timeline["annotations"] = annotation;
            timeline["voiceAnnotations"] = voiceAnnotation;
            timeline["time"] = time;
            timeline["playback"] = playback;
            timeline["inOutRange"] = inOutRange;

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
#ifdef TLRENDER_NDI
                {"NDI", (ndiPanel != nullptr)},
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
#ifdef TLRENDER_NDI
            if (ndiPanel)
                ndiPanel->save();
#endif
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

            if (config.substr(0, 7) != "ocio://")
            {
                file::Path path(config);
                config = getRelativePath(path, fileName).get();
            }

            int layer = ui->uiColorChannel->value();
            const std::string& ocioIcs = ocio::ics();
            const std::string& ocioView = ocio::view();
            const std::string& ocioLook = ocio::look();

            Message ocio = {
                {"config", config},
                {"ics", ocioIcs},
                {"view", ocioView},
                {"look", ocioLook},
            };

            Message json_settings;

            const auto settings = app->settings();
            const auto keys = settings->keys();
            for (const auto& key : keys)
            {
                std::any value = settings->getValue<std::any>(key);
                // // nlohmann::json cannot distinguish floats from doubles
                // try
                // {
                //     double tmpD = std_any_cast< double >(value);
                //     json_settings[key] = tmpD;
                //     continue;
                // }
                // catch (const std::bad_cast& e)
                // {
                // }
                try
                {
                    float tmpF = std::any_cast< float >(value);
                    json_settings[key] = tmpF;
                    continue;
                }
                catch (const std::bad_cast& e)
                {
                }
                try
                {
                    int tmp = std::any_cast< int >(value);
                    json_settings[key] = tmp;
                    continue;
                }
                catch (const std::bad_cast& e)
                {
                }
                try
                {
                    bool tmp = std::any_cast< bool >(value);
                    json_settings[key] = tmp;
                    continue;
                }
                catch (const std::bad_cast& e)
                {
                }
                try
                {
                    const std::string& tmpS =
                        std::any_cast< std::string >(value);
                    json_settings[key] = tmpS;
                    continue;
                }
                catch (const std::bad_cast& e)
                {
                }
                try
                {
                    const std::string tmpS = std::any_cast< char* >(value);
                    json_settings[key] = tmpS;
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
                        "Could not save session ror " << key << " type "
                                                       << value.type().name());
                }
            }

            Message display = app->displayOptions();
            Message compare = model->observeCompareOptions()->get();
            Message stereo = model->observeStereo3DOptions()->get();
            Message image = app->imageOptions();
            Message environmentMap = view->getEnvironmentMapOptions();
            Message background = view->getBackgroundOptions();
            int stereoIndex = model->observeStereoIndex()->get();

            auto options = ui->uiTimeline->getItemOptions();
            Message timelineViewport = options;
            timelineViewport["editable"] = ui->uiTimeline->isEditable();

            session["ui"] = bars;
            session["panels"] = panels;
            session["timeline"] = timeline;
            session["ocio"] = ocio;
            session["layer"] = layer;
            session["settings"] = json_settings;
            session["compareOptions"] = compare;
            session["stereo3DOptions"] = stereo;
            session["stereoIndex"] = stereoIndex;
            session["imageOptions"] = image;
            session["environmentMapOptions"] = environmentMap;
            session["backgroundOptions"] = background;
            session["displayOptions"] = display;
            session["editMode"] = editMode;
            session["metadata"] = sessionMetadata;
            session["timelineViewport"] = timelineViewport;
            session["frameView"] = view->hasFrameView();
            session["viewZoom"] = view->viewZoom();
            session["viewPos"] = view->viewPos();
            session["actionMode"] = view->getActionMode();
                        
            std::ofstream ofs(fileName);
            if (!ofs.is_open())
            {
                /* xgettext:c++-format */
                const std::string& err =
                    string::Format(
                        _("Failed to open the file {0} for writing."))
                        .arg(fileName);
                LOG_ERROR(err);
                return false;
            }

            ofs << session.dump(4) << std::endl;

            if (ofs.fail())
            {
                /* xgettext:c++-format */
                const std::string& err =
                    string::Format(_("Failed to write to the file {0}."))
                        .arg(fileName);
                LOG_ERROR(err);
                return false;
            }
            if (ofs.bad())
            {
                LOG_ERROR(_("The stream is in an unrecoverable error state."));
                return false;
            }
            ofs.close();

            enable_cypher(true);

            App::unsaved_edits = false;
            App::unsaved_annotations = false;

            ui->uiMain->update_title_bar();
            
            /* xgettext:c++-format */
            const std::string msg =
                string::Format(_("Session saved to \"{0}\".")).arg(fileName);
            LOG_STATUS(msg);

            return true;
        }

        bool load(const std::string& fileName)
        {
            using namespace panel;

            ViewerUI* ui = App::ui;
            App* app = ui->app;
            auto view = ui->uiView;
            auto model = app->filesModel();

            std::ifstream ifs(fileName);
            if (!ifs.is_open())
            {
                /* xgettext:c++-format */
                const std::string& err =
                    string::Format(
                        _("Failed to open the file '{0}' for reading."))
                        .arg(fileName);
                LOG_ERROR(err);
                return false;
            }

            enable_cypher(false);

            try
            {
                Message session;

                ifs >> session;

                if (ifs.fail())
                {
                    /* xgettext:c++-format */
                    const std::string& err =
                        string::Format(_("Failed to load the file '{0}."))
                            .arg(fileName);
                    LOG_ERROR(err);
                    return false;
                }
                if (ifs.bad())
                {
                    LOG_ERROR(
                        _("The stream is in an unrecoverable error state."));
                    return false;
                }
                ifs.close();
                
                // Set the edit mode to timeline
                set_edit_mode_cb(EditMode::kTimeline, ui);

                // Change directory to that of session file
                fs::path currentDir = fileName;
                currentDir = fs::absolute(currentDir);
                currentDir = currentDir.parent_path();
                fl_chdir(currentDir.u8string().c_str());

                // Get session version
                int version = session["version"];

                // Close all first, but don't exit.
                close_all_cb(nullptr, ui);
                
                // Turn off auto-playback temporarily
                bool autoPlayback = ui->uiPrefs->uiPrefsAutoPlayback->value();
                ui->uiPrefs->uiPrefsAutoPlayback->value(false);

                for (const auto& j : session["files"])
                {
                    FilesModelItem item;
                    j.get_to(item);

                    std::string path = item.path.get();
                    std::string audioPath;
                    if (!item.audioPath.isEmpty())
                        audioPath = item.audioPath.get();

                    replace_path(path);
                    if (!audioPath.empty())
                        replace_path(audioPath);

                    app->open(path, audioPath);

                    // Copy annotations to both item and player
                    auto Aitem = model->observeA()->get();
                    if (!Aitem)
                        continue;

                    Aitem->ocioIcs = item.ocioIcs;
                    if (!Aitem->ocioIcs.empty())
                        ocio::setIcs(Aitem->ocioIcs);
                    Aitem->annotations = item.annotations;
                    Aitem->voiceAnnotations = item.voiceAnnotations;
                    Aitem->videoLayer = item.videoLayer;
                    Aitem->currentTime = item.currentTime;
                    Aitem->inOutRange = item.inOutRange;

                    ui->uiColorChannel->value(item.videoLayer);
                    ui->uiColorChannel->do_callback();

                    auto player = view->getTimelinePlayer();
                    if (player)
                    {
                        player->setAllAnnotations(item.annotations);
                        player->setInOutRange(Aitem->inOutRange);
                        player->seek(Aitem->currentTime);
                    }
                }

                ui->uiPrefs->uiPrefsAutoPlayback->value(autoPlayback);

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

                unsigned numFiles = model->observeFiles()->getSize();

                // Decode ICS
                if (version >= 2)
                {
                    //
                    // Handle color channel layer
                    //
                    int layer = session["layer"];
                    if (layer < ui->uiColorChannel->size())
                    {
                        ui->uiColorChannel->value(layer);
                        ui->uiColorChannel->do_callback();
                    }

                    //
                    // Handle OCIO
                    //
                    j = session["ocio"];
                    std::string config = j["config"];

                    if (config.substr(0, 7) != "ocio://")
                    {
                        replace_path(config);

                        if (file::isReadable(config))
                            ui->uiPrefs->uiPrefsOCIOConfig->value(
                                config.c_str());
                    }
                    else
                    {
                        ui->uiPrefs->uiPrefsOCIOConfig->value(config.c_str());
                    }

                    ocio::setup();

                    int Aindex = session["Aindex"];
                    if (Aindex < numFiles)
                        model->setA(Aindex);

                    int value;
                    auto Aitem = model->observeA()->get();
                    if (!Aitem || Aitem->ocioIcs.empty())
                    {
                        if (j["ics"].type() == nlohmann::json::value_t::string)
                        {
                            std::string ics;
                            j.at("ics").get_to(ics);
                            ocio::setIcs(ics);
                        }
                        else
                        {
                            value = j["ics"];
                            ui->uiICS->value(value);
                        }
                    }

                    if (j["view"].type() == nlohmann::json::value_t::string)
                    {
                        std::string view;
                        j.at("view").get_to(view);
                        ocio::setView(view);
                    }
                    else
                    {
                        value = j["view"];
                        ui->uiOCIOView->value(value);
                    }

                    if (j.contains("look"))
                    {
                        std::string look;
                        j.at("look").get_to(look);
                        ocio::setLook(look);
                    }

                    view->updateOCIOOptions();

                    // Hide Panels and Windows
                    removePanels(ui);
                    removeWindows(ui);
                    
                    j = session["settings"];

                    auto settings = app->settings();

                    for (const auto& item : j.items())
                    {
                        const std::string& key = item.key();
                        Message value = item.value();
                        Message::value_t type = value.type();
                        switch (type)
                        {
                        case Message::value_t::boolean:
                            settings->setValue(key, value.get<bool>());
                            continue;
                        case Message::value_t::number_unsigned:
                        {
                            int v = value.get<unsigned>();
                            settings->setValue(key, v);
                            continue;
                        }
                        case Message::value_t::number_integer:
                            settings->setValue(key, value.get<int>());
                            continue;
                        case Message::value_t::number_float:
                        {
                            std::any val = settings->getValue<std::any>(key);
                            if (!std_any_empty(val))
                            {
                                try
                                {
                                    double v = std::any_cast<double>(val);
                                    settings->setValue(
                                        key, value.get<double>());
                                }
                                catch (const std::bad_cast& e)
                                {
                                    settings->setValue(key, value.get<float>());
                                }
                            }
                            else
                            {
                                settings->setValue(key, value.get<float>());
                            }
                            continue;
                        }
                        case Message::value_t::string:
                            settings->setValue(key, value.get<std::string>());
                            continue;
                        default:
                            LOG_ERROR(
                                "Unknown Message::value_t for key " << key);
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
                    EnvironmentMapOptions env =
                        session["environmentMapOptions"];
                    view->setEnvironmentMapOptions(env);

                    timeline::CompareOptions compare =
                        session["compareOptions"];
                    model->setCompareOptions(compare);

                    int stereoIndex = session["stereoIndex"];
                    model->setStereo(stereoIndex);

                    Stereo3DOptions stereo = session["stereo3DOptions"];
                    model->setStereo3DOptions(stereo);
                }

                if (version >= 4)
                {
                    timeline::DisplayOptions display =
                        session["displayOptions"];
                    app->setDisplayOptions(display);
                }

                if (version >= 6)
                {
                    if (ui->uiBottomBar->visible())
                    {
                        EditMode editMode = session["editMode"];
                        set_edit_mode_cb(editMode, ui);
                    }
                    else
                    {
                        set_edit_mode_cb(EditMode::kNone, ui);
                    }
                }

                if (version >= 7)
                {
                    session["metadata"].get_to(sessionMetadata);
                }

                if (version >= 8)
                {
                    timeline::BackgroundOptions background =
                        session["backgroundOptions"];
                    view->setBackgroundOptions(background);
                }

                if (version >= 9)
                {
                    Message timelineViewport = session["timelineViewport"];
                    auto options = timelineViewport;
                    bool isEditable = timelineViewport["editable"];
                    ui->uiTimeline->setEditable(isEditable);
                    ui->uiTimeline->setItemOptions(options);
                }

                if (version >= 10)
                {
                    timeline::ImageOptions imageOptions =
                        session["imageOptions"];
                    app->setImageOptions(imageOptions);
                }
                
                if (version >= 2)
                {
                    std::vector<int> Bindexes = session["Bindexes"];
                    model->clearB();
                    for (auto i : Bindexes)
                    {
                        if (i < numFiles)
                            model->setB(i, true);
                    }

                    Message j = session["timeline"];

                    auto tmp = j["annotations"];

                    std::vector< std::shared_ptr<draw::Annotation> >
                        annotations;
                    for (const auto& value : tmp)
                    {
                        auto annotation = draw::messageToAnnotation(value);
                        annotations.push_back(annotation);
                    }


                    auto player = view->getTimelinePlayer();
                    if (player)
                    {
                        player->setAllAnnotations(annotations);

                        if (version >= 16)
                        {
                            std::vector< std::shared_ptr<voice::Annotation> >
                                voiceAnnotations;
                            auto tmp = j["voiceAnnotations"];
                            for (const auto& value : tmp)
                            {
                                auto annotation = voice::messageToAnnotation(value);
                                voiceAnnotations.push_back(annotation);
                            }
                    
                            player->setAllAnnotations(voiceAnnotations);
                        }

                        if (version >= 11)
                        {
                            otime::TimeRange inOutRange;
                            j["inOutRange"].get_to(inOutRange);
                            player->setInOutRange(inOutRange);

                            auto timeRange = player->timeRange();
                            if (inOutRange != timeRange)
                            {
                                TimelineClass* c = ui->uiTimeWindow;
                                if (inOutRange.start_time() !=
                                    timeRange.start_time())
                                {
                                    c->uiStartFrame->setTime(
                                        inOutRange.start_time());
                                    c->uiStartButton->value(1);
                                }

                                if (inOutRange.end_time_exclusive() !=
                                    timeRange.end_time_exclusive())
                                {
                                    c->uiEndFrame->setTime(
                                        inOutRange.end_time_exclusive() -
                                        otime::RationalTime(
                                            1.0, inOutRange.duration().rate()));
                                    c->uiEndButton->value(1);
                                }
                            }
                        }

                        otime::RationalTime time;
                        j["time"].get_to(time);
                        player->seek(time);

                        if (version >= 3)
                        {
                            timeline::Playback playback;
                            if (j["playback"].type() ==
                                nlohmann::json::value_t::string)
                            {
                                j.at("playback").get_to(playback);
                            }
                            else
                            {
                                int v;
                                j.at("playback").get_to(v);
                                playback = static_cast<timeline::Playback>(v);
                            }
                            view->setPlayback(playback);
                        }

                        // Set up the viewport attributes
                        if (version >= 15)
                        {

                            // Get the view attributes
                            auto viewPos = session["viewPos"];
                            double viewZoom = session["viewZoom"];
                            view->setViewPosAndZoom(viewPos, viewZoom);
                        
                            bool frameView = session["frameView"];
                            view->setFrameView(frameView);
                            view->redraw();
                            
                            // If we are running command-line, wait a tad for
                            // the callbacks and the main resize of window
                            // function to update.
                            Fl::wait(1);
                        }

                        if (version >= 17)
                        {
                            auto actionMode = session["actionMode"];
                            view->setActionMode(actionMode);
                        }
                        
                    }
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            enable_cypher(true);
            ui->uiTimeline->redraw();
            ui->uiMain->fill_menu(ui->uiMenuBar);

            // Change current session filename unless it is temp.mrv2s.
            if (! string::ends_with(fileName, "temp.mrv2s") &&
                ! string::ends_with(fileName, "lang.mrv2s"))
                setCurrent(fileName);
                
            
            return true;
        }

        const std::map<std::string, std::string>& metadata()
        {
            return sessionMetadata;
        }

        const std::string& metadata(const std::string& key)
        {
            static std::string empty;
            try
            {
                return sessionMetadata.at(key);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
                return empty;
            }
        }

        void setMetadata(const std::string& key, const std::string& value)
        {
            sessionMetadata[key] = value;
        }

        void setMetadata(const std::map<std::string, std::string>& value)
        {
            sessionMetadata = value;
        }

        void clearMetadata()
        {
            sessionMetadata.clear();
        }

    } // namespace session
} // namespace mrv
