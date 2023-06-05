
#include <fstream>

#include "mrvNetwork/mrvCypher.h"
#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvFilesModelItem.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

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

    bool save_session(const std::string& fileName)
    {
        ViewerUI* ui = App::ui;
        App* app = ui->app;
        auto model = app->filesModel();
        auto files_ptrs = model->observeFiles()->get();

        enable_cypher(false);

        Message session;
        session["version"] = 2;

        std::vector< FilesModelItem > files;
        for (const auto& file : files_ptrs)
        {
            files.push_back(*file.get());
        }
        session["files"] = files;
        session["Aindex"] = model->observeAIndex()->get();
        session["Bindexes"] = model->observeBIndexes()->get();

        auto player = ui->uiView->getTimelinePlayer();

        Message timeline;
        Message annotation;
        Message time;

        if (player)
        {
            auto annotations = player->getAllAnnotations();
            std::vector< tl::draw::Annotation > jAnnotations;
            for (const auto& ann : annotations)
            {
                jAnnotations.push_back(*(ann.get()));
            }
            annotation = jAnnotations;
            time = player->currentTime();
        }

        timeline["annotations"] = annotation;
        timeline["time"] = time;

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
            {"Python", (pythonPanel != nullptr)},
            {"Network", (networkPanel != nullptr)},
            {"Histogram", (histogramPanel != nullptr)},
            {"Vectorscope", (vectorscopePanel != nullptr)},
            {"Stereo 3D", (stereo3DPanel != nullptr)},
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
        if (networkPanel)
            networkPanel->save();
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
                float tmpF = std_any_cast< float >(value);
                settings[key] = tmpF;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std_any_cast< int >(value);
                settings[key] = tmp;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                bool tmp = std_any_cast< bool >(value);
                settings[key] = tmp;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string& tmpS = std_any_cast< std::string >(value);
                settings[key] = tmpS;
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string tmpS = std_any_cast< char* >(value);
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

        session["ui"] = bars;
        session["panels"] = panels;
        session["timeline"] = timeline;
        session["ocio"] = ocio;
        session["layer"] = layer;
        session["settings"] = settings;

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
            LOG_ERROR(_("Failed to open the file for writing."));
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

            auto player = view->getTimelinePlayer();
            if (player)
                player->setAllAnnotations(item.annotations);
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

            std::vector< std::shared_ptr<tl::draw::Annotation> > annotations;
            for (const auto& value : tmp)
            {
                auto annotation = tl::draw::messageToAnnotation(value);
                annotations.push_back(annotation);
            }

            auto player = view->getTimelinePlayer();
            if (player)
            {
                player->setAllAnnotations(annotations);

                tmp = j["time"];
                if (!tmp.is_null())
                {
                    otime::RationalTime time;
                    tmp.get_to(time);
                    player->seek(time);
                }
            }

            TimelineClass* c = ui->uiTimeWindow;
            c->uiTimeline->redraw();
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
                            double v = std_any_cast<double>(val);
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
        for (const auto& item : j.items())
        {
            const std::string& panel = item.key();
            if (item.value())
            {
                show_window_cb(_(panel.c_str()), ui);
            }
        }

        // std::cout << session << std::endl;

        enable_cypher(true);
        ui->uiMain->fill_menu(ui->uiMenuBar);

        return true;
    }

} // namespace mrv
