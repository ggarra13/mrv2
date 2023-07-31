// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/StringFormat.h>

#ifdef __linux__
#    include <FL/platform.H> // for fl_wl_display
#    undef None
#endif

#include <FL/fl_utf8.h>         // for fl_getenv
#include <FL/Fl_Sys_Menu_Bar.H> // for macOS menus

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvMedia.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvFl/mrvMenus.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvFl/mrvAsk.h"

#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"
#include "mrvCore/mrvOS.h"

namespace
{
    const char* kModule = "prefs";
}

extern float kCrops[];

namespace
{

    /**
     * This function allows the user to override a preference setting by
     * using an environment variable.
     *
     * @param variable        environment variable to look for
     * @param defaultValue    default value to use if variable is not set
     * @param inPrefs         boolean specifying whether the value came from
     *                        saved preferences or not.  It is used to print
     *                        a warning if some setting is as of yet undefined.
     *
     * @return a float corresponding to the value set in the environment or to
     *         to the default value.
     */
    int environmentSetting(
        const char* variable, const int defaultValue, const bool inPrefs)
    {
        int r = defaultValue;
        const char* env = fl_getenv(variable);
        if (!env)
        {
            if (!inPrefs)
            {
                std::string msg =
                    tl::string::Format(
                        _("Environment variable \"{0}\" is not set; "
                          "using default value ({1})."))
                        .arg(variable)
                        .arg(defaultValue);
                LOG_WARNING(msg);
            }
        }
        else
        {
            int n = sscanf(env, " %d", &r);
            if (n != 1)
            {
                std::string msg =
                    tl::string::Format(
                        _("Cannnot parse enironment variable \"{0}\" "
                          "as an integer value; using {1}."))
                        .arg(variable)
                        .arg(defaultValue);
                LOG_ERROR(msg);
            }
        }
        return r;
    }

    /**
     * This function allows the user to override a preference setting by
     * using an environment variable.
     *
     * @param variable        environment variable to look for
     * @param defaultValue    default value to use if variable is not set
     * @param inPrefs         boolean specifying whether the value came from
     *                        saved preferences or not.  It is used to print
     *                        a warning if some setting is as of yet undefined.
     *
     * @return a float corresponding to the value set in the environment or to
     *         to the default value.
     */
    float environmentSetting(
        const char* variable, const float defaultValue, const bool inPrefs)
    {
        float r = defaultValue;
        const char* env = fl_getenv(variable);
        if (!env)
        {
            if (!inPrefs)
            {
                std::string msg =
                    tl::string::Format(
                        _("Environment variable \"{0}\" is not set; "
                          "using default value ({1})."))
                        .arg(variable)
                        .arg(defaultValue);
                LOG_WARNING(msg);
            }
        }
        else
        {
            int n = sscanf(env, " %f", &r);
            if (n != 1)
            {
                std::string msg =
                    tl::string::Format(
                        _("Cannnot parse enironment variable \"{0}\" "
                          "as a float value; using {1}."))
                        .arg(variable)
                        .arg(defaultValue);
                LOG_ERROR(msg);
            }
        }
        return r;
    }

    /**
     * This function allows the user to override a preference setting by
     * using an environment variable.
     *
     * @param variable        environment variable to look for
     * @param defaultValue    default value to use if variable is not set
     * @param inPrefs         boolean specifying whether the value came from
     *                        saved preferences or not.  It is used to print
     *                        a warning if some setting is as of yet undefined.
     *
     * @return a string corresponding to the value set in the environment or to
     *         to the default value.
     */
    const char* environmentSetting(
        const char* variable, const char* defaultValue, const bool inPrefs)
    {

        const char* env = fl_getenv(variable);
        if (!env || strlen(env) == 0)
        {
            env = defaultValue;
            if (!inPrefs)
            {
                std::string msg =
                    tl::string::Format(
                        _("Environment variable \"{0}\" is not set; "
                          "using default value ({1})."))
                        .arg(variable)
                        .arg(defaultValue);
                LOG_WARNING(msg);
            }
        }
        return env;
    }

} // anonymous namespace

mrv::App* ViewerUI::app = nullptr;
AboutUI* ViewerUI::uiAbout = nullptr;
PreferencesUI* ViewerUI::uiPrefs = nullptr;
HotkeyUI* ViewerUI::uiHotkey = nullptr;

namespace mrv
{
    ColorSchemes Preferences::schemes;
    bool Preferences::native_file_chooser;
    OCIO::ConstConfigRcPtr Preferences::config;
    std::string Preferences::OCIO_Display;
    std::string Preferences::OCIO_View;

    std::string Preferences::root;
    int Preferences::debug = 0;
    std::string Preferences::hotkeys_file = "mrv2.keys";

    int Preferences::language_index = 0; // English
    int Preferences::switching_images = 0;

    int Preferences::bgcolor;
    int Preferences::textcolor;
    int Preferences::selectioncolor;
    int Preferences::selectiontextcolor;

    static std::string expandVariables(
        const std::string& s, const char* START_VARIABLE,
        const char END_VARIABLE)
    {

        size_t p = s.find(START_VARIABLE);

        if (p == std::string::npos)
            return s;

        std::string pre = s.substr(0, p);
        std::string post = s.substr(p + strlen(START_VARIABLE));

        size_t e = post.find(END_VARIABLE);

        if (e == std::string::npos)
            return s;

        std::string variable = post.substr(0, e);
        std::string value = "";

        post = post.substr(e + 1);

        const char* v = fl_getenv(variable.c_str());
        if (v != NULL)
            value = std::string(v);

        return expandVariables(
            pre + value + post, START_VARIABLE, END_VARIABLE);
    }

