// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <mrvCore/mrvRoot.h>

#include <mrvFl/mrvTimeObject.h>
#include <mrvFl/mrvContextObject.h>
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvGL/mrvGLViewport.h"

#include "mrvPlayApp/mrvFilesModel.h"


#include "mrViewer.h"

#include <FL/platform.H>  // for fl_open_callback (OSX)
#include <FL/Fl.H>

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

        bool fullScreen = false;
        bool hud = true;
        bool loopPlayback = true;

        timeline::ColorConfigOptions colorConfigOptions;
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
        //std::shared_ptr<ColorModel> colorModel;
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
                        "input",
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

        p.contextObject = new mrv::ContextObject(context);

        Fl::scheme("gtk+");
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );
        Fl::use_high_res_GL(true);

        // Read the timeline.
        timeline::Options options;
        auto audioSystem = _context->getSystem<audio::System>();
        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
        options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
        options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
        options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

        auto timeline = timeline::Timeline::create(p.options.fileName,
                                                   _context, options);
        auto timelinePlayer = timeline::TimelinePlayer::create(timeline,
                                                               _context);

        // Initialize FLTK.
        // Create the window.
        p.ui = new ViewerUI();
        if (!p.ui)
        {
            throw std::runtime_error("Cannot create window");
        }
        p.timeObject = new mrv::TimeObject( p.ui );
        p.ui->uiView->setContext( _context );

        // @todo: handle multiple timelinePlayers
        std::vector<TimelinePlayer*> timelinePlayers(1, nullptr);

        TimelinePlayer* player = nullptr;
        player = new TimelinePlayer(timelinePlayer, _context);
        timelinePlayers[0] = player;

        p.timelinePlayers = timelinePlayers;

        std::vector<timeline::ImageOptions> imageOptions;
        std::vector<timeline::DisplayOptions> displayOptions;
        for (const auto& i : p.timelinePlayers)
        {
            imageOptions.push_back(p.imageOptions);
            displayOptions.push_back(p.displayOptions);
        }


        player->setTimelineViewport( p.ui->uiView );

        p.ui->uiFrame->setTimeObject( p.timeObject );
        p.ui->uiStartFrame->setTimeObject( p.timeObject );
        p.ui->uiEndFrame->setTimeObject( p.timeObject );

        p.ui->uiFrame->setTime( player->globalStartTime() );
        p.ui->uiStartFrame->setTime( player->globalStartTime() );
        p.ui->uiEndFrame->setTime( player->globalStartTime() +
                                   player->duration() );

        p.ui->uiTimeline->setTimeObject( p.timeObject );
        p.ui->uiTimeline->setColorConfigOptions( p.options.colorConfigOptions );
        p.ui->uiTimeline->setTimelinePlayer( timelinePlayers[0] );

        // Store all the players in gl view
        p.ui->uiView->setTimelinePlayers( timelinePlayers );
        p.ui->uiView->setImageOptions( imageOptions );
        p.ui->uiView->setDisplayOptions( displayOptions );

        // show window to get its decorated size
        p.ui->uiMain->show();

        // resize window to its maximum size
        p.ui->uiView->resizeWindow();
        p.ui->uiView->take_focus();

        // Start playback @todo: handle preferences setting
        player->setLoop(p.options.loop);
        player->setPlayback(p.options.playback);
    }

    App::App() :
        _p( new Private )
    {}

    App::~App()
    {
        TLRENDER_P();
        delete p.ui;
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

}
