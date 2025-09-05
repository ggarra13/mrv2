// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/AudioSystem.h>
#include <tlCore/StringFormat.h>

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
#include "mrvApp/mrvApp.h"

#include "mrvPreferencesUI.h"
#include "mrvHotkeyUI.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvOCIO.h"
#include "mrvCore/mrvOS.h"

namespace
{
    const char* kModule = "pref";
    const int kPreferencesVersion = 9;
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

    std::string Preferences::root;
    int Preferences::debug = 0;
    int Preferences::logLevel = 0;
    std::string Preferences::hotkeys_file = "mrv2.keys";

    int Preferences::language_index = 0; // English
    int Preferences::switching_images = 0;

    int Preferences::bgcolor;
    int Preferences::textcolor;
    int Preferences::selectioncolor;
    int Preferences::selectiontextcolor;

    Preferences::Preferences(bool resetSettings, bool resetHotkeys)
    {
        load(resetSettings, resetHotkeys);
    }

    void Preferences::load(bool resetSettings, bool resetHotkeys)
    {
        ViewerUI* ui = App::ui;
        PreferencesUI* uiPrefs = ui->uiPrefs;

        bool ok;
        int version;
        int tmp;
        double tmpD;
        float tmpF;
        char tmpS[4096];

        locale::SetAndRestore saved;

        std::string userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.prefs"))
            userprefspath = prefspath();
        
        std::string msg =
            tl::string::Format(_("Reading preferences from \"{0}{1}\"."))
                .arg(userprefspath)
                .arg("mrv2.prefs");
        LOG_INFO(msg);

        Fl_Preferences base(
            userprefspath.c_str(), "filmaura", "mrv2",
            (Fl_Preferences::Root)0);

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
                    fltk_settings.get(key, tmpS, "", 4096);
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

        // If reading a version 7 or earlier, make sure to set ffmpeg color
        // accuracy off to avoid issues of users complaining about playback
        // performance on movies with no color space.
        if (version <= 7)
        {
            settings->setValue("Performance/FFmpegColorAccuracy", 0);
        }

        Fl_Preferences recent_files(base, "recentFiles");
        num = recent_files.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "File #%d", i);
            if (recent_files.get(buf, tmpS, "", 4096))
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

        Fl_Preferences recent_hosts(base, "recentHosts");
        num = recent_hosts.entries();
        settings->addRecentHost("localhost");
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Host #%d", i);
            if (recent_hosts.get(buf, tmpS, "", 4096))
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

