// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <tlCore/AudioSystem.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(Loop, "Loop", "Once", "Ping-Pong");
        TLRENDER_ENUM_SERIALIZE_IMPL(Loop);

        TLRENDER_ENUM_IMPL(
            TimeAction, "Start", "End", "FramePrev", "FramePrevX10",
            "FramePrevX100", "FrameNext", "FrameNextX10", "FrameNextX100",
            "JumpBack1s", "JumpBack10s", "JumpForward1s", "JumpForward10s");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimeAction);

        namespace
        {
#if defined(TLRENDER_AUDIO)

            //! Returns ma_format_unknown for types the player does not ask
            //! the device for; the caller falls back to F32.
            ma_format toMiniaudio(audio::DataType value) noexcept
            {
                ma_format out = ma_format_unknown;
                switch (value)
                {
                case audio::DataType::S16:
                    out = ma_format_s16;
                    break;
                case audio::DataType::S32:
                    out = ma_format_s32;
                    break;
                case audio::DataType::F32:
                    out = ma_format_f32;
                    break;
                default:
                    break;
                }
                return out;
            }
#endif // TLRENDER_AUDIO
        } // namespace

        void Player::_init(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const PlayerOptions& playerOptions)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    Cache read ahead: {0}")
                                    .arg(playerOptions.cache.readAhead));
                lines.push_back(string::Format("    Cache read behind: {0}")
                                    .arg(playerOptions.cache.readBehind));
                lines.push_back(string::Format("    Timer mode: {0}")
                                    .arg(playerOptions.timerMode));
                lines.push_back(
                    string::Format("    Audio buffer frame count: {0}")
                        .arg(playerOptions.audioBufferFrameCount));
                lines.push_back(string::Format("    Mute timeout: {0}ms")
                                    .arg(playerOptions.muteTimeout.count()));
                lines.push_back(string::Format("    Sleep timeout: {0}ms")
                                    .arg(playerOptions.sleepTimeout.count()));
                logSystem->print(
                    string::Format("tl::timeline::Player {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.playerOptions = playerOptions;
            p.timeline = timeline;
            p.ioInfo = timeline->getIOInfo();
            p.timeRange =
                observer::Value<OTIO_NS::TimeRange>::create(timeline->getTimeRange());
            p.sourceAudioInfo = timeline->getIOInfo().audio;

            // Create observers.
            p.speed = observer::Value<double>::create(
                p.timeline->getTimeRange().duration().rate());
            p.playback = observer::Value<Playback>::create(Playback::Stop);
            p.loop = observer::Value<Loop>::create(Loop::Loop);
            p.currentTime = observer::Value<OTIO_NS::RationalTime>::create(
                playerOptions.currentTime != time::invalidTime
                    ? playerOptions.currentTime
                    : p.timeRange->get().start_time());
            p.inOutRange =
                observer::Value<OTIO_NS::TimeRange>::create(p.timeRange->get());
            p.compare = observer::List<std::shared_ptr<Timeline> >::create();
            p.compareTime = observer::Value<CompareTimeMode>::create(
                CompareTimeMode::Relative);
            p.ioOptions = observer::Value<io::Options>::create();
            p.videoLayer = observer::Value<int>::create(0);
            p.compareVideoLayers = observer::List<int>::create();
            p.currentVideoFrame = observer::List<VideoFrame>::create();
            p.volume = observer::Value<float>::create(1.F);
            p.mute = observer::Value<bool>::create(false);
            p.channelMute = observer::List<int>::create();
            p.audioOffset = observer::Value<double>::create(0.0);
            p.currentAudioFrame = observer::List<AudioFrame>::create();
            p.cacheOptions = observer::Value<PlayerCacheOptions>::create(
                playerOptions.cache);
            p.cacheInfo = observer::Value<PlayerCacheInfo>::create();
            auto weak = std::weak_ptr<Player>(shared_from_this());
            p.timelineObserver = observer::ValueObserver<bool>::create(
                p.timeline->observeTimelineChanges(),
                [weak](bool)
                {
                    if (auto player = weak.lock())
                    {
                        player->clearCache();
                    }
                });
            p.framesFoundObserver =
                observer::ListObserver<OTIO_NS::TimeRange>::create(
                    p.timeline->observeFramesFound(),
                    [weak](const std::vector<OTIO_NS::TimeRange>& value)
                        {
                            if (auto player = weak.lock())
                            {
                                auto& pp = *player->_p;
                                std::unique_lock<std::mutex> lock(pp.mutex.mutex);
                                pp.mutex.framesFound.insert(
                                    pp.mutex.framesFound.end(), value.begin(), value.end());
                            }
                        });

            // Create a new thread.
            if (playerOptions.playback == Playback::Reverse)
                p.mutex.cacheDirection = CacheDirection::Reverse;
            p.mutex.currentTime = p.currentTime->get();
            p.mutex.timeRange = p.timeRange->get();
            p.mutex.inOutRange = p.inOutRange->get();
            p.mutex.audioOffset = p.audioOffset->get();
            p.mutex.cacheOptions = p.cacheOptions->get();
            p.mutex.cacheInfo = p.cacheInfo->get();
            p.audioMutex.speed = p.speed->get();
            p.log(context);
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

#if defined(TLRENDER_AUDIO)
                    if (auto context = getContext().lock())
                    {
                        // Initialize audio.
                        auto audioSystem = context->getSystem<audio::System>();
                        auto maContext =
                            audioSystem ? audioSystem->getContext() : nullptr;
                        if (maContext && !audioSystem->getDevices().empty())
                        {
                            p.audioThread.info =
                                audioSystem->getDefaultOutputInfo();
                            if (p.audioThread.info.channelCount > 0 &&
                                p.audioThread.info.dataType !=
                                    audio::DataType::kNone &&
                                p.audioThread.info.sampleRate > 0)
                            {
                                // miniaudio converts to the device's native
                                // format for us, so fall back to float when
                                // we have no direct equivalent (e.g. F64).
                                ma_format format =
                                    toMiniaudio(p.audioThread.info.dataType);
                                if (ma_format_unknown == format)
                                {
                                    format = ma_format_f32;
                                    p.audioThread.info.dataType =
                                        audio::DataType::F32;
                                }

                                ma_device_config config =
                                    ma_device_config_init(
                                        ma_device_type_playback);
                                config.playback.pDeviceID =
                                    static_cast<const ma_device_id*>(
                                        audioSystem->getDeviceId(
                                            audioSystem->getOutputDevice()));
                                config.playback.format = format;
                                config.playback.channels =
                                    p.audioThread.info.channelCount;
                                config.sampleRate =
                                    p.audioThread.info.sampleRate;
                                config.periodSizeInFrames =
                                    p.playerOptions.audioBufferFrameCount;
                                config.dataCallback = p.maDataCallback;
                                config.notificationCallback =
                                    p.maNotificationCallback;
                                config.pUserData = _p.get();

                                std::unique_ptr<ma_device> device(
                                    new ma_device);
                                ma_result r = ma_device_init(
                                    maContext.get(), &config, device.get());
                                if (MA_SUCCESS == r)
                                {
                                    // The device is initialized; from here on
                                    // the deleter uninitializes it.
                                    p.thread.maContext = maContext;
                                    p.thread.maDevice.reset(device.release());
                                    r = ma_device_start(
                                        p.thread.maDevice.get());
                                    if (MA_SUCCESS == r)
                                    {
                                        p.thread.audioStarted = true;
                                    }
                                }
                                if (r != MA_SUCCESS)
                                {
                                    std::stringstream ss;
                                    ss << "Cannot open audio stream: "
                                       << ma_result_description(r);
                                    context->log(
                                        "tl::timeline::Player", ss.str(),
                                        log::Type::Error);
                                }
                            }
                        }
                    }
#endif // TLRENDER_AUDIO

                    p.thread.cacheTimer = std::chrono::steady_clock::now();
                    p.thread.logTimer = std::chrono::steady_clock::now();
                    while (p.thread.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        // Get mutex protected values.
                        std::vector<std::shared_ptr<Timeline> > compare;
                        bool clearRequests = false;
                        bool clearCache = false;
                        std::vector<OTIO_NS::TimeRange> framesFound;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            p.thread.playback = p.mutex.playback;
                            p.thread.currentTime = p.mutex.currentTime;
                            p.thread.timeRange = p.mutex.timeRange;
                            p.thread.inOutRange = p.mutex.inOutRange;
                            compare = p.mutex.compare;
                            p.thread.compareTime = p.mutex.compareTime;
                            p.thread.ioOptions = p.mutex.ioOptions;
                            p.thread.videoLayer = p.mutex.videoLayer;
                            p.thread.compareVideoLayers =
                                p.mutex.compareVideoLayers;
                            p.thread.audioOffset = p.mutex.audioOffset;
                            clearRequests = p.mutex.clearRequests;
                            p.mutex.clearRequests = false;
                            clearCache = p.mutex.clearCache;
                            p.mutex.clearCache = false;
                            framesFound = std::move(p.mutex.framesFound);
                            p.mutex.framesFound.clear();
                            p.thread.cacheDirection = p.mutex.cacheDirection;
                            p.thread.cacheOptions = p.mutex.cacheOptions;
                        }

                        // Clear requests.
                        if (clearRequests)
                        {
                            p.clearRequests();
                        }
                        p.thread.compare = compare;

                        // Clear the cache.
                        if (clearCache)
                        {
                            p.clearCache();
                        }

                        // Pick up frames the timeline found that were missing
                        // before.
                        if (!framesFound.empty())
                        {
                            p.invalidateVideoCache(framesFound);
                        }

                        // Update the cache.
                        p.cacheUpdate();

                        // Update the current video data.
                        updateVideoFrame();

                        // Update the current audio data.
                        if (p.sourceAudioInfo.isValid())
                        {
                            const auto& timeRange = p.timeline->getTimeRange();
                            std::vector<AudioFrame> audioFrameList;
                            {
                                const int64_t seconds =
                                    p.thread.currentTime.rescaled_to(1.0)
                                        .value() -
                                    timeRange.start_time()
                                        .rescaled_to(1.0)
                                        .value();
                                std::unique_lock<std::mutex> lock(
                                    p.audioMutex.mutex);
                                for (int64_t s :
                                     {seconds - 1, seconds, seconds + 1})
                                {
                                    auto i = p.audioMutex.cache.find(s);
                                    if (i != p.audioMutex.cache.end())
                                    {
                                        audioFrameList.push_back(i->second);
                                    }
                                }
                            }
                            {
                                std::unique_lock<std::mutex> lock(
                                    p.mutex.mutex);
                                p.mutex.currentAudioFrame = audioFrameList;
                            }
                        }

                        // Logging.
                        auto t1 = std::chrono::steady_clock::now();
                        const std::chrono::duration<double> diff =
                            t1 - p.thread.logTimer;
                        if (diff.count() > 10.0)
                        {
                            p.thread.logTimer = t1;
                            if (auto context = getContext().lock())
                            {
                                p.log(context);
                            }
                            t1 = std::chrono::steady_clock::now();
                        }

                        // Sleep for a bit.
                        time::sleep(p.playerOptions.sleepTimeout, t0, t1);
                    }

                    p.clearRequests();
                });
        }

        Player::Player() :
            _p(new Private)
        {
        }

        Player::~Player()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
