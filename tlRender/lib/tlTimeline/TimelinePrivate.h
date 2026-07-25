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

            // The pixels per unit for OTIO spatial coordinates, taken from the
            // first clip that has them. The coordinates are unit-less, so a
            // reference is needed to map them onto a pixel size. Stays 1.0 when no
            // clip has bounds, where it is unused.
            double boundsScale = 1.0;
            // The canvas shared by the whole timeline, the union of every clip's
            // spatial coordinates. Empty when no clip has bounds, which leaves
            // the layout to the image sizes as before. The offset translates the
            // canvas minimum to the origin.
            image::Size canvasSize;      // ftk::Size2I
            math::Vector2f canvasOffset;
            // The reference size used by Spatial::Normalize for clips that have
            // no spatial coordinates of their own, taken from the first video
            // clip.
            image::Size normalizeSize;  // ftk::Size2I

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<io::VideoData> image;
                std::future<io::VideoData> imageB;
                std::optional<math::Box2f> bounds;
                std::optional<math::Box2f> boundsB;
                Transition transition = Transition::kNone;
                float transitionValue = 0.F;
            };
            struct VideoRequest
            {
                VideoRequest() {};
                VideoRequest(VideoRequest&&) = default;

                uint64_t id = 0;
                otime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<VideoFrame> promise;

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
