// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include <tlTimeline/OCIOOptions.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvFile.h"

#include "mrvFl/mrvIO.h"

#include "mrvNetwork/mrvLUTOptions.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "ocio";
}

namespace mrv
{
    namespace ocio
    {
        std::string ocioDefault = "ocio://default";

        std::string ocioConfig()
        {
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;
            const char* out = uiPrefs->uiPrefsOCIOConfig->value();
            if (!out)
                return "";
            return out;
        }

        void setOcioConfig(const std::string config)
        {
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;
            if (config.empty())
            {
                throw std::runtime_error(
                    _("OCIO config file cannot be empty."));
            }

            if (config.substr(0, 7) != "ocio://")
            {
                if (!file::isReadable(config))
                {
                    std::string err =
                        string::Format(_("OCIO config '{0}' does not "
                                         "exist or is not readable."))
                            .arg(config);
                    throw std::runtime_error(err);
                }
            }

            const char* oldconfig = uiPrefs->uiPrefsOCIOConfig->value();
            if (oldconfig && strlen(oldconfig) > 0)
            {
                // Same config file.  Nothing to do.
                if (config == oldconfig)
                    return;
            }

            uiPrefs->uiPrefsOCIOConfig->value(config.c_str());
            Preferences::OCIO(ui);
        }

        std::string ocioIcs()
        {
            auto uiICS = App::ui->uiICS;
            int idx = uiICS->value();
            if (idx < 0 || idx >= uiICS->children())
                return "";

            const Fl_Menu_Item* item = uiICS->child(idx);
            if (!item || !item->label() || item->flags & FL_SUBMENU)
                return "";

            char pathname[1024];
            int ret = uiICS->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return "";

            std::string ics = pathname;
            if (ics[0] == '/')
                ics = ics.substr(1, ics.size());

            return ics;
        }

        void setOcioIcs(const std::string& name)
        {
            auto uiICS = App::ui->uiICS;

            int value = -1;
            if (name.empty()) value = 0;
            
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiICS->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname || name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Ics '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiICS->value(value);
            uiICS->do_callback();
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }

