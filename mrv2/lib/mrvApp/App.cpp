// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "App.h"

#include <fstream>
#include <sstream>

#ifdef MRV2_PYBIND11
#    include <pybind11/embed.h>
namespace py = pybind11;
#    include "mrvPy/Cmds.h"
#endif

#include <tlIO/System.h>
#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/AudioSystem.h>
#include <tlCore/StringFormat.h>

#include <tlTimeline/Util.h>

#include "mrvCore/mrvOS.h" // do not move up
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvSignalHandler.h"

#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvContextObject.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvGL/mrvGLViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvDummyClient.h"
#include "mrvNetwork/mrvDisplayOptions.h"
#include "mrvNetwork/mrvLUTOptions.h"

#ifdef MRV2_NETWORK
#    include "mrvNetwork/mrvCommandInterpreter.h"
#    include "mrvNetwork/mrvClient.h"
#    include "mrvNetwork/mrvImageListener.h"
#    include "mrvNetwork/mrvServer.h"
#    include "mrvNetwork/mrvParseHost.h"
#endif

#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvDevicesModel.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvMainControl.h"
#include "mrvApp/mrvOpenSeparateAudioDialog.h"
#include "mrvApp/mrvSettingsObject.h"

#include "mrvPreferencesUI.h"
#include "mrViewer.h"

#include <FL/platform.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>

