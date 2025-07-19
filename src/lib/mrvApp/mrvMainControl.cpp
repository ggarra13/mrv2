// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvApp.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvMainControl.h"

#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvSession.h"

#include "mrvNetwork/mrvTCP.h"
#include "mrvNetwork/mrvCompareOptions.h"

#include "mrvIcons/Mute.h"
#include "mrvIcons/Audio.h"

#include <tlDevice/IOutput.h>

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

        TimelinePlayer* player = nullptr;
        bool floatOnTop = false;
        bool secondaryFloatOnTop = false;
        timeline::OCIOOptions ocioOptions;
        timeline::LUTOptions lutOptions;
        timeline::ImageOptions imageOptions;
        timeline::DisplayOptions displayOptions;
        timeline::CompareOptions compareOptions;
        timeline::BackgroundOptions backgroundOptions;
        Stereo3DOptions stereo3DOptions;
        FilesPanelOptions filesPanelOptions;
        image::VideoLevels outputVideoLevels;
        float volume = 1.F;
        bool mute = false;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        std::shared_ptr<observer::ValueObserver<int> > stereoIndexObserver;
        std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
        std::shared_ptr<observer::ValueObserver<timeline::BackgroundOptions> >
            backgroundOptionsObserver;
        std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
            compareOptionsObserver;
        std::shared_ptr<observer::ValueObserver<Stereo3DOptions> >
            stereo3DOptionsObserver;
        std::shared_ptr<observer::ValueObserver<FilesPanelOptions> >
            filesPanelOptionsObserver;
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
            [this](int value)
            {
                Message msg;
                msg["command"] = "Set A Index";
                msg["value"] = value;
                tcp->pushMessage(msg);
                _widgetUpdate();
            });
        p.bIndexesObserver = observer::ListObserver<int>::create(
            app->filesModel()->observeBIndexes(),
            [this](const std::vector<int>& value)
            {
                Message msg;
                msg["command"] = "Set B Indexes";
                msg["value"] = value;
                tcp->pushMessage(msg);

                _widgetUpdate();
            });
        p.stereoIndexObserver = observer::ValueObserver<int>::create(
            app->filesModel()->observeStereoIndex(),
            [this](int value)
            {
                Message msg;
                msg["command"] = "Set Stereo Index";
                msg["value"] = value;
                tcp->pushMessage(msg);
                _widgetUpdate();
            });
        p.backgroundOptionsObserver =
            observer::ValueObserver<timeline::BackgroundOptions>::create(
                p.ui->uiView->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->backgroundOptions = value;

                    Message msg;
                    Message opts(value);
                    msg["command"] = "setBackgroundOptions";
                    msg["value"] = opts;
                    tcp->pushMessage(msg);

                    _widgetUpdate();
                });
        p.compareOptionsObserver =
            observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->compareOptions = value;

                    Message msg;
                    Message opts(value);
                    msg["command"] = "setCompareOptions";
                    msg["value"] = opts;
                    tcp->pushMessage(msg);

                    _compareUpdate();
                });

        p.stereo3DOptionsObserver =
            observer::ValueObserver<Stereo3DOptions>::create(
                app->filesModel()->observeStereo3DOptions(),
                [this](const Stereo3DOptions& value)
                {
                    _p->stereo3DOptions = value;

                    Message msg;
                    Message opts(value);
                    msg["command"] = "setStereo3DOptions";
                    msg["value"] = opts;
                    tcp->pushMessage(msg);

                    _widgetUpdate();
                });
        p.filesPanelOptionsObserver =
            observer::ValueObserver<FilesPanelOptions>::create(
                app->filesModel()->observeFilesPanelOptions(),
                [this](const FilesPanelOptions& value)
                {
                    _p->filesPanelOptions = value;

                    Message msg;
                    Message opts(value);
                    msg["command"] = "setFilesPanelOptions";
                    msg["value"] = opts;
                    tcp->pushMessage(msg);
                });
    }

    MainControl::~MainControl() {}

    void MainControl::setPlayer(TimelinePlayer* player)
    {
        TLRENDER_P();

        if (p.player == player)
            return;

        p.player = player;
        _timelinePlayersUpdate();
        _widgetUpdate();
    }

    void MainControl::setLUTOptions(const timeline::LUTOptions& value)
    {
        TLRENDER_P();

        p.lutOptions = value;

        _displayUpdate();
    }

    void MainControl::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        TLRENDER_P();

        p.displayOptions = value;

        _displayUpdate();
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
        p.ui->uiView->setTimelinePlayer(p.player);
        p.ui->uiTimeline->setTimelinePlayer(p.player);
        if (p.player)
            p.player->setTimelineViewport(p.ui->uiView);

        if (p.ui->uiSecondary)
        {
            auto view = p.ui->uiSecondary->viewport();
            view->setTimelinePlayer(p.player);
        }
    }

    //
    // In this function, we deal with LUT and Display options all at
    // once as those will need updating of shader values.
    //
    void MainControl::_displayUpdate()
    {
        TLRENDER_P();

        MyViewport* view = p.ui->uiView;
        view->setLUTOptions(p.lutOptions);
        view->setDisplayOptions({p.displayOptions});
        view->updateDisplayOptions();
        if (p.ui->uiSecondary)
        {
            view = p.ui->uiSecondary->viewport();
            view->setLUTOptions(p.lutOptions);
            view->setDisplayOptions({p.displayOptions});
            view->updateDisplayOptions();
        }

        auto display = p.ui->uiTimeline->getDisplayOptions();
        display.lut = p.lutOptions;
        p.ui->uiTimeline->setDisplayOptions(display);
        p.ui->uiTimeline->redraw();

        auto outputDevice = App::app->outputDevice();
        if (outputDevice)
        {
            outputDevice->setLUTOptions(p.lutOptions);
            outputDevice->setDisplayOptions({p.displayOptions});
        }
        
        if (panel::colorPanel)
        {
            panel::colorPanel->setDisplayOptions(p.displayOptions);
        }
    }

    void MainControl::_compareUpdate()
    {
        TLRENDER_P();

        MyViewport* view = p.ui->uiView;
        view->setCompareOptions(p.compareOptions);
        if (p.ui->uiSecondary)
        {
            view = p.ui->uiSecondary->viewport();
            view->setCompareOptions(p.compareOptions);
        }

        auto outputDevice = App::app->outputDevice();
        if (outputDevice)
        {
            outputDevice->setCompareOptions(p.compareOptions);
        }

        if (panel::comparePanel)
        {
            panel::comparePanel->setCompareOptions(p.compareOptions);
        }
    }

    void MainControl::_widgetUpdate()
    {
        TLRENDER_P();

        App* app = p.ui->app;
        p.volume = app->volume();
        p.mute = app->isMuted();

        const auto model = app->filesModel();
        const auto files = model->observeFiles()->get();
        const size_t numFiles = files.size();
        TimelineClass* c = p.ui->uiTimeWindow;
        if (numFiles > 0)
        {
            p.ui->uiTimeline->activate();
            c->uiFrame->activate();
            c->uiStartFrame->activate();
            c->uiEndFrame->activate();
            c->uiFPS->activate();
            c->fpsDefaults->activate();
            c->uiStartButton->activate();
            c->uiEndButton->activate();
            c->uiLoopMode->activate();
            c->uiTimecodeSwitch->activate();
        }
        else
        {
            p.ui->uiTimeline->deactivate();
            c->uiFrame->deactivate();
            c->uiStartFrame->deactivate();
            c->uiEndFrame->deactivate();
            c->uiVolume->deactivate();
            c->uiAudioTracks->deactivate();
            c->uiFPS->deactivate();
            c->fpsDefaults->deactivate();
            c->uiStartButton->deactivate();
            c->uiEndButton->deactivate();
            c->uiLoopMode->deactivate();
            c->uiTimecodeSwitch->deactivate();
        }

        auto player = p.player;
        if (player)
        {
            c->uiFPS->value(player->speed());

            const auto inOutRange = player->inOutRange();
            c->uiFrame->setTime(player->currentTime());
            c->uiStartFrame->setTime(inOutRange.start_time());
            c->uiEndFrame->setTime(inOutRange.end_time_inclusive());

            timeline::Loop loop =
                static_cast<timeline::Loop>(c->uiLoopMode->value());
            player->setLoop(loop);

            // Set the audio tracks
            const auto timeline = player->timeline();
            const auto ioinfo = timeline->getIOInfo();
            const auto audio = ioinfo.audio;
            if (audio.isValid())
            {
                int audio_track = c->uiAudioTracks->current_track();
                if (audio_track < 0)
                    audio_track = audio.currentTrack;

                // Add all the audio tracks
                c->uiAudioTracks->clear_tracks();

                for (unsigned int i = 0; i < audio.trackCount; ++i)
                {
                    //! When playing NDI, audioInfo is not filled.
                    if (audio.audioInfo.empty())
                        continue;
                    const auto& info = *audio.audioInfo[i];
                    const std::string& name = codeToLanguage(info.name);
                    const std::string& trackName =
                        string::Format(_("{0} - Channels: {1} {2} {3}"))
                            .arg(_(name.c_str()))
                            .arg(info.channelCount)
                            .arg(info.dataType)
                            .arg(info.sampleRate);

                    c->uiAudioTracks->add_track(trackName);
                }

                if (audio_track < audio.trackCount)
                    c->uiAudioTracks->current_track(audio_track);

                c->uiVolume->activate();
                c->uiAudioTracks->activate();

                delete c->uiAudioTracks->image();
                c->uiAudioTracks->value(p.mute);
                if (p.mute)
                {
                    c->uiAudioTracks->image(MRV2_LOAD_SVG(Mute));
                }
                else
                {
                    c->uiAudioTracks->image(MRV2_LOAD_SVG(Audio));
                }
                c->uiAudioTracks->redraw();

                // Set the audio volume
                c->uiVolume->value(p.volume);
                c->uiVolume->redraw();
            }
            else
            {
                delete c->uiAudioTracks->image();
                c->uiAudioTracks->image(MRV2_LOAD_SVG(Mute));
            }

            // Set color channel (layer)
            int layer = player->videoLayer();
            if (layer >= 0 && layer < p.ui->uiColorChannel->children())
                p.ui->uiColorChannel->value(layer);
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

        MyViewport* view = p.ui->uiView;
        view->setCompareOptions(p.compareOptions);

        p.ocioOptions = view->getOCIOOptions();
        view->setLUTOptions(p.lutOptions);
        view->setImageOptions({p.imageOptions});
        view->setCompareOptions(p.compareOptions);
        view->setDisplayOptions({p.displayOptions});
        view->setStereo3DOptions(p.stereo3DOptions);
        view->setTimelinePlayer(p.player);
        view->updatePlaybackButtons();
        view->updateDisplayOptions();
        view->redraw();

        auto display = p.ui->uiTimeline->getDisplayOptions();
        display.ocio = p.ocioOptions;
        p.ui->uiTimeline->setDisplayOptions(display);
        p.ui->uiTimeline->redraw();

        if (panel::comparePanel)
        {
            panel::comparePanel->setCompareOptions(p.compareOptions);
        }
        if (panel::colorPanel)
        {
            panel::colorPanel->setLUTOptions(p.lutOptions);
            panel::colorPanel->setDisplayOptions(p.displayOptions);
        }
        if (panel::stereo3DPanel)
        {
            panel::stereo3DPanel->setStereo3DOptions(p.stereo3DOptions);
        }
        if (panel::imageInfoPanel)
        {
            panel::imageInfoPanel->setTimelinePlayer(player);
        }
        // @todo: do we need it?
        // if ( audioPanel )
        // {
        //     audioPanel->setTimelinelinePlayer( player );
        // }

        p.ui->uiMain->update_title_bar();

        if (p.ui->uiSecondary)
        {
            MainWindow* main = p.ui->uiSecondary->window();
            std::string label = "Secondary ";
            label += p.ui->uiMain->label();
            main->copy_label(label.c_str());
            view = p.ui->uiSecondary->viewport();
            view->setOCIOOptions(p.ocioOptions);
            view->setLUTOptions(p.lutOptions);
            view->setImageOptions({p.imageOptions});
            view->setDisplayOptions({p.displayOptions});
            view->setCompareOptions(p.compareOptions);
            view->setStereo3DOptions(p.stereo3DOptions);
            view->setTimelinePlayer(p.player);
            view->updateDisplayOptions();
            view->redraw();
        }

        const auto& outputDevice = app->outputDevice();
        if (outputDevice)
        {
            outputDevice->setBackgroundOptions(p.backgroundOptions);
            outputDevice->setOCIOOptions(p.ocioOptions);
            outputDevice->setLUTOptions(p.lutOptions);
            outputDevice->setImageOptions({p.imageOptions});
            outputDevice->setDisplayOptions({p.displayOptions});
            outputDevice->setCompareOptions(p.compareOptions);
        }

        p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
    }
} // namespace mrv
