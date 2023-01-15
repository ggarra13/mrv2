// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "App.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlTimeline/Util.h>

#include "mrvCore/mrvOS.h"  // do not move up
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvTimelineCreate.h"
#include "mrvFl/mrvTimeObject.h"
#include "mrvFl/mrvContextObject.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvGL/mrvGLViewport.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvDevicesModel.h"
#include "mrvApp/mrvOpenSeparateAudioDialog.h"


#include "mrvPreferencesUI.h"
#include "mrViewer.h"

#include <FL/platform.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>

#ifdef __linux__
#undef None   // macro defined in X11 config files
#endif

#include "mrvFl/mrvIO.h"

namespace {
    const char* kModule = "app";
}

namespace mrv
{
#if defined( FLTK_USE_X11 )
    int xerrorhandler(Display *dsp, XErrorEvent *error)
    {
        char errorstring[128];
        XGetErrorText(dsp, error->error_code, errorstring, 128);

        LOG_ERROR( "X error-- " << errorstring );
        exit(-1);
    }
#endif

    namespace
    {
        const float errorTimeout = 5.F;
    }

    struct Options
    {
        std::string fileName;
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
    };

    struct App::Private
    {
        Options options;

        ContextObject* contextObject = nullptr;
        TimeObject* timeObject = nullptr;
        SettingsObject* settingsObject = nullptr;

        std::shared_ptr<FilesModel> filesModel;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > activeObserver;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > allObserver;
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
        std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

        ViewerUI*                 ui = nullptr;

        std::vector<TimelinePlayer*> timelinePlayers;
        std::map<std::shared_ptr<FilesModelItem>, TimelinePlayer* > itemsMapping;


        bool running = false;
    };

    
    std::vector< std::string > OSXfiles;
    void osx_open_cb(const char *fname)
    {
        OSXfiles.push_back( fname );
    }

    
    App::App(
        int argc,
        char** argv,
        const std::shared_ptr<system::Context>& context) :
        IApp(),
        _p( new Private )
    {
        TLRENDER_P();


#if defined __APPLE__
        setenv( "LC_CTYPE",  "UTF-8", 1 );
#endif

        int lang = -1;
        const char* code = "C";
        {
            Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                 "mrViewer2" );

            // Load ui language preferences
            Fl_Preferences ui( base, "ui" );

            ui.get( "language", lang, -1 );
            if ( lang >= 0 )
            {
                for ( unsigned i = 0;
                      i < sizeof(kLanguages) / sizeof(LanguageTable); ++i)
                {
                    if ( kLanguages[i].index == lang )
                    {
                        code = kLanguages[i].code;
                        break;
                    }
                }
#ifdef _WIN32
                setenv( "LC_CTYPE",  "UTF-8", 1 );
                if ( setenv( "LANGUAGE", code, 1 ) < 0 )
                    LOG_ERROR( "Setting LANGUAGE failed" );
                setlocale( LC_ALL, "" );
                setlocale( LC_ALL, code );
                libintl_setlocale( LC_ALL, "" );
                libintl_setlocale( LC_ALL, code );
                libintl_setlocale( LC_MESSAGES, code );
#else
                setenv( "LANGUAGE", code, 1 );
                setlocale( LC_ALL, "" );
                setlocale(LC_ALL, code);
#ifdef __APPLE__
                setenv( "LC_NUMERIC", code, 1 );
                setenv( "LC_MESSAGES", code, 1 );
#endif
#endif
            }
        }


        const char* tmp;
        if ( lang < 0 )
            tmp = setlocale(LC_ALL, "");
        else
        {
            tmp = setlocale(LC_ALL, NULL);
        }


#if defined __APPLE__ && defined __MACH__
        tmp = setlocale( LC_MESSAGES, NULL );
#endif

        const char* language = getenv( "LANGUAGE" );
        if ( !language || language[0] == '\0' ) language = getenv( "LC_ALL" );
        if ( !language || language[0] == '\0' ) language = getenv( "LC_NUMERIC" );
        if ( !language || language[0] == '\0' ) language = getenv( "LANG" );
        if ( language )
        {
            if (  strcmp( language, "C" ) == 0 ||
                  strncmp( language, "ar", 2 ) == 0 ||
                  strncmp( language, "en", 2 ) == 0 ||
                  strncmp( language, "ja", 2 ) == 0 ||
                  strncmp( language, "ko", 2 ) == 0 ||
                  strncmp( language, "zh", 2 ) == 0 )
                tmp = "C";
        }

