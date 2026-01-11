// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include <tlTimeline/OCIOOptions.h>

#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvIO.h"

#include "mrvOptions/mrvLUTOptions.h"

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvString.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "ocio";

    static std::string kInactive = _("None");

} // namespace

namespace mrv
{
    namespace ocio
    {
        std::string ocioDefault = "ocio://default";
        std::vector<std::string> inputs;
        std::vector<std::string> looks;
        std::vector<std::string> views;

        OCIO::ConstConfigRcPtr OCIOconfig;

        void setup()
        {
            inputs.clear();
            looks.clear();
            views.clear();

            inputs.push_back(kInactive.c_str());
            looks.push_back(kInactive.c_str());
            views.push_back(kInactive.c_str());

            std::string defaultDisplay;
            std::string defaultView;

#ifdef TLRENDER_OCIO
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;

            const char* var = uiPrefs->uiPrefsOCIOConfig->value();
            if (var && strlen(var) > 0)
            {
                setConfig(var);

                try
                {
                    const char* configName =
                        uiPrefs->uiPrefsOCIOConfig->value();
                    OCIOconfig = OCIO::Config::CreateFromFile(configName);
                    uiPrefs->uiPrefsOCIOConfig->tooltip(
                        OCIOconfig->getDescription());

                    defaultDisplay = OCIOconfig->getDefaultDisplay();
                    defaultView =
                        OCIOconfig->getDefaultView(defaultDisplay.c_str());

                    bool use_active = uiPrefs->uiOCIOUseActiveViews->value();

                    std::vector<std::string> active_displays;
                    const char* displaylist = OCIOconfig->getActiveDisplays();
                    if (use_active && displaylist && strlen(displaylist) > 0)
                    {
                        active_displays = string::split(displaylist, ',');

                        // Eliminate forward spaces in names
                        for (unsigned i = 0; i < active_displays.size(); ++i)
                        {
                            while (active_displays[i][0] == ' ')
                                active_displays[i] = active_displays[i].substr(
                                    1, active_displays[i].size());
                        }
                    }
                    else
                    {
                        int numDisplays = OCIOconfig->getNumDisplays();
                        for (int i = 0; i < numDisplays; ++i)
                        {
                            active_displays.push_back(
                                OCIOconfig->getDisplay(i));
                        }
                    }

                    std::vector<std::string> active_views;
                    const char* viewlist = OCIOconfig->getActiveViews();
                    if (use_active && viewlist && strlen(viewlist) > 0)
                    {
                        active_views = string::split(viewlist, ',');

                        // Eliminate forward spaces in names
                        for (unsigned i = 0; i < active_views.size(); ++i)
                        {
                            while (active_views[i][0] == ' ')
                                active_views[i] = active_views[i].substr(
                                    1, active_views[i].size());
                        }
                    }

                    size_t num_active_displays = active_displays.size();
                    size_t num_active_views = active_views.size();

                    for (size_t j = 0; j < num_active_displays; ++j)
                    {
                        std::string display = active_displays[j];
                        std::string quoted_display =
                            string::commentCharacter(display, '/');

                        int numViews = OCIOconfig->getNumViews(display.c_str());

                        // Collect all views

                        if (num_active_views)
                        {
                            for (size_t h = 0; h < num_active_views; ++h)
                            {
                                std::string view;
                                bool add = false;

                                for (int i = 0; i < numViews; ++i)
                                {
                                    view =
                                        OCIOconfig->getView(display.c_str(), i);
                                    if (active_views[h] == view)
                                    {
                                        add = true;
                                        break;
                                    }
                                }

                                if (add)
                                {
                                    std::string name;
                                    if (num_active_displays > 1)
                                    {
                                        name = quoted_display;
                                        name += "/";
                                        name += view;
                                    }
                                    else
                                    {
                                        name = view;
                                        name += " (" + quoted_display + ")";
                                    }
                                    ocio::views.push_back(name);
                                }
                            }
                        }
                        else
                        {
                            for (int i = 0; i < numViews; i++)
                            {
                                std::string view =
                                    OCIOconfig->getView(display.c_str(), i);

                                std::string name;
                                if (num_active_displays > 1)
                                {
                                    name = quoted_display;
                                    name += "/";
                                    name += view;
                                }
                                else
                                {
                                    name = view;
                                    name += " (" + quoted_display + ")";
                                }

                                ocio::views.push_back(name);
                            }
                        }
                    }

                    // Add looks
                    int numLooks = OCIOconfig->getNumLooks();
                    for (int i = 0; i < numLooks; ++i)
                    {
                        looks.push_back(OCIOconfig->getLookNameByIndex(i));
                    }

                    std::vector< std::string > spaces;
                    for (int i = 0; i < OCIOconfig->getNumColorSpaces(); ++i)
                    {
                        std::string csname =
                            OCIOconfig->getColorSpaceNameByIndex(i);
                        spaces.push_back(csname);
                    }

                    if (std::find(
                            spaces.begin(), spaces.end(),
                            OCIO::ROLE_SCENE_LINEAR) == spaces.end())
                    {
                        spaces.push_back(OCIO::ROLE_SCENE_LINEAR);
                    }

                    std::sort(spaces.begin(), spaces.end());
                    size_t idx = 0;
                    const char delim{'/'};
                    const char escape{'\\'};
                    for (size_t i = 0; i < spaces.size(); ++i)
                    {
                        std::string space = spaces[i];
                        OCIO::ConstColorSpaceRcPtr cs =
                            OCIOconfig->getColorSpace(space.c_str());
                        const char* family = cs->getFamily();
                        std::string menu;
                        if (family && strlen(family) > 0)
                        {
                            menu = family;
                            menu += "/";
                        }
                        menu += string::commentCharacter(space, '/');
                        inputs.push_back(menu);
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            }

#endif

            // Update UI
            ui->uiICS->clear();
            ui->uiOCIOView->clear();
            ui->uiOCIOLook->clear();
            for (const auto& value : inputs)
            {
                ui->uiICS->add(value.c_str());
            }

            for (const auto& value : views)
            {
                ui->uiOCIOView->add(value.c_str());
            }

            for (const auto& value : looks)
            {
                ui->uiOCIOLook->add(value.c_str());
            }

            std::string display_view = uiPrefs->uiOCIO_Display_View->value();
            if (uiPrefs->uiOCIOUseDefaultDisplayView->value())
            {
                // Set the default ocio display/view as found in config.ocio
                // file.
                display_view = combineView(defaultDisplay, defaultView);
            }

            std::string view_prefs = uiPrefs->uiOCIO_Display_View->value();
            if (!view_prefs.empty())
                display_view = view_prefs;

            // Set defaults if available in preferences
            std::string look = uiPrefs->uiOCIO_Look->value();
            try
            {
                setLook(look);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            try
            {
                setView(display_view);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            defaultIcs();
        }

        void defaultIcs()
        {
            ViewerUI* ui = App::ui;
            auto player = ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            const auto& tplayer = player->player();
            const auto& info = tplayer->getIOInfo();
            const auto& videos = info.video;
            if (videos.empty())
                return;

            PreferencesUI* uiPrefs = ui->uiPrefs;
            const auto& video = info.video[0];
            tl::image::PixelType pixelType = video.pixelType;
            std::string ics;
            switch (pixelType)
            {
            case tl::image::PixelType::L_U8:
            case tl::image::PixelType::LA_U8:
            case tl::image::PixelType::RGB_U8:
            case tl::image::PixelType::RGB_U10:
            case tl::image::PixelType::RGBA_U8:
            case tl::image::PixelType::YUV_420P_U8:
            case tl::image::PixelType::YUV_422P_U8:
            case tl::image::PixelType::YUV_444P_U8:
                ics = uiPrefs->uiOCIO_8bits_ics->value();
                break;
            case tl::image::PixelType::L_U16:
            case tl::image::PixelType::LA_U16:
            case tl::image::PixelType::RGB_U16:
            case tl::image::PixelType::RGBA_U16:
            case tl::image::PixelType::YUV_420P_U16:
            case tl::image::PixelType::YUV_422P_U16:
            case tl::image::PixelType::YUV_444P_U16:
                ics = uiPrefs->uiOCIO_16bits_ics->value();
                break;
            case tl::image::PixelType::L_U32:
            case tl::image::PixelType::LA_U32:
            case tl::image::PixelType::RGB_U32:
            case tl::image::PixelType::RGBA_U32:
                ics = uiPrefs->uiOCIO_32bits_ics->value();
                break;
                // handle half and float types
            case tl::image::PixelType::L_F16:
            case tl::image::PixelType::LA_F16:
            case tl::image::PixelType::RGB_F16:
            case tl::image::PixelType::RGBA_F16:
                ics = uiPrefs->uiOCIO_half_ics->value();
                break;
            case tl::image::PixelType::L_F32:
            case tl::image::PixelType::LA_F32:
            case tl::image::PixelType::RGB_F32:
            case tl::image::PixelType::RGBA_F32:
                ics = uiPrefs->uiOCIO_float_ics->value();
                break;
            default:
                break;
            }

            try
            {
                if (!ics.empty())
                    setIcs(ics);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }

        std::string config()
        {
            ViewerUI* ui = App::ui;
            PreferencesUI* uiPrefs = ui->uiPrefs;
            const char* out = uiPrefs->uiPrefsOCIOConfig->value();
            if (!out)
                return "";
            return out;
        }

        void setConfig(const std::string config)
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
                    /* xgettext:c++-format */
                    const std::string err =
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
            ocio::setup();
        }

        std::string ics()
        {
            auto uiICS = App::ui->uiICS;
            int idx = uiICS->value();
            if (idx <= 0 || idx >= uiICS->children())
                return kInactive;

            const Fl_Menu_Item* item = uiICS->child(idx);
            if (!item || !item->label() || item->flags & FL_SUBMENU)
                return "";

            std::string ics = item->label();
            
            // char pathname[1024];
            // int ret = uiICS->item_pathname(pathname, 1024, item);
            // if (ret != 0)
            //     return kInactive;

            // std::string ics = pathname;
            // if (ics[0] == '/')
            //     ics = ics.substr(1, ics.size());

            return ics;
        }

        void setIcs(const std::string& name)
        {
            auto uiICS = App::ui->uiICS;

            int value = -1;
            if (name.empty() || name == kInactive ||
                name == _(kInactive.c_str()))
            {
                uiICS->value(0);
                uiICS->do_callback();
                return;
            }

            for (int i = 0; i < uiICS->children(); ++i)
            {
                const Fl_Menu_Item* item = uiICS->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiICS->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                std::string path = pathname;
                if (path[0] == '/')
                    path = path.substr(1, path.size());

                if (name == path)
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                for (int i = 0; i < uiICS->children(); ++i)
                {
                    const Fl_Menu_Item* item = uiICS->child(i);
                    if (!item || !item->label() || item->flags & FL_SUBMENU)
                        continue;

                    if (name == item->label())
                    {
                        value = i;
                        break;
                    }
                }
                if (value == -1)
                {
                    /* xgettext:c++-format */
                    const std::string err =
                        string::Format(_("Invalid OCIO Ics '{0}'.")).arg(name);
                    throw std::runtime_error(err);
                }
            }
            uiICS->value(value);
            uiICS->do_callback();
        }

        int icsIndex(const std::string& name)
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

                std::string path = pathname;
                if (path[0] == '/')
                    path = path.substr(1, path.size());

                if (name == path)
                {
                    value = i;
                    break;
                }
            }
            return value;
        }

        std::string look()
        {
            auto uiOCIOLook = App::ui->uiOCIOLook;
            int idx = uiOCIOLook->value();
            if (idx <= 0 || idx >= uiOCIOLook->children())
                return kInactive;

            const Fl_Menu_Item* item = uiOCIOLook->child(idx);
            char pathname[1024];
            int ret = uiOCIOLook->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return kInactive;

            std::string name = pathname;
            if (name[0] == '/')
                name = name.substr(1, name.size());
            return name;
        }

        void setLook(const std::string& name)
        {
            auto uiOCIOLook = App::ui->uiOCIOLook;

            int value = -1;
            if (name.empty() || name == kInactive ||
                name == _(kInactive.c_str()))
            {
                uiOCIOLook->value(-1);
                uiOCIOLook->do_callback();
                return;
            }

            for (int i = 0; i < uiOCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOLook->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiOCIOLook->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                std::string path = pathname;
                if (path[0] == '/')
                    path = path.substr(1, path.size());

                if (name == path)
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                /* xgettext:c++-format */
                const std::string err =
                    string::Format(_("Invalid OCIO Look '{0}'.")).arg(name);
                throw std::runtime_error(err);
                return;
            }
            uiOCIOLook->value(value);
            uiOCIOLook->do_callback();
        }

        int lookIndex(const std::string& name)
        {
            auto uiOCIOLook = App::ui->uiOCIOLook;
            int value = -1;
            for (int i = 0; i < uiOCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOLook->child(i);
                if (!item || !item->label() || item->flags & FL_SUBMENU)
                    continue;

                char pathname[1024];
                int ret = uiOCIOLook->item_pathname(pathname, 1024, item);
                if (ret != 0)
                    continue;

                std::string path = pathname;
                if (path[0] == '/')
                    path = path.substr(1, path.size());

                if (name == path)
                {
                    value = i;
                    break;
                }
            }
            return value;
        }

        std::string view()
        {
            auto uiOCIOView = App::ui->uiOCIOView;
            int idx = uiOCIOView->value();
            if (idx <= 0 || idx >= uiOCIOView->children())
                return kInactive;

            const Fl_Menu_Item* item = uiOCIOView->child(idx);

            char pathname[1024];
            int ret = uiOCIOView->item_pathname(pathname, 1024, item);
            if (ret != 0)
                return kInactive;

            std::string view = pathname;
            if (view[0] == '/')
                view = view.substr(1, view.size());
            return view;
        }

        void setView(const std::string& name)
        {
            auto uiOCIOView = App::ui->uiOCIOView;
            if (name.empty() || name == kInactive ||
                name == _(kInactive.c_str()))
            {
                uiOCIOView->value(0);
                uiOCIOView->do_callback();
                return;
            }

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

                std::string path = pathname;
                if (path[0] == '/')
                    path = path.substr(1, path.size());

                if (name == path)
                {
                    value = i;
                    break;
                }
            }
            if (value == -1)
            {
                for (int i = 0; i < uiOCIOView->children(); ++i)
                {
                    const Fl_Menu_Item* item = uiOCIOView->child(i);
                    if (!item || !item->label() || (item->flags & FL_SUBMENU))
                        continue;
                    if (name == item->label())
                    {
                        value = i;
                        break;
                    }
                }
            }
            if (value == -1)
            {
                /* xgettext:c++-format */
                const std::string err =
                    string::Format(_("Invalid OCIO Display/View '{0}'."))
                        .arg(name);
                throw std::runtime_error(err);
            }
            uiOCIOView->value(value);
            uiOCIOView->do_callback();
        }

        std::string
        combineView(const std::string& display, const std::string& view)
        {
            if (display.empty() || view.empty() || view == kInactive)
                return kInactive;

            std::string out;
            auto uiOCIOView = App::ui->uiOCIOView;
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

        void splitView(
            const std::string& combined, std::string& display,
            std::string& view)
        {

            if (combined.empty() || combined == kInactive)
            {
                display.clear();
                view.clear();
                return;
            }

            view = combined;
            size_t pos = view.rfind('/');
            if (pos != std::string::npos)
            {
                display = view.substr(0, pos);
                view = view.substr(pos + 1, view.size());
            }
            else
            {
                pos = view.find('(');
                if (pos == std::string::npos)
                {
                    /* xgettext:c++-format */
                    const std::string& err =
                        string::Format(
                            _("Could not split '{0}' into display and view."))
                            .arg(combined);
                    throw std::runtime_error(err);
                }

                display = view.substr(pos + 1, view.size());
                view = view.substr(0, pos - 1);
                pos = display.find(')');
                display = display.substr(0, pos);
            }
        }

        int viewIndex(const std::string& displayViewName)
        {
            int value = -1;
            auto uiOCIOView = App::ui->uiOCIOView;
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

        std::vector<std::string> icsList()
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

        std::vector<std::string> lookList()
        {
            auto uiOCIOLook = App::ui->uiOCIOLook;
            std::vector<std::string> out;
            for (int i = 0; i < uiOCIOLook->children(); ++i)
            {
                const Fl_Menu_Item* item = uiOCIOLook->child(i);
                if (!item || !item->label())
                    continue;

                out.push_back(item->label());
            }
            return out;
        }

        std::vector<std::string> viewList()
        {
            auto uiOCIOView = App::ui->uiOCIOView;
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

        void setDisplay(const std::string& display)
        {
            auto defaultView = OCIOconfig->getDefaultView(display.c_str());
            if (!defaultView || strlen(defaultView) == 0)
            {
                /* xgettext:c++-format */
                const std::string err =
                    string::Format(_("No default view for display "
                                     "'{0}'.  Does display exist?"))
                        .arg(display);
                throw std::runtime_error(err);
            }
            auto display_view = combineView(display, defaultView);
            setView(display_view);
        }

        struct OCIODefaults
        {
            std::string bits8;
            std::string bits16;
            std::string bits32;
            std::string half;
            std::string flt;
        };

        struct OCIOPreset
        {
            std::string name;

            timeline::OCIOOptions ocio;
            timeline::LUTOptions lut;

            std::vector<timeline::OCIOOptions> ocioMonitors;

            OCIODefaults defaults;
        };

        void to_json(nlohmann::json& j, const OCIODefaults& value)
        {
            j = nlohmann::json{
                {"8-bits", value.bits8},   {"16-bits", value.bits16},
                {"32-bits", value.bits32}, {"half", value.half},
                {"float", value.flt},
            };
        }

        void from_json(const nlohmann::json& j, OCIODefaults& value)
        {
            j.at("8-bits").get_to(value.bits8);
            j.at("16-bits").get_to(value.bits16);
            j.at("32-bits").get_to(value.bits32);
            j.at("half").get_to(value.half);
            j.at("float").get_to(value.flt);
        }

        void to_json(nlohmann::json& j, const OCIOPreset& value)
        {
            j = nlohmann::json{
                {"name", value.name},
                {"ocio", value.ocio},
                {"lut", value.lut},
                {"defaults", value.defaults},
                {"ocioMonitors", value.ocioMonitors},
            };
        }

        void from_json(const nlohmann::json& j, OCIOPreset& value)
        {
            j.at("name").get_to(value.name);
            j.at("ocio").get_to(value.ocio);
            j.at("lut").get_to(value.lut);
            j.at("defaults").get_to(value.defaults);
            if (j.contains("ocioMonitors"))
                j.at("ocioMonitors").get_to(value.ocioMonitors);
        }

        std::vector<OCIOPreset> presets;

        std::vector<std::string> presetsList()
        {
            std::vector<std::string> out;
            for (const auto& preset : presets)
            {
                out.push_back(preset.name);
            }
            return out;
        }

        std::string presetSummary(const std::string& presetName)
        {
            std::stringstream s;
            for (auto& preset : presets)
            {
                if (preset.name == presetName)
                {
                    timeline::OCIOOptions& ocio = preset.ocio;
                    const timeline::LUTOptions& lut = preset.lut;
                    const OCIODefaults& d = preset.defaults;

                    s << "OCIO:" << std::endl
                      << "\t  config: " << ocio.fileName << std::endl
                      << "\t     ICS: " << ocio.input << std::endl
                      << "\t    look: " << ocio.look << std::endl;

                    bool found = false;
                    for (auto ocio : preset.ocioMonitors)
                    {
                        if (ocio.view.empty())
                            continue;
                        found = true;
                        break;
                    }

                    if (found)
                    {
                        unsigned idx = 0;
                        for (auto ocio : preset.ocioMonitors)
                        {

                            ++idx;
                            s << "Monitor " << idx << ":" << std::endl
                              << "\t display: " << ocio.display << std::endl
                              << "\t    view: " << ocio.view << std::endl;
                        }
                    }
                    else
                    {
                        int num_screens = Fl::screen_count();
                        if (num_screens > 1)
                        {
                            const auto& monitor_ocio =
                                App::ui->uiView->getOCIOOptions(0);
                            ocio.display = monitor_ocio.display;
                            ocio.view = monitor_ocio.view;
                        }
                        s << "\t display: " << ocio.display << std::endl
                          << "\t    view: " << ocio.view << std::endl;
                    }

                    s << "LUT:" << std::endl
                      << "\tfileName: " << lut.fileName << std::endl
                      << "\t   order: " << lut.order << std::endl
                      << "Defaults:" << std::endl
                      << "\t  8-bits: " << d.bits8 << std::endl
                      << "\t 16-bits: " << d.bits16 << std::endl
                      << "\t 32-bits: " << d.bits32 << std::endl
                      << "\t    half: " << d.half << std::endl
                      << "\t   float: " << d.flt << std::endl;
                }
            }
            return s.str();
        }

        void setPreset(const std::string& presetName)
        {
            for (const auto& preset : presets)
            {
                if (preset.name == presetName)
                {
                    /* xgettext:c++-format */
                    std::string msg =
                        string::Format(_("Setting OCIO Preset '{0}'."))
                            .arg(presetName);
                    LOG_STATUS(msg);

                    const timeline::OCIOOptions& ocio = preset.ocio;
                    setConfig(ocio.fileName);
                    setIcs(ocio.input);
                    std::string view = combineView(ocio.display, ocio.view);
                    setView(view);
                    setLook(ocio.look);

                    for (unsigned i = 0; i < preset.ocioMonitors.size(); ++i)
                    {
                        const auto& ocio = preset.ocioMonitors[i];
                        App::ui->uiView->setOCIOOptions(i, ocio);
                    }

                    App::app->setLUTOptions(preset.lut);
                    return;
                }
            }

            /* xgettext:c++-format */
            const std::string msg =
                string::Format(_("Preset '{0}' not found.")).arg(presetName);
            LOG_ERROR(msg);
        }

        void createPreset(const std::string& presetName)
        {
            for (const auto& preset : presets)
            {
                if (preset.name == presetName)
                {
                    /* xgettext:c++-format */
                    std::string msg =
                        string::Format(_("OCIO Preset '{0}' already exists!"))
                            .arg(presetName);
                    LOG_ERROR(msg);
                    return;
                }
            }

            auto uiPrefs = App::ui->uiPrefs;

            timeline::OCIOOptions ocio;
            ocio.enabled = true;

            ocio.fileName = ocio::config();
            ocio.input = ocio::ics();

            std::string display, view;
            std::string combined = ocio::view();
            splitView(combined, display, view);

            ocio.display = display;
            ocio.view = view;
            ocio.look = ocio::look();

            const timeline::LUTOptions& lut = App::app->lutOptions();

            OCIODefaults defaults;
            defaults.bits8 = uiPrefs->uiOCIO_8bits_ics->value();
            defaults.bits16 = uiPrefs->uiOCIO_16bits_ics->value();
            defaults.bits32 = uiPrefs->uiOCIO_32bits_ics->value();
            defaults.half = uiPrefs->uiOCIO_half_ics->value();
            defaults.flt = uiPrefs->uiOCIO_float_ics->value();

            OCIOPreset preset;
            preset.name = presetName;
            preset.ocio = ocio;
            preset.lut = lut;
            preset.defaults = defaults;

            int num_screens = Fl::screen_count();
            std::vector<timeline::OCIOOptions> ocioMonitors;
            if (num_screens > 1)
            {
                bool same = true;
                const timeline::OCIOOptions& prev =
                    App::ui->uiView->getOCIOOptions(0);

                for (int i = 1; i < num_screens; ++i)
                {
                    const timeline::OCIOOptions& ocio =
                        App::ui->uiView->getOCIOOptions(i);
                    if (prev.display != ocio.display || prev.view != ocio.view)
                    {
                        same = false;
                        break;
                    }
                }
                if (!same)
                {
                    for (int i = 0; i < num_screens; ++i)
                    {
                        const timeline::OCIOOptions& ocio =
                            App::ui->uiView->getOCIOOptions(i);
                        ocioMonitors.push_back(ocio);
                    }
                }
            }

            preset.ocioMonitors = ocioMonitors;

            presets.push_back(preset);
        }

        void removePreset(const std::string& presetName)
        {
            std::vector<OCIOPreset> out;
            bool found = false;
            for (const auto& preset : presets)
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
                /* xgettext:c++-format */
                std::string msg = string::Format(_("Preset '{0}' not found."))
                                      .arg(presetName);
                LOG_ERROR(msg);
            }
            presets = out;
        }

        bool loadPresets(const std::string& fileName)
        {
            try
            {
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

                nlohmann::json j;
                ifs >> j;

                if (ifs.fail())
                {
                    /* xgettext:c++-format */
                    const std::string& err =
                        string::Format(_("Failed to load the file '{0}'."))
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

                presets = j.get<std::vector<OCIOPreset>>();

                /* xgettext:c++-format */
                const std::string& msg =
                    string::Format(_("Loaded {0} ocio presets from \"{1}\"."))
                        .arg(presets.size())
                        .arg(fileName);
                LOG_INFO(msg);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("Error: " << e.what());
            }
            return true;
        }

        bool savePresets(const std::string& fileName)
        {
            try
            {
                std::ofstream ofs(fileName);
                if (!ofs.is_open())
                {
                    /* xgettext:c++-format */
                    const std::string& err =
                        string::Format(
                            _("Failed to open the file '{0}' for saving."))
                            .arg(fileName);
                    LOG_ERROR(err);
                    return false;
                }

                nlohmann::json j = presets;

                ofs << j.dump(4);

                if (ofs.fail())
                {
                    /* xgettext:c++-format */
                    const std::string& err =
                        string::Format(_("Failed to save the file '{0}'."))
                            .arg(fileName);
                    LOG_ERROR(err);
                    return false;
                }
                if (ofs.bad())
                {
                    LOG_ERROR(
                        _("The stream is in an unrecoverable error state."));
                    return false;
                }
                ofs.close();

                /* xgettext:c++-format */
                const std::string& msg =
                    string::Format(
                        _("OCIO presets have been saved to \"{0}\"."))
                        .arg(fileName);
                LOG_INFO(msg);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("Error: " << e.what());
            }
            return true;
        }
    } // namespace ocio
} // namespace mrv