#if defined(TLRENDER_AUDIO)
            // Uninitializing the device stops it and waits for the data
            // callback to finish. Do this before releasing the context.
            p.thread.audioStarted = false;
            p.thread.maDevice.reset();
            p.thread.maContext.reset();
#endif // TLRENDER_AUDIO
        }

        std::shared_ptr<Player> Player::create(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const PlayerOptions& playerOptions)
        {
            auto out = std::shared_ptr<Player>(new Player);
            out->_init(timeline, context, playerOptions);
            return out;
        }

        void Player::updateVideoFrame()
        {
            TLRENDER_P();
            const auto& timeRange = p.thread.timeRange;
            if (!p.ioInfo.video.empty())
            {
                const auto i = p.thread.videoCache.find(p.thread.currentTime);
                if (i != p.thread.videoCache.end())
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.currentVideoFrame = i->second;
                }
                else if (p.thread.playback != Playback::Stop)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.playbackStartTime = p.thread.currentTime;
                        p.mutex.playbackStartTimer =
                            std::chrono::steady_clock::now();
                        if (!timeRange.contains(p.thread.currentTime))
                        {
                            p.mutex.currentVideoFrame.clear();
                        }
                    }
                    p.resetAudioTime();
                    {
                        const auto now = std::chrono::steady_clock::now();
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.muteTimeout =
                            now + p.playerOptions.muteTimeout;
                    }
                }
                else
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (!timeRange.contains(p.thread.currentTime))
                    {
                        p.mutex.currentVideoFrame.clear();
                    }
                }
            }
        }

        const std::weak_ptr<system::Context>& Player::getContext() const
        {
            return _p->timeline->getContext();
        }

        const std::shared_ptr<Timeline>& Player::getTimeline() const
        {
            return _p->timeline;
        }

        const file::Path& Player::getPath() const
        {
            return _p->timeline->getPath();
        }

        const file::Path& Player::getAudioPath() const
        {
            return _p->timeline->getAudioPath();
        }

        const PlayerOptions& Player::getPlayerOptions() const
        {
            return _p->playerOptions;
        }

        const Options& Player::getOptions() const
        {
            return _p->timeline->getOptions();
        }

        const OTIO_NS::TimeRange& Player::getTimeRange() const
        {
            return _p->timeRange->get();
        }

        std::shared_ptr<observer::IValue<OTIO_NS::TimeRange> >
        Player::observeTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Player::getIOInfo() const
        {
            return _p->ioInfo;
        }

        double Player::getDefaultSpeed() const
        {
            return _p->timeline->getTimeRange().duration().rate();
        }

        double Player::getSpeed() const
        {
            return _p->speed->get();
        }

        std::shared_ptr<observer::IValue<double> > Player::observeSpeed() const
        {
            return _p->speed;
        }

        void Player::setSpeed(double value)
        {
            TLRENDER_P();
            if (p.speed->setIfChanged(value))
            {
                if (p.playback->get() != Playback::Stop)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.playbackStartTime = p.currentTime->get();
                        p.mutex.playbackStartTimer =
                            std::chrono::steady_clock::now();
                    }
                    p.resetAudioTime();
                }
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioMutex.speed = value;
                }
            }
        }

        Playback Player::getPlayback() const
        {
            return _p->playback->get();
        }

        std::shared_ptr<observer::IValue<Playback> >
        Player::observePlayback() const
        {
            return _p->playback;
        }

        void Player::setPlayback(Playback value)
        {
            TLRENDER_P();

            // Update the frame for loop modes.
            switch (p.loop->get())
            {
            case Loop::Once:
                switch (value)
                {
                case Playback::Forward:
                    if (p.currentTime->get() ==
                        p.inOutRange->get().end_time_inclusive())
                    {
                        seek(p.inOutRange->get().start_time());
                    }
                    break;
                case Playback::Reverse:
                    if (p.currentTime->get() ==
                        p.inOutRange->get().start_time())
                    {
                        seek(p.inOutRange->get().end_time_inclusive());
                    }
                    break;
                default:
                    break;
                }
                break;
            case Loop::PingPong:
                switch (value)
                {
                case Playback::Forward:
                    if (p.currentTime->get() ==
                        p.inOutRange->get().end_time_inclusive())
                    {
                        value = Playback::Reverse;
                    }
                    break;
                case Playback::Reverse:
                    if (p.currentTime->get() ==
                        p.inOutRange->get().start_time())
                    {
                        value = Playback::Forward;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }

            if (p.playback->setIfChanged(value))
            {
                if (value != Playback::Stop)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.playback = value;
                        p.mutex.playbackStartTime = p.currentTime->get();
                        p.mutex.playbackStartTimer =
                            std::chrono::steady_clock::now();
                        p.mutex.currentTime = p.currentTime->get();
                        p.mutex.cacheDirection = Playback::Forward == value
                                                     ? CacheDirection::Forward
                                                     : CacheDirection::Reverse;
                        p.mutex.clearRequests = true;
                    }
                    p.resetAudioTime();
                }
                else
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.playback = value;
                    p.mutex.clearRequests = true;
                }
            }
        }

        Loop Player::getLoop() const
        {
            return _p->loop->get();
        }

        std::shared_ptr<observer::IValue<Loop> > Player::observeLoop() const
        {
            return _p->loop;
        }

        void Player::setLoop(Loop value)
        {
            _p->loop->setIfChanged(value);
        }

        OTIO_NS::RationalTime Player::getCurrentTime() const
        {
            return _p->currentTime->get();
        }

        std::shared_ptr<observer::IValue<OTIO_NS::RationalTime> >
        Player::observeCurrentTime() const
        {
            return _p->currentTime;
        }

        void Player::seek(const OTIO_NS::RationalTime& time)
        {
            TLRENDER_P();

            // Loop the time.
            const auto& timeRange = p.timeRange->get();
            const auto tmp =
                loop(time.rescaled_to(timeRange.duration()).floor(), timeRange);

            if (p.currentTime->setIfChanged(tmp))
            {
                // std::cout << "seek: " << tmp << std::endl;

                // Update playback.
                if (p.playback->get() != Playback::Stop)
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.playbackStartTime = tmp;
                    p.mutex.playbackStartTimer =
                        std::chrono::steady_clock::now();
                }

                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.currentTime = tmp;
                    p.mutex.clearRequests = true;
                }
                p.resetAudioTime();
            }
        }

        void Player::timeAction(TimeAction time)
        {
            TLRENDER_P();
            const auto& timeRange = p.timeline->getTimeRange();
            const auto& currentTime = p.currentTime->get();
            switch (time)
            {
            case TimeAction::Start:
                setPlayback(timeline::Playback::Stop);
                seek(p.inOutRange->get().start_time());
                break;
            case TimeAction::End:
                setPlayback(timeline::Playback::Stop);
                seek(p.inOutRange->get().end_time_inclusive());
                break;
            case TimeAction::FramePrev:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime -
                    OTIO_NS::RationalTime(1, timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX10:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime -
                    OTIO_NS::RationalTime(10, timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX100:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime -
                    OTIO_NS::RationalTime(100, timeRange.duration().rate()));
                break;
            case TimeAction::FrameNext:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime +
                    OTIO_NS::RationalTime(1, timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX10:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime +
                    OTIO_NS::RationalTime(10, timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX100:
                setPlayback(timeline::Playback::Stop);
                seek(
                    currentTime +
                    OTIO_NS::RationalTime(100, timeRange.duration().rate()));
                break;
            case TimeAction::JumpBack1s:
                seek(currentTime - OTIO_NS::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpBack10s:
                seek(currentTime - OTIO_NS::RationalTime(10.0, 1.0));
                break;
            case TimeAction::JumpForward1s:
                seek(currentTime + OTIO_NS::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpForward10s:
                seek(currentTime + OTIO_NS::RationalTime(10.0, 1.0));
                break;
            default:
                break;
            }
        }

        void Player::start()
        {
            timeAction(TimeAction::Start);
        }

        void Player::end()
        {
            timeAction(TimeAction::End);
        }

        void Player::framePrev()
        {
            timeAction(TimeAction::FramePrev);
        }

        void Player::frameNext()
        {
            timeAction(TimeAction::FrameNext);
        }

        OTIO_NS::TimeRange Player::getInOutRange() const
        {
            return _p->inOutRange->get();
        }

        std::shared_ptr<observer::IValue<OTIO_NS::TimeRange> >
        Player::observeInOutRange() const
        {
            return _p->inOutRange;
        }

        void Player::setInOutRange(const OTIO_NS::TimeRange& value)
        {
            TLRENDER_P();
            if (p.inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.inOutRange = value;
                p.mutex.clearRequests = true;
            }
        }

        void Player::setInPoint()
        {
            TLRENDER_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time(
                p.currentTime->get(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void Player::resetInPoint()
        {
            TLRENDER_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time(
                              p.timeRange->get().start_time(),
                              p.inOutRange->get().end_time_exclusive()));
        }

        void Player::setOutPoint()
        {
            TLRENDER_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(), p.currentTime->get()));
        }

        void Player::resetOutPoint()
        {
            TLRENDER_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.timeRange->get().end_time_inclusive()));
        }

        const std::vector<std::shared_ptr<Timeline> >&
        Player::getCompare() const
        {
            return _p->compare->get();
        }

        std::shared_ptr<observer::IList<std::shared_ptr<Timeline> > >
        Player::observeCompare() const
        {
            return _p->compare;
        }

        void
        Player::setCompare(const std::vector<std::shared_ptr<Timeline> >& value)
        {
            TLRENDER_P();
            if (p.compare->setIfChanged(value))
            {
                // The player's time range depends on the compare timelines.
                const OTIO_NS::TimeRange oldTimeRange = p.timeRange->get();
                p.updateTimeRange();
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.compare = value;
                    p.mutex.timeRange = p.timeRange->get();
                    p.mutex.clearRequests = true;
                    p.mutex.clearCache = true;
                }
                if (p.timeRange->get() != oldTimeRange)
                {
                    // Follow the time range if the in/out points were not
                    // changed by the user, otherwise keep them within it.
                    const OTIO_NS::TimeRange inOutRange = p.inOutRange->get();
                    setInOutRange(
                        inOutRange == oldTimeRange
                        ? p.timeRange->get()
                        : inOutRange.clamped(p.timeRange->get()));

                    // Make sure the current time is still valid.
                    seek(p.currentTime->get());
                }
            }
        }

        CompareTimeMode Player::getCompareTime() const
        {
            return _p->compareTime->get();
        }

        std::shared_ptr<observer::IValue<CompareTimeMode> >
        Player::observeCompareTime() const
        {
            return _p->compareTime;
        }

        void Player::setCompareTime(CompareTimeMode value)
        {
            TLRENDER_P();
            if (p.compareTime->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareTime = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const io::Options& Player::getIOOptions() const
        {
            return _p->ioOptions->get();
        }

        std::shared_ptr<observer::IValue<io::Options> >
        Player::observeIOOptions() const
        {
            return _p->ioOptions;
        }

        void Player::setIOOptions(const io::Options& value)
        {
            TLRENDER_P();
            if (p.ioOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.ioOptions = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        int Player::getVideoLayer() const
        {
            return _p->videoLayer->get();
        }

        std::shared_ptr<observer::IValue<int> >
        Player::observeVideoLayer() const
        {
            return _p->videoLayer;
        }

        void Player::setVideoLayer(int value)
        {
            TLRENDER_P();
            if (p.videoLayer->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.videoLayer = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<int>& Player::getCompareVideoLayers() const
        {
            return _p->compareVideoLayers->get();
        }

        std::shared_ptr<observer::IList<int> >
        Player::observeCompareVideoLayers() const
        {
            return _p->compareVideoLayers;
        }

        void Player::setCompareVideoLayers(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (p.compareVideoLayers->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareVideoLayers = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<VideoFrame>& Player::getCurrentVideo() const
        {
            return _p->currentVideoFrame->get();
        }

        std::shared_ptr<observer::IList<VideoFrame> >
        Player::observeCurrentVideo() const
        {
            return _p->currentVideoFrame;
        }

        float Player::getVolume() const
        {
            return _p->volume->get();
        }

        std::shared_ptr<observer::IValue<float> > Player::observeVolume() const
        {
            return _p->volume;
        }

        void Player::setVolume(float value)
        {
            TLRENDER_P();
            if (p.volume->setIfChanged(math::clamp(value, 0.F, 1.F)))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.volume = value;
            }
        }

        bool Player::isMuted() const
        {
            return _p->mute->get();
        }

        std::shared_ptr<observer::IValue<bool> > Player::observeMute() const
        {
            return _p->mute;
        }

        void Player::setMute(bool value)
        {
            TLRENDER_P();
            if (p.mute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.mute = value;
            }
        }

        double Player::getAudioOffset() const
        {
            return _p->audioOffset->get();
        }

        std::shared_ptr<observer::IValue<double> >
        Player::observeAudioOffset() const
        {
            return _p->audioOffset;
        }

        void Player::setAudioOffset(double value)
        {
            TLRENDER_P();
            if (p.audioOffset->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioOffset = value;
            }
        }

        const std::vector<AudioFrame>& Player::getCurrentAudio() const
        {
            return _p->currentAudioFrame->get();
        }

        std::shared_ptr<observer::IList<AudioFrame> >
        Player::observeCurrentAudio() const
        {
            return _p->currentAudioFrame;
        }

        const PlayerCacheOptions& Player::getCacheOptions() const
        {
            return _p->cacheOptions->get();
        }

        std::shared_ptr<observer::IValue<PlayerCacheOptions> >
        Player::observeCacheOptions() const
        {
            return _p->cacheOptions;
        }

        void Player::setCacheOptions(const PlayerCacheOptions& value)
        {
            TLRENDER_P();
            if (p.cacheOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.cacheOptions = value;
                if (p.timeline)
                    p.timeline->setCacheOptions(value);
            }
        }

        std::shared_ptr<observer::IValue<PlayerCacheInfo> >
        Player::observeCacheInfo() const
        {
            return _p->cacheInfo;
        }

        void Player::updateVideoCache(const OTIO_NS::RationalTime& time)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.videoCache.erase(time);
                p.forwardRequests(
                    time, time, OTIO_NS::RationalTime(1.0, time.rate()), true);
            }
            const auto playback = p.playback->get();
            if (playback == Playback::Stop)
            {
                p.finishedVideoRequests();
                updateVideoFrame();
            }
        }

        void Player::clearCache()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.clearRequests = true;
            p.mutex.clearCache = true;
        }

        void Player::setTimeline(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& value)
        {
            TLRENDER_P();

            p.timeline->setTimeline(value);
            if (!p.timeline)
                return;
            p.updateTimeRange();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.timeRange = p.timeRange->get();
            }
            p.speed->setIfChanged(p.timeline->getTimeRange().duration().rate());
            p.inOutRange->setIfChanged(p.timeRange->get());
        }

        void Player::tick()
        {
            TLRENDER_P();

            // Tick the timeline.
            p.timeline->tick();

            // Calculate the current time.
            const auto& timeRange = p.timeline->getTimeRange();
            const auto playback = p.playback->get();
            if (playback != Playback::Stop)
            {
                const double timelineSpeed = timeRange.duration().rate();
                const double speed = p.speed->get();

                OTIO_NS::RationalTime playbackStartTime = time::invalidTime;
                std::chrono::steady_clock::time_point playbackStartTimer;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    playbackStartTime = p.mutex.playbackStartTime;
                    playbackStartTimer = p.mutex.playbackStartTimer;
                }
                double seconds = 0.0;
#if defined(TLRENDER_AUDIO)
                if (p.thread.audioStarted &&
                    TimerMode::Audio == p.playerOptions.timerMode &&
                    math::fuzzyCompare(timelineSpeed, speed))
                {
                    seconds = static_cast<double>(p.thread.streamFrames.load()) /
                              p.audioThread.info.sampleRate;
                }
                else
#endif // TLRENDER_AUDIO
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<double> diff =
                        now - playbackStartTimer;
                    seconds = diff.count() * (speed / timelineSpeed);
                }
                if (Playback::Reverse == playback)
                {
                    seconds = -seconds;
                }
                const OTIO_NS::RationalTime currentTime = p.loopPlayback(
                    playbackStartTime +
                    OTIO_NS::RationalTime(seconds, 1.0)
                        .rescaled_to(timeRange.duration().rate())
                        .floor());
                // const double currentTimeDiff = abs(currentTime.value() -
                // p.currentTime->get().value());
                if (p.currentTime->setIfChanged(currentTime))
                {
                    // std::cout << "current time: " << p.currentTime->get() <<
                    // " / " << currentTimeDiff << std::endl;
                }
            }

            // Sync with the thread.
            std::vector<VideoFrame> currentVideoFrame;
            std::vector<AudioFrame> currentAudioFrame;
            PlayerCacheInfo cacheInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.currentTime = p.currentTime->get();
                currentVideoFrame = p.mutex.currentVideoFrame;
                currentAudioFrame = p.mutex.currentAudioFrame;
                cacheInfo = p.mutex.cacheInfo;
            }
            p.currentVideoFrame->setIfChanged(currentVideoFrame);
            p.currentAudioFrame->setIfChanged(currentAudioFrame);
            p.cacheInfo->setIfChanged(cacheInfo);
        }
    } // namespace timeline
} // namespace tl