        setlocale( LC_NUMERIC, tmp );


        // Create and install global locale
        fs::path::imbue(std::locale());

#ifdef __linux__
        int ok = XInitThreads();
        if (!ok) throw std::runtime_error( "XInitThreads failed" );

        XSetErrorHandler(xerrorhandler);
#endif

        set_root_path( argc, argv );

        IApp::_init(
            argc,
            argv,
            context,
            "mrViewer",
            "Play timelines, movies, and image sequences.",
            {
                app::CmdLineValueArg<std::string>::create(
                    p.options.fileName,
                    "input",
                    "Timeline, movie, image sequence, or folder.",
                    true)
            },
        {
            app::CmdLineValueOption<int>::create(
                Preferences::debug,
                { "-debug", "-d" },
                "Debug verbosity."),
            app::CmdLineValueOption<std::string>::create(
                p.options.audioFileName,
                { "-audio", "-a" },
                "Audio file name."),
            app::CmdLineValueOption<std::string>::create(
                p.options.compareFileName,
                { "-compare", "-b" },
                "A/B comparison \"B\" file name."),
            app::CmdLineValueOption<timeline::CompareMode>::create(
                p.options.compareMode,
                { "-compareMode", "-c" },
                "A/B comparison mode.",
                string::Format("{0}").arg(p.options.compareMode),
                string::join(timeline::getCompareModeLabels(), ", ")),
            app::CmdLineValueOption<math::Vector2f>::create(
                p.options.wipeCenter,
                { "-wipeCenter", "-wc" },
                "A/B comparison wipe center.",
                string::Format("{0}").arg(p.options.wipeCenter)),
            app::CmdLineValueOption<float>::create(
                p.options.wipeRotation,
                { "-wipeRotation", "-wr" },
                "A/B comparison wipe rotation.",
                string::Format("{0}").arg(p.options.wipeRotation)),
            app::CmdLineValueOption<double>::create(
                p.options.speed,
                { "-speed" },
                "Playback speed."),
            app::CmdLineValueOption<timeline::Playback>::create(
                p.options.playback,
                { "-playback", "-p" },
                "Playback mode.",
                string::Format("{0}").arg(p.options.playback),
                string::join(timeline::getPlaybackLabels(), ", ")),
            app::CmdLineValueOption<timeline::Loop>::create(
                p.options.loop,
                { "-loop" },
                "Playback loop mode.",
                string::Format("{0}").arg(p.options.loop),
                string::join(timeline::getLoopLabels(), ", ")),
            app::CmdLineValueOption<otime::RationalTime>::create(
                p.options.seek,
                { "-seek" },
                "Seek to the given time."),
            app::CmdLineValueOption<otime::TimeRange>::create(
                p.options.inOutRange,
                { "-inOutRange" },
                "Set the in/out points range."),
            app::CmdLineValueOption<std::string>::create(
                p.options.colorConfigOptions.fileName,
                { "-colorConfig", "-cc" },
                "Color configuration file name (config.ocio)."),
            app::CmdLineValueOption<std::string>::create(
                p.options.colorConfigOptions.input,
                { "-colorInput", "-ci" },
                "Input color space."),
            app::CmdLineValueOption<std::string>::create(
                p.options.colorConfigOptions.display,
                { "-colorDisplay", "-cd" },
                "Display color space."),
            app::CmdLineValueOption<std::string>::create(
                p.options.colorConfigOptions.view,
                { "-colorView", "-cv" },
                "View color space."),
            app::CmdLineValueOption<std::string>::create(
                p.options.lutOptions.fileName,
                { "-lut" },
                "LUT file name."),
            app::CmdLineValueOption<timeline::LUTOrder>::create(
                p.options.lutOptions.order,
                { "-lutOrder" },
                "LUT operation order.",
                string::Format("{0}").arg(p.options.lutOptions.order),
                string::join(timeline::getLUTOrderLabels(), ", ")),
            app::CmdLineFlagOption::create(
                p.options.resetSettings,
                { "-resetSettings" },
                "Reset settings to defaults.")
        });


        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }
        DBG;



        p.contextObject = new mrv::ContextObject(context);
        DBG;
        p.filesModel = FilesModel::create(context);
        DBG;



        DBG;
        p.lutOptions = p.options.lutOptions;


        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );
        Fl::use_high_res_GL(true);
        Fl::set_fonts( "-*" );
