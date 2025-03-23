// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>

#ifdef __linux__
#    undef None // macro defined in X11 config files
#    undef Status
#endif

#include <tlIO/System.h>

#include <tlDevice/NDI/NDI.h>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvRoot.h"

#include "hdr/mrvHDRApp.h"
#include "mrvHDRView.h"

namespace
{
    const char* kModule = "main";
    const double kTimeout = 0.005;
} // namespace

namespace mrv
{
#if defined(FLTK_USE_X11)
    int xerrorhandler(Display* dsp, XErrorEvent* error)
    {
        char errorstring[128];
        XGetErrorText(dsp, error->error_code, errorstring, 128);
        std::cerr << "X error-- " << error->error_code << " : " << errorstring
                  << std::endl;
        exit(-1);
    }
#endif

    namespace
    {
        const float errorTimeout = 5.F;
    }

    struct Options
    {
        bool resetSettings = false;
        bool resetHotkeys = false;
        bool displayVersion = false;
    };

    struct HDRApp::Private
    {
        Options options;

        float volume = 1.F;
        bool mute = false;
    };

    HDRUI* HDRApp::ui = nullptr;
    HDRApp* HDRApp::app = nullptr;

    namespace
    {
        void open_console()
        {
#ifdef _WIN32
            // If TERM is defined, user fired the application from
            // the Msys console that does not show these problems.x
            const char* term = fl_getenv("TERM");
            if (!term || strlen(term) == 0)
            {
                if (AttachConsole(ATTACH_PARENT_PROCESS))
                {
                    // Redirect stdout and stderr to the parent console
                    freopen("CONOUT$", "w", stdout);
                    freopen("CONOUT$", "w", stderr);
                    return;
                }
            }
#endif
        }

    } // namespace

    HDRApp::HDRApp(
        int argc, char** argv,
        const std::shared_ptr<system::Context>& context) :
        BaseApp(),
        _p(new Private)
    {
        TLRENDER_P();

        // Establish MRV2_ROOT environment variable
        set_root_path(argc, argv);

#ifdef __linux__

#    ifdef FLTK_USE_X11
        if (fl_x11_display())
        {
            int ok = XInitThreads();
            if (!ok)
                throw std::runtime_error("XInitThreads failed");

            XSetErrorHandler(xerrorhandler);
        }
#    endif

#endif
        // Store the application object for further use down the line
        HDRApp::app = this;

        // DBG;
        // open_console();

        // const std::string& msg = setLanguageLocale();

        BaseApp::_init(
            app::convert(argc, argv), context, "hdr",
            _("Play NDI input streams."), {},
            {app::CmdLineFlagOption::create(
                 p.options.resetSettings, {"-resetSettings"},
                 _("Reset settings to defaults.")),
             app::CmdLineFlagOption::create(
                 p.options.resetHotkeys, {"-resetHotkeys"},
                 _("Reset hotkeys to defaults.")),

             app::CmdLineFlagOption::create(
                 p.options.displayVersion, {"-version", "-v"},
                 _("Return the version and exit."))});

        const int exitCode = getExit();
        if (exitCode != 0)
        {
            return;
        }

        // Not required, but "correct" (see the SDK documentation).
        if (!NDIlib_initialize())
        {
            // Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
            // you can check this directly with a call to NDIlib_is_supported_CPU()
            printf("Cannot run NDI.");
            _exit = -1;
            return;
        }
    
        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
        Fl::use_high_res_GL(true);

        Fl::set_fonts("-*");
        Fl::lock(); // needed for NDI and multithreaded logging

        // Create the Settings
        // p.settings = new SettingsObject();

        // Create the interface.
        ui = new HDRUI();
        if (!ui)
        {
            throw std::runtime_error(_("Cannot create window"));
        }

#ifdef __APPLE__
        Fl_Mac_App_Menu::about = _("About hdr");
        Fl_Mac_App_Menu::print = "Print Front Window";
        Fl_Mac_App_Menu::hide = _("Hide hdr");
        Fl_Mac_App_Menu::hide_others = _("Hide Others");
        Fl_Mac_App_Menu::services = _("Services");
        Fl_Mac_App_Menu::show = _("Show All");
        Fl_Mac_App_Menu::quit = _("Quit hdr");

        // For macOS, to read command-line arguments
        fl_open_display();
#endif

        // p.volume = p.settings->getValue<float>("Audio/Volume");
        // p.mute = p.settings->getValue<bool>("Audio/Mute");

        ui->uiMain->show();
        ui->uiView->take_focus();
    }

    void HDRApp::_cleanResources()
    {
        TLRENDER_P();
        delete ui;
        ui = nullptr;
        
        // Not required, but nice
        NDIlib_destroy();
    }

    HDRApp::~HDRApp()
    {
        TLRENDER_P();

        _cleanResources();
    }

    // SettingsObject* HDRApp::settings() const
    // {
    //     return _p->settings;
    // }

    float HDRApp::volume() const
    {
        return _p->volume;
    }

    bool HDRApp::isMuted() const
    {
        return _p->mute;
    }

    int HDRApp::run()
    {
        TLRENDER_P();
        if (!ui)
            return 0;

        std::cerr << "Start HDRApp::run loop" << std::endl;
        while(ui->uiMain->shown())
        {
            Fl::check();
        }
        return 0;
//        return Fl::run();
    }

    void HDRApp::setVolume(float value)
    {
        TLRENDER_P();
        if (value == p.volume)
            return;
        p.volume = value;
        _volumeChanged(p.volume);
    }

    void HDRApp::_volumeChanged(float value)
    {
        TLRENDER_P();
        // p.settings->setValue("Audio/Volume", value);
    }

    void HDRApp::setMute(bool value)
    {
        TLRENDER_P();
        if (value == p.mute)
            return;
        p.mute = value;
        _muteChanged(p.mute);
    }

    void HDRApp::_muteChanged(bool value)
    {
        TLRENDER_P();
        // p.settings->setValue("Audio/Mute", value);
    }

} // namespace mrv
