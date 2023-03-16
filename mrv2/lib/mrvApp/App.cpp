// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlTimeline/Util.h>

#include "mrvCore/mrvOS.h" // do not move up
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvSignalHandler.h"

#include "mrvFl/mrvTimelineCreate.h"
#include "mrvFl/mrvTimeObject.h"
#include "mrvFl/mrvContextObject.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvLanguages.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvGL/mrvGLViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvDevicesModel.h"
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
        std::string fileName[3];
        std::string audioFileName;
        std::string compareFileName;

        timeline::CompareMode compareMode = timeline::CompareMode::A;
        math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
        float wipeRotation = 0.F;
        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Forward;
        timeline::Loop loop = timeline::Loop::Loop;
        otime::RationalTime seek = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;
        bool fullScreen = false;
        bool hud = true;

        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
        bool resetSettings = false;
        bool displayVersion = false;
    };

    struct App::Private
    {
        Options options;

        ContextObject* contextObject = nullptr;
        TimeObject* timeObject = nullptr;
        SettingsObject* settingsObject = nullptr;

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

        ViewerUI* ui = nullptr;

        std::vector<TimelinePlayer*> timelinePlayers;

        MainControl* mainControl = nullptr;

        bool running = false;
    };

    namespace
    {
        App* _application = nullptr;
    }

    App* App::application()
    {
        return _application;
    }

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

        // Establish MRV_ROOT environment variable
        set_root_path(argc, argv);

#ifdef __linux__
        int ok = XInitThreads();
        if (!ok)
            throw std::runtime_error("XInitThreads failed");

        XSetErrorHandler(xerrorhandler);