        int ocioIcsIndex(const std::string& name)
        {
            auto uiICS = App::ui->uiICS;
            int value = -1;
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiICS->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname)
                {
                    value = i;
                    break;
                }
            }
            return value;
        }

        std::string ocioLook()
        {
            auto OCIOLook = App::ui->OCIOLook;
            int idx = OCIOLook->value();
            if (idx < 0 || idx >= OCIOLook->children())
                return "";

            const Fl_Menu_Item* item = OCIOLook->child(idx);
            return item->label();
        }

        void setOcioLook(const std::string& name)
        {
            auto OCIOLook = App::ui->OCIOLook;
            int value = -1;
            if (name.empty())
                value = 0;
            for (int i = 0; i < OCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = OCIOLook->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = OCIOLook->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname || name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Look '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            OCIOLook->value(value);
            OCIOLook->do_callback();
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }

        int ocioLookIndex(const std::string& name)
        {
            auto uiLook = App::ui->OCIOLook;
            int value = -1;
            for (int i = 0; i < uiLook->children(); ++i)
            {
                const Fl_Menu_Item* item = uiLook->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiLook->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname)
                {
                    value = i;
                    break;
                }
            }
            return value;
        }

        std::string ocioView()
        {
            auto uiOCIOView = App::ui->OCIOView;
            int idx = uiOCIOView->value();
            if (idx < 0 || idx >= uiOCIOView->children())
                return "";

            const Fl_Menu_Item* item = uiOCIOView->child(idx);

            char pathname[1024];
            int ret = uiOCIOView->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return "";
            
            std::string view = pathname;
            if (view[0] == '/')
                view = view.substr(1, view.size());
            return view;
        }

        void setOcioView(const std::string& name)
        {
            auto uiOCIOView = App::ui->OCIOView;
            int value = -1;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (name == pathname || name == item->label())
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                std::string err =
                    string::Format(_("Invalid OCIO Display/View '{0}'."))
                        .arg(name);
                throw std::runtime_error(err);
            }
            uiOCIOView->value(value);
            uiOCIOView->do_callback();
            if (panel::colorPanel)
                panel::colorPanel->refresh();
        }

        std::string ocioDisplayViewShortened(
            const std::string& display, const std::string& view)
        {
            std::string out;
            auto uiOCIOView = App::ui->OCIOView;
            bool has_submenu = false;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item)
                    continue;
                if (item->flags & FL_SUBMENU)
                {
                    has_submenu = true;
                    break;
                }
            }
            if (has_submenu)
            {
                out = display + '/' + view;
            }
            else
            {
                out = view + " (" + display + ')';
            }
            return out;
        }

        int ocioViewIndex(const std::string& displayViewName)
        {
            int value = -1;
            auto uiOCIOView = App::ui->OCIOView;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (displayViewName == pathname)
                {
                    value = i;
                    break;
                }
            }
            return value;
        }

        std::vector<std::string> ocioIcsList()
        {
            auto uiICS = App::ui->uiICS;
            std::vector<std::string> out;
            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiICS->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (pathname[0] == '/')
                    out.push_back(item->label());
                else
                    out.push_back(pathname);
            }
            return out;
        }

        std::vector<std::string> ocioLookList()
        {
            auto OCIOLook = App::ui->OCIOLook;
            std::vector<std::string> out;
            for (int i = 0; i < OCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = OCIOLook->child(i);
                if (!item || !item->label())
                    continue;

                out.push_back(item->label());
            }
            return out;
        }

        std::vector<std::string> ocioViewList()
        {
            auto uiOCIOView = App::ui->OCIOView;
            std::vector<std::string> out;
            for (int i = 0; i < uiOCIOView->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOView->child(i);
                if (!item || !item->label() || (item->flags & FL_SUBMENU))
                    continue;

                char pathname[1024];
                int ret = uiOCIOView->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                if (pathname[0] == '/')
                    out.push_back(item->label());
                else
                    out.push_back(pathname);
            }
            return out;
        }

        struct OCIOPreset
        {
            std::string name;
            
            timeline::OCIOOptions ocio;
            timeline::LUTOptions lut;
        };

        
        void to_json(nlohmann::json& j, const OCIOPreset& value)
        {
            j = nlohmann::json
            {
                {"name", value.name},
                {"ocio", value.ocio},
                {"lut",value.lut}
            };
        }

        void from_json(const nlohmann::json& j, OCIOPreset& value)
        {
            j.at("name").get_to(value.name);
            j.at("ocio").get_to(value.ocio);
            j.at("lut").get_to(value.lut);
        }

        std::vector<OCIOPreset> ocioPresets;

        std::vector<std::string> ocioPresetsList()
        {
            std::vector<std::string> out;
            for (const auto& preset : ocioPresets)
            {
                out.push_back(preset.name);
            }
            return out;
        }
        
        void setOcioPreset(const std::string& presetName)
        {
            for (const auto& preset : ocioPresets)
            {
                if (preset.name == presetName)
                {
                    std::string msg =
                        string::Format(_("Setting OCIO Preset '{0}'."))
                            .arg(presetName);
                    LOG_INFO(msg);
            
                    const timeline::OCIOOptions& ocio = preset.ocio;
                    setOcioConfig(ocio.fileName);
                    setOcioIcs(ocio.input);
                    std::string view = ocioDisplayViewShortened(ocio.display,
                                                                ocio.view);
                    setOcioView(view);
                    setOcioLook(ocio.look);

                    App::app->setLUTOptions(preset.lut);
                    return;
                }
            }

            std::string msg = string::Format(_("Preset '{0}' not found."))
                              .arg(presetName);
            LOG_ERROR(msg);
        }
        
        void createOcioPreset(const std::string& presetName)
        {
            auto view = App::ui->uiView;
            const timeline::OCIOOptions& ocio = view->getOCIOOptions();
            const timeline::LUTOptions&  lut  = App::app->lutOptions();

            OCIOPreset preset;
            preset.name = presetName;
            preset.ocio = ocio;
            preset.lut  = lut;

            ocioPresets.push_back(preset);
        }
        
        void removeOcioPreset(const std::string& presetName)
        {
            std::vector<OCIOPreset> out;
            bool found = false;
            for (const auto& preset : ocioPresets)
            {
                if (preset.name == presetName)
                {
                    found = true;
                    continue;
                }
                out.push_back(preset);
            }
            if (!found)
            {
                std::string msg = string::Format(_("Preset '{0}' not found."))
                                  .arg(presetName);
                LOG_ERROR(msg);
            }
            ocioPresets = out;
        }
        
        bool loadOcioPresets(const std::string& fileName)
        {
            return true;
        }
        
        bool saveOcioPresets(const std::string& fileName)
        {
            return true;
        }
    } // namespace ocio
} // namespace mrv
