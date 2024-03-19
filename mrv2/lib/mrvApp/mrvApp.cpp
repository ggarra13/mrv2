// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <fstream>
#include <sstream>

#include <tlIO/System.h>
#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/StringFormat.h>

#include <tlTimeline/Util.h>

#ifdef MRV2_PYBIND11
#    include <pybind11/embed.h>
namespace py = pybind11;
#    include "mrvPy/Cmds.h"
#    include "mrvPy/PyStdErrOutRedirect.h"
#endif

#include "mrvCore/mrvOS.h" // do not move up
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvSignalHandler.h"

#include "mrvFl/mrvContextObject.h"
#include "mrvFl/mrvLanguages.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvTimelinePlayer.h"

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

#include <FL/platform.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>

#ifdef __linux__
#    undef None // macro defined in X11 config files
#    undef Status
#endif

#include "mrvFl/mrvIO.h"

#ifdef TLRENDER_NDI
#    include <Processing.NDI.Lib.h>
#endif

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

        timeline::CompareOptions compareOptions;

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
        int usdRenderWidth = 1920;
        float usdComplexity = 1.F;
        usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
        bool usdEnableLighting = true;
        bool usdSRGB = true;
        size_t usdStageCache = 10;
        size_t usdDiskCache = 0;
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
        ImageListener* imageListener = nullptr;
#endif

#ifdef MRV2_PYBIND11
        std::unique_ptr<PyStdErrOutStreamRedirect> pythonStdErrOutRedirect;
#endif

        std::shared_ptr<PlaylistsModel> playlistsModel;
        std::shared_ptr<FilesModel> filesModel;
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            activeObserver;
        std::vector<std::shared_ptr<FilesModelItem> > files;
        std::vector<std::shared_ptr<FilesModelItem> > activeFiles;
        std::vector<std::shared_ptr<timeline::Timeline> > timelines;

        std::shared_ptr<observer::ListObserver<int> > layersObserver;
        std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
            compareTimeObserver;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        float volume = 1.F;
        bool mute = false;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

        std::shared_ptr<TimelinePlayer> player;

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
        BaseApp(),
        _p(new Private)
    {
        TLRENDER_P();

        // Establish MRV2_ROOT environment variable
        set_root_path(argc, argv);

#ifdef __linux__

#    ifdef FLTK_USE_X11
        if (!fl_wl_display())
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
                        p.options.compareOptions.mode, {"-compareMode", "-c"},
                        _("A/B comparison mode."),
                        string::Format("{0}").arg(
                            p.options.compareOptions.mode),
                        string::join(timeline::getCompareModeLabels(), ", ")),
                    app::CmdLineValueOption<math::Vector2f>::create(
                        p.options.compareOptions.wipeCenter,
                        {"-wipeCenter", "-wc"},
                        _("A/B comparison wipe center."),
                        string::Format("{0}").arg(
                            p.options.compareOptions.wipeCenter)),
                    app::CmdLineValueOption<float>::create(
                        p.options.compareOptions.wipeRotation,
                        {"-wipeRotation", "-wr"},
                        _("A/B comparison wipe rotation."),
                        string::Format("{0}").arg(
                            p.options.compareOptions.wipeRotation)),
                    app::CmdLineFlagOption::create(
                        p.options.otioEditMode, {"-editMode", "-e"},
                        _("OpenTimelineIO Edit mode.")),
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
                        _("Seek to the given time.")),
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        p.options.inOutRange, {"-inOutRange"},
                        _("Set the in/out points range.")),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.ocioOptions.input,
                        {"-ocioInput", "-ics", "-oi"},
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
                    app::CmdLineValueOption<bool>::create(
                        p.options.usdEnableLighting, {"-usdSRGB"},
                        "USD render SRGB setting.",
                        string::Format("{0}").arg(p.options.usdSRGB)),
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
                !file::isDirectory(unused))
                continue;
            lastPath = path;
            p.options.fileNames.push_back(unused);
        }

        if (p.options.displayVersion)
        {
            std::cout << std::endl
                      << "mrv2 v" << mrv::version() << " " << mrv::build_date()
                      << std::endl
                      << mrv::get_os_version() << std::endl;
            return;
        }