#endif
        _application = this;
        // Store the application object for further use down the line
        ViewerUI::app = this;

        open_console();

        const std::string& msg = setLanguageLocale();

        IApp::_init(
            argc, argv, context, "mrv2",
            _("Play timelines, movies, and image sequences."),
            {app::CmdLineValueArg<std::string>::create(
                 p.options.fileName[0], "input",
                 _("Timeline, movie, image sequence, or folder."), true),
             app::CmdLineValueArg<std::string>::create(
                 p.options.fileName[1], "second",
                 _("Second tmeline, movie, image sequence, or folder."), true),
             app::CmdLineValueArg<std::string>::create(
                 p.options.fileName[2], "third",
                 _("Third timeline, movie, image sequence, or folder."), true)},
            {app::CmdLineValueOption<int>::create(
                 Preferences::debug, {"-debug", "-d"}, _("Debug verbosity.")),
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
             app::CmdLineValueOption<double>::create(
                 p.options.speed, {"-speed"}, _("Playback speed.")),
             app::CmdLineValueOption<timeline::Playback>::create(
                 p.options.playback, {"-playback", "-p"}, _("Playback mode."),
                 string::Format("{0}").arg(p.options.playback),
                 string::join(timeline::getPlaybackLabels(), ", ")),
             app::CmdLineValueOption<timeline::Loop>::create(
                 p.options.loop, {"-loop"}, _("Playback loop mode."),
                 string::Format("{0}").arg(p.options.loop),
                 string::join(timeline::getLoopLabels(), ", ")),
             app::CmdLineValueOption<otime::RationalTime>::create(
                 p.options.seek, {"-seek"}, _("Seek to the given time.")),
             app::CmdLineValueOption<otime::TimeRange>::create(
                 p.options.inOutRange, {"-inOutRange"},
                 _("Set the in/out points range.")),
             app::CmdLineValueOption<std::string>::create(
                 p.options.colorConfigOptions.fileName, {"-colorConfig", "-cc"},
                 _("Color configuration file name (config.ocio).")),
             app::CmdLineValueOption<std::string>::create(
                 p.options.colorConfigOptions.input, {"-colorInput", "-ci"},
                 _("Input color space.")),
             app::CmdLineValueOption<std::string>::create(
                 p.options.colorConfigOptions.display, {"-colorDisplay", "-cd"},
                 _("Display color space.")),
             app::CmdLineValueOption<std::string>::create(
                 p.options.colorConfigOptions.view, {"-colorView", "-cv"},
                 _("View color space.")),
             app::CmdLineValueOption<std::string>::create(
                 p.options.lutOptions.fileName, {"-lut"}, _("LUT file name.")),
             app::CmdLineValueOption<timeline::LUTOrder>::create(
                 p.options.lutOptions.order, {"-lutOrder"},
                 _("LUT operation order."),
                 string::Format("{0}").arg(p.options.lutOptions.order),
                 string::join(timeline::getLUTOrderLabels(), ", ")),
             app::CmdLineFlagOption::create(
                 p.options.resetSettings, {"-resetSettings"},
                 _("Reset settings to defaults.")),
             app::CmdLineFlagOption::create(
                 p.options.displayVersion,
                 {"-version", "--version", "-v", "--v"},
                 _("Return the version and exit."))});

        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        if (p.options.displayVersion)
        {
            std::cout << std::endl
                      << "mrv2 v" << mrv::version() << std::endl
                      << std::endl;
            exit(0);
        }

        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
        Fl::use_high_res_GL(true);
        Fl::set_fonts("-*");

        // Create the window.
        p.ui = new ViewerUI();
        if (!p.ui)
        {
            throw std::runtime_error(_("Cannot create window"));
        }
        Preferences::ui = p.ui;
        p.ui->uiMain->main(p.ui);

        p.timeObject = new mrv::TimeObject(p.ui);
        p.settingsObject = new SettingsObject(p.timeObject);

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

        p.ui->uiView->setContext(_context);
        p.ui->uiTimeWindow->uiTimeline->setContext(_context);

        p.contextObject = new mrv::ContextObject(context);
        p.filesModel = FilesModel::create(context);

        uiLogDisplay = new LogDisplay(0, 20, 340, 320);

        LOG_INFO(msg);

        Preferences prefs(p.ui->uiPrefs, p.options.resetSettings);
        Preferences::run(p.ui);

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
        std_any value = p.settingsObject->value("Devices/DeviceIndex");
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
            imaging::HDRData hdrData;
            from_json(json, hdrData);
            p.devicesModel->setHDRData(hdrData);
        }

        p.logObserver = observer::ListObserver<log::Item>::create(
            p.ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
            {
                for (const auto& i : value)
                {
                    switch (i.type)
                    {
                    case log::Type::Error:
                    {
                        std::string msg =
                            string::Format(_("ERROR: {0}")).arg(i.message);
                        _p->ui->uiStatusBar->timeout(errorTimeout);
                        _p->ui->uiStatusBar->copy_label(msg.c_str());
                        if (LogDisplay::prefs == LogDisplay::kWindowOnError)
                        {
                            if (!logsPanel)
                                logs_panel_cb(NULL, _p->ui);
                            logsPanel->undock();
                        }
                        else if (LogDisplay::prefs == LogDisplay::kDockOnError)
                        {
                            if (!logsPanel)
                                logs_panel_cb(NULL, _p->ui);
                            logsPanel->dock();
                        }
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

        TimelineClass* c = p.ui->uiTimeWindow;
        c->uiTimeline->setTimeObject(p.timeObject);
        c->uiFrame->setTimeObject(p.timeObject);
        c->uiStartFrame->setTimeObject(p.timeObject);
        c->uiEndFrame->setTimeObject(p.timeObject);

        DBG;
        _cacheUpdate();
        _audioUpdate();
        DBG;

        // Create the main control.
        p.mainControl = new MainControl(p.ui);

        if (!OSXfiles.empty())
        {
            int idx = 0;
            if (p.options.fileName[0].empty())
            {
                while (idx < 3)
                {
                    p.options.fileName[idx] = OSXfiles[idx];
                    ++idx;
                }
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
        }

        // Open the input files.
        if (!p.options.fileName[0].empty())
        {
            for (int i = 2; i >= 0; --i)
            {
                if (p.options.fileName[i].empty())
                    continue;
                open(p.options.fileName[i], p.options.audioFileName);
            }
        }

        Preferences::open_windows();

        p.ui->uiMain->fill_menu(p.ui->uiMenuBar);

        p.ui->uiMain->show();
        p.ui->uiView->take_focus();

        if (p.ui->uiSecondary)
        {
            // We raise the secondary window last, so it shows at front
            p.ui->uiSecondary->window()->show();
        }
    }

    App::~App()
    {
        TLRENDER_P();

        delete p.contextObject;
        delete p.timeObject;
        delete p.ui->uiMain;
        p.ui->uiMain = nullptr;
        delete p.ui;
        delete uiLogDisplay;

        // delete p.outputDevice;  // @todo:
        p.outputDevice = nullptr;

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

    TimeObject* App::timeObject() const
    {
        return _p->timeObject;
    }

    SettingsObject* App::settingsObject() const
    {
        return _p->settingsObject;
    }

    const std::shared_ptr<FilesModel>& App::filesModel() const
    {
        return _p->filesModel;
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
        TimelinePlayer* player;
        timeline::Playback playback;
    };

    static void start_playback(void* data)
    {
        PlaybackData* p = (PlaybackData*)data;
        auto player = p->player;
        player->setPlayback(p->playback);
        delete p;
    }

    int App::run()
    {
        TLRENDER_P();
        Fl::flush();
        bool autoPlayback = p.ui->uiPrefs->uiPrefsAutoPlayback->value();
        if (!p.timelinePlayers.empty() && p.timelinePlayers[0] &&
            p.options.playback != timeline::Playback::Stop && autoPlayback)
        {
            // We use a timeout to start playback of the loaded video to
            // make sure to show all frames
            PlaybackData* data = new PlaybackData;
            data->player = p.timelinePlayers[0];
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

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = std_any_cast<int>(
            p.settingsObject->value("Misc/MaxFileSequenceDigits"));
        for (const auto& path :
             timeline::getPaths(fileName, pathOptions, _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFileName);
            p.filesModel->add(item);
        }
    }

    void App::openSeparateAudioDialog()
    {
        auto dialog =
            std::make_unique<OpenSeparateAudioDialog>(_context, _p->ui);
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
        p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
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

    void App::setMute(bool value)
    {
        TLRENDER_P();
        if (value == p.mute)
            return;
        p.mute = value;
        _audioUpdate();
        muteChanged(p.mute);
    }

    void App::_activeCallback(
        const std::vector<std::shared_ptr<FilesModelItem> >& items)
    {
        TLRENDER_P();

        // Flag used to determine if clip was just loaded or we just switched
        // from a compare or a file list change.
        bool loaded = false;

        DBG;
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

        DBG;
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

                DBG;
                options.ioOptions["ffmpeg/YUVToRGBConversion"] =
                    string::Format("{0}").arg(
                        std_any_cast< int>(p.settingsObject->value(
                            "Performance/FFmpegYUVToRGBConversion")));

                DBG;
                const audio::Info audioInfo =
                    audioSystem->getDefaultOutputInfo();
                DBG;
                options.ioOptions["ffmpeg/AudioChannelCount"] =
                    string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] =
                    string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] =
                    string::Format("{0}").arg(audioInfo.sampleRate);

                options.ioOptions["ffmpeg/ThreadCount"] =
                    string::Format("{0}").arg(
                        std_any_cast<int>(p.settingsObject->value(
                            "Performance/FFmpegThreadCount")));
                DBG;

                options.pathOptions.maxNumberDigits = std::min(
                    std_any_cast<int>(
                        p.settingsObject->value("Misc/MaxFileSequenceDigits")),
                    255);

                DBG;
                auto otioTimeline =
                    item->audioPath.isEmpty()
                        ? timeline::create(item->path.get(), _context, options)
                        : timeline::create(
                              item->path.get(), item->audioPath.get(), _context,
                              options);

                DBG;

                if (0)
                {
                    // createMemoryTimeline(otioTimeline,
                    // item->path.getDirectory(), options.pathOptions);
                }
                auto timeline =
                    timeline::Timeline::create(otioTimeline, _context, options);

                DBG;

                timeline::PlayerOptions playerOptions;
                playerOptions.cache.readAhead = _cacheReadAhead();
                playerOptions.cache.readBehind = _cacheReadBehind();

                DBG;

                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/TimerMode"));
                playerOptions.timerMode = (timeline::TimerMode)value;
                value = std_any_cast<int>(p.settingsObject->value(
                    "Performance/AudioBufferFrameCount"));
                playerOptions.audioBufferFrameCount =
                    (timeline::AudioBufferFrameCount)value;
                if (item->init)
                {
                    playerOptions.currentTime = items[0]->currentTime;
                }

                auto timelinePlayer = timeline::TimelinePlayer::create(
                    timeline, _context, playerOptions);

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
                    std::string file = item->path.get();
                    char buf[2048];
                    fl_filename_absolute( buf, 2048, file.c_str() );
                    p.settingsObject->addRecentFile(buf);
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
                    mrvTimelinePlayer->timelinePlayer()->setExternalTime(
                        newTimelinePlayers[0]->timelinePlayer());
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

        if (p.ui)
        {

            DBG;
            if (!validTimelinePlayers.empty())
            {
                auto player = validTimelinePlayers[0];

                p.ui->uiMain->show();

                size_t numFiles = filesModel()->observeFiles()->getSize();
                if (numFiles == 1)
                {
                    // resize the window to the size of the first clip loaded
                    p.ui->uiView->resizeWindow();
                    p.ui->uiView->take_focus();
                }

                Preferences::updateICS();

                if (p.running)
                {
                    if (p.ui->uiPrefs->uiPrefsAutoPlayback->value() && loaded)
                    {
                        player->setPlayback(timeline::Playback::Forward);
                    }
                    p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
                }
            }
        }

        DBG;
        _cacheUpdate();
        DBG;
        _audioUpdate();
        DBG;
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

    void App::_cacheUpdate()
    {
        TLRENDER_P();
        timeline::PlayerCacheOptions options;
        options.readAhead = _cacheReadAhead();
        options.readBehind = _cacheReadBehind();
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
