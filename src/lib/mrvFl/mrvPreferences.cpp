// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvPreferencesUI.h"
#include "mrvHotkeyUI.h"

#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvApp.h"

#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvOCIO.h"

#ifdef MRV2_NETWORK
#    include "mrvNetwork/mrvImageListener.h"
#endif

#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvMenus.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMedia.h"
#include "mrvCore/mrvUtil.h"

#include "mrvOS/mrvOS.h"

#include <tlCore/AudioSystem.h>
#include <tlCore/StringFormat.h>

#include <FL/fl_utf8.h>         // for fl_getenv
#include <FL/Fl_Sys_Menu_Bar.H> // for macOS menus

#include <algorithm>
#include <system_error>
#include <filesystem>
namespace fs = std::filesystem;

namespace
{
    const char* kModule = "pref";
    const int kPreferencesVersion = 10;
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
    std::string Preferences::hotkeys_file = "mrv2.keys";

    int Preferences::language_index = 0; // English
    int Preferences::switching_images = 0;

    int Preferences::bgcolor;
    int Preferences::textcolor;
    int Preferences::selectioncolor;
    int Preferences::selectiontextcolor;

    tl::io::MissingFrames Preferences::missingFrames =
        tl::io::MissingFrames::Hold;

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

        /* xgettext:c++-format */
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
                /* xgettext:c++-format */
                const std::string msg =
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
                /* xgettext:c++-format */
                const std::string msg =
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
                /* xgettext:c++-format */
                const std::string msg =
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
        uiPrefs->SingleInstance->value((bool)tmp);

        gui.get("tooltips", tmp, 1);
        uiPrefs->Tooltips->value((bool)tmp);

        gui.get("menubar", tmp, 1);
        uiPrefs->MenuBar->value((bool)tmp);

        gui.get("topbar", tmp, 1);
        uiPrefs->Topbar->value((bool)tmp);

        gui.get("pixel_toolbar", tmp, 1);
        uiPrefs->PixelToolbar->value((bool)tmp);

        gui.get("timeline_toolbar", tmp, 1);
        uiPrefs->Timeline->value((bool)tmp);

        gui.get("status_toolbar", tmp, 1);
        uiPrefs->StatusBar->value((bool)tmp);

        gui.get("action_toolbar", tmp, 1);
        uiPrefs->ToolBar->value((bool)tmp);

        gui.get("one_panel_only", tmp, 0);
        uiPrefs->OnePanelOnly->value((bool)tmp);

        gui.get("macOS_menus", tmp, 0);
        uiPrefs->MacOSMenus->value((bool)tmp);

        gui.get("raise_on_enter", tmp, 0);
        uiPrefs->RaiseOnEnter->value((bool)tmp);

        gui.get("timeline_display", tmp, 0);
        uiPrefs->TimelineDisplay->value(tmp);

        gui.get("timeline_video_offset", tmpF, 0.0);
        uiPrefs->uiStartTimeOffset->value(tmpF);

        gui.get("timeline_thumbnails", tmp, 0);
        uiPrefs->TimelineThumbnails->value(tmp);

        gui.get("panel_thumbnails", tmp, 1);

#ifdef __APPLE__
        int default_panel_thumbnails = 1; // Small as macOS users use laptops
#else
        int default_panel_thumbnails = 2; // Normal thumbnail size
#endif
        // If old panel thumbnails preferences was off, set size to None
        if (tmp == 0)
            default_panel_thumbnails = 0;
        gui.get("panel_thumbnails_size", tmp, default_panel_thumbnails);

        gui.get("files_panel_thumbnails_size", tmp, default_panel_thumbnails);
        uiPrefs->FilesPanelThumbnails->value(tmp);

        gui.get("compare_panel_thumbnails_size", tmp, default_panel_thumbnails);
        uiPrefs->ComparePanelThumbnails->value(tmp);

        gui.get("stereo3D_panel_thumbnails_size", tmp, default_panel_thumbnails);
        uiPrefs->Stereo3DPanelThumbnails->value(tmp);

        gui.get("panel_thumbnails_manually", tmp, 0);
        uiPrefs->ManualPanelThumbnails->value(tmp);

        gui.get("remove_edls", tmp, 1);
        uiPrefs->RemoveEDLs->value(tmp);

        gui.get("timeline_edit_mode", tmp, 0);
        uiPrefs->EditMode->value(tmp);

        gui.get("timeline_edit_view", tmp, 0);
        uiPrefs->EditView->value(tmp);

        gui.get("timeline_edit_thumbnails", tmp, 1);
        uiPrefs->EditThumbnails->value(tmp);

        gui.get("timeline_edit_transitions", tmp, 1);
        uiPrefs->ShowTransitions->value(tmp);

        gui.get("timeline_edit_markers", tmp, 0);
        uiPrefs->ShowMarkers->value(tmp);

        gui.get("timeline_editable", tmp, 1);
        uiPrefs->TimelineEditable->value(tmp);

        gui.get("timeline_edit_associated_clips", tmp, 1);
        uiPrefs->EditAssociatedClips->value(tmp);

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
            uiPrefs->AutoFitImage->value(tmp);

            win.get("always_on_top", tmp, 0);
            uiPrefs->AlwaysOnTop->value(tmp);

            win.get("secondary_on_top", tmp, 1);
            uiPrefs->SecondaryOnTop->value(tmp);

            win.get("open_mode", tmp, 0);

            {
                Fl_Round_Button* r;
                for (int i = 0; i < uiPrefs->OpenMode->children(); ++i)
                {

                    r = (Fl_Round_Button*)uiPrefs->OpenMode->child(i);
                    r->value(0);
                }

                r = (Fl_Round_Button*)uiPrefs->OpenMode->child(tmp);
                r->value(1);
            }
        }

        //
        // ui/view
        //

        Fl_Preferences view(gui, "view");

        view.get("gain", tmpF, 1.0f);
        uiPrefs->ViewGain->value(tmpF);

        view.get("gamma", tmpF, 1.0f);
        uiPrefs->ViewGamma->value(tmpF);

        view.get("auto_frame", tmp, 1);
        uiPrefs->AutoFrame->value((bool)tmp);

        view.get("safe_areas", tmp, 0);
        uiPrefs->SafeAreas->value((bool)tmp);

        view.get("ocio_in_top_bar", tmp, 0);
        uiPrefs->OCIOInTopBar->value((bool)tmp);

        view.get("debanding", tmp, 0);

