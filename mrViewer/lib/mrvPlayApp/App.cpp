// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlTimeline/Util.h>

#include <mrvCore/mrvRoot.h>

#include <mrvFl/mrvTimeObject.h>
#include <mrvFl/mrvContextObject.h>
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvPreferences.h"
#include "mrvGL/mrvGLViewport.h"

#include "mrvPlayApp/mrvFilesModel.h"
#include <mrvPlayApp/mrvColorModel.h>

// #Include <mrvPlayApp/Devicesmodel.h>
// #include <mrvPlayApp/OpenSeparateAudioDialog.h>


#include "mrvPreferencesUI.h"
#include "mrViewer.h"



#include <FL/platform.H>  // for fl_open_callback (OSX)
#include <FL/Fl.H>

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
        bool loopPlayback = true;

        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
    };

    struct App::Private
    {
        Options options;

        ContextObject* contextObject = nullptr;
        TimeObject* timeObject = nullptr;
        // SettingsObject* settingsObject = nullptr;
        //qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;

        std::shared_ptr<FilesModel> filesModel;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > activeObserver;
        std::vector<std::shared_ptr<FilesModelItem> > active;

        std::shared_ptr<observer::ListObserver<int> > layersObserver;
        std::shared_ptr<ColorModel> colorModel;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        // OutputDevice* outputDevice = nullptr;
        // std::shared_ptr<DevicesModel> devicesModel;
        // std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;

        ViewerUI*                 ui = nullptr;

        std::vector<TimelinePlayer*> timelinePlayers;


        bool running = true;
    };

    void App::_init(
        int argc,
        char* argv[],
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();


        set_root_path( argc, argv );


        IApp::_init(
            argc,
            argv,
            context,
            "mrViewer",
            "Play an opentimelineio timeline, a movie or a flipbook.",
            {
                app::CmdLineValueArg<std::string>::create(
                        p.options.fileName,
                        "filename",
                        "Timeline, movie, image sequence, or folder.",
                        true)
            },
            {
            app::CmdLineValueOption<std::string>::create(
                p.options.audioFileName,
                { "-audio", "-a" },
                "Audio file."),
            app::CmdLineValueOption<std::string>::create(
                p.options.compareFileName,
                { "-compare", "-b" },
                "A/B comparison \"B\" file."),
            app::CmdLineValueOption<timeline::CompareMode>::create(
                p.options.compareMode,
                { "-compareMode", "-c" },
                "A/B comparison mode.",
                string::Format("{0}").arg(p.options.compareMode),
                string::join(timeline::getCompareModeLabels(), ", ")),
            app::CmdLineValueOption<int>::create(
                Preferences::debug,
                { "-debug", "-d" },
                "Print debugging statements.",
                string::Format("{0}").arg(Preferences::debug)),
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
            app::CmdLineValueOption<std::string>::create(
                p.options.colorConfigOptions.fileName,
                { "-colorConfig", "-cc" },
                "Color configuration file (config.ocio)."),
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
                "View color space.")
            });

        // const int exitCode = getExit();
        // if (exitCode != 0)
        // {
        //     exit(exitCode);
        //     return;
        // }

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


        p.colorModel = ColorModel::create(context);
        if (!p.options.colorConfigOptions.fileName.empty())
        {
            p.colorModel->setConfigOptions(p.options.colorConfigOptions);
        }

        p.lutOptions = p.options.lutOptions;

        Fl::scheme("gtk+");
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );
        Fl::use_high_res_GL(true);
        fl_open_display();

        // Read the timeline.
        timeline::Options options;
        auto audioSystem = _context->getSystem<audio::System>();
        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
        options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
        options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
        options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);



        // Initialize FLTK.
        // Create the window.

        p.ui = new ViewerUI();

        if (!p.ui)
        {
            throw std::runtime_error("Cannot create window");
        }
        p.ui->uiView->setContext( _context );
        p.timeObject = new mrv::TimeObject( p.ui );

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
                open( p.options.compareFileName.c_str() );
            }


            open( p.options.fileName.c_str(), p.options.audioFileName.c_str());


            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                TimelinePlayer* player = p.timelinePlayers[0];


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

                player->setTimelineViewport( p.ui->uiView );


                p.ui->uiTimeline->setTimelinePlayer( player );
                p.ui->uiTimeline->setTimeObject( p.timeObject );
                p.ui->uiFrame->setTimeObject( p.timeObject );
                p.ui->uiStartFrame->setTimeObject( p.timeObject );
                p.ui->uiEndFrame->setTimeObject( p.timeObject );

                const auto& startTime = player->globalStartTime();
                const auto& duration  = player->duration();
                p.ui->uiFrame->setTime( startTime );
                p.ui->uiStartFrame->setTime( startTime );
                p.ui->uiEndFrame->setTime( startTime + duration -
                                           otio::RationalTime( 1.0,
                                                               duration.rate() ) );

                p.ui->uiTimeline->setColorConfigOptions( p.options.colorConfigOptions );

                // Store all the players in gl view

                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                for ( const auto& t : p.timelinePlayers )
                {
                    imageOptions.push_back( p.imageOptions );
                    displayOptions.push_back( p.displayOptions );
                }

                p.ui->uiView->setImageOptions( imageOptions );
                p.ui->uiView->setDisplayOptions( displayOptions );

            }
        }


        // show window to get its decorated size

        p.ui->uiMain->show();

        // resize window to its maximum size according to first image loaded
        p.ui->uiView->resizeWindow();
        p.ui->uiView->take_focus();

        // Start playback (after window is shown)
        if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
        {
            TimelinePlayer* player = p.timelinePlayers[0];
            player->setLoop(p.options.loop);
            player->setPlayback(p.options.playback);
        }
    }

    App::App() :
        _p( new Private )
    {}

    App::~App()
    {
        TLRENDER_P();
        delete p.ui;
    }


    TimeObject* App::timeObject() const
    {
        return _p->timeObject;
    }

    const std::shared_ptr<FilesModel>& App::filesModel() const
    {
        return _p->filesModel;
    }

    const std::shared_ptr<ColorModel>& App::colorModel() const
    {
        return _p->colorModel;
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



    std::shared_ptr<App> App::create(
        int argc,
        char* argv[],
        const std::shared_ptr<system::Context>& context)
    {

        auto out = std::shared_ptr<App>(new App);

        out->_init(argc, argv, context);

        return out;
    }


    int App::run()
    {
        return Fl::run();
    }


    void App::open(const std::string& fileName,
                   const std::string& audioFileName)
    {
        TLRENDER_P();
        file::PathOptions pathOptions;
        pathOptions.maxNumberDigits = 255; // @prefs @todo: p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt();
        for (const auto& path : timeline::getPaths(fileName, pathOptions, _context))
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = path;
            item->audioPath = file::Path(audioFileName);
            p.filesModel->add(item);
            // p.settingsObject->addRecentFile(QString::fromUtf8(path.get().c_str()));
        }
    }

    // void App::openDialog()
    // {
    //     TLRENDER_P();

    //     std::vector<std::string> extensions;
    //     for (const auto& i : timeline::getExtensions(
    //              static_cast<int>(io::FileType::Movie) |
    //              static_cast<int>(io::FileType::Sequence) |
    //              static_cast<int>(io::FileType::Audio),
    //              _context))
    //     {
    //         extensions.push_back("*" + i);
    //     }

    //     std::string dir;
    //     if (!p.active.empty())
    //     {
    //         dir = std::string::fromUtf8(p.active[0]->path.get().c_str());
    //     }

    //     const auto fileName = QFileDialog::getOpenFileName(
    //         p.mainWindow,
    //         tr("Open"),
    //         dir,
    //         tr("Files") + " (" + std::string::fromUtf8(string::join(extensions, " ").c_str()) + ")");
    //     if (!fileName.isEmpty())
    //     {
    //         open(fileName);
    //     }
    // }

    // void App::openSeparateAudioDialog()
    // {
    //     auto dialog = std::make_unique<OpenSeparateAudioDialog>(_context);
    //     if (QDialog::Accepted == dialog->exec())
    //     {
    //         open(dialog->videoFileName(), dialog->audioFileName());
    //     }
    // }

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

                    // options.fileSequenceAudio = p.settingsObject->value("FileSequence/Audio").
                    //                             value<timeline::FileSequenceAudio>();
                    // options.fileSequenceAudioFileName = p.settingsObject->value("FileSequence/AudioFileName").
                    //                                     toString().toUtf8().data();
                    // options.fileSequenceAudioDirectory = p.settingsObject->value("FileSequence/AudioDirectory").
                    //                                      toString().toUtf8().data();
                    // options.videoRequestCount = p.settingsObject->value("Performance/VideoRequestCount").toInt();
                    // options.audioRequestCount = p.settingsObject->value("Performance/AudioRequestCount").toInt();
                    // options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").
                    //                                               arg(p.settingsObject->value("Performance/SequenceThreadCount").toInt());
                    // options.ioOptions["ffmpeg/YUVToRGBConversion"] = string::Format("{0}").
                    //                                                  arg(p.settingsObject->value("Performance/FFmpegYUVToRGBConversion").toBool());

                    const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                    options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                    options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                    options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

                    // options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").
                    //                                           arg(p.settingsObject->value("Performance/FFmpegThreadCount").toInt());

                    // options.pathOptions.maxNumberDigits = std::min(
                    //     p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt(),
                    //     255);
                    auto timeline = items[i]->audioPath.isEmpty() ?
                                    timeline::Timeline::create(items[i]->path.get(), _context, options) :
                                    timeline::Timeline::create(items[i]->path.get(), items[i]->audioPath.get(), _context, options);

                    timeline::PlayerOptions playerOptions;
                    playerOptions.cacheReadAhead = _cacheReadAhead();
                    playerOptions.cacheReadBehind = _cacheReadBehind();

                    // playerOptions.timerMode = p.settingsObject->value("Performance/TimerMode").
                    //                           value<timeline::TimerMode>();
                    // playerOptions.audioBufferFrameCount = p.settingsObject->value("Performance/AudioBufferFrameCount").
                    //                                       value<timeline::AudioBufferFrameCount>();
                    auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                    mrvTimelinePlayer = new mrv::TimelinePlayer(timelinePlayer, _context);
                }
                catch (const std::exception& e)
                {
                    _log(e.what(), log::Type::Error);
                }
                timelinePlayers[i] = mrvTimelinePlayer;
            }
        }

        if (!items.empty() &&
            !timelinePlayers.empty() &&
            timelinePlayers[0])
        {
            items[0]->duration = timelinePlayers[0]->duration();
            items[0]->globalStartTime = timelinePlayers[0]->globalStartTime();
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
            p.ui->uiFPS->value( timelinePlayers[0]->speed() );
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
        if (p.ui)
        {
            p.ui->uiView->setTimelinePlayers(timelinePlayersValid);
        }

        p.active = items;
        for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
        {
            delete p.timelinePlayers[i];
        }
        p.timelinePlayers = timelinePlayers;

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