#ifdef MRV2_PYBIND11
        if (!p.options.pythonScript.empty())
        {
            if (!file::isReadable(p.options.pythonScript))
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

        p.settings = new SettingsObject();

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

        std::string version = "mrv2 v";
        version += mrv::version();
        version += " ";
        version += mrv::build_date();
        LOG_INFO(version);

        version = mrv::get_os_version();
        LOG_INFO(version);

        LOG_INFO(msg);

        // Create the main control.
        p.mainControl = new MainControl(ui);

        store_default_hotkeys();

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

        // Redirect stdout/stderr to my own class
        p.pythonStdErrOutRedirect.reset(new PyStdErrOutStreamRedirect);

        // Discover python plugins
        mrv2_discover_python_plugins();
#endif

#if defined(TLRENDER_USD)
        p.settings->setValue(
            "USD/renderWidth", static_cast<int>(p.options.usdRenderWidth));
        p.settings->setValue(
            "USD/complexity", static_cast<float>(p.options.usdComplexity));
        p.settings->setValue(
            "USD/drawMode", static_cast<int>(p.options.usdDrawMode));
        p.settings->setValue(
            "USD/enableLighting",
            static_cast<int>(p.options.usdEnableLighting));
        p.settings->setValue("USD/sRGB", static_cast<int>(p.options.usdSRGB));
        p.settings->setValue(
            "USD/stageCacheByteCount",
            static_cast<int>(p.options.usdStageCache));
        p.settings->setValue(
            "USD/diskCacheByteCount",
            static_cast<int>(p.options.usdDiskCache * memory::gigabyte));
#endif // TLRENDER_USD

#ifdef TLRENDER_NDI
        if (!NDIlib_initialize())
            throw std::runtime_error(_("Could not initialize NDI"));
#endif

        p.volume = p.settings->getValue<float>("Audio/Volume");
        p.mute = p.settings->getValue<bool>("Audio/Mute");

        if (p.options.loop != timeline::Loop::Count)
        {
            TimelineClass* c = ui->uiTimeWindow;
            c->uiLoopMode->value(static_cast<int>(p.options.loop));
            c->uiLoopMode->do_callback();
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

        p.logObserver = observer::ListObserver<log::Item>::create(
            ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
            {
                static std::string lastMessage;
                for (const auto& i : value)
                {
                    const std::string& msg = i.message;
                    const std::string& kModule = i.module;
                    if (msg == lastMessage)
                        continue;
                    lastMessage = msg;
                    switch (i.type)
                    {
                    case log::Type::Error:
                    {
                        LOG_ERROR(msg);
                        break;
                    }
                    case log::Type::Warning:
                    {
                        LOG_WARNING(msg);
                        break;
                    }
                    case log::Type::Status:
                    {
                        LOG_INFO(msg);
                        break;
                    }
                    default:
                        break;
                    }
                }
            });

        // Open the input files.
        if (!p.options.fileNames.empty())
        {
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
                    player->seek(p.options.seek);
                }
                if (p.options.loop != timeline::Loop::Count)
                    player->setLoop(p.options.loop);
            }
        }

        if (!p.options.compareFileName.empty())
        {
            open(p.options.compareFileName);
            p.filesModel->setCompareOptions(p.options.compareOptions);
            size_t numFiles = p.filesModel->observeFiles()->getSize();
            p.filesModel->setB(numFiles - 1, true);
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

        try
        {
            if (!p.options.ocioOptions.input.empty())
                ocio::setOcioIcs(p.options.ocioOptions.input);

            if (!p.options.ocioOptions.look.empty())
                ocio::setOcioLook(p.options.ocioOptions.look);

            if (!p.options.ocioOptions.display.empty() &&
                !p.options.ocioOptions.view.empty())
            {
                const std::string& merged = ocio::ocioDisplayViewShortened(
                    p.options.ocioOptions.display, p.options.ocioOptions.view);
                ocio::setOcioView(merged);
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
        }
    }

    void App::cleanResources()
    {
        TLRENDER_P();

#ifdef TLRENDER_NDI
        // Not required, but nice
        NDIlib_destroy();
#endif

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
        if (p.player &&
            (p.options.playback != timeline::Playback::Count || autoPlayback) &&
            !p.session)
        {
            // We use a timeout to start playback of the loaded video to
            // make sure to show all frames
            PlaybackData* data = new PlaybackData;
            data->view = ui->uiView;
            data->playback = p.options.playback == timeline::Playback::Count
                                 ? timeline::Playback::Forward
                                 : p.options.playback;
            Fl::add_timeout(0.005, (Fl_Timeout_Handler)start_playback, data);
        }
        p.running = true;
        return Fl::run();
    }

    bool App::isRunning() const
    {
        TLRENDER_P();
        return p.running;
    }

    void
    App::open(const std::string& fileName, const std::string& audioFileName)
    {
        TLRENDER_P();

        file::Path filePath(fileName);

        if (filePath.getExtension() == ".mrv2s")
        {
            p.session = true;
            session::load(fileName);
            return;
        }

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits =
            p.settings->getValue<int>("Misc/MaxFileSequenceDigits");

        if (!file::isReadable(fileName))
        {
            std::string err =
                string::Format(_("Filename '{0}' does not exist or does not "
                                 "have read permissions."))
                    .arg(fileName);
            LOG_ERROR(err);
            return;
        }

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
#endif

#if defined(TLRENDER_FFMPEG)
        out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").arg(
            p.settings->getValue<int>("Performance/FFmpegYUVToRGBConversion"));
        out["FFmpeg/ThreadCount"] = string::Format("{0}").arg(
            p.settings->getValue<int>("Performance/FFmpegThreadCount"));
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        out["USD/renderWidth"] = string::Format("{0}").arg(
            p.settings->getValue<int>("USD/renderWidth"));
        float complexity = p.settings->getValue<float>("USD/complexity");
        out["USD/complexity"] = string::Format("{0}").arg(complexity);
        {
            std::stringstream ss;
            usd::DrawMode usdDrawMode = static_cast<usd::DrawMode>(
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
                    for (const auto& video : timelines[i]->getIOInfo().video)
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

        auto otioTimeline =
            item->audioPath.isEmpty()
                ? timeline::create(item->path, _context, options)
                : timeline::create(
                      item->path, item->audioPath, _context, options);

        return timeline::Timeline::create(otioTimeline, _context, options);
    }

    void App::_activeUpdate(
        const std::vector<std::shared_ptr<FilesModelItem> >& activeFiles)
    {
        TLRENDER_P();

        std::shared_ptr<TimelinePlayer> player;
        if (!p.activeFiles.empty() && p.player)
        {
            p.activeFiles[0]->speed = p.player->speed();
            p.activeFiles[0]->playback = p.player->playback();
            p.activeFiles[0]->loop = p.player->loop();
            p.activeFiles[0]->currentTime = p.player->currentTime();
            p.activeFiles[0]->inOutRange = p.player->inOutRange();
            p.activeFiles[0]->audioOffset = p.player->audioOffset();
            p.activeFiles[0]->annotations = p.player->getAllAnnotations();
            p.activeFiles[0]->ocioIcs = ocio::ocioIcs();
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
                            p.timelines[i - p.files.begin()] =
                                _createTimeline(item);
                        }

                        auto timeline = p.timelines[idx];
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

                            bool autoPlayback =
                                ui->uiPrefs->uiPrefsAutoPlayback->value();
                            if (autoPlayback)
                            {
                                player->setPlayback(
                                    timeline::Playback::Forward);
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
                            player->setSpeed(item->speed);
                            player->setLoop(item->loop);
                            player->setInOutRange(item->inOutRange);
                            player->setVolume(p.volume);
                            player->setMute(p.mute);
                            player->setAudioOffset(item->audioOffset);
                            player->setAllAnnotations(item->annotations);
                            player->seek(item->currentTime);
                            player->setPlayback(item->playback);
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

        _layersUpdate(p.filesModel->observeLayers()->get());

        if (ui)
        {
            if (player)
            {
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
                    ui->uiView->togglePixelBar();
                    ui->uiView->take_focus();
                }
                else
                {
                    if (!ui->uiView->getPresentationMode() &&
                        ui->uiEdit->value())
                        set_edit_mode_cb(EditMode::kFull, ui);
                }

                if (!activeFiles.empty() && activeFiles[0]->ocioIcs.empty())
                    Preferences::updateICS();
                else
                {
                    try
                    {
                        ocio::setOcioIcs(p.activeFiles[0]->ocioIcs);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                    }
                }
                if (p.running)
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
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value = p.settings->getValue<double>("Cache/ReadAhead");
        return otime::RationalTime(
            value / static_cast<double>(activeCount), 1.0);
    }

    otime::RationalTime App::_cacheReadBehind() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value = p.settings->getValue<double>("Cache/ReadBehind");
        return otime::RationalTime(
            value / static_cast<double>(activeCount), 1.0);
    }

    void App::cacheUpdate()
    {
        TLRENDER_P();
        if (!p.player)
            return;

        timeline::PlayerCacheOptions options;
        options.readAhead = _cacheReadAhead();
        options.readBehind = _cacheReadBehind();

        uint64_t Gbytes =
            static_cast<uint64_t>(p.settings->getValue<int>("Cache/GBytes"));
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

            // Check if an NDI movie and set cache to 1 gigabyte
            auto Aitem = p.filesModel->observeA()->get();
            if (Aitem && file::isTemporaryNDI(Aitem->path))
            {
                uint64_t NDIGbytes = static_cast<uint64_t>(
                    p.settings->getValue<int>("NDI/GBytes"));
                bytes = NDIGbytes * memory::gigabyte;
            }

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(bytes);

            const auto timeline = p.player->timeline();
            const auto ioInfo = timeline->getIOInfo();
            double seconds = 1.F;
            if (!ioInfo.video.empty())
            {
                const auto& video = ioInfo.video[0];
                auto pixelType = video.pixelType;
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
            }

            double ahead = timeline::PlayerCacheOptions().readAhead.value();
            double behind = timeline::PlayerCacheOptions().readBehind.value();

            const double totalTime = ahead + behind;
            const double readAheadPct = ahead / totalTime;
            const double readBehindPct = behind / totalTime;

            const double readAhead = seconds * readAheadPct;
            const double readBehind = seconds * readBehindPct;

            options.readAhead = otime::RationalTime(readAhead, 1.0);
            options.readBehind = otime::RationalTime(readBehind, 1.0);
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
