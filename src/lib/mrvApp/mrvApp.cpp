// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#endif


#include <tlIO/System.h>

#include <tlCore/StringFormat.h>
#include <tlCore/AudioSystem.h>

#include <tlTimeline/Util.h>

#ifdef MRV2_PYBIND11
#    include <pybind11/embed.h>
namespace py = pybind11;
#    include "mrvPy/Cmds.h"
#    include "mrvPy/Plugin.h"
#    include "mrvPy/PyStdErrOutRedirect.h"
#endif

#include "mrvCore/mrvOS.h" // do not move up
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvLicensing.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvSignalHandler.h"

#include "mrvFl/mrvContextObject.h"
#include "mrvFl/mrvInit.h"
#include "mrvFl/mrvLanguages.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvWidgets/mrvLogDisplay.h"
#include "mrvWidgets/mrvProgressReport.h"
#include "mrvWidgets/mrvPythonOutput.h"

#if defined(TLRENDER_USD)
#    include "mrvOptions/mrvUSD.h"
#endif // TLRENDER_USD

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvNetwork/mrvDummyClient.h"
#include "mrvNetwork/mrvDisplayOptions.h"
#include "mrvNetwork/mrvLUTOptions.h"

#ifdef MRV2_NETWORK
#    include "mrvNetwork/mrvCommandInterpreter.h"
#    include "mrvNetwork/mrvClient.h"
#    include "mrvNetwork/mrvComfyUIListener.h"
#    include "mrvNetwork/mrvImageListener.h"
#    include "mrvNetwork/mrvServer.h"
#    include "mrvNetwork/mrvParseHost.h"
#endif

#include "mrvEdit/mrvEditUtil.h"
#include "mrvEdit/mrvCreateEDLFromFiles.h"

#ifdef MRV2_PYBIND11
#    include "mrvPy/mrvPythonArgs.h"
#endif

#include "mrvApp/mrvApp.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvMainControl.h"
#include "mrvApp/mrvOpenSeparateAudioDialog.h"
#include "mrvApp/mrvSettingsObject.h"

#include "mrvPreferencesUI.h"
#include "mrViewer.h"

// we include it here to avoid tl::image and mrv::image clashes
#include "mrvFl/mrvOCIO.h"

#ifdef MRV2_NETWORK
#    include <Poco/Net/SSLManager.h>
#endif

#include <FL/platform.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>

#ifdef __linux__
#    undef None // macro defined in X11 config files
#    undef Status
#endif

#include "mrvFl/mrvIO.h"

#if defined(TLRENDER_NDI) || defined(TLRENDER_BMD)
#    include <tlDevice/DevicesModel.h>
#endif

#ifdef TLRENDER_BMD
#    include <tlDevice/BMD/BMDOutputDevice.h>
#endif

#ifdef TLRENDER_NDI
#    include <tlDevice/NDI/NDIOutputDevice.h>
#endif

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

#ifndef __APPLE__
    // Apple has a kickass system handler and backtrace functions as part of
    // the OS.  No need for that there.
    static SignalHandler signalHandler;
#endif

    namespace
    {
        const float kLicenseTimeout = 110;
    }

    struct Options
    {
        std::string dummy;
        bool createOtioTimeline = false;
        std::vector<std::string> fileNames;
        std::string audioFileName;
        std::string compareFileName;
#ifdef MRV2_PYBIND11
        bool        noPython = false;
        std::string pythonScript;
        std::string pythonArgs;
#endif

#ifdef MRV2_NETWORK
        bool server = false;
        std::string client;
        unsigned port = 55150;
#endif

        timeline::CompareOptions compareOptions;

        bool singleImages = false;
        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Count;
        timeline::Loop loop = timeline::Loop::Count;
        otime::RationalTime seek = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;

        timeline::OCIOOptions ocioOptions;
        timeline::LUTOptions lutOptions;

        bool hud = true;
        bool resetSettings = false;
        bool resetHotkeys = false;
        bool displayVersion = false;
        bool otioEditMode = false;

#if defined(TLRENDER_USD)
        bool usdOverrides = false;
        usd::RenderOptions usd;
#endif // TLRENDER_USD
    };

    struct App::Private
    {
        Options options;
        
        ContextObject* contextObject = nullptr;
        std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
        SettingsObject* settings = nullptr;

#ifdef MRV2_NETWORK
        CommandInterpreter* commandInterpreter = nullptr;
        std::unique_ptr<ImageListener> imageListener;
        std::unique_ptr<ComfyUIListener> comfyUIListener;
#endif

#ifdef MRV2_PYBIND11
        std::unique_ptr<PyStdErrOutStreamRedirect> pythonStdErrOutRedirect;
        std::unique_ptr<PythonArgs> pythonArgs;
#endif

        std::shared_ptr<PlaylistsModel> playlistsModel;
        std::shared_ptr<FilesModel> filesModel;
        std::vector<std::shared_ptr<FilesModelItem> > files;
        std::vector<std::shared_ptr<FilesModelItem> > activeFiles;
        std::vector<std::shared_ptr<timeline::Timeline> > timelines;

        bool deviceActive = false;
        std::shared_ptr<device::IOutput> outputDevice;
        std::shared_ptr<device::DevicesModel> devicesModel;
        image::VideoLevels outputVideoLevels = image::VideoLevels::First;

        // Observers
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            activeObserver;
        std::shared_ptr<observer::ListObserver<int> > layersObserver;
        std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
            compareTimeObserver;
        std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
        std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
        std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
            cacheInfoObserver;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

        // Options
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        float volume = 1.F;
        bool mute = false;

        std::shared_ptr<TimelinePlayer> player;

        // Progress report FLTK widget
        ProgressReport* progress = nullptr;

        MainControl* mainControl = nullptr;

        bool session = false;
        bool running = false;
    };

    ViewerUI* App::ui = nullptr;
    App* App::app = nullptr;
    bool App::demo_mode = true;

    LicenseType App::license_type = LicenseType::kDemo;

    bool App::supports_layers = false;
    bool App::supports_annotations = false;
    bool App::supports_editing = false;
    bool App::supports_hdr = true;
    bool App::supports_python = false;
    bool App::supports_voice = false;
    
    bool App::unsaved_annotations = false;
    bool App::unsaved_edits = false;

    std::vector<std::string > OSXfiles;
    void osx_open_cb(const char* filename)
    {
        OSXfiles.push_back(filename);
    }

    static void beat_cb(void* data)
    {
        static int counter = 0;
        License ok = license_beat();
        if (ok != License::kValid)
        {
            ++counter;
            if (counter == 3)
            {
                fl_alert("%s", _("Floating license time for demo exceeded"));
                Fl::check();
                exit_cb(nullptr, App::ui);
                exit(0);
            }
        }
        
        Fl::repeat_timeout(kLicenseTimeout, (Fl_Timeout_Handler)beat_cb, data);
    }

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
                if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                    // Redirect stdout and stderr to the parent console
                    freopen("CONOUT$", "w", stdout);
                    freopen("CONOUT$", "w", stderr);
                    return;
                }
            }