#ifdef VULKAN_BACKEND
        if (tmp != 0)
        {
            LOG_WARNING(_("Debanding is not set to None.  This can make images show blurred"));
        }
#endif

        uiPrefs->Debanding->value(tmp);

        view.get("video_levels", tmp, 0);
        uiPrefs->VideoLevels->value(tmp);

        view.get("alpha_blend", tmp, 1);
        uiPrefs->AlphaBlend->value(tmp);

        view.get("minify_filter", tmp, 1);
        uiPrefs->MinifyFilter->value(tmp);

        view.get("magnify_filter", tmp, 1);
        uiPrefs->MagnifyFilter->value(tmp);

        view.get("crop_area", tmp, 0);
        uiPrefs->CropArea->value(tmp);

        view.get("zoom_speed", tmp, 2);
        uiPrefs->ZoomSpeed->value(tmp);

        //
        // HDR
        //
        Fl_Preferences hdr(gui, "hdr");

        hdr.get("vulkan_use_rgb", tmp, 0);
        uiPrefs->VulkanUseRGB->value(tmp);

        hdr.get("chromaticities", tmp, 0);
        uiPrefs->Chromaticities->value(tmp);


        //
        // HDR Peak Detection
        //
        hdr.get("peak_detection", tmp, 0);
        uiPrefs->HDRPeakDetection->value(tmp);

        hdr.get("peak_detection_percentile", tmpF, 100.F);
        uiPrefs->HDRPeakPercentile->value(tmpF);

        hdr.get("peak_detection_smoothing_period", tmpF, 20.F);
        uiPrefs->HDRPeakSmoothingPeriod->value(tmpF);

        hdr.get("peak_detection_low_limit", tmpF, 1.F);
        uiPrefs->HDRPeakLowLimit->value(tmpF);

        hdr.get("peak_detection_high_limit", tmpF, 3.F);
        uiPrefs->HDRPeakHighLimit->value(tmpF);

        hdr.get("hdr_data", tmp, 0);
        uiPrefs->HDRInfo->value(tmp);

        hdr.get("tonemap_algorithm", tmp, 5);  // spline is default as libplacebo and mpv
        uiPrefs->TonemapAlgorithm->value(tmp);

        hdr.get("gamut_mapping", tmp, 0);  // Auto is default
        uiPrefs->GamutMapping->value(tmp);

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
                /* xgettext:c++-format */
                const std::string msg =
                    tl::string::Format(
                        _("Could not open color theme from \"{0}\"."))
                        .arg(colorname);
                LOG_ERROR(msg);
            }
        }

        if (loaded)
        {
            /* xgettext:c++-format */
            const std::string msg =
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
            uiPrefs->ViewBG->color(tmp);

            colors.get("text_overlay_color", tmp, 0xFFFF0000);
            uiPrefs->ViewTextOverlay->color(tmp);

            colors.get("selection_color", tmp, 0xFFFFFF00);
            uiPrefs->ViewSelection->color(tmp);

            colors.get("hud_color", tmp, 0xF0F08000);
            uiPrefs->ViewHud->color(tmp);
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

        var = uiPrefs->OCIOConfig->value();
        if (!var || strlen(var) == 0 || resetSettings)
        {
            setConfig(ocio::ocioDefault);
        }

        ocio.get("use_ocio_auto_ics", tmp, 0);
        uiPrefs->uiOCIOUseAutoICS->value(tmp);

        ocio.get("use_default_display_view", tmp, 0);
        uiPrefs->uiOCIOUseDefaultDisplayView->value(tmp);

        ocio.get("use_active_views", tmp, 1);
        uiPrefs->uiOCIOUseActiveViews->value(tmp);

        ocio.get("not_on_videos", tmp, 1);
        uiPrefs->uiOCIONotOnVideos->value(tmp);

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
        uiPrefs->HudDirectory->value((bool)tmp);
        hud.get("filename", tmp, 0);
        uiPrefs->HudFilename->value((bool)tmp);
        hud.get("fps", tmp, 0);
        uiPrefs->HudFPS->value((bool)tmp);
        hud.get("frame", tmp, 0);
        uiPrefs->HudFrame->value((bool)tmp);
        hud.get("timecode", tmp, 0);
        uiPrefs->HudTimecode->value((bool)tmp);
        hud.get("resolution", tmp, 0);

        uiPrefs->HudResolution->value((bool)tmp);
        hud.get("frame_range", tmp, 0);
        uiPrefs->HudFrameRange->value((bool)tmp);
        hud.get("frame_count", tmp, 0);
        uiPrefs->HudFrameCount->value((bool)tmp);
        hud.get("cache", tmp, 0);
        uiPrefs->HudCache->value((bool)tmp);
        hud.get("memory", tmp, 0);
        uiPrefs->HudMemory->value((bool)tmp);
        hud.get("attributes", tmp, 0);
        uiPrefs->HudAttributes->value((bool)tmp);

        hud.get("font_size", tmp, 12);
        uiPrefs->HudFontSize->value(tmp);

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

        win.get("screen", tmp, 0);
        uiPrefs->uiWindowScreen->value(tmp);

        Fl_Preferences flu(gui, "file_requester");
        //

        flu.get("quick_folder_travel", tmp, 1);
        uiPrefs->FileReqFolder->value((bool)tmp);
        Flu_File_Chooser::singleButtonTravelDrawer = (bool)tmp;

        flu.get("thumbnails", tmp, 1);
        uiPrefs->FileReqThumbnails->value((bool)tmp);
        Flu_File_Chooser::thumbnailsFileReq = (bool)tmp;

        flu.get("usd_thumbnails", tmp, 1);
        uiPrefs->USDThumbnails->value((bool)tmp);
        Flu_File_Chooser::thumbnailsUSD = (bool)tmp;

        //
        // playback
        //
        Fl_Preferences playback(base, "playback");

        playback.get("auto_playback", tmp, 1);
        uiPrefs->AutoPlayback->value(tmp);

        playback.get("single_click_playback", tmp, 0);
        uiPrefs->SingleClickPlayback->value(tmp);

        playback.get("auto_hide_pixel_bar", tmp, kAutoHideOpenGLOnly);
        uiPrefs->AutoHidePixelBar->value(tmp);

        playback.get("fps", tmpF, 24.0);
        uiPrefs->FPS->value(tmpF);

        playback.get("loop", tmp, 0);
        uiPrefs->LoopMode->value(tmp);

        playback.get("scrubbing_sensitivity", tmpF, 5.0f);
        uiPrefs->ScrubbingSensitivity->value(tmpF);

        playback.get("scrub_auto_playback", tmp, 1);
        uiPrefs->ScrubAutoPlay->value(tmp);

        playback.get("scrubbing_loop_mode", tmp, 0);
        uiPrefs->ScrubbingLoopMode->value(tmp);


        Fl_Preferences pixel_toolbar(base, "pixel_toolbar");

        pixel_toolbar.get("RGBA_pixel", tmp, 0);
        uiPrefs->PixelRGBA->value(tmp);

        pixel_toolbar.get("pixel_values", tmp, 0);
        if (version < kPreferencesVersion)
            tmp += 2;
        uiPrefs->PixelValues->value(tmp);

        pixel_toolbar.get("HSV_pixel", tmp, 0);
        uiPrefs->PixelHSV->value(tmp);

        pixel_toolbar.get("Lumma_pixel", tmp, 0);
        uiPrefs->PixelLumma->value(tmp);

        Fl_Preferences loading(base, "loading");

#if defined(__APPLE__) || defined(_WIN32)
        loading.get("native_file_chooser", tmp, 1);
#else
        loading.get("native_file_chooser", tmp, 0);
#endif
        uiPrefs->NativeFileChooser->value((bool)tmp);

        loading.get("missing_frame_type", tmp, 0);
        uiPrefs->uiMissingFrameType->value(tmp);
        missingFrames = static_cast<tl::io::MissingFrames>(tmp);

        loading.get("scratch_frame", tmp, 1);
        uiPrefs->ScratchFrame->value(tmp);

        loading.get("scratch_color", tmp, fl_rgb_color(255, 0, 0));
        uiPrefs->ScratchColor->color(tmp);

        loading.get("scratch_width", tmp, 4);
        uiPrefs->ScratchWidth->value(tmp);

        loading.get("version_regex", tmpS, "_v", 4096);
        if (strlen(tmpS) == 0)
        {
            strcpy(tmpS, "_v");
        }
        uiPrefs->VersionRegex->value(tmpS);

        loading.get("max_images_apart", tmp, 10);
        uiPrefs->MaxImagesApart->value(tmp);

        char key[2048];

        std::string mappingpath = studiopath();
        if (!file::isReadable(mappingpath + "/mrv2.paths.pref"))
            mappingpath = prefspath();

        Fl_Preferences path_mapping(
            mappingpath.c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)0);
        num = path_mapping.entries();

        std::map<std::string, std::string> mapped;
        uiPrefs->PathMappings->clear();
        for (int i = 0; i < num; ++i)
        {
            snprintf(key, 2048, "Path #%d", i + 1);
            path_mapping.get(key, tmpS, "", 4096);
            if (strlen(tmpS) == 0)
                continue;
            const std::string line = tmpS;
            auto splitArray = string::split(line, '\t');
            if (mapped.find(splitArray[0]) != mapped.end())
                continue;
            mapped[splitArray[0]] = splitArray[1];
            uiPrefs->PathMappings->add(tmpS);
        }
        /* xgettext:c++-format */
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

        uiPrefs->RaiseLogWindowOnError->value(tmp);
        LogDisplay::prefs = (LogDisplay::ShowPreferences)tmp;

        errors.get("ffmpeg_log_display", tmp, 0);
        uiPrefs->RaiseLogWindowOnFFmpegError->value(tmp);
        LogDisplay::ffmpegPrefs = (LogDisplay::ShowPreferences)tmp;

        Fl_Preferences video(base, "opengl");

        // VSync: 0 None, 1 = Always, 2 = Presentation
        video.get("vsync", tmp, 2);
