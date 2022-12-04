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

#include <mrvCore/mrvOS.h>  // do not move up
#include <mrvCore/mrvRoot.h>

#include <mrvFl/mrvTimeObject.h>
#include <mrvFl/mrvContextObject.h>
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
#include <FL/Fl.H>

#ifdef __linux__
#undef None   // macro defined in X11 config files
#endif

#include <mrvFl/mrvIO.h>

namespace {
    const char* kModule = "app";
}

namespace mrv
{

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
        OutputDevice* outputDevice = nullptr;
        std::shared_ptr<DevicesModel> devicesModel;
        std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;
        std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

        ViewerUI*                 ui = nullptr;

        std::vector<TimelinePlayer*> timelinePlayers;
        std::map<std::shared_ptr<FilesModelItem>, TimelinePlayer* > itemsMapping;


        bool running = false;
    };

    App::App(
        int argc,
        char** argv,
        const std::shared_ptr<system::Context>& context) :
        IApp(),
        _p( new Private )
    {
        TLRENDER_P();


#if defined __APPLE__ && defined __MACH__
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
#ifdef OSX
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
    try {
        // std::locale::global( std::locale(language) );
        // Make boost.filesystem use it
        fs::path::imbue(std::locale());
    }
    catch( const std::runtime_error& e )
    {
        std::cerr << e.what() << std::endl;
    }


#ifdef __linux__
        int ok = XInitThreads();
        if (!ok) throw std::runtime_error( "XInitThreads failed" );
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



        p.contextObject = new mrv::ContextObject(context);
        p.filesModel = FilesModel::create(context);

        p.activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
            p.filesModel->observeActive(),
            [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
            {
                _activeCallback(value);
            });

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



        p.lutOptions = p.options.lutOptions;


        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );
        Fl::use_high_res_GL(true);
        Fl::set_fonts( "-*" );

        // Store the application object for further use down the line
        ViewerUI::app = this;

        // Create the window.
        p.ui = new ViewerUI();

        if (!p.ui)
        {
            throw std::runtime_error( _("Cannot create window") );
        }
        p.ui->uiView->setContext( _context );
        p.ui->uiTimeline->setContext( _context );

        p.ui->uiMain->main( p.ui );
        Preferences::ui = p.ui;

        p.timeObject = new mrv::TimeObject( p.ui );
        p.settingsObject = new SettingsObject( p.timeObject );


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
                    // _p->outputDevice->setHDR(value.hdrMode, value.hdrData);
                });

        p.ui->uiTimeline->setTimeObject( p.timeObject );
        p.ui->uiFrame->setTimeObject( p.timeObject );
        p.ui->uiStartFrame->setTimeObject( p.timeObject );
        p.ui->uiEndFrame->setTimeObject( p.timeObject );

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
                if (p.options.inOutRange != time::invalidTimeRange)
                {
                    player->setInOutRange(p.options.inOutRange);
                    player->seek(p.options.inOutRange.start_time());
                }
                if (p.options.seek != time::invalidTime)
                {
                    player->seek(p.options.seek);
                }

                p.ui->uiTimeline->setColorConfigOptions( p.options.colorConfigOptions );

            }
        }
        else
        {
            const auto fps = 24.0;
            const auto& startTime = otio::RationalTime( 1.0, fps );
            const auto& duration  = otio::RationalTime( 50.0, fps );
            p.ui->uiFrame->setTime( startTime );
            p.ui->uiStartFrame->setTime( startTime );
            p.ui->uiEndFrame->setTime(
                startTime + duration - otio::RationalTime( 1.0,
                                                           duration.rate() ) );
        }


        Preferences prefs( p.ui->uiPrefs, p.options.resetSettings );
        Preferences::run( p.ui );

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

    int App::run()
    {
        TLRENDER_P();
        Fl::check();
        if ( !p.timelinePlayers.empty() )
        {
            const auto& player = p.timelinePlayers[0];
            player->setPlayback( p.options.playback );
        }
        p.running = true;
        return Fl::run();
    }


    void App::open( const std::string& fileName,
                    const std::string& audioFileName )
    {
        TLRENDER_P();

        char file[1024];
        fl_filename_absolute( file, 1024, fileName.c_str() );

        char audioFile[1024]; audioFile[0] = 0;
        if ( !audioFileName.empty() )
        {
            fl_filename_absolute( audioFile, 1024, audioFileName.c_str() );
        }

        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = std_any_cast<int>(
            p.settingsObject->value("Misc/MaxFileSequenceDigits") );
        for (const auto& path : timeline::getPaths(file, pathOptions,
                                                   _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFile);
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

    void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
    {
        TLRENDER_P();


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
            p.active[0]->volume = p.timelinePlayers[0]->volume();
            p.active[0]->mute = p.timelinePlayers[0]->isMuted();
            p.active[0]->audioOffset = p.timelinePlayers[0]->audioOffset();
       }

        std::vector<TimelinePlayer*> timelinePlayers(items.size(), nullptr);
        auto audioSystem = _context->getSystem<audio::System>();
        for (size_t i = 0; i < items.size(); ++i)
        {
            TimelinePlayer* mrvTimelinePlayer = nullptr;

            // Find the item in the mapping to timelinePlayer
            auto it = p.itemsMapping.find(items[i]);

            if ( it != p.itemsMapping.end() )
            {
                auto player = it->second;
                DBGM2( "Item " << i << " has timeline " << it->second );
                // Check the timelinePlayers for this timeline player
                auto ip = std::find( timelinePlayers.begin(),
                                     timelinePlayers.end(), player );

                if ( ip == timelinePlayers.end() )
                {
                    timelinePlayers[i] = player;
                    continue;
                }
            }

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


                options.ioOptions["ffmpeg/YUVToRGBConversion"] =
                    string::Format("{0}").
                    arg( std_any_cast<int>(
                             p.settingsObject->value("Performance/FFmpegYUVToRGBConversion") ) );

                const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

                options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg( std_any_cast<int>( p.settingsObject->value( "Performance/FFmpegThreadCount" ) ) );

                options.pathOptions.maxNumberDigits = std::min( std_any_cast<int>( p.settingsObject->value("Misc/MaxFileSequenceDigits") ),
                                                                255 );



                auto timeline = items[i]->audioPath.isEmpty() ?
                                timeline::Timeline::create(items[i]->path.get(),
                                                           _context, options) :
                                timeline::Timeline::create(items[i]->path.get(),
                                                           items[i]->audioPath.get(),
                                                           _context, options);
                auto& info = timeline->getIOInfo();
                if ( info.video.empty() )
                    throw std::runtime_error(string::Format("{0}: Error reading file").arg(items[i]->path.get()));
                p.settingsObject->addRecentFile(items[i]->path.get());

                timeline::PlayerOptions playerOptions;
                playerOptions.cacheReadAhead = _cacheReadAhead();
                playerOptions.cacheReadBehind = _cacheReadBehind();


                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/TimerMode") );
                playerOptions.timerMode = (timeline::TimerMode) value;
                value = std_any_cast<int>(
                    p.settingsObject->value("Performance/AudioBufferFrameCount") );
                playerOptions.audioBufferFrameCount = (timeline::AudioBufferFrameCount) value;

                auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                mrvTimelinePlayer = new mrv::TimelinePlayer(timelinePlayer, _context);
                // Don't overwrite a previous timeline (to keep
                // annotations)
                if ( it == p.itemsMapping.end() )
                {
                    p.itemsMapping[items[i]] = mrvTimelinePlayer;
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
            timelinePlayers[i] = mrvTimelinePlayer;
        }

        if (!items.empty() &&
            !timelinePlayers.empty() &&
            timelinePlayers[0])
        {
            items[0]->timeRange = timelinePlayers[0]->timeRange();
            items[0]->ioInfo = timelinePlayers[0]->ioInfo();
            if (!items[0]->init)
            {
                items[0]->init = true;
                items[0]->speed = timelinePlayers[0]->speed();
                items[0]->playback = timelinePlayers[0]->playback();
                items[0]->loop = timelinePlayers[0]->loop();
                items[0]->currentTime = timelinePlayers[0]->currentTime();
                items[0]->inOutRange = timelinePlayers[0]->inOutRange();
                items[0]->videoLayer = timelinePlayers[0]->videoLayer();
                items[0]->volume = timelinePlayers[0]->volume();
                items[0]->mute = timelinePlayers[0]->isMuted();
                items[0]->audioOffset = timelinePlayers[0]->audioOffset();
            }
            else
            {
                timelinePlayers[0]->setAudioOffset(items[0]->audioOffset);
                timelinePlayers[0]->setMute(items[0]->mute);
                timelinePlayers[0]->setVolume(items[0]->volume);
                timelinePlayers[0]->setVideoLayer(items[0]->videoLayer);
                timelinePlayers[0]->setSpeed(items[0]->speed);
                timelinePlayers[0]->setLoop(items[0]->loop);
                timelinePlayers[0]->setInOutRange(items[0]->inOutRange);
                timelinePlayers[0]->seek(items[0]->currentTime);
                timelinePlayers[0]->setPlayback(items[0]->playback);
          }
        }

        for (size_t i = 1; i < items.size(); ++i)
        {
            if (timelinePlayers[i])
            {
                timelinePlayers[i]->setVideoLayer(items[i]->videoLayer);
            }
        }


        for ( auto player : p.timelinePlayers )
        {
            player->stop();
            player->timelinePlayer()->setExternalTime(nullptr);
        }

        std::vector<mrv::TimelinePlayer*> timelinePlayersValid;
        for (const auto& i : timelinePlayers)
        {
            if ( i )
            {
                if (!timelinePlayersValid.empty())
                {
                    i->timelinePlayer()->setExternalTime(timelinePlayersValid[0]->timelinePlayer());
                }
                timelinePlayersValid.push_back(i);
            }
        }

        if ( p.ui )
        {
            Viewport* primary = p.ui->uiView;
            primary->setTimelinePlayers( timelinePlayersValid );
            if ( p.ui->uiSecondary )
            {
                Viewport* view = p.ui->uiSecondary->viewport();
                view->setColorConfigOptions( primary->getColorConfigOptions() );
                view->setLUTOptions( primary->lutOptions() );
                view->setImageOptions( primary->getImageOptions() );
                view->setDisplayOptions( primary->getDisplayOptions() );
                view->setCompareOptions( primary->getCompareOptions() );
                view->setTimelinePlayers( timelinePlayersValid,  false );
                view->frameView();
            }
        }

        p.active = items;

        p.timelinePlayers = timelinePlayersValid;


        // Cleanup the deleted TimelinePlayers that are no longer attached
        // to a valid clip.
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


        if ( p.ui )
        {
            TimelinePlayer* player = nullptr;
            p.ui->uiAudioTracks->clear();

            if ( !p.timelinePlayers.empty() )
            {

                player = timelinePlayers[0];

                p.ui->uiFPS->value( player->speed() );

                p.ui->uiTimeline->setTimelinePlayer( player );
                if ( colorTool ) colorTool->refresh();
                if ( imageInfoTool )
                {
                    imageInfoTool->setTimelinePlayer( player );
                    imageInfoTool->refresh();
                }

                const auto timeRange = player->timeRange();
                p.ui->uiFrame->setTime( player->currentTime() );
                p.ui->uiStartFrame->setTime( timeRange.start_time() );
                p.ui->uiEndFrame->setTime( timeRange.end_time_inclusive() );

                // Set the audio tracks
                const auto timeline = player->timeline();
                const auto  ioinfo = timeline->getIOInfo();
                const auto audio = ioinfo.audio;
                const auto name = audio.name;
                int mode = FL_MENU_RADIO;
                p.ui->uiAudioTracks->add( _("Mute"), 0, 0, 0, mode );
                int idx = p.ui->uiAudioTracks->add( name.c_str(), 0, 0, 0,
                                                    mode | FL_MENU_VALUE );


                p.ui->uiMain->show();

                size_t numFiles = filesModel()->observeFiles()->getSize();
                if ( numFiles == 1 )
                  {
                    // resize the window to the size of the first clip loaded
                    p.ui->uiView->resizeWindow();
                    p.ui->uiView->take_focus();
                  }

                p.ui->uiLoopMode->value( (int)p.options.loop );
                p.ui->uiLoopMode->do_callback();



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

        _cacheUpdate();



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
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        double value = std_any_cast<double>( p.settingsObject->value( "Cache/ReadBehind" ) );
        return otime::RationalTime( value / static_cast<double>(activeCount),
                                    1.0);
    }

    void App::_cacheUpdate()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            if (i)
            {
                i->setCacheReadAhead(_cacheReadAhead());
                i->setCacheReadBehind(_cacheReadBehind());
            }
        }
    }
}
