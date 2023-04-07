// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "mrViewer.h"

#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvDevicesModel.h"
#include "mrvApp/mrvMainControl.h"
#include "mrvApp/App.h"

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace mrv
{
    namespace
    {
        const size_t sliderSteps = 100;
        const size_t errorTimeout = 5000;
        const size_t infoLabelMax = 24;
    } // namespace

    struct MainControl::Private
    {
        ViewerUI* ui = nullptr;

        std::vector<mrv::TimelinePlayer*> timelinePlayers;
        bool floatOnTop = false;
        bool secondaryFloatOnTop = false;
        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        timeline::CompareOptions compareOptions;
        imaging::VideoLevels outputVideoLevels;
        float volume = 1.F;
        bool mute = false;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
        std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
            compareOptionsObserver;
        std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> >
            colorConfigOptionsObserver;
        std::shared_ptr<observer::ValueObserver<DevicesModelData> >
            devicesModelObserver;
    };

    MainControl::MainControl(ViewerUI* ui) :
        _p(new Private)
    {
        TLRENDER_P();

        p.ui = ui;

        App* app = ui->app;

        p.lutOptions = app->lutOptions();
        p.imageOptions = app->imageOptions();
        p.displayOptions = app->displayOptions();
        p.volume = app->volume();
        p.mute = app->isMuted();

        p.filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                { _widgetUpdate(); });
        p.aIndexObserver = observer::ValueObserver<int>::create(
            app->filesModel()->observeAIndex(),
            [this](int) { _widgetUpdate(); });
        p.bIndexesObserver = observer::ListObserver<int>::create(
            app->filesModel()->observeBIndexes(),
            [this](const std::vector<int>&) { _widgetUpdate(); });
        p.compareOptionsObserver =
            observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->compareOptions = value;
                    _widgetUpdate();
                });

        // @ttodo:

        // p.colorConfigOptionsObserver =
        // observer::ValueObserver<timeline::ColorConfigOptions>::create(
        //     app->colorModel()->observeConfigOptions(),
        //     [this](const timeline::ColorConfigOptions& value)
        //         {
        //             _p->colorConfigOptions = value;
        //             _widgetUpdate();
        //         });

        p.devicesModelObserver =
            observer::ValueObserver<DevicesModelData>::create(
                app->devicesModel()->observeData(),
                [this](const DevicesModelData& value)
                {
                    _p->outputVideoLevels = value.videoLevels;
                    _widgetUpdate();
                });
    }

    void MainControl::setTimelinePlayers(
        const std::vector<mrv::TimelinePlayer*>& timelinePlayers)
    {
        TLRENDER_P();

        p.timelinePlayers = timelinePlayers;

        _timelinePlayersUpdate();
        _widgetUpdate();
    }

    void MainControl::setLUTOptions(const timeline::LUTOptions& value)
    {
        TLRENDER_P();

        p.lutOptions = value;

        _widgetUpdate();
    }

    void MainControl::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        TLRENDER_P();

        p.displayOptions = value;

        _widgetUpdate();
    }

    void MainControl::setImageOptions(const timeline::ImageOptions& value)
    {
        TLRENDER_P();

        p.imageOptions = value;

        _widgetUpdate();
    }

    void MainControl::_timelinePlayersUpdate()
    {
        TLRENDER_P();
        p.ui->uiView->setTimelinePlayers(p.timelinePlayers);
        if (p.ui->uiSecondary)
        {
            auto view = p.ui->uiSecondary->viewport();
            view->setTimelinePlayers(p.timelinePlayers);
        }
        // p.app->outputDevice()->setTimelinePlayers(p.timelinePlayers);
    }

    void MainControl::_widgetUpdate()
    {
        TLRENDER_P();

        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles()->get();
        const size_t numFiles = files.size();
        TimelineClass* c = p.ui->uiTimeWindow;
        if (numFiles > 0)
        {
            c->uiTimeline->activate();
            c->uiFrame->activate();
            c->uiStartFrame->activate();
            c->uiEndFrame->activate();
            c->uiFPS->activate();
            c->uiStartButton->activate();
            c->uiEndButton->activate();
            c->uiLoopMode->activate();
            c->uiTimecodeSwitch->activate();
        }
        else
        {
            c->uiTimeline->deactivate();
            c->uiFrame->deactivate();
            c->uiStartFrame->deactivate();
            c->uiEndFrame->deactivate();
            c->uiVolume->deactivate();
            c->uiAudioTracks->deactivate();
            c->uiFPS->deactivate();
            c->uiStartButton->deactivate();
            c->uiEndButton->deactivate();
            c->uiLoopMode->deactivate();
            c->uiTimecodeSwitch->deactivate();
        }

        TimelinePlayer* player = nullptr;

        if (!p.timelinePlayers.empty())
        {
            player = p.timelinePlayers[0];

            const auto timeRange = player->inOutRange();
            c->uiFrame->setTime(player->currentTime());
            c->uiStartFrame->setTime(timeRange.start_time());
            c->uiEndFrame->setTime(timeRange.end_time_inclusive());

            c->uiFPS->value(player->speed());

            timeline::Loop loop =
                static_cast<timeline::Loop>(c->uiLoopMode->value());
            player->setLoop(loop);

            // Set the audio tracks
            const auto timeline = player->timeline();
            const auto ioinfo = timeline->getIOInfo();
            const auto audio = ioinfo.audio;
            if (audio.isValid())
            {
                const auto name = audio.name;
                int mode = FL_MENU_RADIO;
                c->uiAudioTracks->add(_("Mute"), 0, 0, 0, mode);
                int idx = c->uiAudioTracks->add(
                    name.c_str(), 0, 0, 0, mode | FL_MENU_VALUE);

                c->uiVolume->activate();
                c->uiAudioTracks->activate();

                if (player->isMuted())
                {
                    c->uiAudioTracks->value(0);
                }
                c->uiAudioTracks->do_callback();
                c->uiAudioTracks->redraw();

                // Set the audio volume
                c->uiVolume->value(player->volume());
                c->uiVolume->redraw();
            }
            else
            {
                delete c->uiAudioTracks->image();
                c->uiAudioTracks->image(mrv::load_svg("Mute.svg"));
            }
        }
        else
        {
            auto time = time::invalidTime;
            c->uiFrame->setTime(time);
            c->uiStartFrame->setTime(time);
            c->uiEndFrame->setTime(time);

            c->uiFPS->value(0.0);

            // Set the audio volume
            c->uiVolume->value(0);
            c->uiVolume->redraw();
        }

        Viewport* view = p.ui->uiView;

        view->setColorConfigOptions(p.colorConfigOptions);
        view->setLUTOptions(p.lutOptions);
        std::vector<timeline::ImageOptions> imageOptions;
        std::vector<timeline::DisplayOptions> displayOptions;
        for (const auto& i : p.timelinePlayers)
        {
            imageOptions.push_back(p.imageOptions);
            displayOptions.push_back(p.displayOptions);
        }
        view->setImageOptions(imageOptions);
        view->setDisplayOptions(displayOptions);
        view->setCompareOptions(p.compareOptions);
        view->setTimelinePlayers(p.timelinePlayers);
        view->redraw();

        c->uiTimeline->setColorConfigOptions(p.colorConfigOptions);
        c->uiTimeline->setLUTOptions(p.lutOptions);
        c->uiTimeline->setTimelinePlayer(player);

        if (comparePanel)
            comparePanel->setCompareOptions(p.compareOptions);
        if (colorPanel)
        {
            colorPanel->setLUTOptions(p.lutOptions);
            colorPanel->setDisplayOptions(p.displayOptions);
        }
        if (imageInfoPanel)
        {
            imageInfoPanel->setTimelinePlayer(player);
        }
        // @ todo:
        // if ( audioPanel )
        // {
        //     audioPanel->setTimelinelinePlayer( player );
        // }

        const int aIndex = model->observeAIndex()->get();
        std::string fileName;
        if (numFiles > 0 && aIndex >= 0 && aIndex < numFiles)
        {
            const auto& files = model->observeFiles()->get();
            fileName = files[aIndex]->path.get(-1, false);

            const auto& ioInfo = files[aIndex]->ioInfo;
            std::stringstream ss;
            ss.precision(2);
            if (!ioInfo.video.empty())
            {
                {
                    ss << "V:" << ioInfo.video[0].size.w << "x"
                       << ioInfo.video[0].size.h << ":" << std::fixed
                       << ioInfo.video[0].size.getAspect() << " "
                       << ioInfo.video[0].pixelType;
                }
            }
            if (ioInfo.audio.isValid())
            {
                if (!ss.str().empty())
                    ss << ", ";
                ss << "A: " << static_cast<size_t>(ioInfo.audio.channelCount)
                   << " " << ioInfo.audio.dataType << " "
                   << ioInfo.audio.sampleRate;
            }
            char buf[256];
            snprintf(buf, 256, "%s  %s", fileName.c_str(), ss.str().c_str());
            p.ui->uiMain->copy_label(buf);
        }
        else
        {
            p.ui->uiMain->copy_label("mrv2");
        }

        if (p.ui->uiSecondary)
        {
            view = p.ui->uiSecondary->viewport();
            view->setColorConfigOptions(p.colorConfigOptions);
            view->setLUTOptions(p.lutOptions);
            view->setImageOptions(imageOptions);
            view->setDisplayOptions(displayOptions);
            view->setCompareOptions(p.compareOptions);
            view->setTimelinePlayers(p.timelinePlayers);
            view->redraw();
        }

#ifdef TLRENDER_BMD
        p.app->outputDevice()->setColorConfigOptions(p.colorConfigOptions);
        p.app->outputDevice()->setLUTOptions(p.lutOptions);
        p.app->outputDevice()->setImageOptions(imageOptions);
        for (auto& i : displayOptions)
        {
            i.videoLevels = p.outputVideoLevels;
        }
        p.app->outputDevice()->setDisplayOptions(displayOptions);
        p.app->outputDevice()->setCompareOptions(p.compareOptions);
        p.app->outputDevice()->setTimelinePlayers(p.timelinePlayers);
#endif
    }
} // namespace mrv