#ifdef __linux__
#    undef None // macro defined in X11 config files
#endif

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "app";
}

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
        const float errorTimeout = 5.F;
    }

    struct Options
    {
        std::string dummy;
        std::vector<std::string> fileNames;
        std::string audioFileName;
        std::string compareFileName;
#ifdef MRV2_PYBIND11
        std::string pythonScript;
#endif

#ifdef MRV2_NETWORK
        bool server = false;
        std::string client;
        unsigned port = 55150;
#endif

        timeline::CompareMode compareMode = timeline::CompareMode::A;
        math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
        float wipeRotation = 0.F;
        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Forward;
        timeline::Loop loop = timeline::Loop::Count;
        otime::RationalTime seek = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;

        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;

        bool hud = true;
        bool resetSettings = false;
        bool resetHotkeys = false;
        bool displayVersion = false;
        bool otioEditMode = false;

#if defined(TLRENDER_USD)
        int usdRenderWidth = 1920;
        float usdComplexity = 1.F;
        usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
        bool usdEnableLighting = true;
        size_t usdStageCache = 10;
        size_t usdDiskCache = 0;
#endif // TLRENDER_USD
    };

    struct App::Private
    {
        Options options;

        ContextObject* contextObject = nullptr;
        std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
        SettingsObject* settingsObject = nullptr;
#ifdef MRV2_NETWORK
        CommandInterpreter* commandInterpreter = nullptr;
        ImageListener* imageListener = nullptr;
#endif

        std::shared_ptr<PlaylistsModel> playlistsModel;
        std::shared_ptr<FilesModel> filesModel;
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            activeObserver;
        std::vector<std::shared_ptr<FilesModelItem> > active;

        std::shared_ptr<observer::ListObserver<int> > layersObserver;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        float volume = 1.F;
        bool mute = false;
        OutputDevice* outputDevice = nullptr;
        bool deviceActive = false;
        std::shared_ptr<DevicesModel> devicesModel;
        std::shared_ptr<observer::ValueObserver<DevicesModelData> >
            devicesObserver;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

        std::vector<TimelinePlayer*> timelinePlayers;

        MainControl* mainControl = nullptr;

        bool session = false;

        bool running = false;
    };

    ViewerUI* App::ui = nullptr;
    App* App::app = nullptr;

    std::vector< std::string > OSXfiles;
    void osx_open_cb(const char* fname)
    {
        OSXfiles.push_back(fname);
    }

    namespace
    {
        inline void open_console()
        {
#ifdef _WIN32
            const char* term = fl_getenv("TERM");
            if (!term || strlen(term) == 0)
            {
                BOOL ok = AttachConsole(ATTACH_PARENT_PROCESS);
                if (ok)
                {
                    freopen("conout$", "w", stdout);
                    freopen("conout$", "w", stderr);
                }
            }
#endif
        }

    } // namespace

    App::App(
        int argc, char** argv,
        const std::shared_ptr<system::Context>& context) :
        IApp(),
        _p(new Private)
    {
        TLRENDER_P();

        // Establish MRV2_ROOT environment variable
        set_root_path(argc, argv);

#ifdef __linux__
        int ok = XInitThreads();
        if (!ok)
            throw std::runtime_error("XInitThreads failed");

        XSetErrorHandler(xerrorhandler);
#endif
        // Store the application object for further use down the line
        App::app = this;
        ViewerUI::app = this;

        open_console();

        const std::string& msg = setLanguageLocale();

        IApp::_init(
            app::convert(argc, argv), context, "mrv2",
            _("Play timelines, movies, and image sequences."),
            {app::CmdLineValueArg<std::string>::create(
                p.options.dummy, "inputs",
                _("Timelines, movies, image sequences, or folders."), true,
                true)},
            {
                app::CmdLineValueOption<int>::create(
                    Preferences::debug, {"-debug", "-d"},
                    _("Debug verbosity.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.audioFileName, {"-audio", "-a"},
                        _("Audio file name.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.compareFileName, {"-compare", "-b"},
                        _("A/B comparison \"B\" file name.")),
                    app::CmdLineValueOption<timeline::CompareMode>::create(
                        p.options.compareMode, {"-compareMode", "-c"},
                        _("A/B comparison mode."),
                        string::Format("{0}").arg(p.options.compareMode),
                        string::join(timeline::getCompareModeLabels(), ", ")),
                    app::CmdLineValueOption<math::Vector2f>::create(
                        p.options.wipeCenter, {"-wipeCenter", "-wc"},
                        _("A/B comparison wipe center."),
                        string::Format("{0}").arg(p.options.wipeCenter)),
                    app::CmdLineValueOption<float>::create(
                        p.options.wipeRotation, {"-wipeRotation", "-wr"},
                        _("A/B comparison wipe rotation."),
                        string::Format("{0}").arg(p.options.wipeRotation)),
                    app::CmdLineFlagOption::create(
                        p.options.otioEditMode, {"-editMode", "-e"},
                        _("OpenTimelineIO Edit mode.")),
                    app::CmdLineValueOption<double>::create(
                        p.options.speed, {"-speed"}, _("Playback speed.")),
                    app::CmdLineValueOption<timeline::Playback>::create(
                        p.options.playback, {"-playback", "-p"},
                        _("Playback mode."),
                        string::Format("{0}").arg(p.options.playback),
                        string::join(timeline::getPlaybackLabels(), ", ")),
                    app::CmdLineValueOption<timeline::Loop>::create(
                        p.options.loop, {"-loop"}, _("Playback loop mode."),
                        string::Format("{0}").arg(timeline::Loop::Loop),
                        string::join(timeline::getLoopLabels(), ", ")),
                    app::CmdLineValueOption<otime::RationalTime>::create(
                        p.options.seek, {"-seek"},
                        _("Seek to the given time.")),
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        p.options.inOutRange, {"-inOutRange"},
                        _("Set the in/out points range.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.fileName,
                        {"-colorConfig", "-cc"},
                        _("Color configuration file name (config.ocio).")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.input,
                        {"-colorInput", "-ci"}, _("Input color space.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.display,
                        {"-colorDisplay", "-cd"}, _("Display color space.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.view,
                        {"-colorView", "-cv"}, _("View color space.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.lutOptions.fileName, {"-lut"},
                        _("LUT file name.")),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        p.options.lutOptions.order, {"-lutOrder"},
                        _("LUT operation order."),
                        string::Format("{0}").arg(p.options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", ")),
#ifdef MRV2_PYBIND11
                    app::CmdLineValueOption<std::string>::create(
                        p.options.pythonScript, {"-pythonScript", "-ps"},
                        _("Python Script to run and exit.")),
#endif
                    app::CmdLineFlagOption::create(
                        p.options.resetSettings, {"-resetSettings"},
                        _("Reset settings to defaults.")),
                    app::CmdLineFlagOption::create(
                        p.options.resetHotkeys, {"-resetHotkeys"},
                        _("Reset hotkeys to defaults.")),
#if defined(TLRENDER_USD)
                    app::CmdLineValueOption<int>::create(
                        p.options.usdRenderWidth, {"-usdRenderWidth"},
                        "USD render width.",
                        string::Format("{0}").arg(p.options.usdRenderWidth)),
                    app::CmdLineValueOption<float>::create(
                        p.options.usdComplexity, {"-usdComplexity"},
                        "USD render complexity setting.",
                        string::Format("{0}").arg(p.options.usdComplexity)),
                    app::CmdLineValueOption<usd::DrawMode>::create(
                        p.options.usdDrawMode, {"-usdDrawMode"},
                        "USD render draw mode.",
                        string::Format("{0}").arg(p.options.usdDrawMode),
                        string::join(usd::getDrawModeLabels(), ", ")),
                    app::CmdLineValueOption<bool>::create(
                        p.options.usdEnableLighting, {"-usdEnableLighting"},
                        "USD render enable lighting setting.",
                        string::Format("{0}").arg(p.options.usdEnableLighting)),
                    app::CmdLineValueOption<size_t>::create(
                        p.options.usdStageCache, {"-usdStageCache"},
                        "USD stage cache size.",
                        string::Format("{0}").arg(p.options.usdStageCache)),
                    app::CmdLineValueOption<size_t>::create(
                        p.options.usdDiskCache, {"-usdDiskCache"},
                        "USD disk cache size in gigabytes. A size of zero "
                        "disables the cache.",
                        string::Format("{0}").arg(p.options.usdDiskCache)),
#endif // TLRENDER_USD

#ifdef MRV2_NETWORK
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
                        p.options.displayVersion,
                        {"-version", "--version", "-v", "--v"},
                        _("Return the version and exit."))
            });

        const int exitCode = getExit();
        if (exitCode != 0)
        {
            return;
        }

        file::Path lastPath;
        const auto& unusedArgs = getUnusedArgs();
        for (const auto& unused : unusedArgs)
        {
            file::Path path = file::Path(unused);
            if (path.getDirectory() == lastPath.getDirectory() &&
                path.getBaseName() == lastPath.getBaseName() &&
                path.getPadding() == lastPath.getPadding() &&
                path.getExtension() == lastPath.getExtension() &&
                !is_directory(unused))
                continue;
            lastPath = path;
            p.options.fileNames.push_back(unused);
        }

        if (p.options.displayVersion)
        {
            std::cout << std::endl
                      << "mrv2 v" << mrv::version() << " " << mrv::build_date()
                      << std::endl
                      << std::endl;
            return;
        }

#ifdef MRV2_PYBIND11
        if (!p.options.pythonScript.empty())
        {
            if (!is_readable(p.options.pythonScript))
            {
                std::cerr << std::string(
                                 string::Format(
                                     _("Could not read python script '{0}'"))
                                     .arg(p.options.pythonScript))
                          << std::endl;
                _exit = 1;
                return;
            }
            std::cout << std::string(
                             string::Format(_("Running python script '{0}'"))
                                 .arg(p.options.pythonScript))
                      << std::endl;
            std::ifstream is(p.options.pythonScript);
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
            return;
        }
#endif

        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
        Fl::use_high_res_GL(true);
        Fl::set_fonts("-*");

        // Create the window.
        ui = new ViewerUI();
        if (!ui)
        {
            throw std::runtime_error(_("Cannot create window"));
        }
        ui->uiMain->main(ui);

        p.settingsObject = new SettingsObject();

        // Classes used to handle network connections
#ifdef MRV2_NETWORK
        p.commandInterpreter = new CommandInterpreter(ui);
#endif
        tcp = new DummyClient();

        p.lutOptions = p.options.lutOptions;

#ifdef __APPLE__
        Fl_Mac_App_Menu::about = _("About mrv2");
        Fl_Mac_App_Menu::print = "";
        Fl_Mac_App_Menu::hide = _("Hide mrv2");
        Fl_Mac_App_Menu::hide_others = _("Hide Others");
        Fl_Mac_App_Menu::services = _("Services");
        Fl_Mac_App_Menu::show = _("Show All");
        Fl_Mac_App_Menu::quit = _("Quit mrv2");

        // For macOS, to read command-line arguments
        fl_open_callback(osx_open_cb);
        fl_open_display();
#endif
        ui->uiView->setContext(context);

        p.contextObject = new mrv::ContextObject(context);
        p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
        p.filesModel = FilesModel::create(context);
        p.playlistsModel = PlaylistsModel::create(context);

        ui->uiTimeline->setContext(context, p.timeUnitsModel, ui);
        ui->uiTimeline->setScrollBarsVisible(false);

        uiLogDisplay = new LogDisplay(0, 20, 340, 320);

        LOG_INFO(msg);

        std_any value;

        Preferences prefs(
            ui->uiPrefs, p.options.resetSettings, p.options.resetHotkeys);

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

        Preferences::run(ui);

#ifdef MRV2_PYBIND11
        // Import the mrv2 python module so we read all python
        // plug-ins.
        py::module::import("mrv2");

        // Discover python plugins
        mrv2_discover_python_plugins();
#endif

#if defined(TLRENDER_USD)
        p.settingsObject->setValue(
            "USD/renderWidth", static_cast<int>(p.options.usdRenderWidth));
        p.settingsObject->setValue(
            "USD/complexity", static_cast<float>(p.options.usdComplexity));
        p.settingsObject->setValue(
            "USD/drawMode", static_cast<int>(p.options.usdDrawMode));
        p.settingsObject->setValue(
            "USD/enableLighting",
            static_cast<int>(p.options.usdEnableLighting));
        p.settingsObject->setValue(
            "USD/stageCache", static_cast<int>(p.options.usdStageCache));
        p.settingsObject->setValue(
            "USD/diskCacheByte",
            static_cast<int>(p.options.usdDiskCache * memory::gigabyte));
#endif // TLRENDER_USD

        value = p.settingsObject->value("Audio/Volume");
        p.volume = std_any_cast<float>(value);

        value = p.settingsObject->value("Audio/Mute");
        p.mute = std_any_cast<bool>(value);

        if (p.options.loop != timeline::Loop::Count)
        {
            TimelineClass* c = ui->uiTimeWindow;
            c->uiLoopMode->value(static_cast<int>(p.options.loop));
            c->uiLoopMode->do_callback();
        }

        p.activeObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { _activeCallback(value); });

        p.layersObserver = observer::ListObserver<int>::create(
            p.filesModel->observeLayers(),
            [this](const std::vector<int>& value)
            {
                for (size_t i = 0;
                     i < value.size() && i < _p->timelinePlayers.size(); ++i)
                {
                    if (_p->timelinePlayers[i])
                    {
                        _p->timelinePlayers[i]->setVideoLayer(value[i]);
                    }
                }
            });

        // p.outputDevice = new OutputDevice(context);
        p.devicesModel = DevicesModel::create(context);
        value = p.settingsObject->value("Devices/DeviceIndex");
        p.devicesModel->setDeviceIndex(
            value.type() == typeid(void) ? 0 : std_any_cast<int>(value));
        value = p.settingsObject->value("Devices/DisplayModeIndex");
        p.devicesModel->setDisplayModeIndex(
            value.type() == typeid(void) ? 0 : std_any_cast<int>(value));
        value = p.settingsObject->value("Devices/PixelTypeIndex");
        p.devicesModel->setPixelTypeIndex(
            value.type() == typeid(void) ? 0 : std_any_cast<int>(value));
        p.settingsObject->setDefaultValue(
            "Devices/HDRMode", static_cast<int>(device::HDRMode::FromFile));
        p.devicesModel->setHDRMode(static_cast<device::HDRMode>(
            std_any_cast<int>(p.settingsObject->value("Devices/HDRMode"))));
        value = p.settingsObject->value("Devices/HDRData");
        std::string s = value.type() == typeid(void)
                            ? std::string()
                            : std_any_cast< std::string >(value);
        if (!s.empty())
        {
            auto json = nlohmann::json::parse(s);
            image::HDRData data;
            from_json(json, data);
            p.devicesModel->setHDRData(data);
        }

        p.logObserver = observer::ListObserver<log::Item>::create(
            ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
            {
                static std::string lastMessage;
                const char* kModule = "";
                for (const auto& i : value)
                {
                    switch (i.type)
                    {
                    case log::Type::Error:
                    {
                        const std::string& msg = i.message;
                        if (msg == lastMessage)
                            return;
                        lastMessage = msg;
                        ui->uiStatusBar->copy_label(msg.c_str());
                        LOG_ERROR(msg);
                        break;
                    }
                    default:
                        break;
                    }
                }
            });

#ifdef TLRENDER_BMD
        p.devicesObserver = observer::ValueObserver<DevicesModelData>::create(
            p.devicesModel->observeData(),
            [this](const DevicesModelData& value)
            {
                const device::PixelType pixelType =
                    value.pixelTypeIndex >= 0 &&
                            value.pixelTypeIndex < value.pixelTypes.size()
                        ? value.pixelTypes[value.pixelTypeIndex]
                        : device::PixelType::None;
                // @todo:
                _p->outputDevice->setDevice(
                    value.deviceIndex - 1, value.displayModeIndex - 1,
                    pixelType);
                _p->outputDevice->setDeviceEnabled(value.deviceEnabled);
                _p->outputDevice->setHDR(value.hdrMode, value.hdrData);
            });
#endif

        cacheUpdate();
        _audioUpdate();

        // Create the main control.
        p.mainControl = new MainControl(ui);

        // Open the input files.
        if (!p.options.fileNames.empty())
        {
            for (const auto& fileName : p.options.fileNames)
            {
                if (fileName.empty())
                    continue;
                open(fileName, p.options.audioFileName);
            }

            p.filesModel->setA(0);

            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                if (p.options.speed > 0.0)
                {
                    p.timelinePlayers[0]->setSpeed(p.options.speed);
                }
                if (time::isValid(p.options.inOutRange))
                {
                    p.timelinePlayers[0]->setInOutRange(p.options.inOutRange);
                    p.timelinePlayers[0]->seek(
                        p.options.inOutRange.start_time());
                }
                if (time::isValid(p.options.seek))
                {
                    p.timelinePlayers[0]->seek(p.options.seek);
                }
                if (p.options.loop != timeline::Loop::Count)
                    p.timelinePlayers[0]->setLoop(p.options.loop);
            }
        }

        if (!p.options.compareFileName.empty())
        {
            timeline::CompareOptions compareOptions;
            compareOptions.mode = p.options.compareMode;
            compareOptions.wipeCenter = p.options.wipeCenter;
            compareOptions.wipeRotation = p.options.wipeRotation;
            p.filesModel->setCompareOptions(compareOptions);
            open(p.options.compareFileName);

            size_t numFiles = p.filesModel->observeFiles()->getSize();
            p.filesModel->setB(numFiles - 1, true);
        }

        if (!p.options.fileNames.empty())
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

        ui->uiMain->show();
        ui->uiView->take_focus();

        if (!p.session)
            Preferences::open_windows();
        ui->uiMain->fill_menu(ui->uiMenuBar);

        if (ui->uiSecondary)
        {
            // We raise the secondary window last, so it shows at front
            ui->uiSecondary->window()->show();
        }
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

        //@todo:
        // delete p.outputDevice;

        if (p.settingsObject && p.devicesModel)
        {
            const auto& deviceData = p.devicesModel->observeData()->get();
            p.settingsObject->setValue(
                "Devices/DeviceIndex",
                static_cast<int>(deviceData.deviceIndex));
            p.settingsObject->setValue(
                "Devices/DisplayModeIndex",
                static_cast<int>(deviceData.displayModeIndex));
            p.settingsObject->setValue(
                "Devices/PixelTypeIndex",
                static_cast<int>(deviceData.pixelTypeIndex));
            p.settingsObject->setValue(
                "Devices/HDRMode", static_cast<int>(deviceData.hdrMode));
            nlohmann::json json;
            to_json(json, deviceData.hdrData);
            const std::string& data = json.dump();
            p.settingsObject->setValue("Devices/HDRData", data);
        }
    }

    const std::shared_ptr<timeline::TimeUnitsModel>& App::timeUnitsModel() const
    {
        return _p->timeUnitsModel;
    }

    SettingsObject* App::settingsObject() const
    {
        return _p->settingsObject;
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

    OutputDevice* App::outputDevice() const
    {
        return _p->outputDevice;
    }

    const std::shared_ptr<DevicesModel>& App::devicesModel() const
    {
        return _p->devicesModel;
    }

    struct PlaybackData
    {
        TimelineViewport* view;
        timeline::Playback playback;
    };

    static void start_playback(void* data)
    {
        PlaybackData* p = (PlaybackData*)data;
        auto view = p->view;
        switch (p->playback)
        {
        case timeline::Playback::Forward:
            view->playForwards();
            break;
        case timeline::Playback::Reverse:
            view->playBackwards();
            break;
        default:
            view->stop();
            break;
        }
        delete p;
    }

    void App::createListener()
    {
#ifdef MRV2_NETWORK
        TLRENDER_P();

        if (!p.imageListener)
            p.imageListener = new ImageListener(this);
#endif
    }

    void App::removeListener()
    {
#ifdef MRV2_NETWORK
        TLRENDER_P();

        delete p.imageListener;
        p.imageListener = nullptr;
#endif
    }

    int App::run()
    {
        TLRENDER_P();
        if (!ui)
            return 0;

        Fl::flush();
        bool autoPlayback = ui->uiPrefs->uiPrefsAutoPlayback->value();
        if (!p.timelinePlayers.empty() && p.timelinePlayers[0] &&
            p.options.playback != timeline::Playback::Count && autoPlayback &&
            !p.session)
        {
            // We use a timeout to start playback of the loaded video to
            // make sure to show all frames
            PlaybackData* data = new PlaybackData;
            data->view = ui->uiView;
            data->playback = p.options.playback;
            Fl::add_timeout(0.005, (Fl_Timeout_Handler)start_playback, data);
        }
        p.running = true;
        return Fl::run();
    }

    void
    App::open(const std::string& fileName, const std::string& audioFileName)
    {
        TLRENDER_P();

        if (fileName.size() > 6 &&
            fileName.substr(fileName.size() - 6, fileName.size()) == ".mrv2s")
        {
            p.session = true;
            load_session(fileName);
            return;
        }

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = std_any_cast<int>(
            p.settingsObject->value("Misc/MaxFileSequenceDigits"));
        file::Path filePath(fileName);
        for (const auto& path :
             timeline::getPaths(filePath, pathOptions, _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFileName);
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
        ui->uiMain->fill_menu(ui->uiMenuBar);
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
        p.settingsObject->setValue("Audio/Volume", value);
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
        p.settingsObject->setValue("Audio/Mute", value);
    }

    void App::_activeCallback(
        const std::vector<std::shared_ptr<FilesModelItem> >& items)
    {
        TLRENDER_P();

        // Flag used to determine if clip was just loaded or we just switched
        // from a compare or a file list change.
        bool loaded = false;

        if (!p.active.empty() && !p.timelinePlayers.empty() &&
            p.timelinePlayers[0])
        {
            p.active[0]->speed = p.timelinePlayers[0]->speed();
            p.active[0]->playback = p.timelinePlayers[0]->playback();
            p.active[0]->loop = p.timelinePlayers[0]->loop();
            p.active[0]->currentTime = p.timelinePlayers[0]->currentTime();
            p.active[0]->inOutRange = p.timelinePlayers[0]->inOutRange();
            p.active[0]->videoLayer = p.timelinePlayers[0]->videoLayer();
            p.active[0]->audioOffset = p.timelinePlayers[0]->audioOffset();
            p.active[0]->annotations =
                p.timelinePlayers[0]->getAllAnnotations();
        }

        std::vector<TimelinePlayer*> newTimelinePlayers;
        auto audioSystem = _context->getSystem<audio::System>();
        for (size_t i = 0; i < items.size(); ++i)
        {
            const auto& item = items[i];
            TimelinePlayer* mrvTimelinePlayer = nullptr;

            try
            {
                timeline::Options options;

                int value = std_any_cast<int>(
                    p.settingsObject->value("FileSequence/Audio"));

                options.fileSequenceAudio = (timeline::FileSequenceAudio)value;

                std_any v =
                    p.settingsObject->value("FileSequence/AudioFileName");
                options.fileSequenceAudioFileName =
                    std_any_cast<std::string>(v);

                options.fileSequenceAudioDirectory = std_any_cast<std::string>(
                    p.settingsObject->value("FileSequence/AudioDirectory"));
                options.videoRequestCount = std_any_cast<int>(
                    p.settingsObject->value("Performance/VideoRequestCount"));

                options.audioRequestCount = std_any_cast<int>(
                    p.settingsObject->value("Performance/AudioRequestCount"));

                options.ioOptions["SequenceIO/ThreadCount"] =
                    string::Format("{0}").arg(
                        std_any_cast<int>(p.settingsObject->value(
                            "Performance/SequenceThreadCount")));

                options.ioOptions["FFmpeg/YUVToRGBConversion"] =
                    string::Format("{0}").arg(
                        std_any_cast< int>(p.settingsObject->value(
                            "Performance/FFmpegYUVToRGBConversion")));

                const audio::Info audioInfo =
                    audioSystem->getDefaultOutputInfo();
                options.ioOptions["FFmpeg/AudioChannelCount"] =
                    string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["FFmpeg/AudioDataType"] =
                    string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["FFmpeg/AudioSampleRate"] =
                    string::Format("{0}").arg(audioInfo.sampleRate);

                options.ioOptions["FFmpeg/ThreadCount"] =
                    string::Format("{0}").arg(
                        std_any_cast<int>(p.settingsObject->value(
                            "Performance/FFmpegThreadCount")));

                options.ioOptions["SequenceIO/DefaultSpeed"] =
                    string::Format("{0}").arg(ui->uiPrefs->uiPrefsFPS->value());

#if defined(TLRENDER_USD)
                options.ioOptions["USD/renderWidth"] =
                    string::Format("{0}").arg(std_any_cast<int>(
                        p.settingsObject->value("USD/renderWidth")));
                float complexity = std_any_cast<float>(
                    p.settingsObject->value("USD/complexity"));
                options.ioOptions["USD/complexity"] =
                    string::Format("{0}").arg(complexity);
                {
                    std::stringstream ss;
                    usd::DrawMode usdDrawMode =
                        static_cast<usd::DrawMode>(std_any_cast<int>(
                            p.settingsObject->value("USD/drawMode")));
                    ss << usdDrawMode;
                    options.ioOptions["USD/drawMode"] = ss.str();
                }
                options.ioOptions["USD/enableLighting"] =
                    string::Format("{0}").arg(std_any_cast<int>(
                        p.settingsObject->value("USD/enableLighting")));
                options.ioOptions["USD/stageCacheCount"] =
                    string::Format("{0}").arg(std_any_cast<int>(
                        p.settingsObject->value("USD/stageCacheCount")));
                options.ioOptions["USD/diskCacheByteCount"] =
                    string::Format("{0}").arg(std_any_cast<int>(
                        p.settingsObject->value("USD/diskCacheByteCount")));

                auto ioSystem = _context->getSystem<io::System>();
                ioSystem->setOptions(options.ioOptions);
#endif

                options.pathOptions.maxNumberDigits = std::min(
                    std_any_cast<int>(
                        p.settingsObject->value("Misc/MaxFileSequenceDigits")),
                    255);

                auto otioTimeline =
                    item->audioPath.isEmpty()
                        ? timeline::create(item->path, _context, options)
                        : timeline::create(
                              item->path, item->audioPath, _context, options);

                auto timeline =
                    timeline::Timeline::create(otioTimeline, _context, options);

                timeline::PlayerOptions playerOptions;
                playerOptions.cache.readAhead = _cacheReadAhead();
                playerOptions.cache.readBehind = _cacheReadBehind();

                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/TimerMode"));
                playerOptions.timerMode = (timeline::TimerMode)value;
                value = std_any_cast<int>(p.settingsObject->value(
                    "Performance/AudioBufferFrameCount"));
                playerOptions.audioBufferFrameCount = value;
                if (item->init)
                {
                    playerOptions.currentTime = items[0]->currentTime;
                }

                auto timelinePlayer =
                    timeline::Player::create(timeline, _context, playerOptions);

                mrvTimelinePlayer =
                    new mrv::TimelinePlayer(timelinePlayer, _context);

                item->timeRange = mrvTimelinePlayer->timeRange();
                item->ioInfo = mrvTimelinePlayer->ioInfo();
                if (!item->init)
                {
                    loaded = true;
                    item->init = true;
                    item->speed = mrvTimelinePlayer->speed();
                    item->playback = mrvTimelinePlayer->playback();
                    item->loop = mrvTimelinePlayer->loop();
                    item->currentTime = mrvTimelinePlayer->currentTime();
                    item->inOutRange = mrvTimelinePlayer->inOutRange();
                    item->videoLayer = mrvTimelinePlayer->videoLayer();
                    item->audioOffset = mrvTimelinePlayer->audioOffset();

                    // Add the new file to recent files, unless it is an EDL.
                    if (!isTemporaryEDL(item->path))
                    {
                        const std::string& file = item->path.get();
                        p.settingsObject->addRecentFile(file);
                    }
                }
                else if (0 == i)
                {
                    mrvTimelinePlayer->setSpeed(items[0]->speed);
                    mrvTimelinePlayer->setLoop(items[0]->loop);
                    mrvTimelinePlayer->setInOutRange(items[0]->inOutRange);
                    mrvTimelinePlayer->setVideoLayer(items[0]->videoLayer);
                    mrvTimelinePlayer->setVolume(p.volume);
                    mrvTimelinePlayer->setMute(p.mute);
                    mrvTimelinePlayer->setAudioOffset(items[0]->audioOffset);
                    mrvTimelinePlayer->setAllAnnotations(items[0]->annotations);
                    mrvTimelinePlayer->setPlayback(items[0]->playback);
                }
                if (i > 0)
                {
                    mrvTimelinePlayer->setVideoLayer(items[i]->videoLayer);
                    mrvTimelinePlayer->setAllAnnotations(items[i]->annotations);
                    mrvTimelinePlayer->player()->setExternalTime(
                        newTimelinePlayers[0]->player());
                }
            }
            catch (const std::exception& e)
            {
                _log(e.what(), log::Type::Error);
            }
            newTimelinePlayers.push_back(mrvTimelinePlayer);
        }

        std::vector<mrv::TimelinePlayer*> validTimelinePlayers;
        for (const auto& i : newTimelinePlayers)
        {
            if (i)
            {
                validTimelinePlayers.push_back(i);
            }
        }

        if (p.mainControl)
        {
            p.mainControl->setTimelinePlayers(validTimelinePlayers);
        }

        // Delete the previous timeline players.
        for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
        {
            delete p.timelinePlayers[i];
        }

        p.active = items;
        p.timelinePlayers = newTimelinePlayers;

        if (ui)
        {

            if (!validTimelinePlayers.empty())
            {
                auto player = validTimelinePlayers[0];

                ui->uiMain->show();

                size_t numFiles = filesModel()->observeFiles()->getSize();
                if (numFiles == 1)
                {
                    // resize the window to the size of the first clip loaded
                    if (ui->uiView->getPresentationMode())
                    {
                        ui->uiView->frameView();
                    }
                    else
                    {
                        if (p.options.otioEditMode || ui->uiEdit->value() ||
                            ui->uiPrefs->uiPrefsEditMode->value())
                            editMode = EditMode::kFull;
                        ui->uiView->resizeWindow();
                        if (p.options.otioEditMode || ui->uiEdit->value() ||
                            ui->uiPrefs->uiPrefsEditMode->value())
                            set_edit_mode_cb(EditMode::kFull, ui);
                    }
                    ui->uiView->take_focus();
                }

                Preferences::updateICS();

                if (p.running)
                {
                    if (loaded)
                        refreshPanelThumbnails();
                    else
                        redrawPanelThumbnails();
                    if (ui->uiPrefs->uiPrefsAutoPlayback->value() && loaded)
                    {
                        auto player = ui->uiView->getTimelinePlayer();
                        if (player &&
                            player->timeRange().duration().value() > 1.0)
                            ui->uiView->playForwards();
                    }
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                }
            }
        }

        cacheUpdate();
        _audioUpdate();
    }

    otime::RationalTime App::_cacheReadAhead() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value =
            std_any_cast<double>(p.settingsObject->value("Cache/ReadAhead"));
        return otime::RationalTime(
            value / static_cast<double>(activeCount), 1.0);
    }

    otime::RationalTime App::_cacheReadBehind() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value =
            std_any_cast<double>(p.settingsObject->value("Cache/ReadBehind"));
        return otime::RationalTime(
            value / static_cast<double>(activeCount), 1.0);
    }

    void App::cacheUpdate()
    {
        TLRENDER_P();
        if (p.timelinePlayers.empty())
            return;

        timeline::PlayerCacheOptions options;
        options.readAhead = _cacheReadAhead();
        options.readBehind = _cacheReadBehind();

        std_any any = p.settingsObject->value("Cache/GBytes");
        uint64_t Gbytes = static_cast<uint64_t>(
            std_any_empty(any) ? 0 : std_any_cast<int>(any));
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

            const size_t activeCount = p.filesModel->observeActive()->getSize();

            uint64_t bytes = Gbytes * 1024 * 1024 * 1024;
            TimelinePlayer* player = nullptr;
            for (const auto& i : p.timelinePlayers)
            {
                if (i)
                {
                    player = i;
                    break;
                }
            }
            if (!player)
                return;

            const auto timeline = player->timeline();
            const auto ioInfo = timeline->getIOInfo();
            double seconds = 1.F;
            if (!ioInfo.video.empty())
            {
                const auto& video = ioInfo.video[0];
                auto pixelType = video.pixelType;
                std::size_t size = image::getDataByteCount(video);
                double frames = bytes / static_cast<double>(size);
                seconds = frames / player->defaultSpeed();
            }
            if (ioInfo.audio.isValid())
            {
                const auto& audio = ioInfo.audio;
                std::size_t channelCount = audio.channelCount;
                std::size_t byteCount = audio::getByteCount(audio.dataType);
                std::size_t sampleRate = audio.sampleRate;
                uint64_t size = sampleRate * byteCount * channelCount;
                seconds -= size / 1024.0 / 1024.0;
            }

            constexpr double defaultReadAhead =
                timeline::PlayerCacheOptions().readAhead.value();
            constexpr double defaultReadBehind =
                timeline::PlayerCacheOptions().readBehind.value();
            constexpr double totalTime = defaultReadAhead + defaultReadBehind;
            double readAhead = defaultReadAhead / totalTime;
            double readBehind = defaultReadBehind / totalTime;

            readAhead *= seconds / static_cast<double>(activeCount);
            readBehind *= seconds / static_cast<double>(activeCount);

            options.readAhead = otime::RationalTime(readAhead, 1.0);
            options.readBehind = otime::RationalTime(readBehind, 1.0);
        }

        for (const auto& i : p.timelinePlayers)
        {
            if (i)
            {
                i->setCacheOptions(options);
            }
        }
    }

    void App::_audioUpdate()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            if (i)
            {
                i->setVolume(p.volume);
                i->setMute(p.mute || p.deviceActive);
            }
        }
#ifdef TLRENDER_BMD
        if (p.outputDevice)
        {
            // @todo:
            //
            p.outputDevice->setVolume(p.volume);
            p.outputDevice->setMute(p.mute);
        }
#endif
    }
} // namespace mrv
