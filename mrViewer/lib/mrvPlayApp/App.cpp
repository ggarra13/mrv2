// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#ifdef _WIN32
#  define NOMINMAX
#endif

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlTimeline/Util.h>

#include <FL/platform.H>  // for fl_open_callback (OSX)
#include <FL/Fl.H>

#include <mrvCore/mrvRoot.h>

#include <mrvFl/mrvTimeObject.h>
#include <mrvFl/mrvContextObject.h>
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvGL/mrvGLViewport.h"

#include "mrvPlayApp/mrvFilesModel.h"
#include <mrvPlayApp/mrvColorModel.h>
#include <mrvPlayApp/mrvSettingsObject.h>

// #Include <mrvPlayApp/Devicesmodel.h>
#include <mrvPlayApp/mrvOpenSeparateAudioDialog.h>


#include "mrvPreferencesUI.h"
#include "mrViewer.h"

#include <mrvFl/mrvIO.h>

namespace {
    const char* kModule = "app";
}

namespace mrv
{
    
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
        std::vector<std::shared_ptr<FilesModelItem> > active;

        std::shared_ptr<observer::ListObserver<int> > layersObserver;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        // OutputDevice* outputDevice = nullptr;
        // std::shared_ptr<DevicesModel> devicesModel;
        // std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;

        ViewerUI*                 ui = nullptr;

        std::vector<TimelinePlayer*> timelinePlayers;


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

