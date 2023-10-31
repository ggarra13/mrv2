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

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMedia.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvLogDisplay.h"

#ifdef MRV2_NETWORK
#    include "mrvNetwork/mrvImageListener.h"
#endif

#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvMenus.h"

#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

#include "mrvPreferencesUI.h"
#include "mrvHotkeyUI.h"

#include "mrvFl/mrvIO.h"
#include "mrvCore/mrvOS.h"

namespace
{
    const char* kModule = "prefs";
    const int kPreferencesVersion = 7;
} // namespace

extern float kCrops[];

mrv::App* ViewerUI::app = nullptr;
AboutUI* ViewerUI::uiAbout = nullptr;
PreferencesUI* ViewerUI::uiPrefs = nullptr;
HotkeyUI* ViewerUI::uiHotkey = nullptr;

namespace mrv
{
    using namespace panel;

    ColorSchemes Preferences::schemes;
    bool Preferences::native_file_chooser;
#ifdef TLRENDER_OCIO
    OCIO::ConstConfigRcPtr Preferences::config;
#endif
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

    Preferences::Preferences(
        PreferencesUI* uiPrefs, bool resetSettings, bool resetHotkeys)
    {
        ViewerUI* ui = App::ui;

        bool ok;
        int version;
        int tmp;
        double tmpD;
        float tmpF;
        char tmpS[2048];

        locale::SetAndRestore saved;

        std::string msg =
            tl::string::Format(_("Reading preferences from \"{0}mrv2.prefs\"."))
                .arg(prefspath());

        LOG_INFO(msg);

        Fl_Preferences base(
            prefspath().c_str(), "filmaura", "mrv2", (Fl_Preferences::Root)0);

        base.get("version", version, kPreferencesVersion);

        SettingsObject* settings = ViewerUI::app->settings();

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
                    fltk_settings.get(key, tmpF, 0.F);
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
                settings->setValue(keyS, value);
            }
        }
        DBG3;

        Fl_Preferences recent_files(base, "recentFiles");
        num = recent_files.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "File #%d", i);
            if (recent_files.get(buf, tmpS, "", 2048))
            {
                // Only add existing files to the list.
                if (file::isReadable(tmpS))
                    settings->addRecentFile(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        DBG3;
        Fl_Preferences recent_hosts(base, "recentHosts");
        num = recent_hosts.entries();
        settings->addRecentHost("localhost");
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Host #%d", i);
            if (recent_hosts.get(buf, tmpS, "", 2048))
            {
                settings->addRecentHost(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        DBG3;
        Fl_Preferences python_scripts(base, "pythonScripts");
        num = python_scripts.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Script #%d", i);
            if (python_scripts.get(buf, tmpS, "", 2048))
            {
                settings->addPythonScript(tmpS);
            }
            else
            {
                std::string msg =
                    tl::string::Format(_("Failed to retrieve {0}.")).arg(buf);
                LOG_ERROR(msg);
            }
        }

        if (resetSettings)
        {
            settings->reset();
        }

        DBG3;
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

        gui.get("remove_edls", tmp, 1);
        uiPrefs->uiPrefsRemoveEDLs->value(tmp);

        gui.get("timeline_edit_mode", tmp, 0);
        uiPrefs->uiPrefsEditMode->value(tmp);

        gui.get("timeline_edit_thumbnails", tmp, 1);
        uiPrefs->uiPrefsEditThumbnails->value(tmp);

        gui.get("timeline_edit_transitions", tmp, 1);
        uiPrefs->uiPrefsShowTransitions->value(tmp);

        gui.get("timeline_edit_markers", tmp, 0);
        uiPrefs->uiPrefsShowMarkers->value(tmp);

        if (version > 7)
        {
            gui.get("timeline_editable", tmp, 1);
            uiPrefs->uiPrefsTimelineEditable->value(tmp);
        }
        else
        {
            gui.get("timeline_editable", tmp, 0);
            if (version < kPreferencesVersion)
                tmp = 1;
            uiPrefs->uiPrefsTimelineEditable->value(tmp);
        }

        gui.get("timeline_edit_associated_clips", tmp, 1);
        uiPrefs->uiPrefsEditAssociatedClips->value(tmp);

#ifdef __APPLE__
        {
            auto itemOptions = ui->uiTimeline->getItemOptions();
            itemOptions.thumbnailFade = 0;
            ui->uiTimeline->setItemOptions(itemOptions);
        }
#endif

        //
        // ui/window preferences
        //
        {
            Fl_Preferences win(gui, "window");

            win.get("auto_fit_image", tmp, 1);
            uiPrefs->uiPrefsAutoFitImage->value(tmp);

            win.get("always_on_top", tmp, 0);
            uiPrefs->uiPrefsAlwaysOnTop->value(tmp);

            win.get("secondary_on_top", tmp, 1);
            uiPrefs->uiPrefsSecondaryOnTop->value(tmp);

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

        view.get("video_levels", tmp, 0);
        uiPrefs->uiPrefsVideoLevels->value(tmp);

        view.get("alpha_blend", tmp, 1);
        uiPrefs->uiPrefsAlphaBlend->value(tmp);

        view.get("crop_area", tmp, 0);
        uiPrefs->uiPrefsCropArea->value(tmp);

        view.get("zoom_speed", tmp, 2);
        uiPrefs->uiPrefsZoomSpeed->value(tmp);

        DBG3;
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

#ifdef TLRENDER_OCIO
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
                if (file::isReadable(tmpS))
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

        if (!var || strlen(var) == 0 || resetSettings)
        {
            mrvLOG_INFO(
                "ocio", _("Setting OCIO config to default.") << std::endl);
            uiPrefs->uiPrefsOCIOConfig->value(ocioDefault.c_str());
        }

        ocio.get("use_active_views", tmp, 1);
        uiPrefs->uiOCIOUseActiveViews->value(tmp);

        Fl_Preferences ics(ocio, "ICS");
        {
#    define OCIO_ICS(x, d)                                                     \
        ok = ics.get(#x, tmpS, d, 2048);                                       \
        uiPrefs->uiOCIO_##x##_ics->value(tmpS);

            OCIO_ICS(8bits, "");

            OCIO_ICS(16bits, "");

            OCIO_ICS(32bits, "");

            OCIO_ICS(half, "");

            OCIO_ICS(float, "");
        }
#endif

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

        win.get("always_save_on_exit", tmp, 0);
        uiPrefs->uiAlwaysSaveOnExit->value((bool)tmp);

        if (tmp)
        {
            uiPrefs->uiWindowFixedPosition->value((bool)tmp);
            uiPrefs->uiWindowFixedSize->value((bool)tmp);
        }
        else
        {
            win.get("fixed_position", tmp, 0);
            uiPrefs->uiWindowFixedPosition->value((bool)tmp);
            win.get("fixed_size", tmp, 0);
            uiPrefs->uiWindowFixedSize->value((bool)tmp);
        }
        win.get("x_position", tmp, 0);
        uiPrefs->uiWindowXPosition->value(tmp);
        win.get("y_position", tmp, 0);
        uiPrefs->uiWindowYPosition->value(tmp);

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
            prefspath().c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)0);
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
        LogDisplay::prefs = (LogDisplay::ShowPreferences)tmp;

        //
        // Hotkeys
        //
        reset_hotkeys();
        if (!resetHotkeys)
        {
            msg =
                tl::string::Format(_("Loading hotkeys from \"{0}{1}.prefs\"."))
                    .arg(prefspath())
                    .arg(hotkeys_file);
            load_hotkeys();
        }
        else
        {
            msg = tl::string::Format(_("Reseting hotkeys to default."));
        }
        LOG_INFO(msg);

        // Fill the hotkeys window
        HotkeyUI* h = ui->uiHotkey;
        fill_ui_hotkeys(h->uiFunction);

        std_any value;

        int v = settings->getValue<int>("Performance/AudioBufferFrameCount");
        if (v < 1024)
        {
            settings->setValue(
                "Performance/AudioBufferFrameCount",
                (int)timeline::PlayerOptions().audioBufferFrameCount);
        }

        int r = settings->getValue<int>(kPenColorR);
        int g = settings->getValue<int>(kPenColorG);
        int b = settings->getValue<int>(kPenColorB);
        int a = settings->getValue<int>(kPenColorA);

        ui->uiPenColor->color((Fl_Color)61);
        Fl_Color c = (Fl_Color)ui->uiPenColor->color();
        Fl::set_color(c, r, g, b);

        settings->setValue(kPenColorR, r);
        settings->setValue(kPenColorG, g);
        settings->setValue(kPenColorB, b);
        settings->setValue(kPenColorA, a);

        r = settings->getValue<int>(kOldPenColorR);
        g = settings->getValue<int>(kOldPenColorG);
        b = settings->getValue<int>(kOldPenColorB);
        a = settings->getValue<int>(kOldPenColorA);

        ui->uiOldPenColor->color((Fl_Color)62);
        c = (Fl_Color)ui->uiOldPenColor->color();
        Fl::set_color(c, r, g, b);

        settings->setValue(kOldPenColorR, r);
        settings->setValue(kOldPenColorG, g);
        settings->setValue(kOldPenColorB, b);
        settings->setValue(kOldPenColorA, a);

        ui->uiPenOpacity->value(a / 255.0F);

        // Handle background options

        timeline::BackgroundOptions backgroundOptions;
        backgroundOptions.type = static_cast<timeline::Background>(
            settings->getValue<int>("Background/Type"));

        int color = settings->getValue<int>("Background/SolidColor");
        backgroundOptions.solidColor = from_fltk_color(color);

        int size = settings->getValue<int>("Background/CheckersSize");
        backgroundOptions.checkersSize = math::Size2i(size, size);

        color = settings->getValue<int>("Background/CheckersColor0");
        backgroundOptions.checkersColor0 = from_fltk_color(color);

        color = settings->getValue<int>("Background/CheckersColor1");
        backgroundOptions.checkersColor1 = from_fltk_color(color);

        ui->uiView->setBackgroundOptions(backgroundOptions);

        // Handle Dockgroup size (based on percentage)
        float pct = settings->getValue<float>("gui/DockGroup/Width");
        if (pct < 0.2F)
            pct = 0.2F;
        int width = ui->uiViewGroup->w() * pct;

        int visible = settings->getValue<int>("gui/DockGroup/Visible");
        if (visible)
            ui->uiDockGroup->show();

        // Set a minimum size for dockgroup
        if (width < 270)
            width = 270;

        ui->uiViewGroup->fixed(ui->uiDockGroup, width);
    }

    void Preferences::open_windows()
    {
        std_any value;
        int visible;

        ViewerUI* ui = App::ui;
        SettingsObject* settings = ViewerUI::app->settings();

        if (!ui->uiView->getPresentationMode())
        {
            // Handle windows/panels
            const WindowCallback* wc = kWindowCallbacks;
            for (; wc->name; ++wc)
            {
                std::string key = "gui/";
                key += wc->name;
                key += "/Window/Visible";
                visible = settings->getValue<int>(key);
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
        visible = settings->getValue<int>(key);
        if (visible)
            toggle_secondary_cb(nullptr, ui);
    }

    void Preferences::save()
    {
        int i;
        ViewerUI* ui = App::ui;
        auto app = ui->app;
        auto uiPrefs = ViewerUI::uiPrefs;
        auto settings = app->settings();

        locale::SetAndRestore saved;

        int visible = 0;
        if (uiPrefs->uiMain->visible())
            visible = 1;
        settings->setValue("gui/Preferences/Window/Visible", visible);

        // Handle background options
        auto backgroundOptions = ui->uiView->getBackgroundOptions();
        settings->setValue(
            "gui/Background/Options", static_cast<int>(backgroundOptions.type));

        int width = ui->uiDockGroup->w() == 0 ? 1 : ui->uiDockGroup->w();
        float pct = (float)width / ui->uiViewGroup->w();
        settings->setValue("gui/DockGroup/Width", pct);

        visible = 0;
        if (ui->uiDockGroup->visible())
            visible = 1;
        settings->setValue("gui/DockGroup/Visible", visible);

        Fl_Preferences base(
            prefspath().c_str(), "filmaura", "mrv2",
            (Fl_Preferences::Root)(int)Fl_Preferences::CLEAR);
        base.set("version", kPreferencesVersion);

        Fl_Preferences fltk_settings(base, "settings");
        fltk_settings.clear();

        const std::vector< std::string >& keys = settings->keys();
        for (auto key : keys)
        {
            std::any value = settings->getValue<std::any>(key);
            try
            {
                double tmpD = std::any_cast<double>(value);
                key = "d#" + key;
                fltk_settings.set(key.c_str(), tmpD);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                float tmpF = std::any_cast<float>(value);
                key = "f#" + key;
                fltk_settings.set(key.c_str(), tmpF);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std::any_cast<int>(value);
                key = "i#" + key;
                fltk_settings.set(key.c_str(), tmp);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                int tmp = std::any_cast<bool>(value);
                key = "b#" + key;
                fltk_settings.set(key.c_str(), tmp);
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string& tmpS = std::any_cast<std::string>(value);
                key = "s#" + key;
                fltk_settings.set(key.c_str(), tmpS.c_str());
                continue;
            }
            catch (const std::bad_cast& e)
            {
            }
            try
            {
                const std::string tmpS = std::any_cast<char*>(value);
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
        const std::vector< std::string >& files = settings->recentFiles();
        for (unsigned i = 1; i <= files.size(); ++i)
        {
            char buf[16];
            snprintf(buf, 16, "File #%d", i);
            recent_files.set(buf, files[i - 1].c_str());
        }

        Fl_Preferences recent_hosts(base, "recentHosts");
        const std::vector< std::string >& hosts = settings->recentHosts();
        for (unsigned i = 1; i <= hosts.size(); ++i)
        {
            char buf[16];
            snprintf(buf, 16, "Host #%d", i);
            recent_hosts.set(buf, hosts[i - 1].c_str());
        }

        Fl_Preferences python_scripts(base, "pythonScripts");
        const std::vector< std::string >& scripts = settings->pythonScripts();
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
            win.set(
                "secondary_on_top",
                (int)uiPrefs->uiPrefsSecondaryOnTop->value());
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
        gui.set("remove_edls", uiPrefs->uiPrefsRemoveEDLs->value());
        gui.set("timeline_edit_mode", uiPrefs->uiPrefsEditMode->value());
        gui.set(
            "timeline_edit_thumbnails",
            uiPrefs->uiPrefsEditThumbnails->value());
        gui.set(
            "timeline_edit_transitions",
            uiPrefs->uiPrefsShowTransitions->value());
        gui.set("timeline_edit_markers", uiPrefs->uiPrefsShowMarkers->value());
        gui.set("timeline_editable", uiPrefs->uiPrefsTimelineEditable->value());
        gui.set(
            "timeline_edit_associated_clips",
            uiPrefs->uiPrefsEditAssociatedClips->value());

        //
        // ui/view prefs
        //
        Fl_Preferences view(gui, "view");
        view.set("gain", uiPrefs->uiPrefsViewGain->value());
        view.set("gamma", uiPrefs->uiPrefsViewGamma->value());

        view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value());
        view.set("video_levels", uiPrefs->uiPrefsVideoLevels->value());
        view.set("alpha_blend", uiPrefs->uiPrefsAlphaBlend->value());
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
            ocio.set(
                "use_active_views", uiPrefs->uiOCIOUseActiveViews->value());

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
            bool always_save_on_exit = uiPrefs->uiAlwaysSaveOnExit->value();
            win.set("always_save_on_exit", always_save_on_exit);

            if (!always_save_on_exit)
            {
                win.set(
                    "fixed_position", uiPrefs->uiWindowFixedPosition->value());
                win.set("fixed_size", uiPrefs->uiWindowFixedSize->value());
                win.set("x_position", uiPrefs->uiWindowXPosition->value());
                win.set("y_position", uiPrefs->uiWindowYPosition->value());
                win.set("x_size", uiPrefs->uiWindowXSize->value());
                win.set("y_size", uiPrefs->uiWindowYSize->value());
            }
            else
            {
                win.set("fixed_position", 1);
                win.set("fixed_size", 1);
                win.set("x_position", ui->uiMain->x());
                win.set("y_position", ui->uiMain->y());
                win.set("x_size", ui->uiMain->w());
                win.set("y_size", ui->uiMain->h());
            }
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
            prefspath().c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
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

        {

            Fl_Preferences keys(
                prefspath().c_str(), "filmaura", hotkeys_file.c_str(),
                (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
            save_hotkeys(keys);

            msg = tl::string::Format(
                      _("Hotkeys have been saved to \"{0}{1}.prefs\"."))
                      .arg(prefspath())
                      .arg(hotkeys_file);
            LOG_INFO(msg);
        }

        base.flush();

        msg = tl::string::Format(_("Preferences have been saved to: "
                                   "\"{0}mrv2.prefs\"."))
                  .arg(prefspath());
        LOG_INFO(msg);

        check_language(uiPrefs, language_index, app);
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

        check_language(uiPrefs, language_index, app);

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
                ui->uiMenuBar->textsize(12);
                ui->uiMenuGroup->add(ui->uiMenuBar);
                ui->uiMenuGroup->redraw();
            }
        }
#endif

        SettingsObject* settings = ViewerUI::app->settings();

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

        //
        // Edit mode options
        //
        auto options = ui->uiTimeline->getItemOptions();
        options.showTransitions = uiPrefs->uiPrefsShowTransitions->value();
        options.showMarkers = uiPrefs->uiPrefsShowMarkers->value();
        ui->uiTimeline->setEditable(uiPrefs->uiPrefsTimelineEditable->value());
        int thumbnails = uiPrefs->uiPrefsEditThumbnails->value();
        options.thumbnails = true;
        switch (thumbnails)
        {
        case 0:
            options.thumbnails = false;
            break;
        case 1: // Small
            options.thumbnailHeight = 100;
            break;
        case 2: // Medium
            options.thumbnailHeight = 200;
            break;
        case 3: // Large
            options.thumbnailHeight = 300;
            break;
        }
        options.waveformHeight = options.thumbnailHeight / 2;
        ui->uiTimeline->setItemOptions(options);

        if (uiPrefs->uiPrefsTimeline->value())
        {
            ui->uiBottomBar->show();
            if (ui->uiEdit->value())
                set_edit_mode_cb(EditMode::kFull, ui);
            else
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

        panel::onlyOne((bool)uiPrefs->uiPrefsOnePanelOnly->value());

        //
        // Widget/Viewer settings
        //

        {
            ui->uiView->setGhostNext(settings->getValue<int>(kGhostNext));
            ui->uiView->setGhostPrevious(
                settings->getValue<int>(kGhostPrevious));

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

        // Handle Safe areas
        bool safeAreas = (bool)uiPrefs->uiPrefsSafeAreas->value();
        view->setSafeAreas(safeAreas);

        // Handle image options
        auto imageOptions = app->imageOptions();
        int alphaBlend = uiPrefs->uiPrefsAlphaBlend->value();
        int videoLevels = uiPrefs->uiPrefsVideoLevels->value();
        imageOptions.alphaBlend = static_cast<timeline::AlphaBlend>(alphaBlend);
        imageOptions.videoLevels =
            static_cast<timeline::InputVideoLevels>(videoLevels);
        app->setImageOptions(imageOptions);

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

        bool value = uiPrefs->uiPrefsAlwaysOnTop->value();
        int fullscreen_active = ui->uiMain->fullscreen_active();
        if (!fullscreen_active)
        {
            ui->uiMain->always_on_top(value);
        }

        DBG3;

        SecondaryWindow* secondary = ui->uiSecondary;
        if (secondary)
        {
            auto window = secondary->window();
            if (window->visible() && !window->fullscreen_active())
            {
                bool value = uiPrefs->uiPrefsSecondaryOnTop->value();
                window->always_on_top(value);
            }
        }

        view->refreshWindows();

#ifdef MRV2_NETWORK
        if (uiPrefs->uiPrefsSingleInstance->value())
        {
            ImageSender sender;
            if (!sender.isRunning())
            {
                app->createListener();
            }
        }
        else
        {
            app->removeListener();
        }
#endif

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void Preferences::updateICS()
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

        Fl_Preferences base(
            prefspath().c_str(), "filmaura", "mrv2", (Fl_Preferences::Root)0);
        Fl_Preferences gui(base, "ui");
        gui.set("single_instance", uiPrefs->uiPrefsSingleInstance->value());
        gui.set(
            "single_instance", (int)uiPrefs->uiPrefsSingleInstance->value());
        base.flush();
    }

    //////////////////////////////////////////////////////
    // OCIO
    /////////////////////////////////////////////////////
    void Preferences::OCIO(ViewerUI* ui)
    {
#ifdef TLRENDER_OCIO
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

                bool use_active = uiPrefs->uiOCIOUseActiveViews->value();

                std::vector<std::string> active_displays;
                const char* displaylist = config->getActiveDisplays();
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
                    int numDisplays = config->getNumDisplays();
                    for (int i = 0; i < numDisplays; ++i)
                    {
                        active_displays.push_back(config->getDisplay(i));
                    }
                }

                std::vector<std::string> active_views;
                const char* viewlist = config->getActiveViews();
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
                    std::string quoted_display = commentCharacter(display, '/');

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
                    menu += commentCharacter(space, '/');
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
#endif
    }

} // namespace mrv
