// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQt/TimelinePlayer.h>

#include <tlCore/Math.h>
#include <tlCore/Time.h>

#include <QTimer>

#include <atomic>
#include <thread>

namespace tl
{
    namespace qt
    {
        namespace
        {
            const size_t timeout = 5;
        }

        struct TimelinePlayer::Private
        {
            std::shared_ptr<timeline::Player> player;
            std::unique_ptr<QTimer> timer;

            std::shared_ptr<observer::ValueObserver<double> > speedObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> >
                playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> >
                loopObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> >
                inOutRangeObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<timeline::Timeline> > >
                compareObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
                compareTimeObserver;
            std::shared_ptr<observer::ValueObserver<io::Options> >
                ioOptionsObserver;
            std::shared_ptr<observer::ValueObserver<int> > videoLayerObserver;
            std::shared_ptr<observer::ListObserver<int> >
                compareVideoLayersObserver;
            std::shared_ptr<observer::ListObserver<timeline::VideoData> >
                currentVideoObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> >
                audioOffsetObserver;
            std::shared_ptr<observer::ListObserver<timeline::AudioData> >
                currentAudioObserver;
            std::shared_ptr<
                observer::ValueObserver<timeline::PlayerCacheOptions> >
                cacheOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;
        };

        void TimelinePlayer::_init(
            const std::shared_ptr<timeline::Player>& player,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.player = player;

            p.speedObserver = observer::ValueObserver<double>::create(
                p.player->observeSpeed(),
                [this](double value) { Q_EMIT speedChanged(value); });

            p.playbackObserver =
                observer::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    { Q_EMIT playbackChanged(value); });

            p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
                p.player->observeLoop(),
                [this](timeline::Loop value) { Q_EMIT loopChanged(value); });

