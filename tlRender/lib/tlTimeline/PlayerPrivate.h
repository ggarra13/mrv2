// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlTimeline/Util.h>

#include <tlCore/AudioResample.h>
#include <tlCore/LRUCache.h>

#if defined(TLRENDER_AUDIO)
#    include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        struct Player::Private
        {
            otime::RationalTime loopPlayback(const otime::RationalTime&);

            void reverseRequests(
                const otime::RationalTime& start,
                const otime::RationalTime& end, const otime::RationalTime& inc);
            void forwardRequests(
                const otime::RationalTime& start,
                const otime::RationalTime& end, const otime::RationalTime& inc,
                const bool clearFrame = false);
            void clearRequests();
            void clearCache();
            void cacheUpdate();

            void finishedVideoRequests();

            void resetAudioTime();
#if defined(TLRENDER_AUDIO)
            static int rtAudioCallback(
                void* outputBuffer, void* inputBuffer, unsigned int nFrames,
                double streamTime, RtAudioStreamStatus status, void* userData);
            static void rtAudioErrorCallback(
                RtAudioError::Type type, const std::string& errorText);
#endif // TLRENDER_AUDIO

            void log(const std::shared_ptr<system::Context>&);

            PlayerOptions playerOptions;
            std::shared_ptr<Timeline> timeline;
            io::Info ioInfo;

            std::shared_ptr<observer::Value<double> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::List<std::shared_ptr<Timeline> > >
                compare;
            std::shared_ptr<observer::Value<CompareTimeMode> > compareTime;
            std::shared_ptr<observer::Value<io::Options> > ioOptions;
            std::shared_ptr<observer::Value<int> > videoLayer;
            std::shared_ptr<observer::List<int> > compareVideoLayers;
            std::shared_ptr<observer::List<VideoData> > currentVideoData;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::List<int> > channelMute;
            std::shared_ptr<observer::Value<double> > audioOffset;
            std::shared_ptr<observer::List<AudioData> > currentAudioData;
            std::shared_ptr<observer::Value<PlayerCacheOptions> > cacheOptions;
            std::shared_ptr<observer::Value<PlayerCacheInfo> > cacheInfo;
            std::shared_ptr<observer::ValueObserver<bool> > timelineObserver;

            struct Mutex
            {
                Playback playback = Playback::Stop;
                otime::RationalTime playbackStartTime = time::invalidTime;
                std::chrono::steady_clock::time_point playbackStartTimer;
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTimeMode compareTime = CompareTimeMode::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                std::vector<VideoData> currentVideoData;
                double audioOffset = 0.0;
                std::vector<AudioData> currentAudioData;
                bool clearRequests = false;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::Forward;
                PlayerCacheOptions cacheOptions;
                PlayerCacheInfo cacheInfo;
                std::mutex mutex;
            };
            Mutex mutex;

            struct AudioMutex
            {
                double speed = 0.0;
                float volume = 1.F;
                bool mute = false;
                std::vector<int> channelMute;
                std::chrono::steady_clock::time_point muteTimeout;
                std::map<int64_t, AudioData> audioDataCache;
                bool reset = false;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct Thread
            {
                Playback playback = Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTimeMode compareTime = CompareTimeMode::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                double audioOffset = 0.0;
                CacheDirection cacheDirection = CacheDirection::Forward;
                PlayerCacheOptions cacheOptions;

                std::map<otime::RationalTime, std::vector<VideoRequest> >
                    videoDataRequests;
                std::map<otime::RationalTime, std::vector<VideoData> >
                    videoDataCache;
#if defined(TLRENDER_AUDIO)
                std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
                std::map<int64_t, AudioRequest> audioDataRequests;
                std::chrono::steady_clock::time_point cacheTimer;
                std::chrono::steady_clock::time_point logTimer;
                std::atomic<bool> running;
                std::thread thread;
            };
            Thread thread;

            struct AudioThread
            {
                audio::Info info;
                std::shared_ptr<audio::AudioResample> resample;
                std::list<std::shared_ptr<audio::Audio> > buffer;
                std::shared_ptr<audio::Audio> silence;
                size_t rtAudioCurrentFrame = 0;
                size_t backwardsSamples = std::numeric_limits<size_t>::max();
            };
            AudioThread audioThread;
        };
    } // namespace timeline
} // namespace tl
