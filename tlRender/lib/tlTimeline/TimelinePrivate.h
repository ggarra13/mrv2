// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <tlIO/Cache.h>
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
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;

            void tick();

            std::shared_ptr<audio::Audio> padAudioToOneSecond(
                const std::shared_ptr<audio::Audio>&, double seconds,
                const otime::TimeRange&);

            // OTIO works out an item's range in its track by summing the
            // duration of every preceding sibling, and _requests() asks each
            // track child for its range on every request to find the one
            // covering the requested time. Left to OTIO that is quadratic in
            // the number of clips: 20,000 clips took nine seconds to reach
            // the first frame and 100,000 never got there. Built by
            // indexTimeline() in a single pass per track; the OTIO timeline
            // is never written, so this can be read without locking.
            std::map<const otio::Composable*, otio::TimeRange> trimmedRangeInParent;
            // The items of each track in time order. Only one item can cover a
            // given time, but _requests() used to walk and cast every child of
            // every track to find it, which kept a hundred thousand clips from
            // reaching the first frame even once the ranges above were cached.
            struct TrackItem
            {
                otio::Item* item = nullptr;
                otio::TimeRange range;
            };
            std::map<const otio::Track*, std::vector<TrackItem> > trackItems;
            // Rebuilds trimmedRangeInParent, trackItems, mediaByPath and
            // mediaByNormalPath from otioTimeline. Called once from _init(),
            // and again from setTimeline() every time otioTimeline is
            // replaced -- otherwise these would go on pointing at the
            // composables and media references of the timeline that was
            // just released. Only safe to call with the request thread and
            // read pool stopped, since trimmedRangeInParent/trackItems are
            // read without locking elsewhere on the assumption that they
            // never change out from under a reader.
            void indexTimeline();
            // The bundle stays open so that a media reference's byte ranges can
            // be worked out when it is first read. Doing it for every reference
            // at open meant generating a file name, decoding it as a URL and
            // parsing it as a path for all 25,000 frames of a bundle before
            // anything could be shown.
            std::shared_ptr<ZipReader> zipReader;
            std::set<const otio::MediaReference*> bundleMediaReferences;
            // Always the inner of the two locks: creating a reader holds
            // readCacheMutex and then asks getMemoryRead()/mediaUnavailable()
            // where the media lives. Nothing guarded here may reach back for
            // readCacheMutex.
            std::mutex memFilesMutex;
            std::map<const otio::MediaReference*,
                     std::shared_ptr<std::vector<file::MemoryRead> > > memFiles;
            std::shared_ptr<observer::Value<bool> > timelineChanges;
            // Media references named by a bundle but not found inside it. They
            // are not read from their path, since a bundle is meant to be self
            // contained and quietly reading a file from somewhere else would be
            // misleading; reading one of these fails instead. Filled in while
            // the timeline is read and only read afterwards.
            std::set<const otio::MediaReference*> unavailableMediaReferences
            ;
            // Guarded by memFilesMutex once the timeline is running, since a
            // reference can also turn out to be unavailable when its byte
            // ranges are worked out on first read.
            bool mediaUnavailable(const otio::MediaReference*);

            // Where a media reference's files live inside the bundle, worked
            // out on first use. Shared rather than copied: inside a bundle a
            // sequence reference carries a byte range per frame, and a long one
            // is not a vector to hand out by value.
            std::shared_ptr<std::vector<file::MemoryRead> > getMem(
                const otio::MediaReference*);

            // Look up the reader or decoder for a media reference, creating one
            // on a miss. The three caches differ only in what they hold and how
            // an entry is made; the availability checks either side of
            // resolving the byte ranges, the key and the lock are the same for
            // all of them, and were easy to get subtly wrong three times over.
            template<typename T>
            std::shared_ptr<T> getCached(
                memory::LRUCache<std::string, std::shared_ptr<T> >&,
                const otio::MediaReference*,
                const io::Options&,
                const std::function<std::shared_ptr<T>(
                const std::shared_ptr<system::Context>&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&)>&);

            file::Path path;
            file::Path audioPath;
            Options options;
            // Held while a caller drives an unthreaded timeline. _requests()
            // mutates the thread-owned lists without locking, on the assumption
            // that one thread runs it; without a thread that is whichever
            // caller is in getVideo()/getAudio(), and the thumbnail system
            // has three.
            std::mutex driverMutex;
            // Guards the three caches below, and mediaByPath/mediaByNormalPath
            // further down. They were owned by the request thread, but a
            // timeline opened without one is read by whichever thread drives
            // it, and the thumbnail system drives one from three.
            //
            // Always the outer of the two locks; see memFilesMutex.
            std::mutex readCacheMutex;
            // Video and audio are read by separate readers, cached separately
            // so that a reference read for only one of them -- a silent plate,
            // a bundle's .wav -- costs only that one.
            memory::LRUCache<std::string, std::shared_ptr<io::IVideoRead> > videoReadCache;
            memory::LRUCache<std::string, std::shared_ptr<io::IAudioRead> > audioReadCache;
            // Sequences, which unlike the read caches hold no thread and no
            // queue: a decoder is stateless, so what is cached here is only
            // where each frame lives. Evicting one costs nothing, which is
            // why this holds far more entries than the read caches can afford
            // to.
            memory::LRUCache<std::string, std::shared_ptr<io::SeqDecode> > seqCache;

            // Errors observed while building frames (broken promises caught
            // in videoFrame()/audioFrame()). Owned by the request thread.
            size_t frameErrorCount = 0;
            std::string frameError;
            // High water mark of the reader error total, so the count stays
            // monotonic when readers are evicted from the cache. Owned by
            // the request thread.
            size_t readErrorMax = 0;
            // Media by resolved path, built by indexTimeline(). Resolving a
            // path means decoding a URL and parsing it, so doing it per
            // lookup made every thumbnail request walk the whole timeline.
            // Unlike trimmedRangeInParent/trackItems above, this is read
            // from _findMedia() by whichever thread calls readMedia()/
            // readMediaAudio() -- not just the request thread -- so
            // indexTimeline() rebuilds it under readCacheMutex and
            // _findMedia()/getMediaPaths() take the same lock to read it.
            std::map<std::string, otio::MediaReference*> mediaByPath;
            //! The same references keyed by an absolute, normalized path, so
            //! that a caller which opened the timeline with a relative path
            //! still finds them. mediaByPath keeps the paths as written,
            //! which is what getMediaPaths() reports.
            std::map<std::string, otio::MediaReference*> mediaByNormalPath;
            otime::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;
            // The clip whose media references provide the video information,
            // and the information for each of those references. Both are
            // filled in while the timeline is read and only read afterwards,
            // so that getIOInfo() can follow the media reference key without
            // any I/O, and without touching the read cache from the main
            // thread.
            const otio::Clip* videoInfoClip = nullptr;

            std::map<const otio::MediaReference*, io::Info>
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

            // Held for the whole of setTimeline(), which stops the request
            // thread and read pool, swaps in the new timeline, rebuilds the
            // state derived from it, and starts them back up. That sequence
            // assumes it is the only one running; getVideo(), getAudio() and
            // the rest of the state guarded by mutex below are unaffected
            // and do not need this lock.
            std::mutex setTimelineMutex;
            // Shared between the main thread and the request thread; every
            // field is guarded by mutex. The request queues are filled by the
            // main thread (getVideo/getAudio, cancelRequests) and drained by
            // the request thread (_requests). stopped is set by the request
            // thread at shutdown -- including the temporary shutdown inside
            // setTimeline() -- and read by the main thread to reject late
            // requests.
            struct Mutex
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
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
                std::map<const otio::Clip*, std::string> clipMediaReferenceKeys;
                bool mediaReferenceKeysChanged = false;
                std::mutex mutex;
            };
            Mutex mutex;
            // Owned by the request thread; no locking. The in-progress lists
            // hold requests whose IO futures are outstanding. thread and
            // running are the exceptions: the main thread starts the thread
            // (in _init, and again in setTimeline() once it has a new
            // timeline to hand it) and clears running (in ~Timeline, and in
            // setTimeline() before that) to ask it to stop; running is
            // atomic for that handoff.
            struct Thread
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
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
                std::map<const otio::Clip*, std::string> clipMediaReferenceKeys;
            };
            Thread thread;

            // Where sequence frames are decoded. One pool serves every clip in
            // the timeline rather than a reader thread per clip: with 198 clips
            // and room for ten readers, a single pass used to create and join a
            // thread nearly two hundred times.
            struct ReadPool
            {
                struct Task
                {
                    std::function<io::VideoData()> f;
                    std::promise<io::VideoData> promise;
                };
                std::vector<std::thread> threads;
                std::list<Task> tasks;
                std::condition_variable cv;
                std::mutex mutex;
                bool stopped = false;
            };
            ReadPool readPool;

            // Start and stop the decoding threads.
            void startReadPool(size_t threadCount);
            void stopReadPool();
            // Decode on the pool. The future carries an empty VideoData if the
            // decode throws, which is what a reader did with a failed frame.
            std::future<io::VideoData>
            submitRead(std::function<io::VideoData()>);

            // Give up on a request that has not resolved, so that a caller
            // waiting on its future is not left waiting forever. The frame
            // comes back empty and the reason goes to the log.
            void abandon(const std::shared_ptr<PendingVideoRequest>&);
            void abandon(const std::shared_ptr<PendingAudioRequest>&);

            // Global decoded-frame caches for stateful video readers
            // (currently just FFmpeg).  Kept separate from videoReadCache on
            // purpose: a VideoRead can be evicted and recreated by
            // videoReadCache under memory/clip-count pressure,
            // but the backward-scrub frames it accumulated should survive
            // that -- this map is what makes that possible. Effectively
            // unbounded (paths, not bytes), since the actual byte budget is
            // enforced per-entry by io::Cache::setMax().
            std::shared_ptr< io::Cache > frameCache;

            // This is currently unused.
            memory::LRUCache<std::string, std::shared_ptr<io::Cache> >
            pathFrameCache;
            std::shared_ptr<io::Cache> getFrameCache(const file::Path&);

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
            otio::MediaReference* mediaReference(const otio::Clip*) const;

            //! Get a track child's trimmed range in its parent, from
            //! trimmedRangeInParent. Anything not covered by the cache, such
            //! as an item nested below a track, falls back to asking OTIO.
            std::optional<otio::TimeRange> getTrimmedRangeInParent(
                const otio::Composable*) const;

            //! Get the children of a track that can cover the given time,
            //! found by bisecting trackItems. A track that was not indexed
            //! gives back all of its children, so the caller still sees
            //! everything it used to.
            std::vector<otio::Composable*> getTrackChildrenAt(
                const otio::Track*,
                const otime::RationalTime&) const;
        };
    } // namespace timeline
} // namespace tl