#endif
        }

    } // namespace

    App::App(
        int argc, char** argv,
        const std::shared_ptr<system::Context>& context) :
        BaseApp(),
        _p(new Private)
    {
        TLRENDER_P();
        
        // Establish MRV2_ROOT environment variable
        set_root_path(argc, argv);
        mrv::init(context);

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
        App::app = this;
        ViewerUI::app = this;

        open_console();
        
        const std::string& msg = setLanguageLocale();

        BaseApp::_init(
            app::convert(argc, argv), context, "mrv2",
            _("Play timelines, movies, and image sequences."),
            {app::CmdLineValueArg<std::string>::create(
                p.options.dummy, "inputs",
                _("Timelines, movies, image sequences, USD assets or "
                  "folders."),
                true, true)},
            {
                app::CmdLineHeader::create({}, _("Debugging:")),
                app::CmdLineValueOption<int>::create(
                    Preferences::debug, {"-debug", "-d"},
                    _("Debug verbosity.")),
                app::CmdLineValueOption<int>::create(
                    Preferences::logLevel, {"-logLevel", "-l"},
                    _("Log verbosity."),
                    string::Format("{0}").arg(Preferences::logLevel)),
                app::CmdLineHeader::create({}, _("Audio:")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.audioFileName, {"-audio", "-a"},
                    _("Audio file name.")),
                app::CmdLineHeader::create({}, _("Compare:")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.compareFileName, {"-compare", "-b"},
                    _("A/B comparison \"B\" file name.")),
                app::CmdLineValueOption<timeline::CompareMode>::create(
                    p.options.compareOptions.mode, {"-compareMode", "-c"},
                    _("A/B comparison mode."),
                    string::Format("{0}").arg(p.options.compareOptions.mode),
                    string::join(timeline::getCompareModeLabels(), ", ")),
                app::CmdLineValueOption<math::Vector2f>::create(
                    p.options.compareOptions.wipeCenter, {"-wipeCenter", "-wc"},
                    _("A/B comparison wipe center."),
                    string::Format("{0}").arg(
                        p.options.compareOptions.wipeCenter)),
                app::CmdLineValueOption<float>::create(
                    p.options.compareOptions.wipeRotation,
                    {"-wipeRotation", "-wr"},
                    _("A/B comparison wipe rotation."),
                    string::Format("{0}").arg(
                        p.options.compareOptions.wipeRotation)),
                app::CmdLineHeader::create({}, _("Editing:")),
                app::CmdLineFlagOption::create(
                    p.options.createOtioTimeline, {"-otio", "-o", "-edl"},
                    _("Create OpenTimelineIO EDL from the list of clips "
                      "provided.")),
                app::CmdLineFlagOption::create(
                    p.options.otioEditMode, {"-editMode", "-e"},
                    _("OpenTimelineIO Edit mode.")),
                app::CmdLineFlagOption::create(
                    p.options.singleImages, {"-single", "-s"},
                    _("Load the images as still images not sequences.")),
                app::CmdLineHeader::create({}, _("Playback:")),
                app::CmdLineValueOption<double>::create(
                    p.options.speed, {"-speed"}, _("Playback speed.")),
                app::CmdLineValueOption<timeline::Playback>::create(
                    p.options.playback, {"-playback", "-p"},
                    _("Playback mode."),
                    string::Format("{0}").arg(timeline::Playback::Stop),
                    string::join(timeline::getPlaybackLabels(), ", ")),
                app::CmdLineValueOption<timeline::Loop>::create(
                    p.options.loop, {"-loop"}, _("Playback loop mode."),
                    string::Format("{0}").arg(timeline::Loop::Loop),
                    string::join(timeline::getLoopLabels(), ", ")),
                app::CmdLineValueOption<otime::RationalTime>::create(
                    p.options.seek, {"-seek"},
                    _("Seek to the given time, in value/fps format.  "
                      "Example: 50/30.")),
                app::CmdLineValueOption<otime::TimeRange>::create(
                    p.options.inOutRange, {"-inOutRange", "-inout"},
                    _("Set the in/out points range in start/end/fps "
                      "format, like 23/120/24.")),
                app::CmdLineHeader::create({}, _("OCIO:")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.ocioOptions.input, {"-ocioInput", "-ics", "-oi"},
                    _("OpenColorIO input color space.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.ocioOptions.display, {"-ocioDisplay", "-od"},
                    _("OpenColorIO display name.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.ocioOptions.view, {"-ocioView", "-ov"},
                    _("OpenColorIO view name.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.ocioOptions.look, {"-ocioLook", "-ol"},
                    _("OpenColorIO look name.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.lutOptions.fileName, {"-lut"},
                    _("LUT file name.")),
                app::CmdLineValueOption<timeline::LUTOrder>::create(
                    p.options.lutOptions.order, {"-lutOrder"},
                    _("LUT operation order."),
                    string::Format("{0}").arg(p.options.lutOptions.order),
                    string::join(timeline::getLUTOrderLabels(), ", ")),
#ifdef MRV2_PYBIND11
                app::CmdLineHeader::create({}, _("Python (if supported):")),
                app::CmdLineFlagOption::create(
                    p.options.noPython, {"-noPython", "-np"},
                    _("Don't load python for faster startup.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.pythonScript, {"-pythonScript", "-ps"},
                    _("Python Script to run and exit.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.pythonArgs, {"-pythonArgs", "-pa"},
                    _("Python Arguments to pass to the Python script as a "
                      "single quoted string like \"arg1 'arg2 asd' arg3\", "
                      "stored in cmd.argv.")),
#endif
                app::CmdLineHeader::create({}, _("Preferences:")),
                app::CmdLineFlagOption::create(
                    p.options.resetSettings, {"-resetSettings"},
                    _("Reset settings to defaults.")),
                app::CmdLineFlagOption::create(
                    p.options.resetHotkeys, {"-resetHotkeys"},
                    _("Reset hotkeys to defaults.")),
#if defined(TLRENDER_USD)
                app::CmdLineHeader::create({}, _("OpenUSD:")),
                app::CmdLineValueOption<bool>::create(
                    p.options.usdOverrides, {"-usd", "-usdOverrides"},
                    "USD overrides.",
                    string::Format("{0}").arg(p.options.usdOverrides)),
                app::CmdLineValueOption<int>::create(
                    p.options.usd.renderWidth, {"-usdRenderWidth"},
                    "USD render width.",
                    string::Format("{0}").arg(p.options.usd.renderWidth)),
                app::CmdLineValueOption<float>::create(
                    p.options.usd.complexity, {"-usdComplexity"},
                    "USD render complexity setting.",
                    string::Format("{0}").arg(p.options.usd.complexity)),
                app::CmdLineValueOption<tl::usd::DrawMode>::create(
                    p.options.usd.drawMode, {"-usdDrawMode"},
                    "USD render draw mode.",
                    string::Format("{0}").arg(p.options.usd.drawMode),
                    string::join(tl::usd::getDrawModeLabels(), ", ")),
                app::CmdLineValueOption<bool>::create(
                    p.options.usd.enableLighting, {"-usdEnableLighting"},
                    "USD render enable lighting setting.",
                    string::Format("{0}").arg(p.options.usd.enableLighting)),
                app::CmdLineValueOption<bool>::create(
                    p.options.usd.enableSceneLights, {"-usdEnableSceneLights"},
                    "USD render enable scene lights setting.",
                    string::Format("{0}").arg(p.options.usd.enableSceneLights)),
                app::CmdLineValueOption<bool>::create(
                    p.options.usd.enableSceneMaterials,
                    {"-usdEnableSceneMaterials"},
                    "USD render enable scene materials setting.",
                    string::Format("{0}").arg(
                        p.options.usd.enableSceneMaterials)),
                app::CmdLineValueOption<bool>::create(
                    p.options.usd.sRGB, {"-usdSRGB"},
                    "USD render SRGB setting.",
                    string::Format("{0}").arg(p.options.usd.sRGB)),
                app::CmdLineValueOption<size_t>::create(
                    p.options.usd.stageCache, {"-usdStageCache"},
                    "USD stage cache size.",
                    string::Format("{0}").arg(p.options.usd.stageCache)),
                app::CmdLineValueOption<size_t>::create(
                    p.options.usd.diskCache, {"-usdDiskCache"},
                    "USD disk cache size in gigabytes. A size of zero "
                    "disables the cache.",
                    string::Format("{0}").arg(p.options.usd.diskCache)),
#endif // TLRENDER_USD
#ifdef MRV2_NETWORK
                app::CmdLineHeader::create({}, _("Networking:")),
                app::CmdLineFlagOption::create(
                    p.options.server, {"-server"},
                    _("Start a server.  Use -port to specify a port "
                      "number.")),
                app::CmdLineValueOption<std::string>::create(
                    p.options.client, {"-client"},
                    _("Connect to a server at <value>.  Use -port to "
                      "specify a port number.")),
                app::CmdLineValueOption<unsigned>::create(
                    p.options.port, {"-port"},
                    _("Port number for the server to listen to or for the "
                      "client to connect to."),
                    string::Format("{0}").arg(p.options.port)),
#endif

                app::CmdLineFlagOption::create(
                    p.options.displayVersion, {"-version", "-v"},
                    _("Return the version and exit."))});

        DBG;
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            DBG;
            return;
        }

#ifdef __APPLE__
        // For macOS, to read command-line arguments
        fl_open_callback(osx_open_cb);
#endif
        
        DBG;
        file::Path lastPath;
        const auto& unusedArgs = getUnusedArgs();
        for (const auto& unused : unusedArgs)
        {
            file::Path path = file::Path(unused);
            if (path.getDirectory() == lastPath.getDirectory() &&
                path.getBaseName() == lastPath.getBaseName() &&
                path.getPadding() == lastPath.getPadding() &&
                path.getExtension() == lastPath.getExtension() &&
                file::isSequence(unused) && !file::isDirectory(unused))
                continue;
            lastPath = path;
            p.options.fileNames.push_back(unused);
        }

        DBG;
        if (p.options.displayVersion)
        {
            std::cout << std::endl
                      << "\tmrv2 v" << mrv::version() << " "
                      << mrv::backend() << " "
                      << mrv::build_date()
                      << std::endl
                      << mrv::os::getVersion() << std::endl;
#ifdef __linux__
            std::cout << mrv::os::getDesktop() << std::endl;
#endif
            return;
        }


        DBG;
        // Initialize FLTK.
        Fl::scheme("gtk+");
        DBG;
        Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
        DBG;

#ifdef OPENGL_BACKEND
        Fl::use_high_res_GL(true);
#endif

#ifdef VULKAN_BACKEND
        Fl::use_high_res_VK(true);
#endif
        
        
        Fl::set_fonts("-*");
        DBG;
        Fl::lock(); // needed for NDI and multithreaded logging

        DBG;
        // Create the Settings
        p.settings = new SettingsObject();

        DBG;
        
        // Create the interface.
        ui = new ViewerUI();
        if (!ui)
        {
            throw std::runtime_error(_("Cannot create window"));
        }
        DBG;

        //
        // Initialize POCO Net for SSL connections.
        //
#ifdef MRV2_NETWORK
        Poco::Net::initializeSSL();
#endif
        
        // Classes used to handle network connections
#ifdef MRV2_NETWORK
        p.commandInterpreter = new CommandInterpreter(ui);
        DBG;
#endif
        tcp = new DummyClient();

        p.lutOptions = p.options.lutOptions;

        DBG;
#ifdef __APPLE__
        Fl_Mac_App_Menu::about = _("About mrv2");
        Fl_Mac_App_Menu::print = "Print Front Window";
        Fl_Mac_App_Menu::hide = _("Hide mrv2");
        Fl_Mac_App_Menu::hide_others = _("Hide Others");
        Fl_Mac_App_Menu::services = _("Services");
        Fl_Mac_App_Menu::show = _("Show All");
        Fl_Mac_App_Menu::quit = _("Quit mrv2");
        fl_open_display();
#endif
        ui->uiView->setContext(context);

        p.contextObject = new mrv::ContextObject(context);
        p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
        p.filesModel = FilesModel::create(context);
        p.playlistsModel = PlaylistsModel::create(context);

        ui->uiTimeline->setContext(context, p.timeUnitsModel, ui);
        ui->uiTimeline->setScrollBarsVisible(false);


        DBG;
        uiLogDisplay = new LogDisplay(0, 20, 340, 320);


        License ok = license_beat();

#ifdef VULKAN_BACKEND
        // Vulkan backend or floating licenses will pop up the license_helper
        // three times.
        if (license_type == LicenseType::kFloating)
            Fl::add_timeout(kLicenseTimeout, (Fl_Timeout_Handler)beat_cb, this);
#endif
        
        DBG;
        std::string version = "mrv2 v";
        version += mrv::version();
        version += " ";
        version += mrv::backend();
        version += " ";
        version += mrv::build_date();
        LOG_STATUS(version);
        LOG_STATUS(msg);
        
        DBG;

        {
            const std::string& info = mrv::build_info();
            const auto& lines = string::split(info, '\n');
            for (auto line : lines)
            {
                LOG_STATUS(line);
            }
        }

        {
            const std::string& info = mrv::running_info();
            const auto& lines = string::split(info, '\n');
            for (auto line : lines)
            {
                LOG_STATUS(line);
            }
        }

        LOG_STATUS(_("Install Location: "));
        LOG_STATUS("\t" << mrv::rootpath());
        DBG;

        if (!mrv::studiopath().empty())
        {
            LOG_STATUS(_("Studio Location: "));
            LOG_STATUS("\t" << mrv::studiopath());
        }
        
        LOG_STATUS(_("Preferences Location: "));
        if (file::isReadable(mrv::studiopath() + "/mrv2.prefs"))
            LOG_STATUS("\t" << mrv::studiopath());
        else
            LOG_STATUS("\t" << mrv::prefspath());
        
        LOG_STATUS(_("Temp Location: "));
        LOG_STATUS("\t" << mrv::tmppath());

        // Create the main control.
        p.mainControl = new MainControl(ui);

        // Store the default hotkeys, before we override them in the
        // Preferences.
        store_default_hotkeys();

        Preferences prefs(p.options.resetSettings, p.options.resetHotkeys);

        //
        // I don't think this is used, as we pass command-line arguments
        // through a launcher program anyway.
        //
        if (!OSXfiles.empty())
        {
            if (p.options.fileNames.empty())
            {
                p.options.fileNames = OSXfiles;
            }
        }

#ifdef MRV2_NETWORK
        if (ui->uiPrefs->uiPrefsSingleInstance->value())
        {
            ImageSender sender;
            if (sender.isRunning())
            {
                for (const auto& fileName : p.options.fileNames)
                {
                    // If another instance is running, send the new image files
                    // to it.
                    sender.sendImage(fileName);
                }
                return;
            }
        }
#endif

        Preferences::run();

#if defined(TLRENDER_USD)
        if (p.options.usdOverrides)
        {
            p.settings->setValue(
                "USD/renderWidth", static_cast<int>(p.options.usd.renderWidth));
            p.settings->setValue(
                "USD/complexity", static_cast<float>(p.options.usd.complexity));
            p.settings->setValue(
                "USD/drawMode", static_cast<int>(p.options.usd.drawMode));
            p.settings->setValue(
                "USD/enableLighting",
                static_cast<bool>(p.options.usd.enableLighting));
            p.settings->setValue(
                "USD/sRGB", static_cast<bool>(p.options.usd.sRGB));
            p.settings->setValue(
                "USD/stageCacheByteCount",
                static_cast<int>(p.options.usd.stageCache));
            p.settings->setValue(
                "USD/diskCacheByteCount",
                static_cast<int>(p.options.usd.diskCache * memory::gigabyte));
        }
#endif // TLRENDER_USD

#if defined(TLRENDER_BMD)
        device::DevicesModelData bmdDevicesModelData;
        p.settings->setDefaultValue(
            "BMD/DeviceIndex", bmdDevicesModelData.deviceIndex);
        p.settings->setDefaultValue(
            "BMD/DisplayModeIndex", bmdDevicesModelData.displayModeIndex);
        p.settings->setDefaultValue(
            "BMD/PixelTypeIndex", bmdDevicesModelData.pixelTypeIndex);
        p.settings->setDefaultValue(
            "BMD/DeviceEnabled", bmdDevicesModelData.deviceEnabled);
        const auto i = bmdDevicesModelData.boolOptions.find(
            device::Option::_444SDIVideoOutput);
        p.settings->setDefaultValue(
            "BMD/444SDIVideoOutput",
            i != bmdDevicesModelData.boolOptions.end() ? i->second : false);
        p.settings->setDefaultValue("BMD/HDRMode", bmdDevicesModelData.hdrMode);
        p.settings->setDefaultValue("BMD/HDRData", bmdDevicesModelData.hdrData);
#endif // TLRENDER_BMD

#if defined(TLRENDER_NDI)

        device::DevicesModelData devicesModelData;
        p.settings->setDefaultValue(
            "NDI/DeviceIndex", devicesModelData.deviceIndex);
        p.settings->setDefaultValue(
            "NDI/DisplayModeIndex", devicesModelData.displayModeIndex);
        p.settings->setDefaultValue(
            "NDI/PixelTypeIndex", devicesModelData.pixelTypeIndex);
        p.settings->setDefaultValue(
            "NDI/DeviceEnabled", devicesModelData.deviceEnabled);
        const auto i = devicesModelData.boolOptions.find(
            device::Option::_444SDIVideoOutput);
        p.settings->setDefaultValue(
            "NDI/444SDIVideoOutput",
            i != devicesModelData.boolOptions.end() ? i->second : false);
        // p.settings->setDefaultValue("NDI/HDRMode", devicesModelData.hdrMode);
        // p.settings->setDefaultValue("NDI/HDRData", devicesModelData.hdrData);

#endif // TLRENDER_NDI

        p.volume = p.settings->getValue<float>("Audio/Volume");
        p.mute = p.settings->getValue<bool>("Audio/Mute");

        if (p.options.loop != timeline::Loop::Count)
        {
            TimelineClass* c = ui->uiTimeWindow;
            c->uiLoopMode->value(static_cast<int>(p.options.loop));
            c->uiLoopMode->do_callback();
            timeline::Loop loop =
                static_cast<timeline::Loop>(c->uiLoopMode->value());
        }

        p.filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                p.filesModel->observeFiles(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { _filesUpdate(value); });

        p.activeObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { _activeUpdate(value); });

        p.layersObserver = observer::ListObserver<int>::create(
            p.filesModel->observeLayers(),
            [this](const std::vector<int>& value) { _layersUpdate(value); });

        p.compareTimeObserver =
            observer::ValueObserver<timeline::CompareTimeMode>::create(
                p.filesModel->observeCompareTime(),
                [this](timeline::CompareTimeMode value)
                {
                    if (auto player = _p->player.get())
                    {
                        player->setCompareTime(value);
                    }
                });

#if defined(TLRENDER_BMD)
        p.devicesObserver =
            observer::ValueObserver<device::DevicesModelData>::create(
                p.devicesModel->observeData(),
                [this](const device::DevicesModelData& value)
                {
                    TLRENDER_P();

                    device::DeviceConfig config;
                    config.deviceIndex = value.deviceIndex - 1;
                    config.displayModeIndex = value.displayModeIndex - 1;
                    config.pixelType =
                        value.pixelTypeIndex >= 0 &&
                                value.pixelTypeIndex < value.pixelTypes.size()
                            ? value.pixelTypes[value.pixelTypeIndex]
                            : device::PixelType::kNone;
                    config.boolOptions = value.boolOptions;
                    p.outputDevice->setConfig(config);
                    p.outputDevice->setEnabled(value.deviceEnabled);
                    p.putputVideoLevels = value.videoLevels;
                    timeline::DisplayOptions displayOptions =
                        p.viewportModel->getDisplayOptions();
                    displayOptions.videoLevels = p.bmdOutputVideoLevels;
                    p.outputDevice->setDisplayOptions({displayOptions});
                    p.outputDevice->setHDR(value.hdrMode, value.hdrData);

                    p.settings->setValue("BMD/DeviceIndex", value.deviceIndex);
                    p.settings->setValue(
                        "BMD/DisplayModeIndex", value.displayModeIndex);
                    p.settings->setValue(
                        "BMD/PixelTypeIndex", value.pixelTypeIndex);
                    p.settings->setValue(
                        "BMD/DeviceEnabled", value.deviceEnabled);
                    const auto i = value.boolOptions.find(
                        device::Option::_444SDIVideoOutput);
                    p.settings->setValue(
                        "BMD/444SDIVideoOutput",
                        i != value.boolOptions.end() ? i->second : false);
                    // p.settings->setValue("NDI/HDRMode", value.hdrMode);
                    // p.settings->setValue("NDI/HDRData", value.hdrData);
                });

        p.bmdActiveObserver = observer::ValueObserver<bool>::create(
            p.bmdOutputDevice->observeActive(),
            [this](bool value)
            {
                _p->bmdDeviceActive = value;
                _audioUpdate();
            });
        p.bmdSizeObserver = observer::ValueObserver<math::Size2i>::create(
            p.bmdOutputDevice->observeSize(),
            [this](const math::Size2i& value)
            {
                // std::cout << "output device size: " << value << std::endl;
            });
        p.bmdFrameRateObserver =
            observer::ValueObserver<otime::RationalTime>::create(
                p.bmdOutputDevice->observeFrameRate(),
                [this](const otime::RationalTime& value)
                {
                    // std::cout << "output device frame rate: " << value <<
                    // std::endl;
                });

#endif // TLRENDER_BMD

        p.logObserver = observer::ListObserver<log::Item>::create(
            ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
            {
                static std::string lastStatusMessage;
                static std::string lastWarningMessage;
                static std::string lastErrorMessage;
                for (const auto& i : value)
                {
                    const std::string& msg = i.message;
                    const std::string& kModule = i.module;
                    switch (i.type)
                    {
                    case log::Type::Error:
                    {
                        if (msg == lastErrorMessage)
                            continue;

                        lastErrorMessage = msg;

                        LOG_ERROR(msg);
                        break;
                    }
                    case log::Type::Warning:
                    {
                        if (msg == lastWarningMessage)
                            continue;

                        lastWarningMessage = msg;

                        LOG_WARNING(msg);
                        break;
                    }
                    case log::Type::kStatus:
                    {
                        if (msg == lastStatusMessage)
                            continue;

                        lastStatusMessage = msg;

                        LOG_STATUS(msg);
                        break;
                    }
                    default:
                        if (_options.log)
                        {
                            uiLogDisplay->info(msg.c_str());
                        }
                        break;
                    }
                }
            });

        // Open the input files.
        int savedDigits =
            p.settings->getValue<int>("Misc/MaxFileSequenceDigits");
        if (p.options.singleImages)
        {
            p.settings->setValue("Misc/MaxFileSequenceDigits", 0);
        }

#ifdef MRV2_PYBIND11
        // Create Python's output window
        Fl_Group::current(0);
        outputDisplay = new PythonOutput(0, 0, 400, 400);
#endif
        
        //
        // Show the UI if no python script was fed in (when Python is supported).
        // We make sure the UI is visible when we feed a filename.
        // This is needed to avoid an issue with Wayland not properly refreshing the play buttons.
        //
        bool showUI = true;

#ifdef MRV2_PYBIND11
        if (App::supports_python && !p.options.pythonScript.empty())
        {
            showUI = false;
        }
#endif

        if (showUI)
        {
            ui->uiMain->show();
            ui->uiView->take_focus();

            // Fix for always on top on Linux
            bool value = ui->uiPrefs->uiPrefsAlwaysOnTop->value();
            int fullscreen_active = ui->uiMain->fullscreen_active();
            if (!fullscreen_active)
            {
                ui->uiMain->always_on_top(value);
            }
        }            

        if (!p.options.fileNames.empty())
        {
            if (p.options.createOtioTimeline)
            {
                std::string otioFile;
                bool ok = createEDLFromFiles(otioFile, p.options.fileNames);
                if (!ok)
                {
                    LOG_ERROR("Could not create .otio EDL playlist from files");
                    return;
                }

                // Replace filenames with newly created otio file
                p.options.fileNames.clear();
                p.options.fileNames.push_back(otioFile);
            }

            bool foundAudio = false;
            for (const auto& fileName : p.options.fileNames)
            {
                if (file::isSequence(fileName) && !foundAudio)
                {
                    open(fileName, p.options.audioFileName);
                    foundAudio = true;
                }
                else
                {
                    open(fileName);
                }
            }

            if (auto player = p.player.get())
            {
                if (!player)
                {
                    return;
                }

                if (p.options.speed > 0.0)
                {
                    player->setSpeed(p.options.speed);
                }
                if (time::isValid(p.options.inOutRange))
                {
                    player->setInOutRange(p.options.inOutRange);
                    player->seek(p.options.inOutRange.start_time());
                }
                if (time::isValid(p.options.seek))
                {
                    const auto& timeRange = player->inOutRange();

                    if (p.options.seek < timeRange.start_time())
                        p.options.seek = timeRange.start_time();
                    else if (p.options.seek > timeRange.end_time_exclusive())
                        p.options.seek = timeRange.end_time_exclusive();

                    player->seek(p.options.seek);
                }
                if (p.options.loop != timeline::Loop::Count)
                {
                    timeline::Loop loop = p.options.loop;
                    player->setLoop(p.options.loop);
                }
            }
        }

        if (!p.options.compareFileName.empty())
        {
            open(p.options.compareFileName);
            p.filesModel->setCompareOptions(p.options.compareOptions);
            size_t numFiles = p.filesModel->observeFiles()->getSize();
            p.filesModel->setB(numFiles - 1, true);
        }

        if (p.options.singleImages)
        {
            p.settings->setValue("Misc/MaxFileSequenceDigits", savedDigits);
        }

        if (!p.options.fileNames.empty() && !p.session)
        {
            auto model = filesModel();
            if (model->observeFiles()->getSize() > 0)
                model->setA(0);
        }

#ifdef MRV2_NETWORK
        if (p.options.server)
        {
            try
            {
                tcp = new Server(p.options.port);
                store_port(p.options.port);
            }
            catch (const Poco::Exception& e)
            {
                LOG_ERROR(e.displayText());
            }
        }
        else if (!p.options.client.empty())
        {
            std::string port;
            parse_hostname(p.options.client, port);
            if (!port.empty())
            {
                p.options.port = atoi(port.c_str());
            }
            tcp = new Client(p.options.client, p.options.port);
            store_port(p.options.port);
        }

#endif

        //
        // Handle command-line OCIO
        //
        try
        {
            if (!p.options.ocioOptions.input.empty())
                ocio::setIcs(p.options.ocioOptions.input);

            if (!p.options.ocioOptions.look.empty())
                ocio::setLook(p.options.ocioOptions.look);

            if (!p.options.ocioOptions.display.empty())
            {
                if (!p.options.ocioOptions.view.empty())
                {
                    const std::string& merged = ocio::combineView(
                        p.options.ocioOptions.display,
                        p.options.ocioOptions.view);
                    ocio::setView(merged);
                }
                else
                {
                    ocio::setDisplay(p.options.ocioOptions.display);
                }
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }

#ifdef MRV2_PYBIND11
        if (!p.options.noPython && App::supports_python)
        {
            // Import the mrv2 python module so we read all python
            // plug-ins.
            py::module::import("mrv2");

            // Discover Python plugins
            mrv2_discover_python_plugins();

            //
            // Run command-line python script.
            //
            if (!p.options.pythonScript.empty())
            {
                std::string script = p.options.pythonScript;
                if (!file::isReadable(script))
                {
                    // Search for script in $STUDIOPATH/python/ directory
                    std::string studio_script = studiopath() + "/python/" + script;
                    if (file::isReadable(studio_script))
                    {
                        script = studio_script;
                    }
                    else
                    {
                        // Search for script in mrv2's python demos directory.
                        script = pythonpath() + script;
                        if (!file::isReadable(script))
                        {
                            std::cerr
                                << std::string(
                                    string::Format(
                                        _("Could not read python script '{0}'"))
                                    .arg(p.options.pythonScript))
                                << std::endl;
                            _exit = 1;
                            return;
                        }
                    }
                }

                p.pythonArgs = std::make_unique<PythonArgs>(p.options.pythonArgs);

                LOG_STATUS(std::string(
                               string::Format(_("Running python script '{0}'")).arg(script)));
                const auto& args = p.pythonArgs->getArguments();

                if (!args.empty())
                {
                    LOG_STATUS(_("with Arguments:"));
                    std::string out = "[";
                    out += tl::string::join(args, ',');
                    out += "]";
                    LOG_STATUS(out);
                }

                std::ifstream is(script);
                std::stringstream s;
                s << is.rdbuf();
                try
                {
                    py::exec(s.str());
                }
                catch (const std::exception& e)
                {
                    std::cerr << _("Python Error: ") << std::endl
                              << e.what() << std::endl;
                    _exit = 1;
                    return;
                }
                delete ui;
                ui = nullptr;
                return;
            }

            DBG;
            // Redirect Python's stdout/stderr to my own class
            p.pythonStdErrOutRedirect.reset(new PyStdErrOutStreamRedirect);
        }
#endif

        // Open Panel Windows if not loading a session file.
        if (!p.session)
            Preferences::open_windows();

        // We raise the secondary window last, so it shows at front
        if (ui->uiSecondary)
        {
            ui->uiSecondary->window()->show();
        }

        // Update menus
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void App::cleanResources()
    {
        TLRENDER_P();
        
        delete p.mainControl;
        p.mainControl = nullptr;

#ifdef MRV2_NETWORK
        delete p.commandInterpreter;
        p.commandInterpreter = nullptr;
#endif
        removeListener();

        delete ui;
        ui = nullptr;

        delete p.contextObject;
        p.contextObject = nullptr;

        if (tcp)
        {
            tcp->stop();
            tcp->close();
            delete tcp;
            tcp = nullptr;
        }
    }

    App::~App()
    {
        TLRENDER_P();

        cleanResources();
    }

    const std::shared_ptr<timeline::TimeUnitsModel>& App::timeUnitsModel() const
    {
        return _p->timeUnitsModel;
    }

    SettingsObject* App::settings() const
    {
        return _p->settings;
    }

    const std::shared_ptr<FilesModel>& App::filesModel() const
    {
        return _p->filesModel;
    }

    const std::shared_ptr<PlaylistsModel>& App::playlistsModel() const
    {
        return _p->playlistsModel;
    }

    const timeline::LUTOptions& App::lutOptions() const
    {
        return _p->lutOptions;
    }

    const timeline::ImageOptions& App::imageOptions() const
    {
        return _p->imageOptions;
    }

    const timeline::DisplayOptions& App::displayOptions() const
    {
        return _p->displayOptions;
    }

    float App::volume() const
    {
        return _p->volume;
    }

    bool App::isMuted() const
    {
        return _p->mute;
    }

    struct PlaybackData
    {
        std::shared_ptr<mrv::TimelinePlayer> player;
        timeline::Playback playback;
    };

    void App::createComfyUIListener()
    {
#ifdef MRV2_NETWORK
        TLRENDER_P();

        try
        {
            p.comfyUIListener = std::make_unique<ComfyUIListener>();
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR(e.displayText());
        }
#endif
    }

    void App::createListener()
    {
#ifdef MRV2_NETWORK
        TLRENDER_P();

        try
        {
            p.imageListener = std::make_unique<ImageListener>(this);
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR(e.displayText());
        }
#endif
    }

#ifdef MRV2_PYBIND11
    const std::vector<std::string>& App::getPythonArgs() const
    {
        TLRENDER_P();
        return p.pythonArgs->getArguments();
    }
#endif

    void App::removeListener()
    {
#ifdef MRV2_NETWORK
        TLRENDER_P();

        p.imageListener.reset();
#endif
    }

    static void start_playback_cb(App* app)
    {
        app->startPlayback();
    }

    void App::_calculateCacheTimes(otime::RationalTime& startTime,
                                   otime::RationalTime& endTime)
    {
        TLRENDER_P();
        const timeline::Playback& playback = p.options.playback;
        const auto& time = p.player->currentTime();    

        if (playback != timeline::Playback::Reverse)
        {
            const auto& cache =
                p.player->player()->observeCacheOptions()->get();
            const auto& rate = time.rate();
            const auto& readAhead =
                cache.readAhead.rescaled_to(rate);
            const auto& readBehind =
                cache.readBehind.rescaled_to(rate);
            const auto& timeRange = p.player->inOutRange();

            startTime = time;
            endTime = time + readAhead;
            if (endTime >= timeRange.end_time_exclusive())
            {
                endTime = timeRange.end_time_exclusive();
            }

            // Avoid rounding errors
            endTime = endTime.floor();
            startTime = startTime.ceil();
        }
        else
        {
            const auto& cache =
                p.player->player()->observeCacheOptions()->get();
            const auto& rate = time.rate();
            const auto& readAhead =
                cache.readAhead.rescaled_to(rate);
            const auto& readBehind =
                cache.readBehind.rescaled_to(rate);
            const auto& timeRange = p.player->inOutRange();

            startTime = time + readBehind;
            endTime = time - readAhead;
            if (endTime < timeRange.start_time())
            {
                auto diff = timeRange.start_time() - endTime;
                endTime = timeRange.end_time_exclusive();
                startTime = endTime - diff;

                // Check in case of negative frames
                if (startTime > endTime)
                    startTime = endTime;
            }

            // Sanity check just in case
            if (endTime < startTime)
            {
                const auto tmp = endTime;
                endTime = startTime;
                startTime = tmp;
            }

            // Avoid rounding errors
            endTime = endTime.floor();
            startTime = startTime.ceil();
        }
        
    }
    
    void App::startPlayback()
    {
        TLRENDER_P();
        

        p.player->setPlayback(timeline::Playback::Stop);

        //
        // Check if loading 4K or a movie of more than 180 seconds.
        //
        bool use_progress = false;
        auto info = p.player->ioInfo();
        if (!info.video.empty())
        {
            auto video = info.video[0];
            const auto duration = info.videoTime.duration();
            if (duration.to_seconds() > 180.0)
                use_progress = true;
            if (video.size.w > 2048)
                use_progress = true;
        }
        // Calculate start and end time used in progress report
        otime::RationalTime startTime, endTime;
        _calculateCacheTimes(startTime, endTime);
        
        const timeline::Playback& playback = p.options.playback;

        int64_t start = std::floor(startTime.to_seconds());
        int64_t end   = std::ceil(endTime.to_seconds());

        if (!p.progress)
        {
            p.progress = new ProgressReport(ui->uiMain,
                                            start, end,
                                            _("Caching..."));
        }
        else
        {
            p.progress->set_start(start);
            p.progress->set_end(end);
        }
        if (use_progress)
        {
            p.progress->show();
            Fl::flush();
        }
        else
        {
            // Start playback right away.
            ui->uiView->setPlayback(playback);
        }

        if (playback == timeline::Playback::Forward ||
            playback == timeline::Playback::Stop)
        {
            p.cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    p.player->player()->observeCacheInfo(),
                    [this, playback](const timeline::PlayerCacheInfo& value)
                        {
                            TLRENDER_P();

                            otime::RationalTime startTime, endTime;
                            _calculateCacheTimes(startTime, endTime);
                            
                            // std::cerr << "in line " << __LINE__
                            //           << " "
                            //           << value.videoFrames.size()
                            //           << std::endl;
                            
                            
                            // Keep UI responsive
                            if (p.progress)
                            {
                                if (!p.progress->tick())
                                {
                                    delete p.progress;
                                    p.progress = nullptr;
                                    ui->uiView->setPlayback(playback);
                                    p.cacheInfoObserver.reset();
                                    return;
                                }
                            }
                            else
                            {
                                Fl::check();
                            }

                            for (const auto& t : value.videoFrames)
                            {
                                if (t.start_time() <= startTime &&
                                    t.end_time_exclusive() >= endTime)
                                {
                                    delete p.progress;
                                    p.progress = nullptr;
                                    ui->uiView->setPlayback(playback);
                                    p.cacheInfoObserver.reset();
                                    return;
                                }
                            }
                        });
        }
        else if (playback == timeline::Playback::Reverse)
        {

            p.cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    p.player->player()->observeCacheInfo(),
                    [this, playback](const timeline::PlayerCacheInfo& value)
                        {
                            TLRENDER_P();
                            
                            otime::RationalTime startTime, endTime;
                            _calculateCacheTimes(startTime, endTime);
                            
                            // Keep UI responsive
                            if (p.progress)
                            {
                                if (!p.progress->tick())   
                                {
                                    delete p.progress;
                                    p.progress = nullptr;
                                    ui->uiView->setPlayback(playback);
                                    p.cacheInfoObserver.reset();
                                    return;
                                }
                            }
                            else
                            {
                                Fl::check();
                            }
                            
                            for (const auto& t : value.videoFrames)
                            {
                                if (t.start_time() <= startTime &&
                                    t.end_time_exclusive() >= endTime)
                                {
                                    delete p.progress;
                                    p.progress = nullptr;
                                    ui->uiView->setPlayback(playback);
                                    p.cacheInfoObserver.reset();
                                    break;
                                }
                            }
                        });
        }
    }

    int App::run()
    {
        TLRENDER_P();
        if (!ui)
            return 0;

        // Open the viewer window by calling Fl::flush
        Fl::flush();

        if (p.player)
        {
            const auto& timeRange = p.player->inOutRange();
            const bool autoPlayback = ui->uiPrefs->uiPrefsAutoPlayback->value();
            const bool playbackFlag =
                p.options.playback != timeline::Playback::Count &&
                p.options.playback != timeline::Playback::Stop;
            if (!p.session && (autoPlayback || playbackFlag) &&
                timeRange.duration().value() > 1)
            {
                // We use a timeout to start playback of the loaded video to
                // make sure to show all frames
                if (p.options.playback == timeline::Playback::Count)
                    p.options.playback = timeline::Playback::Forward;
            
                Fl::add_timeout(
                    0.0, (Fl_Timeout_Handler)start_playback_cb, this);
            }
        }
        p.running = true;
            
        return Fl::run();
    }

    bool App::isRunning() const
    {
        return _p->running;
    }

    const std::shared_ptr<device::DevicesModel>& App::devicesModel() const
    {
        return _p->devicesModel;
    }

    const std::shared_ptr<device::IOutput>& App::outputDevice() const
    {
        return _p->outputDevice;
    }
    
#if defined(TLRENDER_BMD) || defined(TLRENDER_NDI)
    void App::_timer_update_cb(App* self)
    {
        self->timerUpdate();
    }

    void App::timerUpdate()
    {
        TLRENDER_P();

        if (p.outputDevice)
            p.outputDevice->tick();

        Fl::repeat_timeout(
            kTimeout, (Fl_Timeout_Handler)_timer_update_cb, this);
    }

    void App::_startOutputDeviceTimer()
    {
        TLRENDER_P();

        // Needed to refresh the outputDevice annotations
        ui->uiView->redraw();

        if (p.outputDevice)
        {
            p.outputDevice->setPlayer(p.player ? p.player->player() : nullptr);
            p.outputDevice->setEnabled(true);
        }
        
        Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)_timer_update_cb, this);
    }

    void App::_stopOutputDeviceTimer()
    {
        TLRENDER_P();

        if (p.outputDevice)
            p.outputDevice->setEnabled(false);
        
        // \@todo: Remove.  Needed to refresh the viewport annotations
        ui->uiView->redraw();
        
        Fl::remove_timeout((Fl_Timeout_Handler)_timer_update_cb, this);
    }
#endif // defined(TLRENDER_BMD) || defined(TLRENDER_NDI)

#if defined(TLRENDER_NDI)
    void App::beginNDIOutputStream()
    {
        TLRENDER_P();
        device::DeviceConfig config;
#ifdef OPENGL_BACKEND
        p.outputDevice = ndi::OutputDevice::create(_context);
#endif
#ifdef VULKAN_BACKEND
        p.outputDevice = ndi::OutputDevice::create(
            ui->uiView->getContext(), _context);
#endif
        p.outputDevice->setConfig(config);
        p.devicesModel = device::DevicesModel::create(_context);
        p.devicesModel->setDeviceIndex(
            p.settings->getValue<int>("NDI/DeviceIndex"));
        p.devicesModel->setDisplayModeIndex(
            p.settings->getValue<int>("NDI/DisplayModeIndex"));
        p.devicesModel->setPixelTypeIndex(
            p.settings->getValue<int>("NDI/PixelTypeIndex"));
        p.devicesModel->setDeviceEnabled(
            p.settings->getValue<bool>("NDI/DeviceEnabled"));
        device::BoolOptions deviceBoolOptions;
        deviceBoolOptions[device::Option::_444SDIVideoOutput] =
            p.settings->getValue<bool>("NDI/444SDIVideoOutput");
        p.devicesModel->setBoolOptions(deviceBoolOptions);
        p.devicesModel->setHDRMode(static_cast<device::HDRMode>(
            p.settings->getValue<int>("NDI/HDRMode")));
        std::string s = p.settings->getValue<std::string>("NDI/HDRData");
        if (!s.empty())
        {
            auto json = nlohmann::json::parse(s);
            image::HDRData hdrData;
            try
            {
                from_json(json, hdrData);
            }
            catch (const std::exception&)
            {
            }
            p.devicesModel->setHDRData(hdrData);
        }
        _startOutputDeviceTimer();
    }

    void App::endNDIOutputStream()
    {
        _stopOutputDeviceTimer();
        _p->outputDevice.reset();
        _p->devicesModel.reset();
    }
#endif

#ifdef TLRENDER_BMD
    void App::beginBMDOutputStream()
    {
        TLRENDER_P();
        p.outputDevice = bmd::OutputDevice::create(_context);
        p.devicesModel = device::DevicesModel::create(_context);
        p.devicesModel->setDeviceIndex(
            p.settings->getValue<int>("BMD/DeviceIndex"));
        p.devicesModel->setDisplayModeIndex(
            p.settings->getValue<int>("BMD/DisplayModeIndex"));
        p.devicesModel->setPixelTypeIndex(
            p.settings->getValue<int>("BMD/PixelTypeIndex"));
        p.devicesModel->setDeviceEnabled(
            p.settings->getValue<bool>("BMD/DeviceEnabled"));
        device::BoolOptions deviceBoolOptions;
        deviceBoolOptions[bmd::Option::_444SDIVideoOutput] =
            p.settings->getValue<bool>("BMD/444SDIVideoOutput");
        p.devicesModel->setBoolOptions(deviceBoolOptions);
        p.devicesModel->setHDRMode(static_cast<device::HDRMode>(
            p.settings->getValue<int>("BMD/HDRMode")));
        std::string s = p.settings->getValue<std::string>("BMD/HDRData");
        if (!s.empty())
        {
            auto json = nlohmann::json::parse(s);
            image::HDRData hdrData;
            try
            {
                from_json(json, hdrData);
            }
            catch (const std::exception&)
            {
            }
            p.devicesModel->setHDRData(hdrData);
        }
        _startOutputDeviceTimer();
    }

    void App::endBMDOutputStream()
    {
        _stopOutputDeviceTimer();
        _p->outputDevice.reset();
        _p->devicesModel.reset();
    }
#endif

    void
    App::open(const std::string& fileName, const std::string& audioFileName)
    {
        TLRENDER_P();

        file::Path filePath(file::normalizePath(fileName));

        if (filePath.getExtension() == ".mrv2s")
        {
            p.session = true;
            session::load(fileName);
            return;
        }

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits =
            p.settings->getValue<int>("Misc/MaxFileSequenceDigits");

        if (!file::isDirectory(fileName) && !file::isReadable(fileName))
        {
            /* xgettext:c-format */
            const std::string err =
                string::Format(_("Filename '{0}' does not exist or does not "
                                 "have read permissions."))
                    .arg(fileName);
            LOG_ERROR(err);
            return;
        }
    

        for (const auto& path :
                 timeline::getPaths(filePath, pathOptions, _context) )
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(file::normalizePath(audioFileName));

            p.filesModel->add(item);
        }

        if (ui->uiPrefs->SendMedia->value())
        {
            Message msg;
            msg["command"] = "Open File";
            msg["fileName"] = fileName;
            msg["audioFileName"] = audioFileName;
            tcp->pushMessage(msg);
        }
    }

    void App::openSeparateAudioDialog()
    {
        auto dialog = std::make_unique<OpenSeparateAudioDialog>(_context, ui);
        if (dialog->exec())
        {
            open(dialog->videoFileName(), dialog->audioFileName());
        }
    }

    void App::setLUTOptions(const timeline::LUTOptions& value)
    {
        TLRENDER_P();
        if (value == p.lutOptions)
            return;
        p.lutOptions = value;
        lutOptionsChanged(p.lutOptions);
    }

    void App::lutOptionsChanged(const tl::timeline::LUTOptions& value)
    {
        TLRENDER_P();
        p.mainControl->setLUTOptions(value);
    }

    void App::setImageOptions(const timeline::ImageOptions& value)
    {
        TLRENDER_P();
        if (value == p.imageOptions)
            return;
        p.imageOptions = value;
        imageOptionsChanged(p.imageOptions);
    }

    void App::imageOptionsChanged(const tl::timeline::ImageOptions& value)
    {
        TLRENDER_P();
        p.mainControl->setImageOptions(value);
    }

    void App::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        TLRENDER_P();

        if (value == p.displayOptions)
            return;

        p.displayOptions = value;
        displayOptionsChanged(p.displayOptions);
    }

    void App::displayOptionsChanged(const tl::timeline::DisplayOptions& value)
    {
        TLRENDER_P();
        p.mainControl->setDisplayOptions(value);
        Message msg;
        Message opts(value);
        msg["command"] = "Display Options";
        msg["value"] = opts;
        tcp->pushMessage(msg);
    }

    void App::setVolume(float value)
    {
        TLRENDER_P();
        if (value == p.volume)
            return;
        p.volume = value;
        _audioUpdate();
        volumeChanged(p.volume);
    }

    void App::volumeChanged(float value)
    {
        TLRENDER_P();
        bool send = ui->uiPrefs->SendAudio->value();
        if (send)
            tcp->pushMessage("setVolume", value);
        p.settings->setValue("Audio/Volume", value);
    }

    void App::setMute(bool value)
    {
        TLRENDER_P();
        if (value == p.mute)
            return;
        p.mute = value;
        _audioUpdate();
        muteChanged(p.mute);
    }

    void App::muteChanged(bool value)
    {
        TLRENDER_P();
        bool send = ui->uiPrefs->SendAudio->value();
        if (send)
            tcp->pushMessage("setMute", value);
        p.settings->setValue("Audio/Mute", value);
    }

    io::Options App::_getIOOptions() const
    {
        TLRENDER_P();
        io::Options out;

        out["SequenceIO/ThreadCount"] = string::Format("{0}").arg(
            p.settings->getValue<int>("SequenceIO/ThreadCount"));
        out["SequenceIO/DefaultSpeed"] =
            string::Format("{0}").arg(ui->uiPrefs->uiPrefsFPS->value());

#if defined(TLRENDER_EXR)
        out["OpenEXR/IgnoreDisplayWindow"] =
            string::Format("{0}").arg(ui->uiView->getIgnoreDisplayWindow());
        out["IgnoreChromaticities"] =
            string::Format("{0}").arg(p.displayOptions.ignoreChromaticities);
#endif
#if defined(TLRENDER_EXR) || defined(TLRENDER_STB)
        out["AutoNormalize"] =
            string::Format("{0}").arg(p.displayOptions.normalize.enabled);
        out["InvalidValues"] =
            string::Format("{0}").arg(p.displayOptions.invalidValues);
#endif

#if defined(TLRENDER_EXR)
        int xLevel = 0, yLevel = 0;
        if (panel::imageInfoPanel)
        {
            xLevel = panel::imageInfoPanel->getXLevel();
            yLevel = panel::imageInfoPanel->getYLevel();
        }
        out["X Level"] = string::Format("{0}").arg(xLevel);
        out["Y Level"] = string::Format("{0}").arg(yLevel);
#endif

#if defined(TLRENDER_FFMPEG)
        out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").arg(
            p.settings->getValue<int>("Performance/FFmpegYUVToRGBConversion"));
        int fastYUV420PConversion =
            !(p.settings->getValue<int>("Performance/FFmpegColorAccuracy"));
        out["FFmpeg/FastYUV420PConversion"] =
            string::Format("{0}").arg(fastYUV420PConversion);
        out["FFmpeg/ThreadCount"] = string::Format("{0}").arg(
            p.settings->getValue<int>("Performance/FFmpegThreadCount"));

        TimelineClass* c = ui->uiTimeWindow;
        int idx = c->uiAudioTracks->current_track();
        out["FFmpeg/AudioTrack"] = string::Format("{0}").arg(idx);
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        out["USD/renderWidth"] = string::Format("{0}").arg(
            p.settings->getValue<int>("USD/renderWidth"));
        float complexity = p.settings->getValue<float>("USD/complexity");
        out["USD/complexity"] = string::Format("{0}").arg(complexity);
        {
            std::stringstream ss;
            tl::usd::DrawMode usdDrawMode = static_cast<tl::usd::DrawMode>(
                p.settings->getValue<int>("USD/drawMode"));
            ss << usdDrawMode;
            out["USD/drawMode"] = ss.str();
        }
        out["USD/enableLighting"] = string::Format("{0}").arg(
            p.settings->getValue<bool>("USD/enableLighting"));
        out["USD/sRGB"] =
            string::Format("{0}").arg(p.settings->getValue<bool>("USD/sRGB"));
        out["USD/stageCacheByteCount"] = string::Format("{0}").arg(
            p.settings->getValue<int>("USD/stageCacheByteCount"));
        out["USD/diskCacheByteCount"] = string::Format("{0}").arg(
            p.settings->getValue<int>("USD/diskCacheByteCount"));
