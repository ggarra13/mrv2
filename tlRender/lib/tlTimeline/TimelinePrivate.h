// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <tlIO/Plugin.h>

#include <tlCore/LRUCache.h>

#include <opentimelineio/clip.h>

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        struct Timeline::Private
        {
            bool getVideoInfo(const otio::Composable*);
            bool getAudioInfo(const otio::Composable*);

            float transitionValue(double frame, double in, double out) const;

            void tick();
            void requests();
            void finishRequests();

            std::shared_ptr<io::IRead>
            getRead(const otio::Clip*, const io::Options&);
            std::future<io::VideoData> readVideo(
                const otio::Clip*, const otime::RationalTime&,
                const io::Options&);
            std::future<io::AudioData> readAudio(
                const otio::Clip*, const otime::TimeRange&, const io::Options&);

            std::shared_ptr<audio::Audio> padAudioToOneSecond(
                const std::shared_ptr<audio::Audio>&, double seconds,
                const otime::TimeRange&);

            std::weak_ptr<system::Context> context;
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            std::shared_ptr<observer::Value<bool> > timelineChanges;
            file::Path path;
            file::Path audioPath;
            Options options;
            memory::LRUCache<std::string, std::shared_ptr<io::IRead> >
                readCache;
            otime::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;
            uint64_t requestId = 0;

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<io::VideoData> image;
                std::future<io::VideoData> imageB;
                Transition transition = Transition::None;
                float transitionValue = 0.F;
            };
            struct VideoRequest
            {
                VideoRequest() {};
                VideoRequest(VideoRequest&&) = default;

                uint64_t id = 0;
                otime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<VideoData> promise;

                std::vector<VideoLayerData> layerData;
            };

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                double seconds = -1.0;
                otime::TimeRange timeRange;
                otime::TimeRange clipTimeRange;
                std::future<io::AudioData> audio;
                otio::Transition* inTransition = nullptr;
                otio::Transition* outTransition = nullptr;
            };
            struct AudioRequest
            {
                AudioRequest() {};
                AudioRequest(AudioRequest&&) = default;

                uint64_t id = 0;
                double seconds = -1.0;
                io::Options options;
                std::promise<AudioData> promise;

                std::vector<AudioLayerData> layerData;
            };

            struct Mutex
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
                bool otioTimelineChanged = false;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
                std::list<std::shared_ptr<VideoRequest> >
                    videoRequestsInProgress;
                std::list<std::shared_ptr<AudioRequest> >
                    audioRequestsInProgress;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
                std::chrono::steady_clock::time_point logTimer;
            };
            Thread thread;
        };
    } // namespace timeline
} // namespace tl
