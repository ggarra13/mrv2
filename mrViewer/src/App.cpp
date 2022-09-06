// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include "mrvFlGL/mrvGLViewport.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrViewer.h"

#include <FL/Fl.H>

namespace mrv
{
    struct Options
    {
        imaging::Size windowSize = imaging::Size(1280, 720);
        bool fullScreen = false;
        bool hud = true;
        bool startPlayback = true;
        bool loopPlayback = true;
        imaging::ColorConfig colorConfig;
    };

    struct App::Private
    {
        Options options;

        std::string input;

        std::vector<TimelinePlayer*> timelinePlayers;

        std::unique_ptr<ViewerUI> ui = nullptr;

        bool running = true;
    };

    void App::_init(
        int argc,
        char* argv[],
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        IApp::_init(
            argc,
            argv,
            context,
            "mrViewer",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    p.input,
                    "input",
                    "The input timeline.")
            },
            {
                app::CmdLineValueOption<imaging::Size>::create(
                    p.options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").
                    arg(p.options.windowSize.w).arg(p.options.windowSize.h)),
                app::CmdLineFlagOption::create(
                    p.options.fullScreen,
                    { "-fullScreen", "-fs" },
                    "Enable full screen mode."),
                app::CmdLineValueOption<bool>::create(
                    p.options.hud,
                    { "-hud" },
                    "Enable the HUD (heads up display).",
                    string::Format("{0}").arg(p.options.hud),
                    "0, 1"),
                app::CmdLineValueOption<bool>::create(
                    p.options.startPlayback,
                    { "-startPlayback", "-sp" },
                    "Automatically start playback.",
                    string::Format("{0}").arg(p.options.startPlayback),
                    "0, 1"),
                app::CmdLineValueOption<bool>::create(
                    p.options.loopPlayback,
                    { "-loopPlayback", "-lp" },
                    "Loop playback.",
                    string::Format("{0}").arg(p.options.loopPlayback),
                    "0, 1"),
                app::CmdLineValueOption<std::string>::create(
                    p.options.colorConfig.fileName,
                    { "-colorConfig", "-cc" },
                    "Color configuration file name (e.g., config.ocio)."),
                app::CmdLineValueOption<std::string>::create(
                    p.options.colorConfig.input,
                    { "-colorInput", "-ci" },
                    "Input color space."),
                app::CmdLineValueOption<std::string>::create(
                    p.options.colorConfig.display,
                    { "-colorDisplay", "-cd" },
                    "Display color space."),
                app::CmdLineValueOption<std::string>::create(
                    p.options.colorConfig.view,
                    { "-colorView", "-cv" },
                    "View color space.")
            });
    }

    App::App() :
        _p( new Private )
    {}

    App::~App()
    {
        TLRENDER_P();
        p.ui.reset();
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

        TLRENDER_P();

        // Turn off visible widget to have focus as it messes view window
        Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );

        // Read the timeline.
        timeline::Options options;
        auto audioSystem = _context->getSystem<audio::System>();
        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
        options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
        options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
        options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);

        auto timeline = timeline::Timeline::create(p.input, _context, options);
        auto timelinePlayer = timeline::TimelinePlayer::create(timeline,
                                                               _context);

        // Initialize FLTK.
        // Create the window.
        int X = 0, Y = 0;
        int W = p.options.windowSize.w;
        int H = p.options.windowSize.h;
        p.ui = std::make_unique< ViewerUI >();
        if (!p.ui)
        {
            throw std::runtime_error("Cannot create window");
        }
        p.ui->uiView->setContext( _context );

        p.ui->uiMain->show();
        p.ui->uiView->take_focus();

        // @todo: handle multiple timelinePlayers
        std::vector<TimelinePlayer*> timelinePlayers(1, nullptr);

        TimelinePlayer* flTimelinePlayer = nullptr;
        flTimelinePlayer = new TimelinePlayer(timelinePlayer, _context);
        std::shared_ptr< GLViewport > view( p.ui->uiView );
        flTimelinePlayer->setTimelineViewport( view );
        timelinePlayers[0] = flTimelinePlayer;

        // Store all the players in gl view
        p.ui->uiView->setTimelinePlayers( timelinePlayers );

        // Start playback @todo: handle preferences setting
        flTimelinePlayer->setPlayback(timeline::Playback::Forward);


        while (p.ui->uiMain->shown())
        {
            Fl::check();
        }

        return 0;
    }

}
