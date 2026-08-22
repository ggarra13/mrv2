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
        class ZipReader;

        struct Timeline::Private
        {
            std::weak_ptr<system::Context> context;
            std::weak_ptr<log::System> logSystem;
            std::shared_ptr<file::FileIO> fileIO;
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;

            void tick();

            std::shared_ptr<audio::Audio> padAudioToOneSecond(
                const std::shared_ptr<audio::Audio>&, double seconds,
                const opentime::TimeRange&);
;
            // Media references named by a bundle but not found inside it. They
            // are not read from their path, since a bundle is meant to be self
            // contained and quietly reading a file from somewhere else would be
            // misleading; reading one of these fails instead. Filled in while
            // the timeline is read and only read afterwards.
            std::set<const OTIO_NS::MediaReference*> unavailableMediaReferences;
            // Guarded by memFilesMutex once the timeline is running, since a
            // reference can also turn out to be unavailable when its byte
            // ranges are worked out on first read.
            bool mediaUnavailable(const OTIO_NS::MediaReference*);

            // Where a media reference's files live inside the bundle, worked
            // out on first use. Shared rather than copied: inside a bundle a
            // sequence reference carries a byte range per frame, and a long one
            // is not a vector to hand out by value.
            std::shared_ptr<std::vector<file::MemoryRead> > getMem(
                const OTIO_NS::MediaReference*);

            // The bundle stays open so that a media reference's byte ranges can
            // be worked out when it is first read. Doing it for every reference
            // at open meant generating a file name, decoding it as a URL and
            // parsing it as a path for all 25,000 frames of a bundle before
            // anything could be shown.
            std::shared_ptr<ZipReader> zipReader;
            std::set<const OTIO_NS::MediaReference*> bundleMediaReferences;
            void mapBundleMediaReferences(OTIO_NS::Timeline*);

            // Always the inner of the two locks: creating a reader holds
            // readCacheMutex and then asks getMemoryRead()/mediaUnavailable()
            // where the media lives. Nothing guarded here may reach back for
            // readCacheMutex.
            std::mutex memFilesMutex;
            std::map<const OTIO_NS::MediaReference*,
                     std::shared_ptr<std::vector<file::MemoryRead> > > memFiles;
            std::shared_ptr<observer::Value<bool> > timelineChanges;
            file::Path path;
            file::Path audioPath;
            Options options;
            // Owned by the request thread, like the Thread struct below.
            memory::LRUCache<std::string, std::shared_ptr<io::IRead> >
                readCache;
            // Guards the three caches below. They were owned by the request
            // thread, but a timeline opened without one is read by whichever
            // thread drives it, and the thumbnail system drives one from three.
            //
            // Always the outer of the two locks; see memFilesMutex.
            std::mutex readCacheMutex;
            // Look up the reader or decoder for a media reference, creating one
            // on a miss. The three caches differ only in what they hold and how
            // an entry is made; the availability checks either side of
            // resolving the byte ranges, the key and the lock are the same for
            // all of them, and were easy to get subtly wrong three times over.
            template<typename T>
            std::shared_ptr<T> getCached(
                memory::LRUCache<std::string, std::shared_ptr<T> >&,
                const OTIO_NS::MediaReference*,
                const io::Options&,
                const std::function<std::shared_ptr<T>(
                const std::shared_ptr<system::Context>&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&)>&);
            // Errors observed while building frames (broken promises caught
            // in videoFrame()/audioFrame()). Owned by the request thread.
            size_t frameErrorCount = 0;
            std::string frameError;
            // High water mark of the reader error total, so the count stays
            // monotonic when readers are evicted from the cache. Owned by
            // the request thread.
            size_t readErrorMax = 0;
            // Media by resolved path, built once while the timeline is read.
            // Resolving a path means decoding a URL and parsing it, so doing it
            // per lookup made every thumbnail request walk the whole timeline.
            std::map<std::string, OTIO_NS::MediaReference*> mediaByPath;
            //! The same references keyed by an absolute, normalized path, so
            //! that a caller which opened the timeline with a relative path
            //! still finds them. mediaByPath keeps the paths as written,
            //! which is what getMediaPaths() reports.
            std::map<std::string, OTIO_NS::MediaReference*> mediaByNormalPath;
            opentime::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;
            // The clip whose media references provide the video information,
            // and the information for each of those references. Both are
            // filled in while the timeline is read and only read afterwards,
            // so that getIOInfo() can follow the media reference key without
            // any I/O, and without touching the read cache from the main
            // thread.
            const OTIO_NS::Clip* videoInfoClip = nullptr;

            std::map<const OTIO_NS::MediaReference*, io::Info>
            videoInfoByReference;

            // The pixels per unit for OTIO spatial coordinates, taken from the
            // first clip that has them. The coordinates are unit-less, so a
            // reference is needed to map them onto a pixel size. Stays 1.0
            // when no clip has bounds, where it is unused.
            double boundsScale = 1.0;
            // The canvas shared by the whole timeline, the union of every
            // clip's spatial coordinates. Empty when no clip has bounds, which
            // leaves the layout to the image sizes as before. The offset
            // translates the canvas minimum to the origin.
            math::Size2i canvasSize;
            math::Vector2f canvasOffset;
            // The reference size used by Spatial::Normalize for clips that have
            // no spatial coordinates of their own, taken from the first video
            // clip.
            math::Size2i normalizeSize;

            // The largest resolution among the media references of the first
            // video clip. The canvas is built from this rather than from the
            // resolution of whichever reference happens to be active when the
            // timeline is read, so that switching to a higher resolution
            // reference is not capped by a canvas built for a proxy. Equal to
            // the resolution of the active reference when a clip has only one.
            math::Size2i maxVideoSize;
            uint64_t requestId = 0;

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
            struct PendingVideoRequest
            {
                PendingVideoRequest() {};
                PendingVideoRequest(PendingVideoRequest&&) = default;

                uint64_t id = 0;
                opentime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<VideoFrame> promise;

                std::vector<VideoLayerData> layerData;
            };

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                double seconds = -1.0;
                opentime::TimeRange timeRange;
                opentime::TimeRange clipTimeRange;
                std::future<io::AudioData> audio;
                OTIO_NS::Transition* inTransition = nullptr;
                OTIO_NS::Transition* outTransition = nullptr;
            };
            struct PendingAudioRequest
            {
                PendingAudioRequest() {};
                PendingAudioRequest(PendingAudioRequest&&) = default;

                uint64_t id = 0;
                double seconds = -1.0;
                io::Options options;
                std::promise<AudioFrame> promise;

                std::vector<AudioLayerData> layerData;
            };

            // Shared between the main thread and the request thread; every
            // field is guarded by mutex. The request queues are filled by the
            // main thread (getVideo/getAudio, cancelRequests) and drained by
            // the request thread (_requests). stopped is set by the request
            // thread at shutdown and read by the main thread to reject late
            // requests.
            struct Mutex
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;
                bool otioTimelineChanged = false;
                std::list<std::shared_ptr<PendingVideoRequest> > videoRequests;
                std::list<std::shared_ptr<PendingAudioRequest> > audioRequests;
                bool stopped = false;
                std::string readError;
                size_t readErrorCount = 0;
                // The requested media reference keys, handed to the request
                // thread. The timeline wide key applies to clips that have no
                // entry of their own in clipMediaReferenceKeys. An empty key
                // leaves a clip on the media reference that OTIO has active.
                // The OTIO timeline itself is never written, so that it can be
                // read without locking; see Timeline::setMediaReferenceKey().
                std::string mediaReferenceKey;
                std::map<const OTIO_NS::Clip*, std::string> clipMediaReferenceKeys;
                bool mediaReferenceKeysChanged = false;
                std::mutex mutex;
            };
            Mutex mutex;
            // Owned by the request thread; no locking. The in-progress lists
            // hold requests whose IO futures are outstanding. thread and
            // running are the exceptions: the main thread starts the thread
            // (in _init) and clears running (in ~Timeline) to ask it to stop;
            // running is atomic for that handoff.
            struct Thread
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;
                std::list<std::shared_ptr<PendingVideoRequest> >
                    videoRequestsInProgress;
                std::list<std::shared_ptr<PendingAudioRequest> >
                    audioRequestsInProgress;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
                std::chrono::steady_clock::time_point logTimer;
                // Copies of the media reference keys, refreshed under the mutex
                // when the main thread changes them.
                std::string mediaReferenceKey;
                std::map<const OTIO_NS::Clip*, std::string> clipMediaReferenceKeys;
            };
            Thread thread;

            // Build a finished frame from a request whose futures are ready.
            // Calling these blocks on the layer futures via get(), so callers
            // must ensure readiness (poll with wait_for, or accept the block at
            // shutdown).
            VideoFrame videoFrame(PendingVideoRequest&);
            AudioFrame audioFrame(PendingAudioRequest&);
            // Resolve which media reference a clip should be read from, using
            // the thread-owned key state. Request thread only; the main thread
            // goes through Timeline::getMediaReference(), which takes the
            // mutex.
            OTIO_NS::MediaReference* mediaReference(const OTIO_NS::Clip*) const;
        };
    } // namespace timeline
} // namespace tl