#ifdef __APPLE__
        // For macOS, to read command-line arguments
        fl_open_callback( osx_open_cb );
        fl_open_display();
        Fl::check();
#endif 

        DBG;
        // Store the application object for further use down the line
        ViewerUI::app = this;

        DBG;
        // Create the window.
        p.ui = new ViewerUI();
        DBG;

        if (!p.ui)
        {
            throw std::runtime_error( _("Cannot create window") );
        }
        p.ui->uiView->setContext( _context );
        p.ui->uiTimeWindow->uiTimeline->setContext( _context );

        DBG;
        p.ui->uiMain->main( p.ui );
        Preferences::ui = p.ui;
        DBG;

        p.timeObject = new mrv::TimeObject( p.ui );
        p.settingsObject = new SettingsObject( p.timeObject );

        Preferences prefs( p.ui->uiPrefs, p.options.resetSettings );
        Preferences::run( p.ui );

        p.activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
            p.filesModel->observeActive(),
            [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
            {
        DBG;
                _activeCallback(value);
        DBG;
            });

        DBG;
        p.layersObserver = observer::ListObserver<int>::create(
            p.filesModel->observeLayers(),
            [this](const std::vector<int>& value)
            {
        DBG;
                for (size_t i = 0;
                     i < value.size() && i < _p->timelinePlayers.size(); ++i)
                {
                    if (_p->timelinePlayers[i])
                    {
                        _p->timelinePlayers[i]->setVideoLayer(value[i]);
                    }
                }

        DBG;
            });


        DBG;
        // p.outputDevice = new OutputDevice(context);
        p.devicesModel = DevicesModel::create(context);
        std_any value = p.settingsObject->value("Devices/DeviceIndex");
        p.devicesModel->setDeviceIndex( value.type() == typeid(void) ? 0 :
                                        std_any_cast<int>(value) );
        value = p.settingsObject->value("Devices/DisplayModeIndex");
        p.devicesModel->setDisplayModeIndex( value.type() == typeid(void) ? 0 :
                                             std_any_cast<int>(value) );
        value = p.settingsObject->value("Devices/PixelTypeIndex");
        p.devicesModel->setPixelTypeIndex( value.type() == typeid(void) ? 0 :
                                           std_any_cast<int>(value));
        p.settingsObject->setDefaultValue("Devices/HDRMode",
                                          static_cast<int>(device::HDRMode::FromFile));
        p.devicesModel->setHDRMode( static_cast<device::HDRMode>( std_any_cast<int>( p.settingsObject->value("Devices/HDRMode") ) ) );
        value = p.settingsObject->value("Devices/HDRData");
        std::string s = value.type() == typeid(void) ? std::string() :
                        std_any_cast< std::string >( value );
        if (!s.empty())
        {
            auto json = nlohmann::json::parse(s);
            imaging::HDRData hdrData;
            from_json(json, hdrData);
            p.devicesModel->setHDRData(hdrData);
        }

        DBG;
        p.logObserver = observer::ListObserver<log::Item>::create(
            p.ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Error:
                            _p->ui->uiStatusBar->timeout( errorTimeout );
                            _p->ui->uiStatusBar->copy_label(
                                std::string( string::Format(_("ERROR: {0}")).
                                             arg(i.message) ).c_str() );
                            break;
                        default: break;
                        }
                    }
                });

        DBG;
       p.devicesObserver = observer::ValueObserver<DevicesModelData>::create(
                p.devicesModel->observeData(),
                [this](const DevicesModelData& value)
                {
                    const device::PixelType pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        device::PixelType::None;
                    // @todo:
                    // _p->outputDevice->setDevice(
                    //     value.deviceIndex - 1,
                    //     value.displayModeIndex - 1,
                    //     pixelType);
                    // _p->outputDevice->setDeviceEnabled(value.deviceEnabled);
                    // _p->outputDevice->setHDR(value.hdrMode, value.hdrData);
                });

        TimelineClass* c = p.ui->uiTimeWindow;
        c->uiTimeline->setTimeObject( p.timeObject );
        c->uiFrame->setTimeObject( p.timeObject );
        c->uiStartFrame->setTimeObject( p.timeObject );
        c->uiEndFrame->setTimeObject( p.timeObject );

        DBG;
        _cacheUpdate();
        _audioUpdate();
        DBG;

        if ( ! OSXfiles.empty() )
        {
            int idx = 0;
            if (p.options.fileName.empty())
            {
                p.options.fileName = OSXfiles[idx];
                ++idx;
            }
            if (p.options.compareFileName.empty() && idx < OSXfiles.size() )
            {
                p.options.compareFileName = OSXfiles[idx];
            }
        }

        // Open the input files.
        if (!p.options.fileName.empty())
        {

            if (!p.options.compareFileName.empty())
            {
                timeline::CompareOptions compareOptions;
                compareOptions.mode = p.options.compareMode;
                compareOptions.wipeCenter = p.options.wipeCenter;
                compareOptions.wipeRotation = p.options.wipeRotation;
                p.filesModel->setCompareOptions(compareOptions);
                p.ui->uiView->setCompareOptions(compareOptions);
                open( p.options.compareFileName.c_str() );
            }



            open( p.options.fileName.c_str(), p.options.audioFileName.c_str());



            TimelinePlayer* player = nullptr;

            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {

                player = p.timelinePlayers[ 0 ];

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

                c->uiTimeline->setColorConfigOptions(
                    p.options.colorConfigOptions );

            }
        }
        else
        {
            const auto fps = 24.0;
            const auto& startTime = otio::RationalTime( 1.0, fps );
            const auto& duration  = otio::RationalTime( 50.0, fps );
            c->uiFrame->setTime( startTime );
            c->uiStartFrame->setTime( startTime );
            c->uiEndFrame->setTime(
                startTime + duration - otio::RationalTime( 1.0,
                                                           duration.rate() ) );
        }

        Preferences::open_windows();

        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );

        p.ui->uiMain->show();
        p.ui->uiView->take_focus();

        if ( p.ui->uiSecondary )
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
        delete p.ui->uiMain; p.ui->uiMain = nullptr;
        delete p.ui;

        //delete p.outputDevice;  // @todo:
        p.outputDevice = nullptr;

        if (p.settingsObject && p.devicesModel)
        {
            const auto& deviceData = p.devicesModel->observeData()->get();
            p.settingsObject->setValue("Devices/DeviceIndex", static_cast<int>(deviceData.deviceIndex));
            p.settingsObject->setValue("Devices/DisplayModeIndex", static_cast<int>(deviceData.displayModeIndex));
            p.settingsObject->setValue("Devices/PixelTypeIndex", static_cast<int>(deviceData.pixelTypeIndex));
            p.settingsObject->setValue("Devices/HDRMode", static_cast<int>(deviceData.hdrMode));
            nlohmann::json json;
            to_json(json, deviceData.hdrData);
            const std::string& data = json.dump();
            p.settingsObject->setValue("Devices/HDRData", data );
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
    
    static void start_playback( void* data )
    {
        PlaybackData* p = (PlaybackData*) data;
        auto player = p->player;
        player->setPlayback( p->playback );
        delete p;
    }
    
    int App::run()
    {
        TLRENDER_P();
        Fl::flush();
        if ( !p.timelinePlayers.empty() &&
             p.options.playback != timeline::Playback::Stop )
        {
            // We use a timeout to start playback of the loaded video to
            // make sure to show all frames
            PlaybackData* data = new PlaybackData;
            data->player = p.timelinePlayers[0];
            data->playback = p.options.playback;
            Fl::add_timeout( 0.005,
                             (Fl_Timeout_Handler) start_playback, data );
        }
        p.running = true;
        return Fl::run();
    }


    void App::open( const std::string& fileName,
                    const std::string& audioFileName )
    {
        TLRENDER_P();

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = std_any_cast<int>(
            p.settingsObject->value("Misc/MaxFileSequenceDigits") );
        for (const auto& path : timeline::getPaths(fileName, pathOptions,
                                                   _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFileName);
            p.filesModel->add(item);
        }
    }

    void App::openSeparateAudioDialog()
    {
        auto dialog = std::make_unique<OpenSeparateAudioDialog>(_context, _p->ui);
        if ( dialog->exec() )
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

    void App::setImageOptions(const timeline::ImageOptions& value)
    {
        TLRENDER_P();
        if (value == p.imageOptions)
            return;
        p.imageOptions = value;
        imageOptionsChanged(p.imageOptions);
    }

    void App::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        TLRENDER_P();
        if (value == p.displayOptions)
            return;
        p.displayOptions = value;
        displayOptionsChanged(p.displayOptions);
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

    void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
    {
        TLRENDER_P();


        DBG;
        if (!p.active.empty() &&
            !p.timelinePlayers.empty() &&
            p.timelinePlayers[0])
        {
            p.active[0]->init = true;
            p.active[0]->speed = p.timelinePlayers[0]->speed();
            p.active[0]->playback = p.timelinePlayers[0]->playback();
            p.active[0]->loop = p.timelinePlayers[0]->loop();
            p.active[0]->currentTime = p.timelinePlayers[0]->currentTime();
            p.active[0]->inOutRange = p.timelinePlayers[0]->inOutRange();
            p.active[0]->videoLayer = p.timelinePlayers[0]->videoLayer();
            p.active[0]->audioOffset = p.timelinePlayers[0]->audioOffset();
        }

        DBG;
        std::vector<TimelinePlayer*> newTimelinePlayers;
        auto audioSystem = _context->getSystem<audio::System>();
        for (const auto& i : items)
        {
            TimelinePlayer* mrvTimelinePlayer = nullptr;

            // Find the item in the mapping to timelinePlayer
            auto it = p.itemsMapping.find(i);

            if ( it != p.itemsMapping.end() )
            {
                auto player = it->second;
                DBGM2( "Item " << i << " has timeline " << it->second );
                // Check the timelinePlayers for this timeline player's item
                auto ip = std::find( newTimelinePlayers.begin(),
                                     newTimelinePlayers.end(), player );

                if ( ip == newTimelinePlayers.end() )
                {
                    newTimelinePlayers.push_back( player );
                    continue;
                }
            }

            DBG;
            try
            {
                timeline::Options options;

                int value = std_any_cast<int>( p.settingsObject->value("FileSequence/Audio") );

                options.fileSequenceAudio = (timeline::FileSequenceAudio)
                                            value;

                std_any v = p.settingsObject->value("FileSequence/AudioFileName");
                options.fileSequenceAudioFileName = std_any_cast<std::string>( v );

                options.fileSequenceAudioDirectory = std_any_cast<std::string>( p.settingsObject->value("FileSequence/AudioDirectory") );
                options.videoRequestCount = std_any_cast<int>( p.settingsObject->value( "Performance/VideoRequestCount" ) );

                options.audioRequestCount = std_any_cast<int>( p.settingsObject->value( "Performance/AudioRequestCount" ) );

                options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg( std_any_cast<int>( p.settingsObject->value( "Performance/SequenceThreadCount" ) ) );


                DBG;
                options.ioOptions["ffmpeg/YUVToRGBConversion"] =
                    string::Format("{0}").
                    arg( std_any_cast<int>(
                             p.settingsObject->value("Performance/FFmpegYUVToRGBConversion") ) );

                const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

                options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg( std_any_cast<int>( p.settingsObject->value( "Performance/FFmpegThreadCount" ) ) );
                DBG;

                options.pathOptions.maxNumberDigits = std::min( std_any_cast<int>( p.settingsObject->value("Misc/MaxFileSequenceDigits") ),
                                                                255 );


                DBG;
#if 0
                auto otioTimeline = i->audioPath.isEmpty() ?
                                    timeline::create(i->path.get(), _context,
                                                     options) :
                                    timeline::create(i->path.get(),
                                                     i->audioPath.get(),
                                                     _context, options);
#else
                std::vector< std::string > files;
                files.push_back( i->path.get() );
#if 1
                    files.push_back( "/Users/gga/code/applications/mrv2/tlRender/etc/SampleData/Dinky_2015-06-11.m4v" );
                    files.push_back( "/Users/gga/code/applications/mrv2/tlRender/etc/SampleData/PSR63_2012-06-02.mov" );
                    //files.push_back( "/Users/gga/Movies/frozen-2-trailer-2_h1080p.mov" );
#endif
                std::cerr << "files.size=" <<  files.size() << std::endl;
                auto otioTimeline = timeline::create(files, _context,
                                                     options);
#endif

                if (0)
                {
                    //createMemoryTimeline(otioTimeline, i->path.getDirectory(),
                    //                     options.pathOptions);
                }
                auto timeline = timeline::Timeline::create(otioTimeline,
                                                           _context, options);

                auto& info = timeline->getIOInfo();
                if ( info.video.empty() )
                    throw std::runtime_error(string::Format("{0}: Error reading file").arg(i->path.get()));
                p.settingsObject->addRecentFile(i->path.get());

                timeline::PlayerOptions playerOptions;
                playerOptions.cache.readAhead = _cacheReadAhead();
                playerOptions.cache.readBehind = _cacheReadBehind();

                DBG;

                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/TimerMode") );
                playerOptions.timerMode = (timeline::TimerMode) value;
                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/AudioBufferFrameCount") );
                playerOptions.audioBufferFrameCount = (timeline::AudioBufferFrameCount) value;

                auto timelinePlayer =
                    timeline::TimelinePlayer::create(timeline, _context,
                                                     playerOptions);

                mrvTimelinePlayer = new mrv::TimelinePlayer(timelinePlayer,
                                                            _context);
                // Don't overwrite a previous timeline (to keep
                // annotations)
                if ( it == p.itemsMapping.end() )
                {
                    p.itemsMapping[i] = mrvTimelinePlayer;
                }
            }
            catch (const std::exception& e)
            {
                if ( ! logsTool )
                {
                    logs_tool_grp( NULL, p.ui  );
                }
                _log(e.what(), log::Type::Error);
                // Remove this invalid file
                p.filesModel->close();
            }
            newTimelinePlayers.push_back( mrvTimelinePlayer );
        }

        // Furst, stop all old timelinePlayers
        for ( auto& player : p.timelinePlayers )
        {
            player->stop();
        }
        
        DBG;
        if (!items.empty() &&
            !newTimelinePlayers.empty() &&
            newTimelinePlayers[0])
        {
            items[0]->timeRange = newTimelinePlayers[0]->timeRange();
            items[0]->ioInfo = newTimelinePlayers[0]->ioInfo();
            if (!items[0]->init)
            {
                items[0]->init = true;
                items[0]->speed = newTimelinePlayers[0]->speed();
                items[0]->playback = newTimelinePlayers[0]->playback();
                items[0]->loop = newTimelinePlayers[0]->loop();
                items[0]->currentTime = newTimelinePlayers[0]->currentTime();
                items[0]->inOutRange = newTimelinePlayers[0]->inOutRange();
                items[0]->videoLayer = newTimelinePlayers[0]->videoLayer();
                items[0]->audioOffset = newTimelinePlayers[0]->audioOffset();
            }
            else
            {
                newTimelinePlayers[0]->setAudioOffset(items[0]->audioOffset);
                newTimelinePlayers[0]->setVideoLayer(items[0]->videoLayer);
                newTimelinePlayers[0]->setSpeed(items[0]->speed);
                newTimelinePlayers[0]->setLoop(items[0]->loop);
                newTimelinePlayers[0]->setInOutRange(items[0]->inOutRange);
                newTimelinePlayers[0]->seek(items[0]->currentTime);
                newTimelinePlayers[0]->setPlayback(items[0]->playback);
          }
        }


        for (size_t i = 1; i < items.size(); ++i)
        {
            if (newTimelinePlayers[i])
            {
                newTimelinePlayers[i]->setVideoLayer(items[i]->videoLayer);
            }
        }

        // Set the external time.
        std::shared_ptr<timeline::TimelinePlayer> externalTime;
        if (!newTimelinePlayers.empty() && newTimelinePlayers[0])
        {
            externalTime = newTimelinePlayers[0]->timelinePlayer();
            externalTime->setExternalTime(nullptr);
        }
        for (size_t i = 1; i < newTimelinePlayers.size(); ++i)
        {
            if (newTimelinePlayers[i])
            {
                newTimelinePlayers[i]->timelinePlayer()->setExternalTime(externalTime);
            }
        }

        std::vector<mrv::TimelinePlayer*> validTimelinePlayers;
        for (const auto& i : newTimelinePlayers)
        {
            if (i)
            {
                validTimelinePlayers.push_back(i);
            }
        }


        if ( p.ui )
        {
            Viewport* primary = p.ui->uiView;
            primary->setTimelinePlayers( validTimelinePlayers );
            
            if ( p.ui->uiSecondary )
            {
                Viewport* view = p.ui->uiSecondary->viewport();
                view->setColorConfigOptions( primary->getColorConfigOptions() );
                view->setLUTOptions( primary->lutOptions() );
                view->setImageOptions( primary->getImageOptions() );
                view->setDisplayOptions( primary->getDisplayOptions() );
                view->setCompareOptions( primary->getCompareOptions() );
                view->setTimelinePlayers( validTimelinePlayers,  false );
                view->frameView();
            }
        }

        //
        // Note: Unlike Darby's code we must not delete all the timeline players
        //       here, as they keep the annotations.  That's why we go thru all
        //       the trouble of reusing the timelinePlayers.
        //

        p.active = items;
        p.timelinePlayers = validTimelinePlayers;


        // Cleanup the TimelinePlayers that are no longer attached
        // to a valid clip.  That is, no file uses them.
        auto allItems = p.filesModel->observeFiles()->get();
        for ( auto it = p.itemsMapping.begin(); it != p.itemsMapping.end(); )
        {
            bool must_delete = true;

            for ( const auto& item : allItems )
            {
                if ( item == it->first )
                {
                    must_delete = false;
                    break;
                }
            }

            if (must_delete)
            {
                for ( auto& player : p.timelinePlayers )
                {
                    if ( player == it->second )
                        player = nullptr;
                }
                p.timelinePlayers.erase(std::remove(p.timelinePlayers.begin(),
                                                    p.timelinePlayers.end(),
                                                    nullptr),
                                        p.timelinePlayers.end());
                delete it->second;
                p.itemsMapping.erase(it++);
            }
            else
            {
                ++it;
            }
        }

        DBG;

        if ( p.ui )
        {
            TimelinePlayer* player = nullptr;
            TimelineClass* c = p.ui->uiTimeWindow;
            c->uiAudioTracks->clear();

            if ( !p.timelinePlayers.empty() )
            {

                player = p.timelinePlayers[0];

                c->uiFPS->value( player->speed() );

                c->uiTimeline->setTimelinePlayer( player );
                if ( colorTool ) colorTool->refresh();
                if ( imageInfoTool )
                {
                    imageInfoTool->setTimelinePlayer( player );
                    imageInfoTool->refresh();
                }

                const auto timeRange = player->timeRange();
                c->uiFrame->setTime( player->currentTime() );
                c->uiStartFrame->setTime( timeRange.start_time() );
                c->uiEndFrame->setTime( timeRange.end_time_inclusive() );

                // Set the audio tracks
                const auto timeline = player->timeline();
                const auto  ioinfo = timeline->getIOInfo();
                const auto audio = ioinfo.audio;
                const auto name = audio.name;
                int mode = FL_MENU_RADIO;
                c->uiAudioTracks->add( _("Mute"), 0, 0, 0, mode );
                int idx = c->uiAudioTracks->add( name.c_str(), 0, 0, 0,
                                                 mode | FL_MENU_VALUE );

                if ( player->isMuted() )
                {
                    c->uiAudioTracks->value( 0 );
                }
                c->uiAudioTracks->do_callback();
                c->uiAudioTracks->redraw();

                // Set the audio volume
                c->uiVolume->value( player->volume() );
                c->uiVolume->redraw();

                p.ui->uiMain->show();

                size_t numFiles = filesModel()->observeFiles()->getSize();
                if ( numFiles == 1 )
                  {
                    // resize the window to the size of the first clip loaded
                    p.ui->uiView->resizeWindow();
                    p.ui->uiView->take_focus();
                  }

                c->uiLoopMode->value( (int)p.options.loop );
                c->uiLoopMode->do_callback();



                std::vector<timeline::ImageOptions>& imageOptions =
                    p.ui->uiView->getImageOptions();
                std::vector<timeline::DisplayOptions>& displayOptions =
                    p.ui->uiView->getDisplayOptions();
                imageOptions.resize( p.timelinePlayers.size() );
                displayOptions.resize( p.timelinePlayers.size() );


                if ( p.running )
                {
                    p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
                }
            }
        }

        DBG;
        _cacheUpdate();
        _audioUpdate();
        DBG;



    }


    otime::RationalTime App::_cacheReadAhead() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value = std_any_cast<double>( p.settingsObject->value( "Cache/ReadAhead" ) );
        return otime::RationalTime( value / static_cast<double>(activeCount),
                                    1.0);
    }

    otime::RationalTime App::_cacheReadBehind() const
    {
        TLRENDER_P();
        DBG;
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        DBG;
        double value = std_any_cast<double>( p.settingsObject->value( "Cache/ReadBehind" ) );
        DBG;
        return otime::RationalTime( value / static_cast<double>(activeCount),
                                    1.0);
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
        if (p.outputDevice)
        {
            // @todo:
            //
            // p.outputDevice->setVolume(p.volume);
            // p.outputDevice->setMute(p.mute);
        }
    }
}