            p.currentTimeObserver =
                observer::ValueObserver<otime::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [this](const otime::RationalTime& value)
                    { Q_EMIT currentTimeChanged(value); });

            p.inOutRangeObserver =
                observer::ValueObserver<otime::TimeRange>::create(
                    p.player->observeInOutRange(),
                    [this](const otime::TimeRange value)
                    { Q_EMIT inOutRangeChanged(value); });

            p.compareObserver = observer::
                ListObserver<std::shared_ptr<timeline::Timeline> >::create(
                    p.player->observeCompare(),
                    [this](
                        const std::vector<std::shared_ptr<timeline::Timeline> >&
                            value) { Q_EMIT compareChanged(value); });

            p.compareTimeObserver =
                observer::ValueObserver<timeline::CompareTimeMode>::create(
                    p.player->observeCompareTime(),
                    [this](timeline::CompareTimeMode value)
                    { Q_EMIT compareTimeChanged(value); });

            p.ioOptionsObserver = observer::ValueObserver<io::Options>::create(
                p.player->observeIOOptions(), [this](const io::Options& value)
                { Q_EMIT ioOptionsChanged(value); });

            p.videoLayerObserver = observer::ValueObserver<int>::create(
                p.player->observeVideoLayer(),
                [this](int value) { Q_EMIT videoLayerChanged(value); });

            p.compareVideoLayersObserver = observer::ListObserver<int>::create(
                p.player->observeCompareVideoLayers(),
                [this](const std::vector<int>& value)
                { Q_EMIT compareVideoLayersChanged(value); });

            p.currentVideoObserver =
                observer::ListObserver<timeline::VideoData>::create(
                    p.player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    { Q_EMIT currentVideoChanged(value); },
                    observer::CallbackAction::Suppress);

            p.volumeObserver = observer::ValueObserver<float>::create(
                p.player->observeVolume(),
                [this](float value) { Q_EMIT volumeChanged(value); });

            p.muteObserver = observer::ValueObserver<bool>::create(
                p.player->observeMute(),
                [this](bool value) { Q_EMIT muteChanged(value); });

            p.audioOffsetObserver = observer::ValueObserver<double>::create(
                p.player->observeAudioOffset(),
                [this](double value) { Q_EMIT audioOffsetChanged(value); });

            p.currentAudioObserver =
                observer::ListObserver<timeline::AudioData>::create(
                    p.player->observeCurrentAudio(),
                    [this](const std::vector<timeline::AudioData>& value)
                    { Q_EMIT currentAudioChanged(value); });

            p.cacheOptionsObserver =
                observer::ValueObserver<timeline::PlayerCacheOptions>::create(
                    p.player->observeCacheOptions(),
                    [this](const timeline::PlayerCacheOptions& value)
                    { Q_EMIT cacheOptionsChanged(value); });

            p.cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    p.player->observeCacheInfo(),
                    [this](const timeline::PlayerCacheInfo& value)
                    { Q_EMIT cacheInfoChanged(value); });

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(
                p.timer.get(), &QTimer::timeout, this,
                &TimelinePlayer::_timerCallback);
            p.timer->start(timeout);
        }

        TimelinePlayer::TimelinePlayer(
            const std::shared_ptr<timeline::Player>& player,
            const std::shared_ptr<system::Context>& context, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            _init(player, context);
        }

        TimelinePlayer::~TimelinePlayer() {}

        const std::weak_ptr<system::Context>& TimelinePlayer::context() const
        {
            return _p->player->getContext();
        }

        const std::shared_ptr<timeline::Player>& TimelinePlayer::player() const
        {
            return _p->player;
        }

        const std::shared_ptr<timeline::Timeline>&
        TimelinePlayer::timeline() const
        {
            return _p->player->getTimeline();
        }

        const file::Path& TimelinePlayer::path() const
        {
            return _p->player->getPath();
        }

        const file::Path& TimelinePlayer::audioPath() const
        {
            return _p->player->getAudioPath();
        }

        const timeline::PlayerOptions& TimelinePlayer::getPlayerOptions() const
        {
            return _p->player->getPlayerOptions();
        }

        const timeline::Options& TimelinePlayer::getOptions() const
        {
            return _p->player->getOptions();
        }

        const otime::TimeRange& TimelinePlayer::timeRange() const
        {
            return _p->player->getTimeRange();
        }

        const io::Info& TimelinePlayer::ioInfo() const
        {
            return _p->player->getIOInfo();
        }

        double TimelinePlayer::defaultSpeed() const
        {
            return _p->player->getDefaultSpeed();
        }

        double TimelinePlayer::speed() const
        {
            return _p->player->observeSpeed()->get();
        }

        timeline::Playback TimelinePlayer::playback() const
        {
            return _p->player->observePlayback()->get();
        }

        timeline::Loop TimelinePlayer::loop() const
        {
            return _p->player->observeLoop()->get();
        }

        const otime::RationalTime& TimelinePlayer::currentTime() const
        {
            return _p->player->observeCurrentTime()->get();
        }

        const otime::TimeRange& TimelinePlayer::inOutRange() const
        {
            return _p->player->observeInOutRange()->get();
        }

        const std::vector<std::shared_ptr<timeline::Timeline> >&
        TimelinePlayer::compare() const
        {
            return _p->player->getCompare();
        }

        timeline::CompareTimeMode TimelinePlayer::compareTime() const
        {
            return _p->player->getCompareTime();
        }

        const io::Options& TimelinePlayer::ioOptions() const
        {
            return _p->player->observeIOOptions()->get();
        }

        int TimelinePlayer::videoLayer() const
        {
            return _p->player->getVideoLayer();
        }

        const std::vector<int>& TimelinePlayer::compareVideoLayers() const
        {
            return _p->player->getCompareVideoLayers();
        }

        const std::vector<timeline::VideoData>&
        TimelinePlayer::currentVideo() const
        {
            return _p->player->getCurrentVideo();
        }

        float TimelinePlayer::volume() const
        {
            return _p->player->observeVolume()->get();
        }

        bool TimelinePlayer::isMuted() const
        {
            return _p->player->observeMute()->get();
        }

        double TimelinePlayer::audioOffset() const
        {
            return _p->player->observeAudioOffset()->get();
        }

        const std::vector<timeline::AudioData>&
        TimelinePlayer::currentAudio() const
        {
            return _p->player->getCurrentAudio();
        }

        const timeline::PlayerCacheOptions& TimelinePlayer::cacheOptions() const
        {
            return _p->player->observeCacheOptions()->get();
        }

        const timeline::PlayerCacheInfo& TimelinePlayer::cacheInfo() const
        {
            return _p->player->observeCacheInfo()->get();
        }

        void TimelinePlayer::setSpeed(double value)
        {
            _p->player->setSpeed(value);
        }

        void TimelinePlayer::setPlayback(timeline::Playback value)
        {
            _p->player->setPlayback(value);
        }

        void TimelinePlayer::stop()
        {
            _p->player->setPlayback(timeline::Playback::Stop);
        }

        void TimelinePlayer::forward()
        {
            _p->player->setPlayback(timeline::Playback::Forward);
        }

        void TimelinePlayer::reverse()
        {
            _p->player->setPlayback(timeline::Playback::Reverse);
        }

        void TimelinePlayer::togglePlayback()
        {
            _p->player->setPlayback(
                timeline::Playback::Stop == _p->player->observePlayback()->get()
                    ? timeline::Playback::Forward
                    : timeline::Playback::Stop);
        }

        void TimelinePlayer::setLoop(timeline::Loop value)
        {
            _p->player->setLoop(value);
        }

        void TimelinePlayer::seek(const otime::RationalTime& value)
        {
            _p->player->seek(value);
        }

        void TimelinePlayer::timeAction(timeline::TimeAction value)
        {
            _p->player->timeAction(value);
        }

        void TimelinePlayer::start()
        {
            _p->player->start();
        }

        void TimelinePlayer::end()
        {
            _p->player->end();
        }

        void TimelinePlayer::framePrev()
        {
            _p->player->framePrev();
        }

        void TimelinePlayer::frameNext()
        {
            _p->player->frameNext();
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            _p->player->setInOutRange(value);
        }

        void TimelinePlayer::setInPoint()
        {
            _p->player->setInPoint();
        }

        void TimelinePlayer::resetInPoint()
        {
            _p->player->resetInPoint();
        }

        void TimelinePlayer::setOutPoint()
        {
            _p->player->setOutPoint();
        }

        void TimelinePlayer::resetOutPoint()
        {
            _p->player->resetOutPoint();
        }

        void TimelinePlayer::setIOOptions(const io::Options& value)
        {
            _p->player->setIOOptions(value);
        }

        void TimelinePlayer::setCompare(
            const std::vector<std::shared_ptr<timeline::Timeline> >& value)
        {
            _p->player->setCompare(value);
        }

        void TimelinePlayer::setCompareTime(timeline::CompareTimeMode value)
        {
            _p->player->setCompareTime(value);
        }

        void TimelinePlayer::setVideoLayer(int value)
        {
            _p->player->setVideoLayer(value);
        }

        void
        TimelinePlayer::setCompareVideoLayers(const std::vector<int>& value)
        {
            _p->player->setCompareVideoLayers(value);
        }

        void TimelinePlayer::setVolume(float value)
        {
            _p->player->setVolume(value);
        }

        void TimelinePlayer::setMute(bool value)
        {
            _p->player->setMute(value);
        }

        void TimelinePlayer::setAudioOffset(double value)
        {
            _p->player->setAudioOffset(value);
        }

        void TimelinePlayer::setCacheOptions(
            const timeline::PlayerCacheOptions& value)
        {
            _p->player->setCacheOptions(value);
        }

        void TimelinePlayer::_timerCallback()
        {
            if (_p && _p->player)
            {
                _p->player->tick();
            }
        }
    } // namespace qt
} // namespace tl