    Preferences::Preferences(PreferencesUI* uiPrefs, bool reset)
    {
        ViewerUI* ui = App::ui;

        bool ok;
        int version;
        int tmp;
        double tmpD;
        float tmpF;
        char tmpS[2048];

        char* saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
        setlocale(LC_NUMERIC, "C");

        std::string msg =
            tl::string::Format(_("Reading preferences from \"{0}mrv2.prefs\"."))
                .arg(prefspath());

        LOG_INFO(msg);

        Fl_Preferences base(prefspath().c_str(), "filmaura", "mrv2");

        base.get("version", version, 7);

        SettingsObject* settingsObject = ViewerUI::app->settingsObject();

        Fl_Preferences fltk_settings(base, "settings");
        unsigned num = fltk_settings.entries();
        for (unsigned i = 0; i < num; ++i)
        {
            const char* key = fltk_settings.entry(i);
            if (key[1] == '#')
            {
                char type = key[0];
                std_any value;
                const char* keyS = key + 2;
                switch (type)
                {
                case 'b':
                    fltk_settings.get(key, tmp, 0);
                    value = (bool)tmp;
                    break;
                case 'i':
                    fltk_settings.get(key, tmp, 0);
                    value = tmp;
                    break;
                case 'f':
                    fltk_settings.get(key, tmpF, 0.0f);
                    value = tmpF;
                    break;
                case 'd':
                    fltk_settings.get(key, tmpD, 0.0);
                    value = tmpD;
                    break;
                case 's':
                    fltk_settings.get(key, tmpS, "", 2048);
                    value = std::string(tmpS);
                    break;
                case 'v':
                    // void values are not cleared nor stored as that can
                    // corrupt the prefs.
                    continue;
                    break;
                default:
                    LOG_ERROR("Unknown type " << type << " for key " << keyS);
                    break;
                }
                settingsObject->setValue(keyS, value);
            }
        }

        Fl_Preferences recent_files(base, "recentFiles");
        num = recent_files.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "File #%d", i);
            if (recent_files.get(buf, tmpS, "", 2048))
            {
                // Only add existing files to the list.
                if (is_readable(tmpS))
                    settingsObject->addRecentFile(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        Fl_Preferences recent_hosts(base, "recentHosts");
        num = recent_hosts.entries();
        settingsObject->addRecentHost("localhost");
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Host #%d", i);
            if (recent_hosts.get(buf, tmpS, "", 2048))
            {
                settingsObject->addRecentHost(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        Fl_Preferences python_scripts(base, "pythonScripts");
        num = python_scripts.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Script #%d", i);
            if (python_scripts.get(buf, tmpS, "", 2048))
            {
                settingsObject->addPythonScript(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        if (reset)
        {
            settingsObject->reset();
        }

        //
        // Get ui preferences
        //

        Fl_Preferences gui(base, "ui");

        gui.get("single_instance", tmp, 0);
        uiPrefs->uiPrefsSingleInstance->value((bool)tmp);

        gui.get("menubar", tmp, 1);
        uiPrefs->uiPrefsMenuBar->value((bool)tmp);

        gui.get("topbar", tmp, 1);
        uiPrefs->uiPrefsTopbar->value((bool)tmp);

        gui.get("pixel_toolbar", tmp, 1);
        uiPrefs->uiPrefsPixelToolbar->value((bool)tmp);

        gui.get("timeline_toolbar", tmp, 1);
        uiPrefs->uiPrefsTimeline->value((bool)tmp);

        gui.get("status_toolbar", tmp, 1);
        uiPrefs->uiPrefsStatusBar->value((bool)tmp);

        gui.get("action_toolbar", tmp, 1);
        uiPrefs->uiPrefsToolBar->value((bool)tmp);

        gui.get("one_panel_only", tmp, 0);
        uiPrefs->uiPrefsOnePanelOnly->value((bool)tmp);

        gui.get("macOS_menus", tmp, 0);
        uiPrefs->uiPrefsMacOSMenus->value((bool)tmp);

        gui.get("timeline_display", tmp, 0);
        uiPrefs->uiPrefsTimelineDisplay->value(tmp);

        gui.get("timeline_thumbnails", tmp, 1);
#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
        {
            tmp = 0;
        }
#endif
        uiPrefs->uiPrefsTimelineThumbnails->value(tmp);

        gui.get("timeline_edit_mode", tmp, 0);
        uiPrefs->uiPrefsEditMode->value(tmp);

        //
        // ui/window preferences
        //
        {
            Fl_Preferences win(gui, "window");

            win.get("auto_fit_image", tmp, 1);
            uiPrefs->uiPrefsAutoFitImage->value(tmp);

            win.get("always_on_top", tmp, 0);
            uiPrefs->uiPrefsAlwaysOnTop->value(tmp);

            win.get("open_mode", tmp, 0);

            {
                Fl_Round_Button* r;
                for (int i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i)
                {

                    r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(i);
                    r->value(0);
                }

                r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(tmp);
                r->value(1);
            }
        }

        //
        // ui/view
        //

        Fl_Preferences view(gui, "view");

        view.get("gain", tmpF, 1.0f);
        uiPrefs->uiPrefsViewGain->value(tmpF);

        view.get("gamma", tmpF, 1.0f);
        uiPrefs->uiPrefsViewGamma->value(tmpF);

        view.get("safe_areas", tmp, 0);
        uiPrefs->uiPrefsSafeAreas->value((bool)tmp);

        view.get("crop_area", tmp, 0);
        uiPrefs->uiPrefsCropArea->value(tmp);

        view.get("zoom_speed", tmp, 2);
        uiPrefs->uiPrefsZoomSpeed->value(tmp);

        //
        // ui/colors
        //

        Fl_Preferences colors(gui, "colors");

        colors.get("background_color", bgcolor, 0x43434300);

        colors.get("text_color", textcolor, 0xababab00);

        colors.get("selection_color", selectioncolor, 0x97a8a800);

        colors.get("selection_text_color", selectiontextcolor, 0x00000000);

        colors.get("scheme", tmpS, "gtk+", 2048);

        const Fl_Menu_Item* item = uiPrefs->uiScheme->find_item(tmpS);
        if (item)
        {
            uiPrefs->uiScheme->picked(item);
            Fl::scheme(tmpS);
        }

        bool loaded = false;

        std::string colorname = prefspath() + "mrv2.colors";
        if (!(loaded = schemes.read_themes(colorname.c_str())))
        {

            colorname = root + "/colors/mrv2.colors";
            if (!(loaded = schemes.read_themes(colorname.c_str())))
            {
                std::string msg =
                    tl::string::Format(
                        _("Could not open color theme from \"{0}\"."))
                        .arg(colorname);
                LOG_ERROR(msg);
            }
        }

        if (loaded)
        {

            std::string msg =
                tl::string::Format(_("Loaded color themes from \"{0}\"."))
                    .arg(colorname);

            LOG_INFO(msg);
        }

        for (auto& t : schemes.themes)
        {

            uiPrefs->uiColorTheme->add(t.name.c_str());
        }

        colors.get("theme", tmpS, "Black", 2048);

        item = uiPrefs->uiColorTheme->find_item(tmpS);
        if (item)
        {

            uiPrefs->uiColorTheme->picked(item);
        }

        const char* language = getenv("LANGUAGE");
        if (!language || language[0] == '\0')
            language = getenv("LC_ALL");
        if (!language || language[0] == '\0')
            language = getenv("LC_MESSAGES");
        if (!language || language[0] == '\0')
            language = getenv("LANG");

        int uiIndex = 0;
        if (language && strlen(language) > 1)
        {
            for (unsigned i = 0; i < sizeof(kLanguages) / sizeof(LanguageTable);
                 ++i)
            {
                if (strcmp(language, "C") == 0)
                {
                    break;
                }
                if (strncmp(language, kLanguages[i].code, 2) == 0)
                {
                    uiIndex = language_index = i;
                    language = kLanguages[i].code;
                    break;
                }
            }
        }

        uiPrefs->uiLanguage->value(uiIndex);

        //
        // ui/view/colors
        //
        {

            Fl_Preferences colors(view, "colors");

            colors.get("background_color", tmp, 0x20202000);
            uiPrefs->uiPrefsViewBG->color(tmp);

            colors.get("text_overlay_color", tmp, 0xFFFF0000);
            uiPrefs->uiPrefsViewTextOverlay->color(tmp);

            colors.get("selection_color", tmp, 0xFFFFFF00);
            uiPrefs->uiPrefsViewSelection->color(tmp);

            colors.get("hud_color", tmp, 0xF0F08000);
            uiPrefs->uiPrefsViewHud->color(tmp);
        }

        Fl_Preferences ocio(view, "ocio");

        //////////////////////////////////////////////////////
        // OCIO
        /////////////////////////////////////////////////////

        // Check OCIO variable first, then saved prefs and finally if nothing,
        // use this default.
        std::string ocioDefault =
            root + "/ocio/cg-config/cg-config-v1.0.0_aces-v1.3_ocio-v2.1.ocio";
        static std::string old_ocio;

        const char* var = getenv("OCIO");
        if (!var || strlen(var) == 0)
        {
            ocio.get("config", tmpS, "", 2048);

            if (strlen(tmpS) != 0)
            {
                if (is_readable(tmpS))
                {
                    mrvLOG_INFO(
                        "ocio", _("Setting OCIO config from preferences.")
                                    << std::endl);
                    uiPrefs->uiPrefsOCIOConfig->value(tmpS);
                    var = uiPrefs->uiPrefsOCIOConfig->value();
                }
                else
                {
                    std::string root = tmpS;
                    if (root.find("mrv2") != std::string::npos)
                    {
                        mrvLOG_INFO(
                            "ocio", _("Setting OCIO config to default.")
                                        << std::endl);
                        uiPrefs->uiPrefsOCIOConfig->value(ocioDefault.c_str());
                        var = uiPrefs->uiPrefsOCIOConfig->value();
                    }
                }
            }
        }
        else
        {
            mrvLOG_INFO(
                "ocio", _("Setting OCIO config from OCIO "
                          "environment variable.")
                            << std::endl);
            uiPrefs->uiPrefsOCIOConfig->value(var);
        }

        if (!var || strlen(var) == 0 || reset)
        {
            mrvLOG_INFO(
                "ocio", _("Setting OCIO config to default.") << std::endl);
            uiPrefs->uiPrefsOCIOConfig->value(ocioDefault.c_str());
        }

        Fl_Preferences ics(ocio, "ICS");
        {
#define OCIO_ICS(x, d)                                                         \
    ok = ics.get(#x, tmpS, d, 2048);                                           \
    environmentSetting("MRV_OCIO_" #x "_ICS", tmpS, ok);                       \
    uiPrefs->uiOCIO_##x##_ics->value(tmpS);

            OCIO_ICS(8bits, "");

            OCIO_ICS(16bits, "");

            OCIO_ICS(32bits, "");

            OCIO_ICS(half, "");

            OCIO_ICS(float, "");
        }

        //
        // ui/view/hud
        //
        Fl_Preferences hud(view, "hud");

        hud.get("directory", tmp, 0);
        uiPrefs->uiPrefsHudDirectory->value((bool)tmp);
        hud.get("filename", tmp, 0);
        uiPrefs->uiPrefsHudFilename->value((bool)tmp);
        hud.get("fps", tmp, 0);
        uiPrefs->uiPrefsHudFPS->value((bool)tmp);
        hud.get("frame", tmp, 0);
        uiPrefs->uiPrefsHudFrame->value((bool)tmp);
        hud.get("timecode", tmp, 0);
        uiPrefs->uiPrefsHudTimecode->value((bool)tmp);
        hud.get("resolution", tmp, 0);

        uiPrefs->uiPrefsHudResolution->value((bool)tmp);
        hud.get("frame_range", tmp, 0);
        uiPrefs->uiPrefsHudFrameRange->value((bool)tmp);
        hud.get("frame_count", tmp, 0);
        uiPrefs->uiPrefsHudFrameCount->value((bool)tmp);
        hud.get("cache", tmp, 0);
        uiPrefs->uiPrefsHudCache->value((bool)tmp);
        hud.get("memory", tmp, 0);
        uiPrefs->uiPrefsHudMemory->value((bool)tmp);
        hud.get("attributes", tmp, 0);
        uiPrefs->uiPrefsHudAttributes->value((bool)tmp);

        Fl_Preferences win(view, "window");
        win.get("fixed_position", tmp, 0);
        uiPrefs->uiWindowFixedPosition->value((bool)tmp);
        win.get("x_position", tmp, 0);

        uiPrefs->uiWindowXPosition->value(tmp);
        win.get("y_position", tmp, 0);
        uiPrefs->uiWindowYPosition->value(tmp);
        win.get("fixed_size", tmp, 0);

        uiPrefs->uiWindowFixedSize->value((bool)tmp);
        win.get("x_size", tmp, 640);
        uiPrefs->uiWindowXSize->value(tmp);
        win.get("y_size", tmp, 530);

        uiPrefs->uiWindowYSize->value(tmp);

        Fl_Preferences flu(gui, "file_requester");
        //

        flu.get("quick_folder_travel", tmp, 1);
        uiPrefs->uiPrefsFileReqFolder->value((bool)tmp);
        Flu_File_Chooser::singleButtonTravelDrawer = (bool)tmp;

        flu.get("thumbnails", tmp, 1);
        uiPrefs->uiPrefsFileReqThumbnails->value((bool)tmp);
        Flu_File_Chooser::thumbnailsFileReq = (bool)tmp;

        flu.get("usd_thumbnails", tmp, 1);
        uiPrefs->uiPrefsUSDThumbnails->value((bool)tmp);
        Flu_File_Chooser::thumbnailsUSD = (bool)tmp;

        //
        // playback
        //
        Fl_Preferences playback(base, "playback");

        playback.get("auto_playback", tmp, 1);
        uiPrefs->uiPrefsAutoPlayback->value(tmp);

        playback.get("override_fps", tmp, 0);
        uiPrefs->uiPrefsOverrideFPS->value(tmp);

        playback.get("fps", tmpF, 24.0);
        uiPrefs->uiPrefsFPS->value(tmpF);

        playback.get("loop", tmp, 0);
        uiPrefs->uiPrefsLoopMode->value(tmp);

        playback.get("scrubbing_sensitivity", tmpF, 5.0f);
        uiPrefs->uiPrefsScrubbingSensitivity->value(tmpF);

        Fl_Preferences pixel_toolbar(base, "pixel_toolbar");

        pixel_toolbar.get("RGBA_pixel", tmp, 0);
        uiPrefs->uiPrefsPixelRGBA->value(tmp);

        pixel_toolbar.get("pixel_values", tmp, 0);
        uiPrefs->uiPrefsPixelValues->value(tmp);

        pixel_toolbar.get("HSV_pixel", tmp, 0);
        uiPrefs->uiPrefsPixelHSV->value(tmp);

        pixel_toolbar.get("Lumma_pixel", tmp, 0);
        uiPrefs->uiPrefsPixelLumma->value(tmp);

        Fl_Preferences loading(base, "loading");

#ifdef __APPLE__
        loading.get("native_file_chooser", tmp, 1);
#else
        loading.get("native_file_chooser", tmp, 0);
#endif
        uiPrefs->uiPrefsNativeFileChooser->value((bool)tmp);

        loading.get("missing_frame_type", tmp, 0);
        uiPrefs->uiMissingFrameType->value(tmp);

        loading.get("version_regex", tmpS, "_v", 2048);
        if (strlen(tmpS) == 0)
        {
            strcpy(tmpS, "_v");
        }
        uiPrefs->uiPrefsVersionRegex->value(tmpS);

        loading.get("max_images_apart", tmp, 10);
        uiPrefs->uiPrefsMaxImagesApart->value(tmp);

        char key[256];
        Fl_Preferences path_mapping(
            prefspath().c_str(), "filmaura", "mrv2.paths");
        num = path_mapping.entries();
        for (int i = 0; i < num; ++i)
        {
            snprintf(key, 256, "Path #%d", i + 1);
            path_mapping.get(key, tmpS, "", 256);
            if (strlen(tmpS) == 0)
                continue;
            uiPrefs->PathMappings->add(tmpS);
        }
        msg = tl::string::Format(_("Path mappings have been loaded from "
                                   "\"{0}mrv2.paths.prefs\"."))
                  .arg(prefspath());
        LOG_INFO(msg);

        Fl_Preferences network(base, "network");

        network.get("send_media", tmp, 1);
        uiPrefs->SendMedia->value(tmp);

        network.get("send_ui", tmp, 1);
        uiPrefs->SendUI->value(tmp);

        network.get("send_pan_and_zoom", tmp, 1);
        uiPrefs->SendPanAndZoom->value(tmp);

        network.get("send_color", tmp, 1);
        uiPrefs->SendColor->value(tmp);

        network.get("send_timeline", tmp, 1);
        uiPrefs->SendTimeline->value(tmp);

        network.get("send_annotations", tmp, 1);
        uiPrefs->SendAnnotations->value(tmp);

        network.get("send_audio", tmp, 1);
        uiPrefs->SendAudio->value(tmp);

        network.get("receive_media", tmp, 1);
        uiPrefs->ReceiveMedia->value(tmp);

        network.get("receive_ui", tmp, 1);
        uiPrefs->ReceiveUI->value(tmp);

        network.get("receive_pan_and_zoom", tmp, 1);
        uiPrefs->ReceivePanAndZoom->value(tmp);

        network.get("receive_color", tmp, 1);
        uiPrefs->ReceiveColor->value(tmp);

        network.get("receive_timeline", tmp, 1);
        uiPrefs->ReceiveTimeline->value(tmp);

        network.get("receive_annotations", tmp, 1);
        uiPrefs->ReceiveAnnotations->value(tmp);

        network.get("receive_audio", tmp, 1);
        uiPrefs->ReceiveAudio->value(tmp);

        Fl_Preferences errors(base, "errors");
        errors.get("log_display", tmp, 2);

        uiPrefs->uiPrefsRaiseLogWindowOnError->value(tmp);

        //
        // Hotkeys
        //
        Fl_Preferences* keys;

        Fl_Preferences hotkeys(base, "hotkeys");
        hotkeys.get("default", tmpS, "mrv2.keys", 2048);

        hotkeys_file = tmpS;

        if (hotkeys_file.empty())
            hotkeys_file = "mrv2.keys";

        msg = tl::string::Format(_("Loading hotkeys from \"{0}{1}.prefs\"."))
                  .arg(prefspath())
                  .arg(hotkeys_file);

        LOG_INFO(msg);

        keys = new Fl_Preferences(
            prefspath().c_str(), "filmaura", hotkeys_file.c_str());

        load_hotkeys(ui, keys);

        delete keys;

        std_any value;

        value = settingsObject->value("Performance/AudioBufferFrameCount");
        int v = std_any_cast<int>(value);
        if (v < 1024)
        {
            settingsObject->setValue(
                "Performance/AudioBufferFrameCount",
                (int)timeline::PlayerOptions().audioBufferFrameCount);
        }

        value = settingsObject->value(kPenColorR);
        int r = std_any_cast<int>(value);

        value = settingsObject->value(kPenColorG);
        int g = std_any_cast<int>(value);

        value = settingsObject->value(kPenColorB);
        int b = std_any_cast<int>(value);

        value = settingsObject->value(kPenColorA);
        int a = std_any_cast<int>(value);

        ui->uiPenColor->color((Fl_Color)61);
        Fl_Color c = (Fl_Color)ui->uiPenColor->color();
        Fl::set_color(c, r, g, b);

        ui->uiPenOpacity->value(a / 255.0F);

        settingsObject->setValue(kPenColorR, (int)r);
        settingsObject->setValue(kPenColorG, (int)g);
        settingsObject->setValue(kPenColorB, (int)b);
        settingsObject->setValue(kPenColorA, (int)a);

        // Handle Dockgroup size (based on percentage)
        value = settingsObject->value("gui/DockGroup/Width");
        float pct = std_any_empty(value) ? 0.2 : std_any_cast<float>(value);
        int width = ui->uiViewGroup->w() * pct;

        value = settingsObject->value("gui/DockGroup/Visible");
        int visible = std_any_empty(value) ? 0 : std_any_cast<int>(value);
        if (visible)
            ui->uiDockGroup->show();

        // Set a minimum size for dockgroup
        if (width < 270)
            width = 270;

        ui->uiViewGroup->fixed(ui->uiDockGroup, width);

        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);
    }

    void Preferences::open_windows()
    {
        std_any value;
        int visible;

        ViewerUI* ui = App::ui;
        SettingsObject* settingsObject = ViewerUI::app->settingsObject();

        if (!ui->uiView->getPresentationMode())
        {
            // Handle windows/panels
            const WindowCallback* wc = kWindowCallbacks;
            for (; wc->name; ++wc)
            {
                std::string key = "gui/";
                key += wc->name;
                key += "/Window/Visible";
                value = settingsObject->value(key);
                visible = std_any_empty(value) ? 0 : std_any_cast< int >(value);
                if (visible)
                {
                    if (std::string("Logs") == wc->name && logsPanel)
                        continue;
                    show_window_cb(wc->name, ui);
                }
            }
        }

        // Handle secondary window which is a tad special
        std::string key = "gui/Secondary/Window/Visible";
        value = settingsObject->value(key);
        visible = std_any_empty(value) ? 0 : std_any_cast< int >(value);
        if (visible)
            toggle_secondary_cb(nullptr, ui);
    }

    void Preferences::save()
    {
        int i;
        ViewerUI* ui = App::ui;
        PreferencesUI* uiPrefs = ViewerUI::uiPrefs;
        SettingsObject* settingsObject = ViewerUI::app->settingsObject();

        char* saved_locale = strdup(setlocale(LC_NUMERIC, NULL));
        setlocale(LC_NUMERIC, "C");

        int visible = 0;
        if (uiPrefs->uiMain->visible())
            visible = 1;
        settingsObject->setValue("gui/Preferences/Window/Visible", visible);

        int width = ui->uiDockGroup->w() == 0 ? 1 : ui->uiDockGroup->w();
        float pct = (float)width / ui->uiViewGroup->w();
        settingsObject->setValue("gui/DockGroup/Width", pct);

        visible = 0;
        if (ui->uiDockGroup->visible())
            visible = 1;
        settingsObject->setValue("gui/DockGroup/Visible", visible);

        Fl_Preferences base(prefspath().c_str(), "filmaura", "mrv2");
        base.set("version", 7);

        Fl_Preferences fltk_settings(base, "settings");
        fltk_settings.clear();

        const std::vector< std::string >& keys = settingsObject->keys();
        for (auto key : keys)
        {
            std_any value = settingsObject->value(key);
            try
            {
                double tmpD = std_any_cast< double >(value);
                key = "d#" + key;
                fltk_settings.set(key.c_str(), tmpD);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                float tmpF = std_any_cast< float >(value);
                key = "f#" + key;
                fltk_settings.set(key.c_str(), tmpF);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std_any_cast< int >(value);
                key = "i#" + key;
                fltk_settings.set(key.c_str(), tmp);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std_any_cast< bool >(value);
                key = "b#" + key;
                fltk_settings.set(key.c_str(), tmp);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string& tmpS = std_any_cast< std::string >(value);
                key = "s#" + key;
                fltk_settings.set(key.c_str(), tmpS.c_str());
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string tmpS = std_any_cast< char* >(value);
                key = "s#" + key;
                fltk_settings.set(key.c_str(), tmpS.c_str());
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                // If we don't know the type, don't store anything
                // key = "v#" + key;
                // fltk_settings.set( key.c_str(), 0 );
                continue;
            }
            catch (const std::bad_cast& e)
            {
                LOG_ERROR(
                    "Could not save preference for " << key << " type "
                                                     << value.type().name());
            }
        }

        Fl_Preferences recent_files(base, "recentFiles");
        const std::vector< std::string >& files = settingsObject->recentFiles();
        for (unsigned i = 1; i <= files.size(); ++i)
        {
            char buf[16];
            snprintf(buf, 16, "File #%d", i);
            recent_files.set(buf, files[i - 1].c_str());
        }

        Fl_Preferences recent_hosts(base, "recentHosts");
        const std::vector< std::string >& hosts = settingsObject->recentHosts();
        for (unsigned i = 1; i <= hosts.size(); ++i)
        {
            char buf[16];
            snprintf(buf, 16, "Host #%d", i);
            recent_hosts.set(buf, hosts[i - 1].c_str());
        }

        Fl_Preferences python_scripts(base, "pythonScripts");
        const std::vector< std::string >& scripts =
            settingsObject->pythonScripts();
        for (unsigned i = 1; i <= scripts.size(); ++i)
        {
            char buf[16];
            snprintf(buf, 16, "Script #%d", i);
            python_scripts.set(buf, scripts[i - 1].c_str());
        }

        // Save ui preferences
        Fl_Preferences gui(base, "ui");

        //
        // window options
        //
        {
            Fl_Preferences win(gui, "window");
            win.set(
                "auto_fit_image", (int)uiPrefs->uiPrefsAutoFitImage->value());
            win.set("always_on_top", (int)uiPrefs->uiPrefsAlwaysOnTop->value());
            int tmp = 0;
            for (i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i)
            {
                Fl_Round_Button* r =
                    (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(i);
                if (r->value())
                {
                    tmp = i;
                    break;
                }
            }
            win.set("open_mode", tmp);
        }

        //
        // ui options
        //
        const char* language = fl_getenv("LANGUAGE");
        if (language && strlen(language) != 0)
        {
            gui.set("language_code", language);
        }

        gui.set("menubar", (int)uiPrefs->uiPrefsMenuBar->value());
        gui.set("topbar", (int)uiPrefs->uiPrefsTopbar->value());
        gui.set(
            "single_instance", (int)uiPrefs->uiPrefsSingleInstance->value());
        gui.set("pixel_toolbar", (int)uiPrefs->uiPrefsPixelToolbar->value());
        gui.set("timeline_toolbar", (int)uiPrefs->uiPrefsTimeline->value());
        gui.set("status_toolbar", (int)uiPrefs->uiPrefsStatusBar->value());
        gui.set("action_toolbar", (int)uiPrefs->uiPrefsToolBar->value());
        gui.set("one_panel_only", (int)uiPrefs->uiPrefsOnePanelOnly->value());
        gui.set("macOS_menus", (int)uiPrefs->uiPrefsMacOSMenus->value());

        gui.set("timeline_display", uiPrefs->uiPrefsTimelineDisplay->value());
        gui.set(
            "timeline_thumbnails", uiPrefs->uiPrefsTimelineThumbnails->value());
        gui.set("timeline_edit_mode", uiPrefs->uiPrefsEditMode->value());

        //
        // ui/view prefs
        //
        Fl_Preferences view(gui, "view");
        view.set("gain", uiPrefs->uiPrefsViewGain->value());
        view.set("gamma", uiPrefs->uiPrefsViewGamma->value());

        view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value());
        view.set("crop_area", uiPrefs->uiPrefsCropArea->value());
        view.set("zoom_speed", (int)uiPrefs->uiPrefsZoomSpeed->value());

        //
        // view/colors prefs
        //
        {
            Fl_Preferences colors(view, "colors");
            int tmp = uiPrefs->uiPrefsViewBG->color();
            colors.set("background_color", tmp);
            tmp = uiPrefs->uiPrefsViewTextOverlay->color();
            colors.set("text_overlay_color", tmp);
            tmp = uiPrefs->uiPrefsViewSelection->color();
            colors.set("selection_color", tmp);
            tmp = uiPrefs->uiPrefsViewHud->color();
            colors.set("hud_color", tmp);
        }

        {
            Fl_Preferences ocio(view, "ocio");

            ocio.set("config", uiPrefs->uiPrefsOCIOConfig->value());

            Fl_Preferences ics(ocio, "ICS");
            {
                ics.set("8bits", uiPrefs->uiOCIO_8bits_ics->value());
                ics.set("16bits", uiPrefs->uiOCIO_16bits_ics->value());
                ics.set("32bits", uiPrefs->uiOCIO_32bits_ics->value());
                ics.set("half", uiPrefs->uiOCIO_half_ics->value());
                ics.set("float", uiPrefs->uiOCIO_float_ics->value());
            }
        }

        //
        // view/hud prefs
        //
        Fl_Preferences hud(view, "hud");
        hud.set("directory", uiPrefs->uiPrefsHudDirectory->value());
        hud.set("filename", uiPrefs->uiPrefsHudFilename->value());
        hud.set("fps", uiPrefs->uiPrefsHudFPS->value());
        hud.set("non_drop_timecode", uiPrefs->uiPrefsHudTimecode->value());
        hud.set("frame", uiPrefs->uiPrefsHudFrame->value());
        hud.set("resolution", uiPrefs->uiPrefsHudResolution->value());
        hud.set("frame_range", uiPrefs->uiPrefsHudFrameRange->value());
        hud.set("frame_count", uiPrefs->uiPrefsHudFrameCount->value());
        hud.set("cache", uiPrefs->uiPrefsHudCache->value());
        hud.set("memory", uiPrefs->uiPrefsHudMemory->value());
        hud.set("attributes", uiPrefs->uiPrefsHudAttributes->value());

        {
            Fl_Preferences win(view, "window");
            win.set("fixed_position", uiPrefs->uiWindowFixedPosition->value());
            win.set("x_position", uiPrefs->uiWindowXPosition->value());
            win.set("y_position", uiPrefs->uiWindowYPosition->value());
            win.set("fixed_size", uiPrefs->uiWindowFixedSize->value());
            win.set("x_size", uiPrefs->uiWindowXSize->value());
            win.set("y_size", uiPrefs->uiWindowYSize->value());
        }

        //
        // ui/colors prefs
        //
        Fl_Preferences colors(gui, "colors");
        colors.set("scheme", uiPrefs->uiScheme->text());
        colors.set("theme", uiPrefs->uiColorTheme->text());
        colors.set("background_color", bgcolor);
        colors.set("text_color", textcolor);
        colors.set("selection_color", selectioncolor);
        colors.set("selection_text_color", selectiontextcolor);
        colors.set("theme", uiPrefs->uiColorTheme->text());

        Fl_Preferences flu(gui, "file_requester");
        flu.set("quick_folder_travel", uiPrefs->uiPrefsFileReqFolder->value());
        flu.set("thumbnails", uiPrefs->uiPrefsFileReqThumbnails->value());
        flu.set("usd_thumbnails", uiPrefs->uiPrefsUSDThumbnails->value());

        //
        Flu_File_Chooser::singleButtonTravelDrawer =
            uiPrefs->uiPrefsFileReqFolder->value();
        Flu_File_Chooser::thumbnailsFileReq =
            uiPrefs->uiPrefsFileReqThumbnails->value();
        Flu_File_Chooser::thumbnailsUSD =
            uiPrefs->uiPrefsUSDThumbnails->value();

        //
        // playback prefs
        //
        Fl_Preferences playback(base, "playback");
        playback.set(
            "auto_playback", (int)uiPrefs->uiPrefsAutoPlayback->value());
        playback.set("override_fps", uiPrefs->uiPrefsOverrideFPS->value());
        playback.set("fps", uiPrefs->uiPrefsFPS->value());
        playback.delete_entry("loop_mode"); // legacy preference
        playback.set("loop", uiPrefs->uiPrefsLoopMode->value());
        playback.set(
            "scrubbing_sensitivity",
            uiPrefs->uiPrefsScrubbingSensitivity->value());

        Fl_Preferences pixel_toolbar(base, "pixel_toolbar");
        pixel_toolbar.set("RGBA_pixel", uiPrefs->uiPrefsPixelRGBA->value());
        pixel_toolbar.set("pixel_values", uiPrefs->uiPrefsPixelValues->value());
        pixel_toolbar.set("HSV_pixel", uiPrefs->uiPrefsPixelHSV->value());
        pixel_toolbar.set("Lumma_pixel", uiPrefs->uiPrefsPixelLumma->value());

        Fl_Preferences loading(base, "loading");

        loading.set(
            "native_file_chooser",
            (int)uiPrefs->uiPrefsNativeFileChooser->value());

        loading.set("missing_frame_type", uiPrefs->uiMissingFrameType->value());

        loading.set("version_regex", uiPrefs->uiPrefsVersionRegex->value());
        loading.set(
            "max_images_apart", (int)uiPrefs->uiPrefsMaxImagesApart->value());

        char key[256];
        Fl_Preferences path_mapping(
            prefspath().c_str(), "filmaura", "mrv2.paths");
        path_mapping.clear();
        for (int i = 2; i <= uiPrefs->PathMappings->size(); ++i)
        {
            snprintf(key, 256, "Path #%d", i - 1);
            path_mapping.set(key, uiPrefs->PathMappings->text(i));
        }
        std::string msg =
            tl::string::Format(_("Path mappings have been saved to "
                                 "\"{0}mrv2.paths.prefs\"."))
                .arg(prefspath());
        LOG_INFO(msg);

        Fl_Preferences network(base, "network");

        network.set("send_media", (int)uiPrefs->SendMedia->value());

        network.set("send_ui", (int)uiPrefs->SendUI->value());

        network.set("send_pan_and_zoom", (int)uiPrefs->SendPanAndZoom->value());

        network.set("send_color", (int)uiPrefs->SendColor->value());

        network.set("send_annotations", (int)uiPrefs->SendAnnotations->value());

        network.set("send_audio", (int)uiPrefs->SendAudio->value());

        network.set("receive_media", (int)uiPrefs->ReceiveMedia->value());

        network.set("receive_ui", (int)uiPrefs->ReceiveUI->value());

        network.set(
            "receive_pan_and_zoom", (int)uiPrefs->ReceivePanAndZoom->value());

        network.set("receive_color", (int)uiPrefs->ReceiveColor->value());

        network.set(
            "receive_annotations", (int)uiPrefs->ReceiveAnnotations->value());

        network.set("receive_audio", (int)uiPrefs->ReceiveAudio->value());

        Fl_Preferences errors(base, "errors");
        errors.set(
            "log_display", (int)uiPrefs->uiPrefsRaiseLogWindowOnError->value());

        Fl_Preferences hotkeys(base, "hotkeys");
        hotkeys.set("default", hotkeys_file.c_str());

        if (!is_readable(prefspath() + hotkeys_file))
        {
            Fl_Preferences keys(
                prefspath().c_str(), "filmaura", hotkeys_file.c_str());
            save_hotkeys(keys);
        }

        base.flush();

        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);

        msg = tl::string::Format(_("Preferences have been saved to: "
                                   "\"{0}mrv2.prefs\"."))
                  .arg(prefspath());
        LOG_INFO(msg);

        check_language(uiPrefs, language_index);
    }

    bool Preferences::set_transforms()
    {
        return true;
    }

    Preferences::~Preferences() {}

    void Preferences::run(ViewerUI* ui)
    {
        PreferencesUI* uiPrefs = ui->uiPrefs;
        App* app = ui->app;

        check_language(uiPrefs, language_index);

#ifdef __APPLE__
        if (uiPrefs->uiPrefsMacOSMenus->value())
        {
            ui->uiMenuBar->clear();
            ui->uiMenuGroup->redraw();
            delete ui->uiMenuBar;
            ui->uiMenuBar = static_cast<MenuBar*>(
                static_cast<Fl_Menu_Bar*>((new Fl_Sys_Menu_Bar(0, 0, 0, 25))));
        }
        else
        {
            Fl_Menu_Bar* basePtr = dynamic_cast<Fl_Menu_Bar*>(ui->uiMenuBar);
            Fl_Sys_Menu_Bar* smenubar =
                dynamic_cast< Fl_Sys_Menu_Bar* >(basePtr);
            if (smenubar)
            {
                smenubar->clear();
                delete ui->uiMenuBar;
                ui->uiMenuBar = new MenuBar(0, 0, ui->uiStatus->x(), 25);
                ui->uiMenuGroup->add(ui->uiMenuBar);
                ui->uiMenuGroup->redraw();
            }
        }
#endif

        SettingsObject* settingsObject = ViewerUI::app->settingsObject();

        //
        // Windows
        //

        //
        // Toolbars
        //

        Viewport* view = ui->uiView;

        if (uiPrefs->uiPrefsMenuBar->value())
        {
            ui->uiMenuGroup->show();
        }
        else
        {
            ui->uiMenuGroup->hide();
        }

        if (uiPrefs->uiPrefsTopbar->value())
        {
            ui->uiTopBar->show();
        }
        else
        {
            ui->uiTopBar->hide();
        }

        if (uiPrefs->uiPrefsPixelToolbar->value())
        {
            ui->uiPixelBar->show();
        }
        else
        {
            ui->uiPixelBar->hide();
        }

        if (uiPrefs->uiPrefsTimeline->value())
        {
            ui->uiBottomBar->show();
            set_edit_mode_cb(EditMode::kSaved, ui);
        }
        else
        {
            ui->uiBottomBar->hide();
            set_edit_mode_cb(EditMode::kNone, ui);
        }

        if (uiPrefs->uiPrefsStatusBar->value())
        {
            ui->uiStatusGroup->show();
        }
        else
        {
            ui->uiStatusGroup->hide();
        }

        if (uiPrefs->uiPrefsToolBar->value())
        {
            ui->uiToolsGroup->show();
            ui->uiToolsGroup->size(45, 433);
            ui->uiViewGroup->layout();
            ui->uiViewGroup->init_sizes();
        }
        else
        {
            ui->uiToolsGroup->hide();
            ui->uiViewGroup->layout();
            ui->uiViewGroup->init_sizes();
        }

        ui->uiRegion->layout();

        onePanelOnly((bool)uiPrefs->uiPrefsOnePanelOnly->value());

        //
        // Widget/Viewer settings
        //

        {
            std_any value;
            value = settingsObject->value(kGhostNext);
            ui->uiView->setGhostNext(
                std_any_empty(value) ? 5 : std_any_cast< int >(value));
            value = settingsObject->value(kGhostPrevious);
            ui->uiView->setGhostPrevious(
                std_any_empty(value) ? 5 : std_any_cast< int >(value));

            ui->uiView->setMissingFrameType(static_cast<MissingFrameType>(
                uiPrefs->uiMissingFrameType->value()));
        }

        TimelineClass* t = ui->uiTimeWindow;
        t->uiLoopMode->value(uiPrefs->uiPrefsLoopMode->value());
        t->uiLoopMode->do_callback();

        t->uiTimecodeSwitch->value(uiPrefs->uiPrefsTimelineDisplay->value());
        t->uiTimecodeSwitch->do_callback();

        ui->uiGain->value(uiPrefs->uiPrefsViewGain->value());
        ui->uiGamma->value(uiPrefs->uiPrefsViewGamma->value());

        // OCIO
        OCIO(ui);

        //
        // Handle file requester
        //

        Flu_File_Chooser::thumbnailsFileReq =
            (bool)uiPrefs->uiPrefsFileReqThumbnails->value();

        Flu_File_Chooser::singleButtonTravelDrawer =
            (bool)uiPrefs->uiPrefsFileReqFolder->value();

        native_file_chooser = uiPrefs->uiPrefsNativeFileChooser->value();

        //
        // Handle pixel values
        //
        PixelToolBarClass* c = ui->uiPixelWindow;
        c->uiAColorType->value(uiPrefs->uiPrefsPixelRGBA->value());
        c->uiAColorType->do_callback();
        c->uiAColorType->redraw();

        c->uiPixelValue->value(uiPrefs->uiPrefsPixelValues->value());
        c->uiPixelValue->do_callback();
        c->uiPixelValue->redraw();

        c->uiBColorType->value(uiPrefs->uiPrefsPixelHSV->value());
        c->uiBColorType->do_callback();
        c->uiBColorType->redraw();

        c->uiLType->value(uiPrefs->uiPrefsPixelLumma->value());
        c->uiLType->do_callback();
        c->uiLType->redraw();

        //
        // Handle crop area (masking)
        //

        int crop = uiPrefs->uiPrefsCropArea->value();
        float mask = kCrops[crop];
        view->setMask(mask);

        //
        // Handle HUD
        //
        int hud = HudDisplay::kNone;
        if (uiPrefs->uiPrefsHudDirectory->value())
            hud |= HudDisplay::kDirectory;

        if (uiPrefs->uiPrefsHudFilename->value())
            hud |= HudDisplay::kFilename;

        if (uiPrefs->uiPrefsHudFPS->value())
            hud |= HudDisplay::kFPS;

        if (uiPrefs->uiPrefsHudTimecode->value())
            hud |= HudDisplay::kTimecode;

        if (uiPrefs->uiPrefsHudFrame->value())
            hud |= HudDisplay::kFrame;

        if (uiPrefs->uiPrefsHudResolution->value())
            hud |= HudDisplay::kResolution;

        if (uiPrefs->uiPrefsHudFrameRange->value())
            hud |= HudDisplay::kFrameRange;

        if (uiPrefs->uiPrefsHudFrameCount->value())
            hud |= HudDisplay::kFrameCount;

        if (uiPrefs->uiPrefsHudAttributes->value())
            hud |= HudDisplay::kAttributes;

        if (uiPrefs->uiPrefsHudCache->value())
            hud |= HudDisplay::kCache;

        if (uiPrefs->uiPrefsHudMemory->value())
            hud |= HudDisplay::kMemory;

        view->setHudDisplay((HudDisplay)hud);

        //
        // Handle fullscreen and presentation mode
        //
        if (uiPrefs->uiWindowFixedPosition->value())
        {
            int x = int(uiPrefs->uiWindowXPosition->value());
            int y = int(uiPrefs->uiWindowYPosition->value());
            ui->uiMain->position(x, y);
        }

        if (uiPrefs->uiWindowFixedSize->value())
        {
            int w = int(uiPrefs->uiWindowXSize->value());
            int h = int(uiPrefs->uiWindowYSize->value());
            ui->uiMain->resize(ui->uiMain->x(), ui->uiMain->y(), w, h);
        }

        bool frameView = (bool)uiPrefs->uiPrefsAutoFitImage->value();
        view->setFrameView(frameView);

        LogDisplay::prefs = (LogDisplay::ShowPreferences)
                                uiPrefs->uiPrefsRaiseLogWindowOnError->value();

        Fl_Round_Button* r;
        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(1);
        int fullscreen = r->value();
        if (fullscreen)
            view->setFullScreenMode(true);

        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(2);
        int presentation = r->value();
        if (presentation)
            view->setPresentationMode(true);

        if (!fullscreen && !presentation)
            view->setFullScreenMode(false);

        int fullscreen_active = ui->uiMain->fullscreen_active();
        if (!fullscreen_active)
            ui->uiMain->always_on_top(uiPrefs->uiPrefsAlwaysOnTop->value());

        ui->uiMain->fill_menu(ui->uiMenuBar);

        if (debug > 1)
            schemes.debug();
    }

    void Preferences::updateICS()
    {
        ViewerUI* ui = App::ui;
        auto players = ui->uiView->getTimelinePlayers();
        if (players.empty())
            return;

        const auto& tplayer = players[0]->player();
        const auto& info = tplayer->getIOInfo();
        const auto& videos = info.video;
        if (videos.empty())
            return;

        PreferencesUI* uiPrefs = ui->uiPrefs;
        const auto& video = info.video[0];
        tl::imaging::PixelType pixelType = video.pixelType;
        std::string ics;
        switch (pixelType)
        {
        case tl::imaging::PixelType::L_U8:
        case tl::imaging::PixelType::LA_U8:
        case tl::imaging::PixelType::RGB_U8:
        case tl::imaging::PixelType::RGB_U10:
        case tl::imaging::PixelType::RGBA_U8:
        case tl::imaging::PixelType::YUV_420P_U8:
        case tl::imaging::PixelType::YUV_422P_U8:
        case tl::imaging::PixelType::YUV_444P_U8:
            ics = uiPrefs->uiOCIO_8bits_ics->value();
            break;
        case tl::imaging::PixelType::L_U16:
        case tl::imaging::PixelType::LA_U16:
        case tl::imaging::PixelType::RGB_U16:
        case tl::imaging::PixelType::RGBA_U16:
        case tl::imaging::PixelType::YUV_420P_U16:
        case tl::imaging::PixelType::YUV_422P_U16:
        case tl::imaging::PixelType::YUV_444P_U16:
            ics = uiPrefs->uiOCIO_16bits_ics->value();
            break;
        case tl::imaging::PixelType::L_U32:
        case tl::imaging::PixelType::LA_U32:
        case tl::imaging::PixelType::RGB_U32:
        case tl::imaging::PixelType::RGBA_U32:
            ics = uiPrefs->uiOCIO_32bits_ics->value();
            break;
            // handle half and float types
        case tl::imaging::PixelType::L_F16:
        case tl::imaging::PixelType::LA_F16:
        case tl::imaging::PixelType::RGB_F16:
        case tl::imaging::PixelType::RGBA_F16:
            ics = uiPrefs->uiOCIO_half_ics->value();
            break;
        case tl::imaging::PixelType::L_F32:
        case tl::imaging::PixelType::LA_F32:
        case tl::imaging::PixelType::RGB_F32:
        case tl::imaging::PixelType::RGBA_F32:
            ics = uiPrefs->uiOCIO_float_ics->value();
            break;
        default:
            break;
        }

        mrv::PopupMenu* w = ui->uiICS;
        for (size_t i = 0; i < w->children(); ++i)
        {
            const Fl_Menu_Item* o = w->child(i);
            if (!o || !o->label())
                continue;

            if (ics == o->label())
            {
                w->copy_label(o->label());
                w->value(i);
                w->do_callback();
                break;
            }
        }
    }

    //////////////////////////////////////////////////////
    // OCIO
    /////////////////////////////////////////////////////
    void Preferences::OCIO(ViewerUI* ui)
    {
        PreferencesUI* uiPrefs = ui->uiPrefs;

        static std::string old_ocio;
        const char* var = uiPrefs->uiPrefsOCIOConfig->value();
        if (var && strlen(var) > 0)
        {

            if (old_ocio != var)
            {
                old_ocio = var;
                mrvLOG_INFO("ocio", _("OCIO config is now:") << std::endl);
                mrvLOG_INFO("ocio", var << std::endl);
            }

            std::string parsed = expandVariables(var, "%", '%');
            parsed = expandVariables(parsed, "${", '}');
            if (old_ocio != parsed)
            {
                mrvLOG_INFO("ocio", _("Expanded OCIO config to:") << std::endl);
                mrvLOG_INFO("ocio", parsed << std::endl);
            }

            uiPrefs->uiPrefsOCIOConfig->value(var);

            // First, remove all additional defaults if any from pulldown
            // menu
            ui->OCIOView->clear();
            ui->uiICS->clear();

            try
            {

                config = OCIO::Config::CreateFromFile(parsed.c_str());

                uiPrefs->uiPrefsOCIOConfig->tooltip(config->getDescription());

                OCIO_Display = config->getDefaultDisplay();

                OCIO_View = config->getDefaultView(OCIO_Display.c_str());

                int numDisplays = config->getNumDisplays();

                stringArray active_displays;
                const char* displaylist = config->getActiveDisplays();
                if (displaylist)
                {
                    mrv::split(active_displays, displaylist, ',');

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
                    int num = config->getNumDisplays();
                    for (int i = 0; i < num; ++i)
                    {
                        active_displays.push_back(config->getDisplay(i));
                    }
                }

                stringArray active_views;
                const char* viewlist = config->getActiveViews();
                if (viewlist)
                {
                    mrv::split(active_views, viewlist, ',');

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
                    std::string quoted_display = quoteSlashes(display);

                    int numViews = config->getNumViews(display.c_str());

                    // Collect all views

                    if (num_active_views)
                    {
                        for (size_t h = 0; h < num_active_views; ++h)
                        {
                            std::string view;
                            bool add = false;

                            for (int i = 0; i < numViews; ++i)
                            {
                                view = config->getView(display.c_str(), i);
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

                                ui->OCIOView->add(name.c_str());

                                if (view == OCIO_View && !OCIO_View.empty())
                                {
                                    ui->OCIOView->copy_label(view.c_str());
                                    ui->uiGamma->value(1.0f);
                                    ui->uiGammaInput->value(1.0f);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < numViews; i++)
                        {
                            std::string view =
                                config->getView(display.c_str(), i);

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

                            ui->OCIOView->add(name.c_str());

                            if (view == OCIO_View && !OCIO_View.empty())
                            {
                                ui->OCIOView->copy_label(view.c_str());
                                ui->uiGamma->value(1.0f);
                                ui->uiGammaInput->value(1.0f);
                            }
                        }
                    }
                }

                ui->OCIOView->redraw();

                std::vector< std::string > spaces;
                for (int i = 0; i < config->getNumColorSpaces(); ++i)
                {

                    std::string csname = config->getColorSpaceNameByIndex(i);
                    spaces.push_back(csname);
                }

                if (std::find(
                        spaces.begin(), spaces.end(),
                        OCIO::ROLE_SCENE_LINEAR) == spaces.end())
                {
                    spaces.push_back(OCIO::ROLE_SCENE_LINEAR);
                }

                mrv::PopupMenu* w = ui->uiICS;
                std::sort(spaces.begin(), spaces.end());
                size_t idx = 0;
                const char delim{'/'};
                const char escape{'\\'};
                for (size_t i = 0; i < spaces.size(); ++i)
                {
                    std::string space = spaces[i];
                    OCIO::ConstColorSpaceRcPtr cs =
                        config->getColorSpace(space.c_str());
                    const char* family = cs->getFamily();
                    std::string menu;
                    if (family && strlen(family) > 0)
                    {
                        menu = family;
                        menu += "/";
                    }
                    menu += quoteSlashes(space);
                    w->add(menu.c_str());
                }
            }
            catch (const OCIO::Exception& e)
            {
                mrvLOG_ERROR("ocio", e.what() << std::endl);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
        }

        ui->uiICS->show();

        updateICS();
    }

} // namespace mrv