        DBG;
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        DBG;

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
                for (size_t i = 0; i < value.size() && i < _p->timelinePlayers.size(); ++i)
                {
                    if (_p->timelinePlayers[i])
                    {
                        _p->timelinePlayers[i]->setVideoLayer(value[i]);
                    }
                }

            });

        DBG;

        p.lutOptions = p.options.lutOptions;




        DBG;
        // Read the timeline.
        timeline::Options options;
        auto audioSystem = _context->getSystem<audio::System>();
        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
        options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
        options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
        options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);


        DBG;

        // Initialize FLTK.
        Fl::scheme("gtk+");
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );
        Fl::use_high_res_GL(true);

        // Store the application object for further use down the line
        ViewerUI::app = this;
        
        // Create the window.
        p.ui = new ViewerUI();

        if (!p.ui)
        {
            throw std::runtime_error("Cannot create window");
        }
        p.ui->uiView->setContext( _context );
        p.ui->uiTimeline->setContext( _context );

        p.ui->uiMain->main( p.ui );

        p.timeObject = new mrv::TimeObject( p.ui );
        p.settingsObject = new SettingsObject( p.options.resetSettings,
                                               p.timeObject );
        mrv::Preferences prefs( p.ui->uiPrefs, p.settingsObject );
        mrv::Preferences::run( p.ui );
        
        DBG;

        p.ui->uiTimeline->setTimeObject( p.timeObject );
        p.ui->uiFrame->setTimeObject( p.timeObject );
        p.ui->uiStartFrame->setTimeObject( p.timeObject );
        p.ui->uiEndFrame->setTimeObject( p.timeObject );

        std_any value;
        int visible;
        
        value = p.settingsObject->value( "gui/Color/Window/Visible" );
        visible = value.empty() ? 0 : std_any_cast< int >( value );
        if ( visible ) color_tool_grp( nullptr, p.ui );
        
        value = p.settingsObject->value( "gui/Reel/Window/Visible" );
        visible = value.empty() ? 0 : std_any_cast< int >( value );
        if ( visible ) reel_tool_grp( nullptr, p.ui );
        
        value = p.settingsObject->value( "gui/Compare/Window/Visible" );
        visible = value.empty() ? 0 : std_any_cast< int >( value );
        if ( visible ) compare_tool_grp( nullptr, p.ui );
        
        value = p.settingsObject->value( "gui/Settings/Window/Visible" );
        visible = value.empty() ? 0 : std_any_cast< int >( value );
        if ( visible ) settings_tool_grp( nullptr, p.ui );

        DBG;
        // Open the input files.
        if (!p.options.fileName.empty())
        {
        DBG;
            if (!p.options.compareFileName.empty())
            {
        DBG;
                timeline::CompareOptions compareOptions;
                compareOptions.mode = p.options.compareMode;
                compareOptions.wipeCenter = p.options.wipeCenter;
                compareOptions.wipeRotation = p.options.wipeRotation;
                p.filesModel->setCompareOptions(compareOptions);
                open( p.options.compareFileName.c_str() );
            }

        DBG;

            open( p.options.fileName.c_str(), p.options.audioFileName.c_str());


        DBG;
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


        DBG;
        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
        DBG;
        p.ui->uiMain->show();
        p.ui->uiView->take_focus();



    }

    App::~App()
    {
        TLRENDER_P();
        delete p.contextObject;
        delete p.timeObject;
        delete p.ui;
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
        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = std_any_cast<int>(
            p.settingsObject->value("Misc/MaxFileSequenceDigits") );
        for (const auto& path : timeline::getPaths(fileName, pathOptions, _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFileName);
            p.filesModel->add(item);
            // p.settingsObject->addRecentFile(path.get().c_str());
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

	bool start = false;
	if ( p.running ) start = true;

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

            if (i < p.active.size() && items[i] == p.active[i])
            {
                timelinePlayers[i] = p.timelinePlayers[i];
                p.timelinePlayers[i] = nullptr;
            }
            else
            {
                TimelinePlayer* mrvTimelinePlayer = nullptr;
                try
                {
                    timeline::Options options;
                    DBG;
                    int value = std_any_cast<int>( p.settingsObject->value("FileSequence/Audio") );
                    DBG;
                    options.fileSequenceAudio = (timeline::FileSequenceAudio)
                        value;
                    DBG;
                    std_any v = p.settingsObject->value("FileSequence/AudioFileName");
                    options.fileSequenceAudioFileName = std_any_cast<std::string>( v );
                    DBG;
                    options.fileSequenceAudioDirectory = std_any_cast<std::string>( p.settingsObject->value("FileSequence/AudioDirectory") );
                    options.videoRequestCount = (int)p.ui->uiPrefs->uiPrefsVideoRequestCount->value();
                    DBG;
                    options.audioRequestCount = (int)p.ui->uiPrefs->uiPrefsAudioRequestCount->value();
                    DBG;
                    options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg((int)p.ui->uiPrefs->uiPrefsSequenceThreadCount->value());

                    DBG;
                    options.ioOptions["ffmpeg/YUVToRGBConversion"] =
                        string::Format("{0}").
                        arg( std_any_cast<int>(
                                 p.settingsObject->value("Performance/FFmpegYUVToRGBConversion") ) );
                    DBG;
                    const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                    options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                    options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                    options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

                    options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg((int)p.ui->uiPrefs->uiPrefsFFmpegThreadCount->value());
                    DBG;
                    options.pathOptions.maxNumberDigits = std::min( std_any_cast<int>( p.settingsObject->value("Misc/MaxFileSequenceDigits") ),
                                                                    255 );

                    DBG;

                    auto timeline = items[i]->audioPath.isEmpty() ?
                                    timeline::Timeline::create(items[i]->path.get(),
                                                               _context, options) :
                                    timeline::Timeline::create(items[i]->path.get(),
                                                               items[i]->audioPath.get(),
                                                               _context, options);


                    timeline::PlayerOptions playerOptions;
                    playerOptions.cacheReadAhead = _cacheReadAhead();
                    playerOptions.cacheReadBehind = _cacheReadBehind();

                    DBG;
                    value = std_any_cast<int>(
                        p.settingsObject->value("Performance/TimerMode") );
                    playerOptions.timerMode = (timeline::TimerMode) value;
                    value = std_any_cast<int>(
                        p.settingsObject->value("Performance/AudioBufferFrameCount") );
                    playerOptions.audioBufferFrameCount = (timeline::AudioBufferFrameCount) value;
                    DBG;
                    auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                    mrvTimelinePlayer = new mrv::TimelinePlayer(timelinePlayer, _context);
                    timelinePlayers[i] = mrvTimelinePlayer;
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    _log(e.what(), log::Type::Error);

                }
            }
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


        std::vector<mrv::TimelinePlayer*> timelinePlayersValid;
        for (const auto& i : timelinePlayers)
        {
            if (i)
            {
                if (!timelinePlayersValid.empty())
                {
                    i->timelinePlayer()->setExternalTime(timelinePlayersValid[0]->timelinePlayer());
                }
                timelinePlayersValid.push_back(i);
            }
        }



        p.active = items;
        for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
        {

            delete p.timelinePlayers[i];
        }

        p.timelinePlayers = timelinePlayersValid;

        if ( p.ui )
        {

            p.ui->uiView->setTimelinePlayers( p.timelinePlayers );


            TimelinePlayer* player = nullptr;
            p.ui->uiAudioTracks->clear();

            if ( !p.timelinePlayers.empty() )
            {

                player = timelinePlayers[0];
                
                p.ui->uiFPS->value( player->speed() );

                p.ui->uiInfo->uiInfoText->setTimelinePlayer( player );
                p.ui->uiTimeline->setTimelinePlayer( player );
		if ( colorTool ) colorTool->refresh();

                const auto& timeRange = player->timeRange();
                p.ui->uiFrame->setTime( timeRange.start_time() );
                p.ui->uiStartFrame->setTime( timeRange.start_time() );
                p.ui->uiEndFrame->setTime( timeRange.end_time_inclusive() );

                // Set the audio tracks
                const auto& timeline = player->timeline();
                const auto&  ioinfo = timeline->getIOInfo();
                const auto& audio = ioinfo.audio;
                const auto& name = audio.name;
                int mode = FL_MENU_RADIO;
                p.ui->uiAudioTracks->add( _("Mute"), 0, 0, 0, mode );
                int idx = p.ui->uiAudioTracks->add( name.c_str(), 0, 0, 0,
                                                    mode | FL_MENU_VALUE );

                // resize the window to the size of the first clip loaded
                p.ui->uiMain->show();
                p.ui->uiView->resizeWindow();
                p.ui->uiView->take_focus();

                p.ui->uiLoopMode->value( (int)p.options.loop );
                p.ui->uiLoopMode->do_callback();



                std::vector<timeline::ImageOptions>& imageOptions =
                    p.ui->uiView->getImageOptions();
                std::vector<timeline::DisplayOptions>& displayOptions =
                    p.ui->uiView->getDisplayOptions();
                imageOptions.resize( p.timelinePlayers.size() );
                displayOptions.resize( p.timelinePlayers.size() );

		if ( start )
		  {
		    // We don't start playback here if fltk is not running
		    player->setPlayback( p.options.playback );
		  }
            }
        }

        _cacheUpdate();



    }


    otime::RationalTime App::_cacheReadAhead() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();
        return otime::RationalTime(
            p.ui->uiPrefs->uiPrefsCacheReadAhead->value() /
            static_cast<double>(activeCount), 1.0);
    }

    otime::RationalTime App::_cacheReadBehind() const
    {
        TLRENDER_P();
        const size_t activeCount = p.filesModel->observeActive()->getSize();

        return otime::RationalTime(
            p.ui->uiPrefs->uiPrefsCacheReadBehind->value() /
            static_cast<double>(activeCount), 1.0);
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