// #ifdef __linux__
//         tmp = 1;  // \@bug: Linux must have Always VSync for now (NVidia bug?)
// #endif
        uiPrefs->OpenGLVsync->value(tmp);

        video.get("color_buffers_accuracy", tmp, 0);
        uiPrefs->ColorAccuracy->value(tmp);

        video.get("blit_viewports", tmp, 0);
        uiPrefs->BlitMainViewport->value(tmp);
        uiPrefs->BlitSecondaryViewport->value(tmp);

        video.get("blit_main_viewport", tmp, 0);
        uiPrefs->BlitMainViewport->value(tmp);

        video.get("blit_secondary_viewport", tmp, 0);
        uiPrefs->BlitSecondaryViewport->value(tmp);

        video.get("blit_timeline", tmp, 0);
        uiPrefs->BlitTimeline->value(tmp);

        //
        // Vulkan
        //
        Fl_Preferences vulkan(base, "vulkan");
        vulkan.get("gpu_main_viewport", tmp, 0);
        uiPrefs->MainViewportGPU->value(tmp);

        vulkan.get("gpu_secondary_viewport", tmp, 0);
        uiPrefs->SecondaryViewportGPU->value(tmp);

        vulkan.get("gpu_timeline", tmp, 0);
        uiPrefs->TimelineGPU->value(tmp);

        //
        // Audio
        //
        Fl_Preferences audio(base, "audio");

        audio.get("API", tmp, 0);
        uiPrefs->AudioAPI->value(tmp);

        audio.get("output_device", tmp, 0);
        uiPrefs->AudioOutputDevice->value(tmp);

        //
        // Voice Overs
        //
        Fl_Preferences voice(base, "voice");

        voice.get("path", tmpS, tmppath().c_str(), 4096);
        uiPrefs->VoiceOverPath->value(tmpS);

        voice.get("speed", tmp, 0);
        uiPrefs->VoiceOverSpeed->value(tmp);

        voice.get("microphone", tmp, 0);
        uiPrefs->VoiceOverMicrophone->value(tmp);

        voice.get("volume", tmpF, 100.F);
        uiPrefs->VoiceOverSpeed->value(tmpF);

        //
        // ComfyUI
        //
        Fl_Preferences ComfyUI(base, "comfyUI");

        ComfyUI.get("input_pipe", tmp, 0);
        uiPrefs->UseComfyUIPipe->value((bool)tmp);

        //
        // WebRTC
        //
        Fl_Preferences WebRTC(base, "WebRTC") ;

        WebRTC.get("stun_server", tmpS, "stun:stun.l.google.com:19302", 4096);
        uiPrefs->WebRTCStunServer->value(tmpS);

        WebRTC.get("turn_server", tmpS, "", 4096);
        uiPrefs->WebRTCTurnServer->value(tmpS);

        std::string webrtc_signaling = "wss://sync.filmaura.cloud/sync";
        WebRTC.get("webrtc_signaling", tmpS, webrtc_signaling.c_str(), 4096);
        if (strlen(tmpS) != 0)
            webrtc_signaling = tmpS;
        uiPrefs->WebRTCSignalingServer->value(webrtc_signaling.c_str());



        WebRTC.get("webrtc_studio", tmpS, "", 4096);
        uiPrefs->WebRTCStudio->value(tmpS);


        WebRTC.get("clean_directory", tmp, 1);
        uiPrefs->WebRTCCleanDirectory->value(tmp);

        WebRTC.get("choice", tmp, 0);
        uiPrefs->WebRTCCacheSetting->value(tmp);

        std::string defaultValue = mrv::homepath() +
                                   "/.config/mrv2/cache/remote";
        WebRTC.get("cache_directory", tmpS, defaultValue.c_str(), 4096);
        uiPrefs->WebRTCCacheDirectory->value(tmpS);

        if (uiPrefs->WebRTCCleanDirectory->value())
        {
            std::string dir = uiPrefs->WebRTCCacheDirectory->value();
            // Make sure path is > 5 letters long for safety.
            if (dir.size() > 5)
            {
                fs::path path = dir;
                std::error_code ec;

                fs::remove_all(path, ec);
                if (ec)
                {
                    std::string msg =
                        tl::string::Format(_("Error deleting directory: "
                                             "\"{0}\".")).
                                           arg(ec.message());
                    LOG_ERROR(msg);
                }
                else
                {
                    std::string msg =
                        tl::string::Format(_("Cleaned directory: "
                                             "\"{0}\".")).
                                           arg(dir);
                    LOG_STATUS(msg);
                }
            }
        }

        //
        // Behavior
        //
        Fl_Preferences behavior(base, "behavior");

        behavior.get("check_for_updates", tmp, 0);
        uiPrefs->CheckForUpdates->value(tmp);

        behavior.get("allow_screen_saver", tmp, 0);
        uiPrefs->AllowScreenSaver->value(tmp);


        //
        // Hotkeys
        //
        reset_hotkeys();
        if (!resetHotkeys)
        {
            std::string hotkeyPath = prefspath();
            std::string hotkeyFile = hotkeyPath + hotkeys_file + ".prefs";
            if (!file::isReadable(hotkeyFile))
            {
                hotkeyPath = studiopath();
            }
            hotkeyFile = hotkeyPath + hotkeys_file + ".prefs";
            if (file::isReadable(hotkeyFile))
            {
                /* xgettext:c++-format */
                msg = tl::string::Format(_("Loading hotkeys from \"{0}{1}.prefs\"."))
                      .arg(hotkeyPath)
                      .arg(hotkeys_file);

                load_hotkeys(hotkeyPath);
                LOG_STATUS(msg);
            }
        }
        else
        {
            msg = tl::string::Format(_("Resetting hotkeys to default."));
            LOG_STATUS(msg);
        }

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

        // Handle Shader Options
        timeline::ShaderOptions shaderOptions;
        shaderOptions.debanding =
            static_cast<timeline::Debanding>(uiPrefs->Debanding->value());
        ui->uiView->setShaderOptions(shaderOptions);

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
                "auto_fit_image", (int)uiPrefs->AutoFitImage->value());
            win.set("always_on_top", (int)uiPrefs->AlwaysOnTop->value());
            win.set(
                "secondary_on_top",
                (int)uiPrefs->SecondaryOnTop->value());
            int tmp = 0;
            for (i = 0; i < uiPrefs->OpenMode->children(); ++i)
            {
                Fl_Round_Button* r =
                    (Fl_Round_Button*)uiPrefs->OpenMode->child(i);
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

        gui.set("tooltips", (int)uiPrefs->Tooltips->value());
        gui.set("menubar", (int)uiPrefs->MenuBar->value());
        gui.set("topbar", (int)uiPrefs->Topbar->value());
        gui.set(
            "single_instance", (int)uiPrefs->SingleInstance->value());
        gui.set("pixel_toolbar", (int)uiPrefs->PixelToolbar->value());
        gui.set("timeline_toolbar", (int)uiPrefs->Timeline->value());
        gui.set("status_toolbar", (int)uiPrefs->StatusBar->value());
        gui.set("action_toolbar", (int)uiPrefs->ToolBar->value());
        gui.set("one_panel_only", (int)uiPrefs->OnePanelOnly->value());
        gui.set("macOS_menus", (int)uiPrefs->MacOSMenus->value());
        gui.set("raise_on_enter", (int)uiPrefs->RaiseOnEnter->value());

        gui.set("timeline_display", uiPrefs->TimelineDisplay->value());
        gui.set("timeline_video_offset", uiPrefs->uiStartTimeOffset->value());
        gui.set(
            "timeline_thumbnails", uiPrefs->TimelineThumbnails->value());

        // Thumbnails sizes on all panels
        gui.set("files_panel_thumbnails_size",
                uiPrefs->FilesPanelThumbnails->value());
        gui.set("compare_panel_thumbnails_size",
                uiPrefs->ComparePanelThumbnails->value());
        gui.set("stereo3D_panel_thumbnails_size",
                uiPrefs->Stereo3DPanelThumbnails->value());

        gui.set("panel_thumbnails_manually",
                uiPrefs->ManualPanelThumbnails->value());
        gui.set("remove_edls", uiPrefs->RemoveEDLs->value());
        gui.set("timeline_edit_mode", uiPrefs->EditMode->value());
        gui.set("timeline_edit_view", uiPrefs->EditView->value());
        gui.set(
            "timeline_edit_thumbnails",
            uiPrefs->EditThumbnails->value());
        gui.set(
            "timeline_edit_transitions",
            uiPrefs->ShowTransitions->value());
        gui.set("timeline_edit_markers", uiPrefs->ShowMarkers->value());
        gui.set("timeline_editable", uiPrefs->TimelineEditable->value());
        gui.set(
            "timeline_edit_associated_clips",
            uiPrefs->EditAssociatedClips->value());

        //
        // ui/view prefs
        //
        Fl_Preferences view(gui, "view");
        view.set("gain", uiPrefs->ViewGain->value());
        view.set("gamma", uiPrefs->ViewGamma->value());

        view.set("auto_frame", uiPrefs->AutoFrame->value());
        view.set("safe_areas", uiPrefs->SafeAreas->value());
        view.set("ocio_in_top_bar", uiPrefs->OCIOInTopBar->value());
        view.set("video_levels", uiPrefs->VideoLevels->value());
        view.set("debanding", uiPrefs->Debanding->value());

        view.set("alpha_blend", uiPrefs->AlphaBlend->value());
        view.set("minify_filter", uiPrefs->MinifyFilter->value());
        view.set("magnify_filter", uiPrefs->MagnifyFilter->value());
        view.set("crop_area", uiPrefs->CropArea->value());
        view.set("zoom_speed", (int)uiPrefs->ZoomSpeed->value());

        Fl_Preferences hdr(gui, "hdr");
        hdr.set("vulkan_use_rgb", uiPrefs->VulkanUseRGB->value());
        hdr.set("chromaticities", uiPrefs->Chromaticities->value());

        // HDR Peak detection
        hdr.set("peak_detection", uiPrefs->HDRPeakDetection->value());


        hdr.set("peak_detection_percentile",
                uiPrefs->HDRPeakPercentile->value());
        hdr.set("peak_detection_smoothing_period",
                uiPrefs->HDRPeakSmoothingPeriod->value());
        hdr.set("peak_detection_low_limit",
                uiPrefs->HDRPeakLowLimit->value());
        hdr.set("peak_detection_high_limit",
                uiPrefs->HDRPeakHighLimit->value());

        hdr.set("hdr_data", uiPrefs->HDRInfo->value());
        hdr.set("tonemap_algorithm",
                uiPrefs->TonemapAlgorithm->value());
        hdr.set("gamut_mapping", uiPrefs->GamutMapping->value());

        //
        // view/colors prefs
        //
        {
            Fl_Preferences colors(view, "colors");
            int tmp = uiPrefs->ViewBG->color();
            colors.set("background_color", tmp);
            tmp = uiPrefs->ViewTextOverlay->color();
            colors.set("text_overlay_color", tmp);
            tmp = uiPrefs->ViewSelection->color();
            colors.set("selection_color", tmp);
            tmp = uiPrefs->ViewHud->color();
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

            ocio.set("config", uiPrefs->OCIOConfig->value());
            ocio.set("use_ocio_auto_ics",
                     uiPrefs->uiOCIOUseAutoICS->value());
            ocio.set(
                "use_default_display_view",
                uiPrefs->uiOCIOUseDefaultDisplayView->value());
            ocio.set(
                "use_active_views", uiPrefs->uiOCIOUseActiveViews->value());
            ocio.set(
                "not_on_videos", uiPrefs->uiOCIONotOnVideos->value());

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
        hud.set("directory", uiPrefs->HudDirectory->value());
        hud.set("filename", uiPrefs->HudFilename->value());
        hud.set("fps", uiPrefs->HudFPS->value());
        hud.set("non_drop_timecode", uiPrefs->HudTimecode->value());
        hud.set("frame", uiPrefs->HudFrame->value());
        hud.set("resolution", uiPrefs->HudResolution->value());
        hud.set("frame_range", uiPrefs->HudFrameRange->value());
        hud.set("frame_count", uiPrefs->HudFrameCount->value());
        hud.set("cache", uiPrefs->HudCache->value());
        hud.set("memory", uiPrefs->HudMemory->value());
        hud.set("attributes", uiPrefs->HudAttributes->value());
        hud.set("font_size", uiPrefs->HudFontSize->value());

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
                win.set("screen", uiPrefs->uiWindowScreen->value());
            }
            else
            {
                win.set("fixed_position", 1);
                win.set("fixed_size", 1);
                win.set("x_position", ui->uiMain->x());
                win.set("y_position", ui->uiMain->y());
                win.set("x_size", ui->uiMain->w());
                win.set("y_size", ui->uiMain->h());
                win.set("screen", ui->uiMain->screen_num());
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
        flu.set("quick_folder_travel", uiPrefs->FileReqFolder->value());
        flu.set("thumbnails", uiPrefs->FileReqThumbnails->value());
        flu.set("usd_thumbnails", uiPrefs->USDThumbnails->value());

        //
        Flu_File_Chooser::singleButtonTravelDrawer =
            uiPrefs->FileReqFolder->value();
        Flu_File_Chooser::thumbnailsFileReq =
            uiPrefs->FileReqThumbnails->value();
        Flu_File_Chooser::thumbnailsUSD =
            uiPrefs->USDThumbnails->value();

        //
        // playback prefs
        //
        Fl_Preferences playback(base, "playback");
        playback.set(
            "auto_playback", (int)uiPrefs->AutoPlayback->value());
        playback.set(
            "single_click_playback",
            (int)uiPrefs->SingleClickPlayback->value());
        playback.set(
            "auto_hide_pixel_bar",
            (int)uiPrefs->AutoHidePixelBar->value());
        playback.set("fps", uiPrefs->FPS->value());
        playback.delete_entry("loop_mode"); // legacy preference
        playback.set("loop", uiPrefs->LoopMode->value());
        playback.set(
            "scrubbing_sensitivity",
            uiPrefs->ScrubbingSensitivity->value());
        playback.set(
            "scrub_auto_playback", uiPrefs->ScrubAutoPlay->value());

        playback.set("scrubbing_loop_mode",
                     uiPrefs->ScrubbingLoopMode->value());

        Fl_Preferences pixel_toolbar(base, "pixel_toolbar");
        pixel_toolbar.set("RGBA_pixel", uiPrefs->PixelRGBA->value());
        pixel_toolbar.set("pixel_values", uiPrefs->PixelValues->value());
        pixel_toolbar.set("HSV_pixel", uiPrefs->PixelHSV->value());
        pixel_toolbar.set("Lumma_pixel", uiPrefs->PixelLumma->value());

        Fl_Preferences loading(base, "loading");

        loading.set(
            "native_file_chooser",
            (int)uiPrefs->NativeFileChooser->value());

        loading.set("missing_frame_type", uiPrefs->uiMissingFrameType->value());

        loading.set("scratch_frame", uiPrefs->ScratchFrame->value());
        loading.set("scratch_color",
                    (int)uiPrefs->ScratchColor->color());
        loading.set("scratch_width", uiPrefs->ScratchWidth->value());

        loading.set("version_regex", uiPrefs->VersionRegex->value());
        loading.set(
            "max_images_apart", (int)uiPrefs->MaxImagesApart->value());

        char key[256];


        userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.paths.prefs"))
            userprefspath = prefspath();

        Fl_Preferences path_mapping(
            userprefspath.c_str(), "filmaura", "mrv2.paths",
            (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
        path_mapping.clear();
        for (int i = 1; i <= uiPrefs->PathMappings->size(); ++i)
        {
            snprintf(key, 256, "Path #%d", i);
            path_mapping.set(key, uiPrefs->PathMappings->text(i));
        }

        /* xgettext:c++-format */
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
            "log_display", (int)uiPrefs->RaiseLogWindowOnError->value());
        errors.set(
            "ffmpeg_log_display",
            (int)uiPrefs->RaiseLogWindowOnFFmpegError->value());

        Fl_Preferences video(base, "opengl");
        video.set("vsync", (int)uiPrefs->OpenGLVsync->value());
        video.set(
            "color_buffers_accuracy",
            (int)uiPrefs->ColorAccuracy->value());
        video.set(
            "blit_main_viewport", (int)uiPrefs->BlitMainViewport->value());
        video.set(
            "blit_secondary_viewport", (int)uiPrefs->BlitSecondaryViewport->value());
        video.set("blit_timeline", (int)uiPrefs->BlitTimeline->value());

        Fl_Preferences vulkan(base, "vulkan");
        vulkan.set(
            "gpu_main_viewport",
            (int)uiPrefs->MainViewportGPU->value());
        vulkan.set(
            "gpu_secondary_viewport",
            (int)uiPrefs->SecondaryViewportGPU->value());
        vulkan.set("gpu_timeline",
                   (int)uiPrefs->TimelineGPU->value());

        //
        // Voice Overs
        //
        Fl_Preferences voice(base, "voice");

        voice.set("path", uiPrefs->VoiceOverPath->value());
        voice.set("speed", uiPrefs->VoiceOverSpeed->value());
        voice.set("microphone", uiPrefs->VoiceOverMicrophone->value());
        voice.set("volume", uiPrefs->VoiceOverSpeed->value());


        Fl_Preferences ComfyUI(base, "comfyUI");
        ComfyUI.set("input_pipe", (int)uiPrefs->UseComfyUIPipe->value());

        Fl_Preferences WebRTC(base, "WebRTC");

        WebRTC.set("stun_server", uiPrefs->WebRTCStunServer->value());
        WebRTC.set("turn_server", uiPrefs->WebRTCTurnServer->value());

        WebRTC.set("webrtc_signaling", uiPrefs->WebRTCSignalingServer->value());

        WebRTC.set("webrtc_studio", uiPrefs->WebRTCStudio->value());

        WebRTC.set("choice",
                   (int)uiPrefs->WebRTCCacheSetting->value());
        WebRTC.set("clean_directory",
                   (int)uiPrefs->WebRTCCleanDirectory->value());
        WebRTC.set("cache_directory",
                   uiPrefs->WebRTCCacheDirectory->value());

        Fl_Preferences audio(base, "audio");

        audio.set("API", (int)uiPrefs->AudioAPI->value());
        audio.set(
            "output_device", (int)uiPrefs->AudioOutputDevice->value());

        Fl_Preferences behavior(base, "behavior");
        behavior.set(
            "check_for_updates", (int)uiPrefs->CheckForUpdates->value());

        behavior.set("allow_screen_saver",
                     (int)uiPrefs->AllowScreenSaver->value());

        {
            userprefspath = prefspath();

            Fl_Preferences keys(
                userprefspath.c_str(), "filmaura", hotkeys_file.c_str(),
                (Fl_Preferences::Root)((int)Fl_Preferences::CLEAR));
            save_hotkeys(keys);

            /* xgettext:c++-format */
            msg = tl::string::Format(
                      _("Hotkeys have been saved to \"{0}{1}.prefs\"."))
                      .arg(userprefspath)
                      .arg(hotkeys_file);
            LOG_STATUS(msg);
        }

        base.flush();

        /* xgettext:c++-format */
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
        if (uiPrefs->MacOSMenus->value())
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
            if (uiPrefs->MenuBar->value())
            {
                ui->uiMenuGroup->show();
            }
            else
            {
                ui->uiMenuGroup->hide();
            }

            if (uiPrefs->Topbar->value())
            {
                ui->uiTopBar->show();
            }
            else
            {
                ui->uiTopBar->hide();
            }

            const bool showPixelBar = uiPrefs->PixelToolbar->value();
            if (showPixelBar)
            {
                const auto player = ui->uiView->getTimelinePlayer();
                const int autoHide = uiPrefs->AutoHidePixelBar->value();
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
            options.transitions = uiPrefs->ShowTransitions->value();
            options.markers = uiPrefs->ShowMarkers->value();
            ui->uiTimeline->setEditable(
                uiPrefs->TimelineEditable->value());
            int thumbnails = uiPrefs->EditThumbnails->value();
            options.thumbnails = true;
            switch (thumbnails)
            {
            case 0:
                options.thumbnails = false;
                break;
            case 1: // Small
                options.thumbnailHeight = 50 * ui->uiView->pixels_per_unit();
                break;
            case 2: // Medium
                options.thumbnailHeight = 75 * ui->uiView->pixels_per_unit();
                break;
            case 3: // Large
                options.thumbnailHeight = 100 * ui->uiView->pixels_per_unit();
                break;
            }
            options.waveformHeight = options.thumbnailHeight / 2;
            options.trackInfo = settings->getValue<int>("Timeline/TrackInfo");
            options.clipInfo = settings->getValue<int>("Timeline/ClipInfo");
            ui->uiTimeline->setDisplayOptions(options);

            if (uiPrefs->Timeline->value())
            {
                ui->uiBottomBar->show();
            }
            else
            {
                ui->uiBottomBar->hide();
                set_edit_mode_cb(EditMode::kNone, ui);
            }

            if (uiPrefs->StatusBar->value())
            {
                ui->uiStatusGroup->show();
            }
            else
            {
                ui->uiStatusGroup->hide();
            }

            if (uiPrefs->ToolBar->value())
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

        panel::onlyOne((bool)uiPrefs->OnePanelOnly->value());

        //
        // Widget/Viewer settings
        //

        {
            ui->uiView->setGhostNext(settings->getValue<int>(kGhostNext));
            ui->uiView->setGhostPrevious(
                settings->getValue<int>(kGhostPrevious));

            tl::io::MissingFrames value = static_cast<tl::io::MissingFrames>(uiPrefs->uiMissingFrameType->value());
            ui->uiView->setMissingFrameType(value);
            if (value != missingFrames)
            {
                refresh_media_cb(nullptr, ui);
            }
            missingFrames = value;
        }

        TimelineClass* t = ui->uiTimeWindow;
        t->uiLoopMode->value(uiPrefs->LoopMode->value());
        t->uiLoopMode->do_callback();

        t->uiTimecodeSwitch->value(uiPrefs->TimelineDisplay->value());
        t->uiTimecodeSwitch->do_callback();

        ui->uiGain->value(uiPrefs->ViewGain->value());
        ui->uiGamma->value(uiPrefs->ViewGamma->value());

        // OCIO
        ocio::setup();

        //
        // Handle file requester
        //

        Flu_File_Chooser::thumbnailsFileReq =
            (bool)uiPrefs->FileReqThumbnails->value();

        Flu_File_Chooser::singleButtonTravelDrawer =
            (bool)uiPrefs->FileReqFolder->value();

        native_file_chooser = uiPrefs->NativeFileChooser->value();

        //
        // Handle pixel values
        //
        PixelToolBarClass* c = ui->uiPixelWindow;
        c->uiAColorType->value(uiPrefs->PixelRGBA->value());
        c->uiAColorType->do_callback();
        c->uiAColorType->redraw();

        c->uiPixelValue->value(uiPrefs->PixelValues->value());
        c->uiPixelValue->do_callback();
        c->uiPixelValue->redraw();

        c->uiBColorType->value(uiPrefs->PixelHSV->value());
        c->uiBColorType->do_callback();
        c->uiBColorType->redraw();

        c->uiLType->value(uiPrefs->PixelLumma->value());
        c->uiLType->do_callback();
        c->uiLType->redraw();

        if (uiPrefs->Tooltips->value())
        {
            Fl::option(Fl::OPTION_SHOW_TOOLTIPS, true);
        }
        else
        {
            Fl::option(Fl::OPTION_SHOW_TOOLTIPS, false);
        }

        //
        // Handle crop area (masking)
        //

        int crop = uiPrefs->CropArea->value();
        float mask = kCrops[crop];
        view->setMask(mask);

        // Handle Safe areas
        bool safeAreas = (bool)uiPrefs->SafeAreas->value();
        view->setSafeAreas(safeAreas);

        bool ocioInTopBar = uiPrefs->OCIOInTopBar->value();
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

        if (!uiPrefs->uiOCIOUseAutoICS->value())
        {
            ui->uiAutoICS->value(0);
        }
        else
        {
            ui->uiAutoICS->value(1);
        }
        ui->uiAutoICS->do_callback();

        // Handle image options
        auto imageOptions = app->imageOptions();
        int alphaBlend = uiPrefs->AlphaBlend->value();
        int videoLevels = uiPrefs->VideoLevels->value();
        int minifyFilter = uiPrefs->MinifyFilter->value();
        int magnifyFilter = uiPrefs->MagnifyFilter->value();
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
            !uiPrefs->Chromaticities->value();
        displayOptions.hdrInfo =
            static_cast<timeline::HDRInformation>(
                uiPrefs->HDRInfo->value());
        app->setDisplayOptions(displayOptions);

        timeline::HDROptions hdrOptions = ui->uiView->getHDROptions();
        hdrOptions.algorithm =
            static_cast<timeline::HDRTonemapAlgorithm>(uiPrefs->TonemapAlgorithm->value());
        hdrOptions.gamutMapping =
            static_cast<timeline::HDRGamutMapping>(uiPrefs->GamutMapping->value());
        hdrOptions.peak_detection = uiPrefs->HDRPeakDetection->value();
        hdrOptions.peak_percentile = uiPrefs->HDRPeakPercentile->value();
        hdrOptions.peak_smoothing_period = uiPrefs->HDRPeakSmoothingPeriod->value();
        hdrOptions.peak_scene_high_limit = uiPrefs->HDRPeakHighLimit->value();
        hdrOptions.peak_scene_low_limit = uiPrefs->HDRPeakLowLimit->value();
        ui->uiView->setHDROptions(hdrOptions);

        //
        // Handle HUD
        //
        int hud = HudDisplay::kNone;
        if (uiPrefs->HudDirectory->value())
            hud |= HudDisplay::kDirectory;

        if (uiPrefs->HudFilename->value())
            hud |= HudDisplay::kFilename;

        if (uiPrefs->HudFPS->value())
            hud |= HudDisplay::kFPS;

        if (uiPrefs->HudTimecode->value())
            hud |= HudDisplay::kTimecode;

        if (uiPrefs->HudFrame->value())
            hud |= HudDisplay::kFrame;

        if (uiPrefs->HudResolution->value())
            hud |= HudDisplay::kResolution;

        if (uiPrefs->HudFrameRange->value())
            hud |= HudDisplay::kFrameRange;

        if (uiPrefs->HudFrameCount->value())
            hud |= HudDisplay::kFrameCount;

        if (uiPrefs->HudAttributes->value())
            hud |= HudDisplay::kAttributes;

        if (uiPrefs->HudCache->value())
            hud |= HudDisplay::kCache;

        if (uiPrefs->HudMemory->value())
            hud |= HudDisplay::kMemory;

        view->setHudDisplay((HudDisplay)hud);

        bool frameView = (bool)uiPrefs->AutoFitImage->value();
        view->setFrameView(frameView);

        LogDisplay::prefs = (LogDisplay::ShowPreferences)
                                uiPrefs->RaiseLogWindowOnError->value();
        LogDisplay::ffmpegPrefs =
            (LogDisplay::ShowPreferences)
                uiPrefs->RaiseLogWindowOnFFmpegError->value();

        bool hasPresentation = view->getPresentationMode();

        Fl_Round_Button* r;

        r = (Fl_Round_Button*)uiPrefs->OpenMode->child(0);
        int normal = r->value();

        r = (Fl_Round_Button*)uiPrefs->OpenMode->child(1);
        int fullscreen = r->value();
        if (fullscreen)
        {
            ui->uiMain->show();
            view->setFullScreenMode(true);
        }

        r = (Fl_Round_Button*)uiPrefs->OpenMode->child(2);
        int presentation = r->value();
        if (presentation)
        {
            ui->uiMain->show();
            view->setPresentationMode(true);
        }

        if (normal)
        {
            if (view->getPresentationMode())
            {
                view->setPresentationMode(false);
            }
            else
            {
                view->setFullScreenMode(false);
            }
        }

        r = (Fl_Round_Button*)uiPrefs->OpenMode->child(3);
        int maximized = r->value();
        if (maximized)
        {
            ui->uiMain->show();
            view->setMaximized();
        }

        bool value = uiPrefs->AlwaysOnTop->value();
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
                bool value = uiPrefs->SecondaryOnTop->value();
                window->always_on_top(value);
            }
        }

        int vsync = ui->uiPrefs->OpenGLVsync->value();
        if (vsync == MonitorVSync::kVSyncPresentationOnly ||
            vsync == MonitorVSync::kVSyncNone)
        {
            view->swap_interval(0);
            ui->uiTimeline->swap_interval(0);
            if (secondary)
            {
                auto window = secondary->viewport();
                if (window->visible())
                {
                    window->swap_interval(0);
                }
            }
        }
        else
        {
            view->swap_interval(1);
            ui->uiTimeline->swap_interval(1);
            if (secondary)
            {
                auto window = secondary->viewport();
                if (window->visible())
                {
                    window->swap_interval(1);
                }
            }
        }

#ifdef TLRENDER_AUDIO
        auto context = App::app->getContext();
        auto audioSystem = context->getSystem<audio::System>();
        if (audioSystem)
        {
            int api = uiPrefs->AudioAPI->value();
            const Fl_Menu_Item* item = uiPrefs->AudioAPI->child(api);
            if (item && item->label())
            {
                audioSystem->setAPI(item->label());
            }

            size_t outputDevice = uiPrefs->AudioOutputDevice->value();
            item = uiPrefs->AudioOutputDevice->child(outputDevice);
            if (item && item->label())
            {
                audioSystem->setOutputDevice(item->label());
            }
        }
#endif

        view->refreshWindows();

#ifdef MRV2_NETWORK
        if (uiPrefs->SingleInstance->value())
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

        if (uiPrefs->UseComfyUIPipe->value())
        {
            app->createComfyUIListener();
        }
#endif

        ui->uiMain->allow_screen_saver((bool)uiPrefs->AllowScreenSaver->value());

        std::string userprefspath = studiopath();
        if (!file::isReadable(userprefspath + "/mrv2.prefs"))
            userprefspath = prefspath();

        Fl_Preferences base(
            userprefspath.c_str(), "filmaura", "mrv2", (Fl_Preferences::Root)0);
        Fl_Preferences gui(base, "ui");
        gui.set("single_instance", uiPrefs->SingleInstance->value());
        gui.set(
            "single_instance", (int)uiPrefs->SingleInstance->value());
        base.flush();

        panel::refreshThumbnails();

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
                /* xgettext:c++-format */
                const std::string msg =
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
        uiPrefs->OCIOConfig->value(configName.c_str());
        oldConfigName = configName;
    }

} // namespace mrv