#endif

        return out;
    }

    void App::_filesUpdate(
        const std::vector<std::shared_ptr<FilesModelItem> >& files)
    {
        TLRENDER_P();

        std::vector<std::shared_ptr<timeline::Timeline> > timelines(
            files.size());

        for (size_t i = 0; i < files.size(); ++i)
        {
            const auto j = std::find(p.files.begin(), p.files.end(), files[i]);
            if (j != p.files.end())
            {
                timelines[i] = p.timelines[j - p.files.begin()];
            }
        }

        for (size_t i = 0; i < files.size(); ++i)
        {
            if (!timelines[i])
            {
                const auto& item = files[i];
                try
                {
                    timelines[i] = _createTimeline(item);
                    const auto info = timelines[i]->getIOInfo();
                    for (const auto& video : info.video)
                    {
                        files[i]->videoLayers.push_back(video.name);
                    }
                }
                catch (const std::exception& e)
                {
                    _log(e.what(), log::Type::Error);
                }
            }
        }

        p.files = files;
        p.timelines = timelines;

        panel::refreshThumbnails();
    }

    void App::_playerOptions(
        timeline::PlayerOptions& playerOptions,
        const std::shared_ptr<FilesModelItem>& item)
    {
        TLRENDER_P();

        playerOptions.cache.readAhead = time::invalidTime;
        playerOptions.cache.readBehind = time::invalidTime;

        playerOptions.timerMode = static_cast<timeline::TimerMode>(
            p.settings->getValue<int>("Performance/TimerMode"));
        playerOptions.audioBufferFrameCount =
            p.settings->getValue<int>("Performance/AudioBufferFrameCount");
    }

    std::shared_ptr<timeline::Timeline>
    App::_createTimeline(const std::shared_ptr<FilesModelItem>& item)
    {
        TLRENDER_P();

        timeline::Options options;

        options.fileSequenceAudio = static_cast<timeline::FileSequenceAudio>(
            p.settings->getValue<int>("FileSequence/Audio"));
        options.fileSequenceAudioFileName =
            p.settings->getValue<std::string>("FileSequence/AudioFileName");
        options.fileSequenceAudioDirectory =
            p.settings->getValue<std::string>("FileSequence/AudioDirectory");

        options.videoRequestCount =
            p.settings->getValue<int>("Performance/VideoRequestCount");
        options.audioRequestCount =
            p.settings->getValue<int>("Performance/AudioRequestCount");

        options.ioOptions = _getIOOptions();
        options.pathOptions.maxNumberDigits = std::min(
            p.settings->getValue<int>("Misc/MaxFileSequenceDigits"), 255);

        otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
        otime::RationalTime offsetTime;
        double value = ui->uiPrefs->uiStartTimeOffset->value();
        offsetTime = otime::RationalTime(value, 24.0); // rate is not used.

        if (file::isUSD(item->path))
        {
#ifdef MRV2_PYBIND11
            if (!p.options.noPython)
                py::gil_scoped_release release;
#endif
            otioTimeline = item->audioPath.isEmpty()
                               ? timeline::create(
                                     item->path, _context, offsetTime, options)
                               : timeline::create(
                                     item->path, item->audioPath, _context,
                                     offsetTime, options);
        }
        else
        {
            otioTimeline = item->audioPath.isEmpty()
                               ? timeline::create(
                                     item->path, _context, offsetTime, options)
                               : timeline::create(
                                     item->path, item->audioPath, _context,
                                     offsetTime, options);
        }

        auto out = timeline::Timeline::create(otioTimeline, _context, options);

#ifdef MRV2_PYBIND11
        const std::string& path = item->path.get();
        const std::string& audioPath = item->audioPath.get();
        for (const auto& pythonCb : pythonOpenFileCallbacks)
        {
            run_python_open_file_cb(pythonCb, path, audioPath);
        }
#endif
        return out;
    }

    void App::_activeUpdate(
        const std::vector<std::shared_ptr<FilesModelItem> >& activeFiles)
    {
        TLRENDER_P();

        DBG;
        
        std::shared_ptr<TimelinePlayer> player;
        if (!p.activeFiles.empty() && isRunning() && p.player)
        {
            p.activeFiles[0]->speed = p.player->speed();
            // p.activeFiles[0]->playback = p.player->playback();
            p.activeFiles[0]->loop = p.player->loop();
            p.activeFiles[0]->currentTime = p.player->currentTime();
            p.activeFiles[0]->inOutRange = p.player->inOutRange();
            p.activeFiles[0]->audioOffset = p.player->audioOffset();
            p.activeFiles[0]->annotations = p.player->getAllAnnotations();
            p.activeFiles[0]->voiceAnnotations = p.player->getAllVoiceAnnotations();
            p.activeFiles[0]->ocioIcs = ocio::ics();
            p.activeFiles[0]->ocioLook = ocio::look();
            p.activeFiles[0]->lutOptions = p.lutOptions;
        }

        if (!activeFiles.empty())
        {
            if (!p.activeFiles.empty() && activeFiles[0] == p.activeFiles[0])
            {
                player = p.player;
            }
            else
            {

                auto i =
                    std::find(p.files.begin(), p.files.end(), activeFiles[0]);
                if (i != p.files.end())
                {
                    const auto& item = *i;
                    try
                    {
                        timeline::PlayerOptions playerOptions;
                        _playerOptions(playerOptions, item);

                        bool isEDL = file::isTemporaryEDL(item->path);
                        size_t idx = i - p.files.begin();

                        if (isEDL)
                        {
                            p.timelines[idx] = _createTimeline(item);
                        }

                        auto timeline = p.timelines[idx];
                        if (!timeline)
                            return;
                        player.reset(new TimelinePlayer(
                            timeline::Player::create(
                                timeline, _context, playerOptions),
                            _context));

                        item->timeRange = player->timeRange();
                        item->ioInfo = player->ioInfo();
                        if (!item->init)
                        {
                            item->init = true;
                            item->speed = player->speed();
                            item->playback = player->playback();
                            item->loop = player->loop();
                            item->currentTime = player->currentTime();
                            item->inOutRange = player->inOutRange();
                            item->audioOffset = player->audioOffset();
                            item->lutOptions = p.lutOptions;

                            bool autoPlayback =
                                ui->uiPrefs->uiPrefsAutoPlayback->value();
                            if (item->inOutRange.duration().value() <= 1)
                                autoPlayback = false;
                            if (autoPlayback)
                            {
                                item->playback = timeline::Playback::Forward;
                            }

                            if (!file::isTemporaryEDL(item->path) &&
                                autoPlayback && isRunning())
                            {
                                
                                // If we have autoplayback on and auto hide
                                // pixel bar, do so here.
                                const int autoHide =
                                    ui->uiPrefs->uiPrefsAutoHidePixelBar
                                    ->value();
                                const bool pixelToolbar =
                                    ui->uiPixelBar->visible();
#ifdef OPENGL_BACKEND
                                if (autoHide && pixelToolbar)
#endif
#ifdef VULKAN_BACKEND
                                if (autoHide == kAutoHideOpenGLAndVulkan &&
                                    pixelToolbar)
#endif
                                {
                                    toggle_pixel_bar(nullptr, ui);
                                    Fl::flush();
                                }

                                p.options.playback = timeline::Playback::Forward;
                                Fl::add_timeout(
                                    0.0, (Fl_Timeout_Handler)start_playback_cb, this);
                            }

                            // Add the new file to recent files, unless it is
                            // an EDL.
                            if (!file::isTemporaryEDL(item->path) &&
                                !file::isTemporaryNDI(item->path))
                            {
                                const std::string& file = item->path.get();
                                p.settings->addRecentFile(file);
                            }
                        }
                        else
                        {
                            if (isRunning())
                            {
                                player->setSpeed(item->speed);
                                player->setLoop(item->loop);
                                player->setInOutRange(item->inOutRange);
                                player->setVolume(p.volume);
                                player->setMute(p.mute);
                                player->setAudioOffset(item->audioOffset);
                                player->setAllAnnotations(item->annotations);
                                player->setAllAnnotations(item->voiceAnnotations);
                                player->seek(item->currentTime);
                                player->setPlayback(item->playback);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                }
            }
        }

        if (player)
        {
            std::vector<std::shared_ptr<timeline::Timeline> > compare;
            for (size_t i = 1; i < activeFiles.size(); ++i)
            {
                auto j =
                    std::find(p.files.begin(), p.files.end(), activeFiles[i]);
                if (j != p.files.end())
                {
                    auto timeline = p.timelines[j - p.files.begin()];
                    auto info = timeline->getIOInfo();
                    compare.push_back(timeline);
                }
            }
            player->setCompare(compare);
            player->setCompareTime(p.filesModel->getCompareTime());
        }

        if (p.mainControl)
        {
            p.mainControl->setPlayer(player.get());

            auto view = ui->uiView;
            if (view->hasFrameView())
                view->frameView();

            if (ui->uiSecondary && ui->uiSecondary->viewport())
                view = ui->uiSecondary->viewport();
            if (view->hasFrameView())
                view->frameView();
        }

        p.activeFiles = activeFiles;
        p.player = player;
#if defined(TLRENDER_BMD) || defined(TLRENDER_NDI)
        if (p.outputDevice)
            p.outputDevice->setPlayer(p.player ? p.player->player() : nullptr);
#endif // TLRENDER_BMD

        DBG;
        
        _layersUpdate(p.filesModel->observeLayers()->get());

        if (ui)
        {
            if (player)
            {
                size_t numFiles = filesModel()->observeFiles()->getSize();
                if (numFiles == 1)
                {
                    if (!ui->uiView->getPresentationMode())
                    {
                        if (p.options.otioEditMode || ui->uiEdit->value() ||
                            ui->uiPrefs->uiPrefsEditMode->value())
                        {
                            // We need to call it explicitally to handle
                            // audio tracks that don't send a resizeWindow.
                            set_edit_mode_cb(EditMode::kFull, ui);
                        }
                    }
                }
                else
                {
                    if (!ui->uiView->getPresentationMode() &&
                        ui->uiEdit->value())
                        set_edit_mode_cb(EditMode::kFull, ui);
                }

                if (!activeFiles.empty())
                {
                    try
                    {
                        if (activeFiles[0]->ocioIcs.empty())
                        {
                            ocio::defaultIcs();
                        }
                        else
                        {
                            ocio::setIcs(activeFiles[0]->ocioIcs);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                    }

                    try
                    {
                        if (!activeFiles[0]->ocioLook.empty())
                            ocio::setLook(activeFiles[0]->ocioLook);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                    }

                    if (activeFiles[0]->lutOptions != p.lutOptions)
                        setLUTOptions(activeFiles[0]->lutOptions);
                }

                if (isRunning())
                {
                    panel::redrawThumbnails();
                }
            }
        }

        cacheUpdate();

        _audioUpdate();

    }

    otime::RationalTime App::_cacheReadAhead() const
    {
        TLRENDER_P();
        double value = p.settings->getValue<double>("Cache/ReadAhead");
        return otime::RationalTime(value, 1.0);
    }

    otime::RationalTime App::_cacheReadBehind() const
    {
        TLRENDER_P();
        double value = p.settings->getValue<double>("Cache/ReadBehind");
        return otime::RationalTime(value, 1.0);
    }

    void App::cacheUpdate()
    {
        TLRENDER_P();
        if (!p.player)
            return;

        auto context = App::app->getContext();
        auto audioSystem = context->getSystem<audio::System>();
        if (audioSystem)
        {
            PreferencesUI* uiPrefs = ui->uiPrefs;
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

        uint64_t Gbytes =
            static_cast<uint64_t>(p.settings->getValue<int>("Cache/GBytes"));

        timeline::PlayerCacheOptions options;
        options.readAhead = _cacheReadAhead();
        options.readBehind = _cacheReadBehind();

        const auto& info = p.player->ioInfo();

        bool movieIsLong = false;
        if (info.audio.trackCount > 1)
        {
            // If movie is longer than 30 minutes, and has multiple audio tracks
            // use a short readAhead/readBehind so we can quickly switch among
            // them.
            const auto& timeRange = p.player->inOutRange();
            if (timeRange.duration().to_seconds() > 60 * 30)
                movieIsLong = true;
        }

        if (file::isTemporaryNDI(p.player->path()) || movieIsLong)
        {
            options.readAhead = otime::RationalTime(4.0, 1.0);
            options.readBehind = otime::RationalTime(0.0, 1.0);
        }
        else if (Gbytes == 0)
        {
            Gbytes = 4;
        }

        if (Gbytes > 0)
        {
            // Do some sanity checking in case the user is using several mrv2
            // instances and cache would not fit.
            uint64_t totalVirtualMem = 0;
            uint64_t virtualMemUsed = 0;
            uint64_t virtualMemUsedByMe = 0;
            uint64_t totalPhysMem = 0;
            uint64_t physMemUsed = 0;
            uint64_t physMemUsedByMe = 0;
            memory_information(
                totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe);
            totalPhysMem /= 1024;
            if (totalPhysMem < Gbytes)
                Gbytes = totalPhysMem / 2;

            uint64_t bytes = Gbytes * memory::gigabyte;

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(bytes);

            // old readAhead/readBehind code used when playing sequences.
            const auto timeline = p.player->timeline();
            const auto ioInfo = timeline->getIOInfo();

            const auto path = p.player->path();
            const bool isSequence = file::isSequence(path.get());
            if (isSequence)
            {
                double seconds = 1.F;
                if (!ioInfo.video.empty())
                {
                    const auto& video = ioInfo.video[0];
                    std::size_t size = tl::image::getDataByteCount(video);
                    double frames = bytes / static_cast<double>(size);
                    seconds = frames / p.player->defaultSpeed();
                }

                if (ioInfo.audio.isValid())
                {
                    const auto& audio = ioInfo.audio;
                    std::size_t channelCount = audio.channelCount;
                    std::size_t byteCount = audio::getByteCount(audio.dataType);
                    std::size_t sampleRate = audio.sampleRate;
                    uint64_t size = sampleRate * byteCount * channelCount;
                    seconds -= size / 1024.0 / 1024.0;
                    // Sanity check just in case
                    if (seconds < 0.01F)
                        seconds = 0.01F;
                }

                double ahead = timeline::PlayerCacheOptions().readAhead.value();
                double behind =
                    timeline::PlayerCacheOptions().readBehind.value();

                const double totalTime = ahead + behind;
                const double readAheadPct = ahead / totalTime;
                const double readBehindPct = behind / totalTime;

                double readAhead = seconds * readAheadPct;
                double readBehind = seconds * readBehindPct;
                if (readBehind < behind)
                    readBehind = behind;

                options.readAhead = otime::RationalTime(readAhead, 1.0);
                options.readBehind = otime::RationalTime(readBehind, 1.0);
            }
        }

        p.player->setCacheOptions(options);
    }

    void App::_audioUpdate()
    {
        TLRENDER_P();

        if (p.player)
        {
            p.player->setVolume(p.volume);
            p.player->setMute(p.mute);
        }

#if defined(TLRENDER_NDI) || defined(TLRENDER_BMD)
        if (p.outputDevice)
        {
            p.outputDevice->setVolume(p.volume);
            p.outputDevice->setMute(p.mute);
        }
#endif
    }

    void App::_layersUpdate(const std::vector<int>& value)
    {
        TLRENDER_P();
        if (auto player = p.player)
        {
            int videoLayer = 0;
            std::vector<int> compareVideoLayers;
            if (!value.empty() && value.size() == p.files.size() &&
                !p.activeFiles.empty())
            {
                auto i = std::find(
                    p.files.begin(), p.files.end(), p.activeFiles.front());
                if (i != p.files.end())
                {
                    videoLayer = value[i - p.files.begin()];
                }
                for (size_t j = 1; j < p.activeFiles.size(); ++j)
                {
                    i = std::find(
                        p.files.begin(), p.files.end(), p.activeFiles[j]);
                    if (i != p.files.end())
                    {
                        compareVideoLayers.push_back(
                            value[i - p.files.begin()]);
                    }
                }
            }
            player->setVideoLayer(videoLayer);
            player->setCompareVideoLayers(compareVideoLayers);
        }
    }
} // namespace mrv