        Fl_Preferences python_scripts(base, "pythonScripts");
        num = python_scripts.entries();
        for (unsigned i = num; i > 0; --i)
        {
            char buf[16];
            snprintf(buf, 16, "Script #%d", i);
            if (python_scripts.get(buf, tmpS, "", 4096))
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

        int rgb =
            settings->getValue<int>("Performance/FFmpegYUVToRGBConversion");
        if (rgb)
        {
            LOG_WARNING(_("FFmpeg YUV to RGB Conversion is on in Settings "
                          "Panel.  mrv2 will play back movies slower."));
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

        gui.get("raise_on_enter", tmp, 0);
        uiPrefs->uiPrefsRaiseOnEnter->value((bool)tmp);

        gui.get("timeline_display", tmp, 0);
        uiPrefs->uiPrefsTimelineDisplay->value(tmp);

        gui.get("timeline_video_offset", tmpF, 0.0);
        uiPrefs->uiStartTimeOffset->value(tmpF);

        gui.get("timeline_thumbnails", tmp, 1);
        uiPrefs->uiPrefsTimelineThumbnails->value(tmp);

        gui.get("panel_thumbnails", tmp, 1);
        uiPrefs->uiPrefsPanelThumbnails->value(tmp);

        gui.get("panel_thumbnails_manually", tmp, 0);
        uiPrefs->uiPrefsManualPanelThumbnails->value(tmp);

        gui.get("remove_edls", tmp, 1);
        uiPrefs->uiPrefsRemoveEDLs->value(tmp);

        gui.get("timeline_edit_mode", tmp, 0);
        uiPrefs->uiPrefsEditMode->value(tmp);

        gui.get("timeline_edit_view", tmp, 0);
        uiPrefs->uiPrefsEditView->value(tmp);

        gui.get("timeline_edit_thumbnails", tmp, 1);
        uiPrefs->uiPrefsEditThumbnails->value(tmp);

        gui.get("timeline_edit_transitions", tmp, 1);
        uiPrefs->uiPrefsShowTransitions->value(tmp);

        gui.get("timeline_edit_markers", tmp, 0);
        uiPrefs->uiPrefsShowMarkers->value(tmp);

        gui.get("timeline_editable", tmp, 1);
        uiPrefs->uiPrefsTimelineEditable->value(tmp);

        gui.get("timeline_edit_associated_clips", tmp, 1);
        uiPrefs->uiPrefsEditAssociatedClips->value(tmp);

#ifdef __APPLE__
        {
            auto itemOptions = ui->uiTimeline->getDisplayOptions();
            itemOptions.thumbnailFade = 0;
            ui->uiTimeline->setDisplayOptions(itemOptions);
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

        view.get("auto_frame", tmp, 1);
        uiPrefs->uiPrefsAutoFrame->value((bool)tmp);

        view.get("safe_areas", tmp, 0);
        uiPrefs->uiPrefsSafeAreas->value((bool)tmp);

        view.get("ocio_in_top_bar", tmp, 0);
        uiPrefs->uiPrefsOCIOInTopBar->value((bool)tmp);

        view.get("video_levels", tmp, 0);
        uiPrefs->uiPrefsVideoLevels->value(tmp);

        view.get("alpha_blend", tmp, 1);
        uiPrefs->uiPrefsAlphaBlend->value(tmp);

        view.get("minify_filter", tmp, 1);
        uiPrefs->uiPrefsMinifyFilter->value(tmp);

        view.get("magnify_filter", tmp, 1);
        uiPrefs->uiPrefsMagnifyFilter->value(tmp);

        view.get("crop_area", tmp, 0);
        uiPrefs->uiPrefsCropArea->value(tmp);

        view.get("zoom_speed", tmp, 2);
        uiPrefs->uiPrefsZoomSpeed->value(tmp);

        //
        // HDR
        //
        Fl_Preferences hdr(gui, "hdr");
        hdr.get("chromaticities", tmp, 0);
        uiPrefs->uiPrefsChromaticities->value(tmp);
        
        hdr.get("hdr_data", tmp, 0);
        uiPrefs->uiPrefsHDRInfo->value(tmp);
        
        hdr.get("tonemap_algorithm", tmp, 8);  // Hable is default, as VLC
        uiPrefs->uiPrefsTonemapAlgorithm->value(tmp);
        
        hdr.get("gamut_mapping", tmp, 0);  // Auto is default
        uiPrefs->uiPrefsGamutMapping->value(tmp);
        
        DBG3;
        //
        // ui/colors
        //

        Fl_Preferences colors(gui, "colors");

        colors.get("background_color", bgcolor, 0x43434300);

        colors.get("text_color", textcolor, 0xababab00);

        colors.get("selection_color", selectioncolor, 0x97a8a800);

        colors.get("selection_text_color", selectiontextcolor, 0x00000000);

        colors.get("scheme", tmpS, "gtk+", 4096);

        Fl::scheme(tmpS);
        
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

        colors.get("theme", tmpS, "Black", 4096);

        auto context = App::app->getContext();
        schemes.setContext(context);

        const Fl_Menu_Item* item = uiPrefs->uiColorTheme->find_item(tmpS);
        if (item)
        {
            uiPrefs->uiColorTheme->picked(item);
        }

        const char* language = fl_getenv("LANGUAGE");
        if (!language || language[0] == '\0')
            language = fl_getenv("LC_ALL");
        if (!language || language[0] == '\0')
            language = fl_getenv("LC_MESSAGES");
        if (!language || language[0] == '\0')
            language = fl_getenv("LANG");

        int uiIndex = 0;
        if (language && strlen(language) > 1)
        {
            language_index = -1;
            const auto languageCodes = getLanguageCodes();
            for (const auto& code : languageCodes)
            {
                if (code == language)
                {
                    language_index = uiIndex;
                    language = strdup(code.c_str());
                    break;
                }
                ++uiIndex;
            }

            if (language_index == -1)
            {
                uiIndex = 0;
                for (const auto& code : languageCodes)
                {
                    if (strcmp(language, "C") == 0)
                    {
                        language = "en_US.UTF-8";
                        break;
                    }
                    if (strncmp(language, code.c_str(), 2) == 0)
                    {
                        language_index = uiIndex;
                        language = strdup(code.c_str());
                        break;
                    }
                    ++uiIndex;
                }
            }

            if (language_index == -1)
                language_index = 0;
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

        //
        // UI Fonts
        //
        Fl_Preferences fonts(gui, "fonts");
        fonts.get("menus", tmp, FL_HELVETICA);
        uiPrefs->uiFontMenus->value(tmp);

        fonts.get("panels", tmp, FL_HELVETICA);
        uiPrefs->uiFontPanels->value(tmp);

        Fl_Preferences ocio(view, "ocio");

        //////////////////////////////////////////////////////
        // OCIO
        /////////////////////////////////////////////////////
        std::string ocioPath = studiopath() + "mrv2.ocio.json";
        if (file::isReadable(ocioPath))
        {
            ocio::loadPresets(ocioPath);
        }
        else
        {
            ocioPath = prefspath() + "mrv2.ocio.json";
            if (file::isReadable(ocioPath))
            {
                ocio::loadPresets(ocioPath);
            }
        }

        const char* var = fl_getenv("OCIO");
        {
            const char* kModule = "ocio";
            
            if (!var || strlen(var) == 0)
            {
                ocio.get("config", tmpS, "", 4096);

                if (strlen(tmpS) != 0)
                {
                    if (ocio::ocioDefault != tmpS)
                    {
                        LOG_INFO(_("Setting OCIO config from preferences."));
                        setConfig(tmpS);
                    }
                }
            }
            else
            {
                LOG_INFO(_("Setting OCIO config from OCIO environment variable."));
                setConfig(var);
            }
        }

        var = uiPrefs->uiPrefsOCIOConfig->value();
        if (!var || strlen(var) == 0 || resetSettings)
        {
            setConfig(ocio::ocioDefault);
        }

        ocio.get("use_default_display_view", tmp, 0);
        uiPrefs->uiOCIOUseDefaultDisplayView->value(tmp);

        ocio.get("use_active_views", tmp, 1);
        uiPrefs->uiOCIOUseActiveViews->value(tmp);

        Fl_Preferences ics(ocio, "ICS");
        {
#define OCIO_ICS(x, d)                                                         \
    ok = ics.get(#x, tmpS, d, 4096);                                           \
    uiPrefs->uiOCIO_##x##_ics->value(tmpS);

            OCIO_ICS(8bits, "");

            OCIO_ICS(16bits, "");

            OCIO_ICS(32bits, "");

            OCIO_ICS(half, "");

            OCIO_ICS(float, "");
        }

        Fl_Preferences display_view(ocio, "DisplayView");
        display_view.get("DisplayView", tmpS, "", 4096);
        uiPrefs->uiOCIO_Display_View->value(tmpS);

        Fl_Preferences look(ocio, "Look");
        look.get("Look", tmpS, "", 4096);
        uiPrefs->uiOCIO_Look->value(tmpS);

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

        playback.get("single_click_playback", tmp, 0);
        uiPrefs->uiPrefsSingleClickPlayback->value(tmp);

        playback.get("auto_hide_pixel_bar", tmp, kAutoHideOpenGLOnly);
        uiPrefs->uiPrefsAutoHidePixelBar->value(tmp);

        playback.get("fps", tmpF, 24.0);
        uiPrefs->uiPrefsFPS->value(tmpF);

        playback.get("loop", tmp, 0);
        uiPrefs->uiPrefsLoopMode->value(tmp);

        playback.get("scrubbing_sensitivity", tmpF, 5.0f);
        uiPrefs->uiPrefsScrubbingSensitivity->value(tmpF);

        playback.get("scrub_auto_playback", tmp, 1);
        uiPrefs->uiPrefsScrubAutoPlay->value(tmp);

        playback.get("scrubbing_loop_mode", tmp, 0);
        uiPrefs->uiPrefsScrubbingLoopMode->value(tmp);

            
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

#if defined(__APPLE__) || defined(_WIN32)
        loading.get("native_file_chooser", tmp, 1);
#else
        loading.get("native_file_chooser", tmp, 0);
#endif
        uiPrefs->uiPrefsNativeFileChooser->value((bool)tmp);

        loading.get("missing_frame_type", tmp, 0);
        uiPrefs->uiMissingFrameType->value(tmp);

        loading.get("version_regex", tmpS, "_v", 4096);
        if (strlen(tmpS) == 0)
        {
            strcpy(tmpS, "_v");
        }
        uiPrefs->uiPrefsVersionRegex->value(tmpS);

        loading.get("max_images_apart", tmp, 10);
        uiPrefs->uiPrefsMaxImagesApart->value(tmp);

        char key[2048];

        std::string mappingpath = studiopath();
        if (!file::isReadable(mappingpath + "/mrv2.paths.pref"))
            mappingpath = prefspath();

        Fl_Preferences path_mapping(
            mappingpath.c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)0);
        num = path_mapping.entries();
        for (int i = 0; i < num; ++i)
        {
            snprintf(key, 2048, "Path #%d", i + 1);
            path_mapping.get(key, tmpS, "", 4096);
            if (strlen(tmpS) == 0)
                continue;
            uiPrefs->PathMappings->add(tmpS);
        }
        msg = tl::string::Format(_("Path mappings have been loaded from "
                                   "\"{0}{1}\"."))
                  .arg(mappingpath)
                  .arg("mrv2.paths.prefs");
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

        errors.get("ffmpeg_log_display", tmp, 0);
        uiPrefs->uiPrefsRaiseLogWindowOnFFmpegError->value(tmp);
        LogDisplay::ffmpegPrefs = (LogDisplay::ShowPreferences)tmp;

        Fl_Preferences opengl(base, "opengl");

        opengl.get("vsync", tmp, 1);
        uiPrefs->uiPrefsOpenGLVsync->value(tmp);

        opengl.get("color_buffers_accuracy", tmp, 0);
        uiPrefs->uiPrefsColorAccuracy->value(tmp);

        opengl.get("blit_viewports", tmp, 0);
        uiPrefs->uiPrefsBlitMainViewport->value(tmp);
        uiPrefs->uiPrefsBlitSecondaryViewport->value(tmp);
        
        opengl.get("blit_main_viewport", tmp, 0);
        uiPrefs->uiPrefsBlitMainViewport->value(tmp);
        
        opengl.get("blit_secondary_viewport", tmp, 0);
        uiPrefs->uiPrefsBlitSecondaryViewport->value(tmp);

        opengl.get("blit_timeline", tmp, 0);
        uiPrefs->uiPrefsBlitTimeline->value(tmp);
        
        Fl_Preferences vulkan(base, "vulkan");
        vulkan.get("gpu_main_viewport", tmp, 0);
        uiPrefs->uiPrefsMainViewportGPU->value(tmp);
        
        vulkan.get("gpu_secondary_viewport", tmp, 0);
        uiPrefs->uiPrefsSecondaryViewportGPU->value(tmp);
    
        vulkan.get("gpu_timeline", tmp, 0);
        uiPrefs->uiPrefsTimelineGPU->value(tmp);

        //
        // Audio
        //
        Fl_Preferences audio(base, "audio");

        audio.get("API", tmp, 0);
        uiPrefs->uiPrefsAudioAPI->value(tmp);

        audio.get("output_device", tmp, 0);
        uiPrefs->uiPrefsAudioOutputDevice->value(tmp);

        Fl_Preferences ComfyUI(base, "comfyUI");

        ComfyUI.get("input_pipe", tmp, 0);
        uiPrefs->uiPrefsUseComfyUIPipe->value((bool)tmp);

        
        Fl_Preferences behavior(base, "behavior");

        behavior.get("check_for_updates", tmp, 0);
        uiPrefs->uiPrefsCheckForUpdates->value(tmp);

        behavior.get("allow_screen_saver", tmp, 0);
        uiPrefs->uiPrefsAllowScreenSaver->value(tmp);
        
        
        //
        // Hotkeys
        //
        reset_hotkeys();
        if (!resetHotkeys)
        {
            std::string hotkeyPath = studiopath() + hotkeys_file + ".prefs";
            if (file::isReadable(hotkeyPath))
            {
                msg =
                    tl::string::Format(_("Loading hotkeys from \"{0}{1}.prefs\"."))
                    .arg(studiopath())
                    .arg(hotkeys_file);
            }
            else
            {
                msg =
                    tl::string::Format(_("Loading hotkeys from \"{0}{1}.prefs\"."))
                    .arg(prefspath())
                    .arg(hotkeys_file);;
            }
            load_hotkeys();
        }
        else
        {
            msg = tl::string::Format(_("Resetting hotkeys to default."));
        }
        LOG_STATUS(msg);

        // Fill the hotkeys window
        HotkeyUI* h = ui->uiHotkey;
        fill_ui_hotkeys(h->uiFunction);

        // Update hotkeys tooltips in UI.
        update_hotkey_tooltips();

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

        Fl_Color color;
        int size = settings->getValue<int>("Background/CheckersSize");
        backgroundOptions.checkersSize = math::Size2i(size, size);

        color = settings->getValue<int>("Background/color0");
        backgroundOptions.color0 = from_fltk_color(color);

        color = settings->getValue<int>("Background/color1");
        backgroundOptions.color1 = from_fltk_color(color);

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

        std::string userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.prefs"))
            userprefspath = prefspath();
        
        if (!ui->uiView->getPresentationMode())
        {
            // Handle panels
            Fl_Preferences base(
                userprefspath.c_str(), "filmaura", "mrv2",
                (Fl_Preferences::Root)0);

            Fl_Preferences panel_list(base, "panels");
            unsigned numPanels = panel_list.entries();
            for (unsigned i = 0; i < numPanels; ++i)
            {
                const char* key = panel_list.entry(i);
                show_window_cb(key, ui);
            }

            // Handle windows
            const WindowCallback* wc = kWindowCallbacks;
            for (; wc->name; ++wc)
            {
                std::string key = "gui/";
                key += wc->name;
                key += "/Window";
                std_any value = settings->getValue<std::any>(key);
                int window =
                    std_any_empty(value) ? 0 : std_any_cast<int>(value);
                if (!window)
                    continue;

                key = "gui/";
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

        int W = ui->uiMain->w();
        int H = ui->uiMain->h();

        settings->setValue("gui/Main/Window/Width", W);
        settings->setValue("gui/Main/Window/Height", H);

        int visible = 0;
        if (uiPrefs->uiMain->visible())
            visible = 1;
        settings->setValue("gui/Preferences/Window/Visible", visible);

        int width = ui->uiDockGroup->w() <= 0 ? 1 : ui->uiDockGroup->w();
        float pct = (float)width / ui->uiViewGroup->w();
        settings->setValue("gui/DockGroup/Width", pct);

        visible = 0;
        if (ui->uiDockGroup->visible())
            visible = 1;
        settings->setValue("gui/DockGroup/Visible", visible);

        std::string ocioPath = prefspath() + "mrv2.ocio.json";
        ocio::savePresets(ocioPath);

        std::string userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.prefs"))
            userprefspath = prefspath();
        
        Fl_Preferences base(
            userprefspath.c_str(), "filmaura", "mrv2",
            (Fl_Preferences::Root)(int)Fl_Preferences::CLEAR);
        base.set("version", kPreferencesVersion);

        Fl_Preferences panel_list(base, "panels");
        panel_list.clear();
        // Get panel list so we keep the order
        auto panels = ui->uiDock->getPanelList();
        for (auto panel : panels)
        {
            panel_list.set(panel.c_str(), 1);
        }

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
        gui.set("raise_on_enter", (int)uiPrefs->uiPrefsRaiseOnEnter->value());

        gui.set("timeline_display", uiPrefs->uiPrefsTimelineDisplay->value());
        gui.set("timeline_video_offset", uiPrefs->uiStartTimeOffset->value());
        gui.set(
            "timeline_thumbnails", uiPrefs->uiPrefsTimelineThumbnails->value());
        gui.set("panel_thumbnails", uiPrefs->uiPrefsPanelThumbnails->value());
        gui.set("panel_thumbnails_manually",
                uiPrefs->uiPrefsManualPanelThumbnails->value());
        gui.set("remove_edls", uiPrefs->uiPrefsRemoveEDLs->value());
        gui.set("timeline_edit_mode", uiPrefs->uiPrefsEditMode->value());
        gui.set("timeline_edit_view", uiPrefs->uiPrefsEditView->value());
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

        view.set("auto_frame", uiPrefs->uiPrefsAutoFrame->value());
        view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value());
        view.set("ocio_in_top_bar", uiPrefs->uiPrefsOCIOInTopBar->value());
        view.set("video_levels", uiPrefs->uiPrefsVideoLevels->value());
        view.set("alpha_blend", uiPrefs->uiPrefsAlphaBlend->value());
        view.set("minify_filter", uiPrefs->uiPrefsMinifyFilter->value());
        view.set("magnify_filter", uiPrefs->uiPrefsMagnifyFilter->value());
        view.set("crop_area", uiPrefs->uiPrefsCropArea->value());
        view.set("zoom_speed", (int)uiPrefs->uiPrefsZoomSpeed->value());

        Fl_Preferences hdr(gui, "hdr");
        hdr.set("chromaticities", uiPrefs->uiPrefsChromaticities->value());
        hdr.set("hdr_data", uiPrefs->uiPrefsHDRInfo->value());
        hdr.set("tonemap_algorithm",
                uiPrefs->uiPrefsTonemapAlgorithm->value());
        hdr.set("gamut_mapping", uiPrefs->uiPrefsGamutMapping->value());
        
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

        //
        // UI Fonts
        //
        {
            Fl_Preferences fonts(gui, "fonts");
            fonts.set("menus", uiPrefs->uiFontMenus->value());
            fonts.set("panels", uiPrefs->uiFontPanels->value());
        }

        {
            Fl_Preferences ocio(view, "ocio");

            ocio.set("config", uiPrefs->uiPrefsOCIOConfig->value());
            ocio.set(
                "use_default_display_view",
                uiPrefs->uiOCIOUseDefaultDisplayView->value());
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

            Fl_Preferences display_view(ocio, "DisplayView");
            display_view.set(
                "DisplayView", uiPrefs->uiOCIO_Display_View->value());

            Fl_Preferences look(ocio, "Look");
            look.set("Look", uiPrefs->uiOCIO_Look->value());
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
        colors.set("scheme", Fl::scheme());
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
        playback.set(
            "single_click_playback",
            (int)uiPrefs->uiPrefsSingleClickPlayback->value());
        playback.set(
            "auto_hide_pixel_bar",
            (int)uiPrefs->uiPrefsAutoHidePixelBar->value());
        playback.set("fps", uiPrefs->uiPrefsFPS->value());
        playback.delete_entry("loop_mode"); // legacy preference
        playback.set("loop", uiPrefs->uiPrefsLoopMode->value());
        playback.set(
            "scrubbing_sensitivity",
            uiPrefs->uiPrefsScrubbingSensitivity->value());
        playback.set(
            "scrub_auto_playback", uiPrefs->uiPrefsScrubAutoPlay->value());

        playback.set("scrubbing_loop_mode", 
                     uiPrefs->uiPrefsScrubbingLoopMode->value());

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
            userprefspath.c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
        path_mapping.clear();
        for (int i = 2; i <= uiPrefs->PathMappings->size(); ++i)
        {
            snprintf(key, 256, "Path #%d", i - 1);
            path_mapping.set(key, uiPrefs->PathMappings->text(i));
        }
        std::string msg =
            tl::string::Format(_("Path mappings have been saved to "
                                 "\"{0}{1}\"."))
                .arg(prefspath())
                .arg("mrv2.paths.prefs");
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
        errors.set(
            "ffmpeg_log_display",
            (int)uiPrefs->uiPrefsRaiseLogWindowOnFFmpegError->value());

        Fl_Preferences opengl(base, "opengl");
        opengl.set("vsync", (int)uiPrefs->uiPrefsOpenGLVsync->value());
        opengl.set(
            "color_buffers_accuracy",
            (int)uiPrefs->uiPrefsColorAccuracy->value());
        opengl.set(
            "blit_main_viewport", (int)uiPrefs->uiPrefsBlitMainViewport->value());
        opengl.set(
            "blit_secondary_viewport", (int)uiPrefs->uiPrefsBlitSecondaryViewport->value());
        opengl.set("blit_timeline", (int)uiPrefs->uiPrefsBlitTimeline->value());
        
        Fl_Preferences vulkan(base, "vulkan");
        vulkan.set(
            "gpu_main_viewport",
            (int)uiPrefs->uiPrefsMainViewportGPU->value());
        vulkan.set(
            "gpu_secondary_viewport",
            (int)uiPrefs->uiPrefsSecondaryViewportGPU->value());
        vulkan.set("gpu_timeline",
                   (int)uiPrefs->uiPrefsTimelineGPU->value());

        Fl_Preferences ComfyUI(base, "comfyUI");
        ComfyUI.set("input_pipe", (int)uiPrefs->uiPrefsUseComfyUIPipe->value());

        Fl_Preferences audio(base, "audio");

        audio.set("API", (int)uiPrefs->uiPrefsAudioAPI->value());
        audio.set(
            "output_device", (int)uiPrefs->uiPrefsAudioOutputDevice->value());

        Fl_Preferences behavior(base, "behavior");
        behavior.set(
            "check_for_updates", (int)uiPrefs->uiPrefsCheckForUpdates->value());

        behavior.set("allow_screen_saver",
                     (int)uiPrefs->uiPrefsAllowScreenSaver->value());
        
        {

            Fl_Preferences keys(
                userprefspath.c_str(), "filmaura", hotkeys_file.c_str(),
                (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
            save_hotkeys(keys);

            msg = tl::string::Format(
                      _("Hotkeys have been saved to \"{0}{1}.prefs\"."))
                      .arg(userprefspath)
                      .arg(hotkeys_file);
            LOG_INFO(msg);
        }

        base.flush();

        msg = tl::string::Format(_("Preferences have been saved to: "
                                   "\"{0}{1}\"."))
                  .arg(userprefspath)
                  .arg("mrv2.prefs");
        LOG_INFO(msg);

        check_language(uiPrefs, language_index, app);
    }

    bool Preferences::set_transforms()
    {
        return true;
    }

    Preferences::~Preferences() {}

    void Preferences::reset()
    {
        const std::string prefs = prefspath() + "mrv2.prefs";
        LOG_INFO(_("Removing ") << prefs);
        fs::remove(prefs);
        Preferences::load();
        Preferences::run();
    }

    void Preferences::run()
    {
        auto ui = App::ui;
        PreferencesUI* uiPrefs = ui->uiPrefs;
        App* app = ui->app;
        Fl_Menu_Item* item = nullptr;

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

        MyViewport* view = ui->uiView;

        // Only redisplay the tool bars if not on Presentation
        // Mode. (User changed Preferences while on Presentation mode).
        if (!view->getPresentationMode())
        {
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

            const bool showPixelBar = uiPrefs->uiPrefsPixelToolbar->value();
            if (showPixelBar)
            {
                const auto player = ui->uiView->getTimelinePlayer();
                const int autoHide = uiPrefs->uiPrefsAutoHidePixelBar->value();
#ifdef OPENGL_BACKEND
                if (!autoHide || !player ||
                    player->playback() == timeline::Playback::Stop)
                {
                    ui->uiPixelBar->show();
                }
                else
                {
                    ui->uiPixelBar->hide();
                }
#endif

#ifdef VULKAN_BACKEND
                if (autoHide != kAutoHideOpenGLAndVulkan || !player ||
                    player->playback() == timeline::Playback::Stop)
                {
                    ui->uiPixelBar->show();
                }
                else
                {
                    ui->uiPixelBar->hide();
                }
#endif
            }
            else
            {
                ui->uiPixelBar->hide();
            }

            //
            // Edit mode options
            //
            auto options = ui->uiTimeline->getDisplayOptions();
            options.transitions = uiPrefs->uiPrefsShowTransitions->value();
            options.markers = uiPrefs->uiPrefsShowMarkers->value();
            ui->uiTimeline->setEditable(
                uiPrefs->uiPrefsTimelineEditable->value());
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
            options.trackInfo = settings->getValue<int>("Timeline/TrackInfo");
            options.clipInfo = settings->getValue<int>("Timeline/ClipInfo");
            ui->uiTimeline->setDisplayOptions(options);

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
            }
            else
            {
                ui->uiToolsGroup->hide();
            }

            ui->uiViewGroup->layout();
            ui->uiViewGroup->init_sizes();

            ui->uiRegion->layout();
         }

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
        ocio::setup();

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

        bool ocioInTopBar = uiPrefs->uiPrefsOCIOInTopBar->value();
        if (ocioInTopBar)
        {
            ui->uiOCIO->show();
            ui->uiCOLORS->hide();
        }
        else
        {
            ui->uiOCIO->hide();
            ui->uiCOLORS->show();
        }

        // Handle image options
        auto imageOptions = app->imageOptions();
        int alphaBlend = uiPrefs->uiPrefsAlphaBlend->value();
        int videoLevels = uiPrefs->uiPrefsVideoLevels->value();
        int minifyFilter = uiPrefs->uiPrefsMinifyFilter->value();
        int magnifyFilter = uiPrefs->uiPrefsMagnifyFilter->value();
        imageOptions.alphaBlend = static_cast<timeline::AlphaBlend>(alphaBlend);
        imageOptions.videoLevels =
            static_cast<timeline::InputVideoLevels>(videoLevels);
        app->setImageOptions(imageOptions);

        auto displayOptions = app->displayOptions();
        displayOptions.imageFilters.minify =
            static_cast<timeline::ImageFilter>(minifyFilter);
        displayOptions.imageFilters.magnify =
            static_cast<timeline::ImageFilter>(magnifyFilter);
        displayOptions.ignoreChromaticities =
            !uiPrefs->uiPrefsChromaticities->value();
        displayOptions.hdrInfo =
            static_cast<timeline::HDRInformation>(
                uiPrefs->uiPrefsHDRInfo->value());
        app->setDisplayOptions(displayOptions);
        
        timeline::HDROptions hdrOptions = ui->uiView->getHDROptions();
        hdrOptions.algorithm =
            static_cast<timeline::HDRTonemapAlgorithm>(uiPrefs->uiPrefsTonemapAlgorithm->value());
        hdrOptions.gamutMapping =
            static_cast<timeline::HDRGamutMapping>(uiPrefs->uiPrefsGamutMapping->value());
        ui->uiView->setHDROptions(hdrOptions);

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
        if (uiPrefs->uiWindowFixedPosition->value() ||
            uiPrefs->uiWindowFixedSize->value())
        {
            ui->uiView->resizeWindow();
        }

        bool frameView = (bool)uiPrefs->uiPrefsAutoFitImage->value();
        view->setFrameView(frameView);

        LogDisplay::prefs = (LogDisplay::ShowPreferences)
                                uiPrefs->uiPrefsRaiseLogWindowOnError->value();
        LogDisplay::ffmpegPrefs =
            (LogDisplay::ShowPreferences)
                uiPrefs->uiPrefsRaiseLogWindowOnFFmpegError->value();

        bool hasPresentation = view->getPresentationMode();

        Fl_Round_Button* r;

        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(0);
        int normal = r->value();

        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(1);
        int fullscreen = r->value();
        if (fullscreen)
        {
            ui->uiMain->show();
            view->setFullScreenMode(true);
        }
        
        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(2);
        int presentation = r->value();
        if (presentation)
        {
            ui->uiMain->show();
            view->setPresentationMode(true);
        }
        
        if (normal)
            view->setFullScreenMode(false);

        r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child(3);
        int maximized = r->value();
        if (maximized)
        {
            ui->uiMain->show();
            view->setMaximized();
        }

        bool value = uiPrefs->uiPrefsAlwaysOnTop->value();
        int fullscreen_active = ui->uiMain->fullscreen_active();
        if (!fullscreen_active)
        {
            ui->uiMain->always_on_top(value);
        }

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

#ifdef TLRENDER_AUDIO
        auto context = App::app->getContext();
        auto audioSystem = context->getSystem<audio::System>();
        if (audioSystem)
        {
            int api = uiPrefs->uiPrefsAudioAPI->value();
            const Fl_Menu_Item* item = uiPrefs->uiPrefsAudioAPI->child(api);
            if (item && item->label())
            {
                audioSystem->setAPI(item->label());
            }

            size_t outputDevice = uiPrefs->uiPrefsAudioOutputDevice->value();
            item = uiPrefs->uiPrefsAudioOutputDevice->child(outputDevice);
            if (item && item->label())
            {
                audioSystem->setOutputDevice(item->label());
            }
        }
#endif

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

        if (uiPrefs->uiPrefsUseComfyUIPipe->value())
        {
            app->createComfyUIListener();
        }
#endif

        ui->uiMain->allow_screen_saver((bool)uiPrefs->uiPrefsAllowScreenSaver->value());

        std::string userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.prefs"))
            userprefspath = prefspath();
        
        Fl_Preferences base(
            userprefspath.c_str(), "filmaura", "mrv2", (Fl_Preferences::Root)0);
        Fl_Preferences gui(base, "ui");
        gui.set("single_instance", uiPrefs->uiPrefsSingleInstance->value());
        gui.set(
            "single_instance", (int)uiPrefs->uiPrefsSingleInstance->value());
        base.flush();

        panel::redrawThumbnails();

        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    //////////////////////////////////////////////////////
    // OCIO
    /////////////////////////////////////////////////////
    void Preferences::setConfig(std::string configName)
    {
        static std::string oldConfigName;
        static const char* kModule = "ocio";

        if (oldConfigName == configName)
            return;

        PreferencesUI* uiPrefs = App::ui->uiPrefs;
        if (configName.substr(0, 7) != "ocio://")
        {
            if (file::isReadable(configName))
            {
                LOG_STATUS(_("OCIO config is now:"));
            }
            else
            {
                std::string msg =
                    tl::string::Format(
                        _("OCIO file \"{0}\" not found or not readable."))
                        .arg(configName);
                LOG_ERROR(msg);
                LOG_STATUS(_("Setting OCIO config to default:"));
                configName = ocio::ocioDefault;
            }
        }
        else if (configName == ocio::ocioDefault)
        {
            LOG_STATUS(_("Setting OCIO config to default:"));
            configName = ocio::ocioDefault;
        }
        else
        {
            LOG_STATUS(_("Setting OCIO config to built-in:"));
        }

        LOG_STATUS("\t" << configName);
        uiPrefs->uiPrefsOCIOConfig->value(configName.c_str());
        oldConfigName = configName;
    }

} // namespace mrv
