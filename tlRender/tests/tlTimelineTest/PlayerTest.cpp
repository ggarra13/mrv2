// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerTest.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

#include <sstream>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerTest::PlayerTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::PlayerTest", context)
        {
        }

        std::shared_ptr<PlayerTest>
        PlayerTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PlayerTest>(new PlayerTest(context));
        }

        void PlayerTest::run()
        {
            _enums();
            _loop();
            _player();
        }

        void PlayerTest::_enums()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void PlayerTest::_loop()
        {
            {
                const otime::TimeRange timeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0));
                TLRENDER_ASSERT(
                    otime::RationalTime(0.0, 24.0) ==
                    loop(otime::RationalTime(0.0, 24.0), timeRange));
                TLRENDER_ASSERT(
                    otime::RationalTime(1.0, 24.0) ==
                    loop(otime::RationalTime(1.0, 24.0), timeRange));
                TLRENDER_ASSERT(
                    otime::RationalTime(23.0, 24.0) ==
                    loop(otime::RationalTime(23.0, 24.0), timeRange));
                TLRENDER_ASSERT(
                    otime::RationalTime(0.0, 24.0) ==
                    loop(otime::RationalTime(24.0, 24.0), timeRange));
                TLRENDER_ASSERT(
                    otime::RationalTime(23.0, 24.0) ==
                    loop(otime::RationalTime(-1.0, 24.0), timeRange));
            }
            {
                auto ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Forward);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(24.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(24.0, 24.0),
                        otime::RationalTime(12.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Forward);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(12.0, 24.0),
                                     otime::RationalTime(12.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(-12.0, 24.0),
                        otime::RationalTime(12.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Forward);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(12.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(10.0, 24.0),
                        otime::RationalTime(48.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Forward);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(10.0, 24.0),
                                     otime::RationalTime(14.0, 24.0)));
                TLRENDER_ASSERT(
                    ranges[1] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(10.0, 24.0)));
            }
            {
                auto ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(24.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(24.0, 24.0),
                        otime::RationalTime(12.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(12.0, 24.0),
                                     otime::RationalTime(12.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(-12.0, 24.0),
                        otime::RationalTime(12.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(12.0, 24.0)));

                ranges = loopCache(
                    otime::TimeRange(
                        otime::RationalTime(-10.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(
                    ranges[0] == otime::TimeRange(
                                     otime::RationalTime(0.0, 24.0),
                                     otime::RationalTime(14.0, 24.0)));
                TLRENDER_ASSERT(
                    ranges[1] == otime::TimeRange(
                                     otime::RationalTime(14.0, 24.0),
                                     otime::RationalTime(10.0, 24.0)));
            }
        }

        void PlayerTest::_player()
        {
            // Test timeline players.
            const std::vector<file::Path> paths = {
                file::Path(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v"),
                file::Path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionGap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otioz")};
            for (const auto& path : paths)
            {
                try
                {
                    _print(string::Format("Timeline: {0}").arg(path.get()));
                    auto timeline = Timeline::create(path.get(), _context);
                    auto player = Player::create(timeline, _context);
                    TLRENDER_ASSERT(player->getTimeline());
                    _player(player);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (const auto& path : paths)
            {
                try
                {
                    _print(
                        string::Format("Memory timeline: {0}").arg(path.get()));
                    auto otioTimeline = timeline::create(path, _context);
                    toMemoryReferences(
                        otioTimeline, path.getDirectory(),
                        ToMemoryReference::Shared);
                    auto timeline = Timeline::create(otioTimeline, _context);
                    auto player = Player::create(timeline, _context);
                    _player(player);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }

        void
        PlayerTest::_player(const std::shared_ptr<timeline::Player>& player)
        {
            const file::Path& path = player->getPath();
            const file::Path& audioPath = player->getAudioPath();
            const PlayerOptions& playerOptions = player->getPlayerOptions();
            const Options options = player->getOptions();
            const otime::TimeRange& timeRange = player->getTimeRange();
            const io::Info& ioInfo = player->getIOInfo();
            const double defaultSpeed = player->getDefaultSpeed();
            double speed = player->getSpeed();
            _print(string::Format("Path: {0}").arg(path.get()));
            _print(string::Format("Audio path: {0}").arg(audioPath.get()));
            _print(string::Format("Time range: {0}").arg(timeRange));
            if (!ioInfo.video.empty())
            {
                _print(string::Format("Video: {0}").arg(ioInfo.video.size()));
            }
            if (ioInfo.audio.isValid())
            {
                _print(string::Format("Audio: {0} {1} {2}")
                           .arg(ioInfo.audio.channelCount)
                           .arg(ioInfo.audio.dataType)
                           .arg(ioInfo.audio.sampleRate));
            }
            _print(string::Format("Default speed: {0}").arg(defaultSpeed));
            _print(string::Format("Speed: {0}").arg(speed));

            // Test the playback speed.
            auto speedObserver = observer::ValueObserver<double>::create(
                player->observeSpeed(),
                [&speed](double value) { speed = value; });
            const double doubleSpeed = defaultSpeed * 2.0;
            player->setSpeed(doubleSpeed);
            TLRENDER_ASSERT(doubleSpeed == speed);
            player->setSpeed(defaultSpeed);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = observer::ValueObserver<Playback>::create(
                player->observePlayback(),
                [&playback](Playback value) { playback = value; });
            player->setPlayback(Playback::Forward);
            TLRENDER_ASSERT(Playback::Forward == player->getPlayback());
            TLRENDER_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = observer::ValueObserver<Loop>::create(
                player->observeLoop(), [&loop](Loop value) { loop = value; });
            player->setLoop(Loop::Once);
            TLRENDER_ASSERT(Loop::Once == player->getLoop());
            TLRENDER_ASSERT(Loop::Once == loop);

            // Test the current time.
            player->setPlayback(Playback::Stop);
            otime::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver =
                observer::ValueObserver<otime::RationalTime>::create(
                    player->observeCurrentTime(),
                    [&currentTime](const otime::RationalTime& value)
                    { currentTime = value; });
            player->seek(timeRange.start_time());
            TLRENDER_ASSERT(timeRange.start_time() == player->getCurrentTime());
            TLRENDER_ASSERT(timeRange.start_time() == currentTime);
            const double rate = timeRange.duration().rate();
            player->seek(
                timeRange.start_time() + otime::RationalTime(1.0, rate));
            TLRENDER_ASSERT(
                timeRange.start_time() + otime::RationalTime(1.0, rate) ==
                currentTime);
            player->end();
            TLRENDER_ASSERT(timeRange.end_time_inclusive() == currentTime);
            player->start();
            TLRENDER_ASSERT(timeRange.start_time() == currentTime);
            player->frameNext();
            TLRENDER_ASSERT(
                timeRange.start_time() + otime::RationalTime(1.0, rate) ==
                currentTime);
            player->timeAction(TimeAction::FrameNextX10);
            player->timeAction(TimeAction::FrameNextX100);
            player->framePrev();
            player->timeAction(TimeAction::FramePrevX10);
            player->timeAction(TimeAction::FramePrevX100);
            player->timeAction(TimeAction::JumpForward1s);
            player->timeAction(TimeAction::JumpForward10s);
            player->timeAction(TimeAction::JumpBack1s);
            player->timeAction(TimeAction::JumpBack10s);

            // Test the in/out points.
            otime::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver =
                observer::ValueObserver<otime::TimeRange>::create(
                    player->observeInOutRange(),
                    [&inOutRange](const otime::TimeRange& value)
                    { inOutRange = value; });
            player->setInOutRange(otime::TimeRange(
                timeRange.start_time(), otime::RationalTime(10.0, rate)));
            TLRENDER_ASSERT(
                otime::TimeRange(
                    timeRange.start_time(), otime::RationalTime(10.0, rate)) ==
                player->getInOutRange());
            TLRENDER_ASSERT(
                otime::TimeRange(
                    timeRange.start_time(), otime::RationalTime(10.0, rate)) ==
                inOutRange);
            player->seek(
                timeRange.start_time() + otime::RationalTime(1.0, rate));
            player->setInPoint();
            player->seek(
                timeRange.start_time() + otime::RationalTime(10.0, rate));
            player->setOutPoint();
            TLRENDER_ASSERT(
                otime::TimeRange(
                    timeRange.start_time() + otime::RationalTime(1.0, rate),
                    otime::RationalTime(10.0, rate)) == inOutRange);
            player->resetInPoint();
            player->resetOutPoint();
            TLRENDER_ASSERT(
                otime::TimeRange(
                    timeRange.start_time(), timeRange.duration()) ==
                inOutRange);

            // Test the I/O options.
            io::Options ioOptions;
            auto ioOptionsObserver =
                observer::ValueObserver<io::Options>::create(
                    player->observeIOOptions(),
                    [&ioOptions](const io::Options& value)
                    { ioOptions = value; });
            io::Options ioOptions2;
            ioOptions2["Layer"] = "1";
            player->setIOOptions(ioOptions2);
            TLRENDER_ASSERT(ioOptions2 == player->getIOOptions());
            TLRENDER_ASSERT(ioOptions2 == ioOptions);
            player->setIOOptions({});

            // Test the video layers.
            int videoLayer = 0;
            std::vector<int> compareVideoLayers;
            auto videoLayerObserver = observer::ValueObserver<int>::create(
                player->observeVideoLayer(),
                [&videoLayer](int value) { videoLayer = value; });
            auto compareVideoLayersObserver =
                observer::ListObserver<int>::create(
                    player->observeCompareVideoLayers(),
                    [&compareVideoLayers](const std::vector<int>& value)
                    { compareVideoLayers = value; });
            int videoLayer2 = 1;
            player->setVideoLayer(videoLayer2);
            TLRENDER_ASSERT(videoLayer2 == player->getVideoLayer());
            TLRENDER_ASSERT(videoLayer2 == videoLayer);
            std::vector<int> compareVideoLayers2 = {2, 3};
            player->setCompareVideoLayers(compareVideoLayers2);
            TLRENDER_ASSERT(
                compareVideoLayers2 == player->getCompareVideoLayers());
            TLRENDER_ASSERT(compareVideoLayers2 == compareVideoLayers);
            player->setVideoLayer(0);
            player->setCompareVideoLayers({});

            // Test audio.
            float volume = 1.F;
            auto volumeObserver = observer::ValueObserver<float>::create(
                player->observeVolume(),
                [&volume](float value) { volume = value; });
            player->setVolume(.5F);
            TLRENDER_ASSERT(.5F == player->getVolume());
            TLRENDER_ASSERT(.5F == volume);
            player->setVolume(1.F);
            bool mute = false;
            auto muteObserver = observer::ValueObserver<bool>::create(
                player->observeMute(), [&mute](bool value) { mute = value; });
            player->setMute(true);
            TLRENDER_ASSERT(player->isMuted());
            TLRENDER_ASSERT(mute);
            player->setMute(false);
            double audioOffset = 0.0;
            auto audioOffsetObserver = observer::ValueObserver<double>::create(
                player->observeAudioOffset(),
                [&audioOffset](double value) { audioOffset = value; });
            player->setAudioOffset(0.5);
            TLRENDER_ASSERT(0.5 == player->getAudioOffset());
            TLRENDER_ASSERT(0.5 == audioOffset);
            player->setAudioOffset(0.0);

            // Test frames.
            {
                PlayerCacheOptions cacheOptions;
                auto cacheOptionsObserver =
                    observer::ValueObserver<PlayerCacheOptions>::create(
                        player->observeCacheOptions(),
                        [&cacheOptions](const PlayerCacheOptions& value)
                        { cacheOptions = value; });
                cacheOptions.readAhead = otime::RationalTime(1.0, 1.0);
                player->setCacheOptions(cacheOptions);
                TLRENDER_ASSERT(cacheOptions == player->getCacheOptions());

                auto currentVideoObserver =
                    observer::ListObserver<timeline::VideoData>::create(
                        player->observeCurrentVideo(),
                        [this](const std::vector<timeline::VideoData>& value)
                        {
                            std::stringstream ss;
                            ss << "Video time: ";
                            if (!value.empty())
                            {
                                ss << value.front().time;
                            }
                            _print(ss.str());
                        });
                auto currentAudioObserver =
                    observer::ListObserver<timeline::AudioData>::create(
                        player->observeCurrentAudio(),
                        [this](const std::vector<timeline::AudioData>& value)
                        {
                            for (const auto& i : value)
                            {
                                std::stringstream ss;
                                ss << "Audio time: " << i.seconds;
                                _print(ss.str());
                            }
                        });
                auto cacheInfoObserver =
                    observer::ValueObserver<PlayerCacheInfo>::create(
                        player->observeCacheInfo(),
                        [this](const PlayerCacheInfo& value)
                        {
                            {
                                std::stringstream ss;
                                ss << "Video/audio cached frames: "
                                   << value.videoFrames.size() << "/"
                                   << value.audioFrames.size();
                                _print(ss.str());
                            }
                        });

                for (const auto& loop : getLoopEnums())
                {
                    player->seek(timeRange.start_time());
                    player->setLoop(loop);
                    player->setPlayback(Playback::Forward);
                    auto t = std::chrono::steady_clock::now();
                    std::chrono::duration<float> diff;
                    do
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.end_time_inclusive());
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.end_time_inclusive());
                    player->setPlayback(Playback::Reverse);
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.start_time());
                    player->setSpeed(doubleSpeed);
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);
                    player->setSpeed(defaultSpeed);
                }
                player->setPlayback(Playback::Stop);
                player->clearCache();
            }
        }
    } // namespace timeline_tests
} // namespace tl
