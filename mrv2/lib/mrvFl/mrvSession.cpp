
#include <fstream>

#include "mrvNetwork/mrvCypher.h"
#include "mrvNetwork/mrvMessage.h"
#include "mrvNetwork/mrvFilesModelItem.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

#include "mrvSession.h"

namespace
{
    const char* kModule = "m2s";
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

    void save_session(const std::string& file)
    {
        ViewerUI* ui = App::ui;
        App* app = ui->app;
        auto files_ptrs = app->filesModel()->observeFiles()->get();

        enable_cypher(false);

        Message session;
        session["version"] = 1;

        std::vector< FilesModelItem > files;
        for (const auto& file : files_ptrs)
        {
            files.push_back(*file.get());
        }
        session["files"] = files;

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
            {"Logs", (logsPanel != nullptr)},
        };

        session["ui"] = bars;
        session["panels"] = panels;

        std::ofstream ofs(file);
        if (!ofs.is_open())
        {
            LOG_ERROR(_("Failed to open the file for writing."));
            return;
        }

        ofs << std::setw(4) << session << std::endl;

        if (ofs.fail())
        {
            LOG_ERROR(_("Failed to write to the file."));
            return;
        }
        if (ofs.bad())
        {
            LOG_ERROR(_("The stream is in an unrecoverable error state."));
            return;
        }
        ofs.close();

        enable_cypher(true);
    }

    void load_session(const std::string& file)
    {
        ViewerUI* ui = App::ui;
        App* app = ui->app;

        std::ifstream ifs(file);
        if (!ifs.is_open())
        {
            LOG_ERROR(_("Failed to open the file for writing."));
            return;
        }

        Message session;
        enable_cypher(false);

        ifs >> session;

        if (ifs.fail())
        {
            LOG_ERROR(_("Failed to write to the file."));
            return;
        }
        if (ifs.bad())
        {
            LOG_ERROR(_("The stream is in an unrecoverable error state."));
            return;
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
            app->open(item.path.get(), item.audioPath.get());
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

        // Decode panels
        j = session["panels"];

        removePanels(ui);
        for (const auto& item : j.items())
        {
            std::string panel = item.key();
            if (item.value())
                show_window_cb(_(panel.c_str()), ui);
        }

        // std::cout << session << std::endl;

        enable_cypher(true);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

} // namespace mrv
