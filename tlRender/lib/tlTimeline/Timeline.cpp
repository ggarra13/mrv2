// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>
#include <tlTimeline/ZipPrivate.h>

#include <tlIO/SequenceIO.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/URL.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

#include <algorithm>

namespace tl
{
    namespace timeline
    {

        namespace
        {
            const std::chrono::milliseconds timeout(5);

            // How often an otherwise idle timeline wakes to log itself, and so
            // how long it will wait for a request before looking again.
            const std::chrono::seconds logInterval(10);
            // How long a timeline without a thread waits for one of its own
            // requests before giving up on it.
            const std::chrono::seconds syncRequestTimeout(60);
        }

        namespace
        {
            std::atomic<size_t> objectCount = 0;
        }

        Timeline::Timeline() :
            _p(new Private)
        {
            ++objectCount;
        }

        Timeline::~Timeline()
        {
            TLRENDER_P();

            if (auto logSystem = p.logSystem.lock())
            {
                logSystem->print(
                    string::Format("tl::~Timeline {0}").arg(this),
                    p.path.get());
            }

            _stopThreads();

            --objectCount;
        }

        size_t Timeline::getObjectCount()
        {
            return objectCount;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(context, timeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            file::Path& path,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path empty;
            out->_init(context, path, empty, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            file::Path& path,
            file::Path& audioPath,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(context, path, audioPath, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const std::string& fileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path path(fileName, options.pathOptions);
            file::Path empty;
            out->_init(
                context,
                path,
                empty,
                options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const std::string& fileName,
            const std::string& audioFileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path path(fileName, options.pathOptions);
            file::Path audioPath(audioFileName, options.pathOptions);
            out->_init(
                context,
                path,
                audioPath,
                options);
            return out;
        }

        const std::weak_ptr<system::Context>& Timeline::getContext() const
        {
            return _p->context;
        }

        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&
        Timeline::getTimeline() const
        {
            return _p->otioTimeline;
        }

        std::shared_ptr<observer::IValue<bool> >
        Timeline::observeTimelineChanges() const
        {
            return _p->timelineChanges;
        }

        std::shared_ptr<observer::IList<OTIO_NS::TimeRange> >
        Timeline::observeFramesFound() const
        {
            return _p->framesFound;
        }

        void Timeline::setTimeline(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& value)
        {
            TLRENDER_P();

            // Only one of these at a time: the sequence below stops and
            // restarts the request thread and read pool, which is not safe
            // to do from two threads at once.
            std::unique_lock<std::mutex> setTimelineLock(p.setTimelineMutex);

            // Stop the request thread and read pool before touching
            // anything they read without a lock -- otioTimeline itself, the
            // indexes indexTimeline() rebuilds below, and the time range,
            // caches and canvas/video info _timelineUpdate() recomputes.
            // Previously this function swapped otioTimeline and called
            // _timelineUpdate() while _requests() and the read pool were
            // still running against the old timeline, racing with them.
            // This mirrors the stop sequence in ~Timeline(), except the
            // thread and pool are started back up afterwards instead of
            // staying down; any getVideo()/getAudio() request that arrives
            // in the meantime comes back with an empty frame immediately,
            // the same as it would after the timeline is destroyed.
            _stopThreads();

            // The request thread and read pool are stopped, so it is now
            // safe to swap in the new timeline and rebuild everything
            // derived from it -- including trimmedRangeInParent, trackItems,
            // mediaByPath and mediaByNormalPath, which used to be built only
            // once, in _init(), and were left pointing at the composables
            // and media references of whichever timeline was previously set.
            // ABA hazard: a media reference in the *new* timeline could be
            // allocated at the same address as a freed one from the old
            // timeline and be wrongly treated as already resolved, already
            // known-unavailable, or already read into memFiles.
            //
            // The zip reader and its memory mapped file are only good for
            // resolving byte ranges for those same, now-cleared, bundle
            // media references -- nothing maps the new timeline's media
            // references into the bundle the way _init() does when it first
            // opens an .otioz -- so they are dropped here too rather than
            // left open for no reason.
            {
                std::unique_lock<std::mutex> lock(p.memFilesMutex);
                p.memFiles.clear();
                p.bundleMediaReferences.clear();
                p.unavailableMediaReferences.clear();
            }
            p.zipReader.reset();
            p.fileIO.reset();

            // The request thread and read pool are stopped, so it is now
            // safe to swap in the new timeline and rebuild everything
            // derived from it -- including trimmedRangeInParent, trackItems,
            // mediaByPath and mediaByNormalPath, which used to be built only
            // once, in _init(), and were left pointing at the composables
            // and media references of whichever timeline was previously set.
            p.otioTimeline = value;
            p.indexTimeline();
            p.findSeqWatches();
            if (p.otioTimeline.value)
            {
                _timelineUpdate();
            }

            // Start the request thread and read pool back up, mirroring the
            // end of _init(). mutex.stopped has to be cleared explicitly:
            // _finishRequests(), run by the thread just joined above, set it
            // when that thread exited.
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.stopped = false;
                p.mutex.otioTimeline = p.otioTimeline;
            }
            _startThreads();
        }

        const file::Path& Timeline::getPath() const
        {
            return _p->path;
        }

        const file::Path& Timeline::getAudioPath() const
        {
            return _p->audioPath;
        }

        const Options& Timeline::getOptions() const
        {
            return _p->options;
        }

        std::shared_ptr<std::vector<file::MemoryRead> >
        Timeline::Private::getMem(const OTIO_NS::MediaReference* otioRef)
        {
            std::unique_lock<std::mutex> lock(memFilesMutex);
            if (const auto i = memFiles.find(otioRef); i != memFiles.end())
            {
                return i->second;
            }
            if (bundleMediaReferences.find(otioRef) ==
                bundleMediaReferences.end())
            {
                // Not in a bundle: read from its path.
                return std::make_shared<std::vector<file::MemoryRead> >();
            }

            // First use of this reference: work out where each of its files
            // lives inside the bundle.
            //
            // A sequence member is placed at its offset from the first frame
            // rather than packed against the previous one, so that the result
            // is indexed by frame number. Packing them would misplace every
            // frame/ after a gap, and every frame at all when the step is
            // greater than one.
            auto out = std::make_shared<std::vector<file::MemoryRead> >();
            std::vector<std::pair<size_t, std::string> > mediaFileNames;
            if (auto externalReference = dynamic_cast<const OTIO_NS::ExternalReference*>(otioRef))
            {
                mediaFileNames.push_back(std::make_pair(
                                             size_t(0),
                                             file::Path(url::decode(externalReference->target_url())).get()));
            }
            else if (auto imageSeqReference =
                     dynamic_cast<const OTIO_NS::ImageSequenceReference*>(otioRef))
            {
                const int count = imageSeqReference->number_of_images_in_sequence();
                const size_t step = std::max(imageSeqReference->frame_step(), 1);
                mediaFileNames.reserve(count);
                for (int number = 0; number < count; ++number)
                {
                    mediaFileNames.push_back(std::make_pair(
                                                 number * step,
                                                 file::Path(url::decode(
                                                               imageSeqReference->target_url_for_image_number(number))).get()));
                }
            }
            if (!mediaFileNames.empty())
            {
                out->resize(mediaFileNames.back().first + 1);
            }
            size_t found = 0;
            std::string missing;
            size_t missingCount = 0;
            // A bundle that is not mapped (on a share, or a big one on iOS)
            // gives each member as a window on the bundle file: its path,
            // offset and size, and no memory. The FFmpeg reader opens a movie
            // through the file; an image is read into memory by the sequence
            // decoder when that frame is decoded (SeqDecode). Reading them
            // all here, as this once did, read a bundle of 19,517 JPEG frames
            // whole -- 5 GB -- before the first thumbnail, and iOS killed the
            // application for it.
            const bool mapped = fileIO->getMemoryStart() != nullptr;
            for (const auto& mediaFileName : mediaFileNames)
            {
                const auto entry = zipReader->find(mediaFileName.second);
                if (!entry.has_value())
                {
                    // A sequence member the bundle does not hold is a missing
                    // frame, which the sequence decoder deals with. Leave its slot
                    // empty and carry on.
                    ++missingCount;
                    if (missing.empty())
                    {
                        missing = mediaFileName.second;
                    }
                    continue;
                }
                (*out)[mediaFileName.first] = file::MemoryRead(
                    fileIO,
                    fileIO->getMemoryStart() + entry->offset,
                    entry->size);
                ++found;
            }
            if (0 == found)
            {
                // The bundle holds none of this media. Mark the reference
                // unavailable rather than returning nothing: an empty result reads
                // as "not in a bundle", and the caller would go on to read the
                // media from its path, which is a different file than the bundle
                // describes.
                if (auto log = logSystem.lock())
                {
                    log->print(
                        "tl::Timeline",
                        string::Format(
                            "Cannot find zip entry: \"{0}\"; this media "
                            "reference cannot be used").arg(missing),
                        log::Type::Error);
                }
                unavailableMediaReferences.insert(otioRef);
                out->clear();
            }
            else if (missingCount > 0)
            {
                if (auto log = logSystem.lock())
                {
                    log->print(
                        "tl::Timeline",
                        string::Format(
                            "Bundle is missing {0} of {1} sequence frames, "
                            "starting with \"{2}\"").
                        arg(missingCount).
                        arg(mediaFileNames.size()).
                        arg(missing),
                        log::Type::Warning);
                }
            }
            memFiles[otioRef] = out;
            return out;
        }

        const OTIO_NS::TimeRange& Timeline::getTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            TLRENDER_P();
            // Follow the media reference being read, so that the information
            // describes the media on screen rather than the media that
            // happened to be active when the timeline was read. The entries
            // are fixed once the timeline has been read, so returning a
            // reference to one is safe.
            if (p.videoInfoClip)
            {
                const auto i = p.videoInfoByReference.find(
                    getMediaReference(p.videoInfoClip));
                if (i != p.videoInfoByReference.end())
                {
                    return i->second;
                }
            }
            return p.ioInfo;
        }

        void Timeline::tick()
        {
            TLRENDER_P();
            bool otioTimelineChanged = false;
            std::vector<OTIO_NS::TimeRange> foundFrames;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                otioTimelineChanged = p.mutex.otioTimelineChanged;
                p.mutex.otioTimelineChanged = false;
                foundFrames = std::move(p.mutex.foundFrames);
                p.mutex.foundFrames.clear();
            }
            if (otioTimelineChanged)
            {
                p.timelineChanges->setAlways(true);
            }
            if (!foundFrames.empty())
            {
                p.framesFound->setAlways(foundFrames);
            }
        }


        void Timeline::_getMaxVideoSize()
        {
            TLRENDER_P();
            // Scan every clip in the timeline (not just the first, and not just
            // its alternate media references) so that transitions between clips
            // of different native resolution/aspect ratio are measured against
            // the true timeline-wide maximum, not whichever clip happened to be
            // first. Readers opened here stay in the read cache.
            for (const auto& otioClip :
                     p.otioTimeline.value->find_children<OTIO_NS::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    if (auto read = _getVideoRead(i.second, p.options.ioOptions))
                    {
                        const io::Info& info = read->getInfo().get();
                        if (!info.video.empty())
                        {
                            const math::Size2i size(
                                info.video[0].size.w,
                                info.video[0].size.h);
                            if (size.w * size.h >
                                p.maxVideoSize.w * p.maxVideoSize.h)
                            {
                                p.maxVideoSize = size;
                            }
                        }
                    }
                }
            }
        }

        void Timeline::Private::findSeqWatches()
        {
            seqWatches.clear();
            seqWatchCursor = 0;
            if (zipReader || !otioTimeline.value)
            {
                // A bundle is read as it was baked; nothing here changes
                // underfoot, so there is nothing to watch.
                return;
            }
            const io::MissingFrames policy = io::getMissingFrames(options.ioOptions);
            if (io::MissingFrames::Hold != policy && io::MissingFrames::Black != policy)
            {
                return;
            }
            for (const auto& otioClip : otioTimeline.value->find_children<OTIO_NS::Clip>())
            {
                auto* mediaReference = otioClip->media_reference();
                auto* imageSeqReference =
                    dynamic_cast<OTIO_NS::ImageSequenceReference*>(mediaReference);
                if (!imageSeqReference ||
                    imageSeqReference->number_of_images_in_sequence() <= 0)
                {
                    continue;
                }

                // The window this clip actually plays from the media, not the
                // whole asset the reference describes -- a clip only trimming
                // out the first half of a longer render has no business being
                // flagged for gaps in the second half.
                const OTIO_NS::TimeRange trimmed = otioClip->trimmed_range();
                const int64_t start =
                    static_cast<int64_t>(trimmed.start_time().round().value());
                const int64_t count =
                    static_cast<int64_t>(trimmed.duration().round().value());
                const math::Int64Range frames(start, start + count - 1);

                file::Path mediaPath = timeline::getPath(
                    mediaReference, path.getDirectory(), options.pathOptions);
                mediaPath.setFrames(frames);

                io::SeqWatch watch(mediaPath, options.pathOptions);
                if (watch.hasMissing())
                {
                    seqWatches.push_back({ otioClip, std::move(watch) });
                }
            }
            seqWatchTimer = std::chrono::steady_clock::now();
        }

        std::vector<OTIO_NS::TimeRange> Timeline::Private::pollSeqWatches()
        {
            std::vector<OTIO_NS::TimeRange> out;
            if (seqWatches.empty())
            {
                return out;
            }
            const auto now = std::chrono::steady_clock::now();
            if (now - seqWatchTimer < seqWatchInterval)
            {
                return out;
            }
            seqWatchTimer = now;

            if (seqWatchCursor >= seqWatches.size())
            {
                seqWatchCursor = 0;
            }
            auto& entry = seqWatches[seqWatchCursor];
            const std::vector<int64_t> found = entry.watch.poll();
            if (!found.empty())
            {
                if (const auto range = getTrimmedRangeInParent(entry.clip))
                {
                    const OTIO_NS::TimeRange mediaRange = entry.clip->trimmed_range();
                    const double mediaRate = mediaRange.start_time().rate();
                    const double composedRate = range->start_time().rate();

                    // found is sorted; coalesce consecutive frames into one
                    // range apiece.
                    size_t i = 0;
                    while (i < found.size())
                    {
                        size_t j = i;
                        while (j + 1 < found.size() && found[j + 1] == found[j] + 1)
                        {
                            ++j;
                        }
                        const OTIO_NS::RationalTime mediaOffset(
                            static_cast<double>(found[i]) - mediaRange.start_time().value(),
                            mediaRate);
                        const OTIO_NS::RationalTime composedStart =
                            range->start_time() + mediaOffset.rescaled_to(composedRate);
                        const OTIO_NS::RationalTime composedDuration =
                            OTIO_NS::RationalTime(
                                static_cast<double>(found[j] - found[i] + 1), mediaRate)
                            .rescaled_to(composedRate);
                        out.push_back(OTIO_NS::TimeRange(composedStart, composedDuration));
                        i = j + 1;
                    }
                }
            }
            if (!entry.watch.hasMissing())
            {
                // Complete: nothing left to find until the declared range grows,
                // which only a fresh index can do.
                seqWatches.erase(seqWatches.begin() + seqWatchCursor);
                // Leave the cursor where it is -- erase() shifted the next entry
                // into this slot.
            }
            else
            {
                ++seqWatchCursor;
            }
            return out;
        }

        void Timeline::_timelineUpdate()
        {
            TLRENDER_P();

            auto timeRange = timeline::getTimeRange(p.otioTimeline.value);
            p.timeRange = timeRange.has_value() ? timeRange.value() :
                          time::invalidTimeRange;
            // The old videoInfoClip/videoInfoByReference pointers belonged
            // to the tree we just released above and are now dangling --
            // they must be rebuilt against the new tree, not reused.
            p.videoInfoClip = nullptr;
            p.videoInfoByReference.clear();
            {
                // Guarded because readMedia()/readMediaAudio() can be
                // reading these caches from another thread regardless of
                // whether the request thread is running; see readCacheMutex.
                std::unique_lock<std::mutex> lock(p.readCacheMutex);
                p.videoReadCache.clear();
                p.audioReadCache.clear();
                p.seqCache.clear();
            }
            p.maxVideoSize = math::Size2i();
            p.canvasSize = math::Size2i();
            p.canvasOffset = math::Vector2f();
            p.normalizeSize = math::Size2i();
            p.boundsScale = 1.0;

            bool videoFound = false;
            bool audioFound = false;

            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
                {
                    if (!videoFound && OTIO_NS::Track::Kind::video == otioTrack->kind())
                    {
                        videoFound = _getVideoInfo(otioTrack);
                    }
                    else if (!audioFound && OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        audioFound = _getAudioInfo(otioTrack);
                    }

                    // Break early if we've successfully found both
                    if (videoFound && audioFound)
                    {
                        break;
                    }
                }
            }
            _getMaxVideoSize();
            _getCanvas();

            for (auto& i : p.videoInfoByReference)
            {
                io::Info ioInfo = p.ioInfo;
                ioInfo.video = i.second.video;
                ioInfo.videoTime = i.second.videoTime;
                for (const auto& tag : i.second.tags)
                {
                    ioInfo.tags[tag.first] = tag.second;
                }
                i.second = ioInfo;
            }
        }

        size_t Timeline::getVideoRequestMax() const
        {
            // At least one, whatever the options say. Zero here would not mean
            // "no limit", it would mean no request is ever picked up, and a
            // timeline without a thread would wait for one that never came.
            return std::max(_p->options.readThreadCount, size_t(1)) * 2;
        }

        size_t Timeline::getReadThreadCount() const
        {
            return _p->readPool.threads.size();
        }

        void Timeline::setCacheOptions(const PlayerCacheOptions& options)
        {
            TLRENDER_P();

            double sum = (options.videoGB + options.audioGB);
            p.frameCache->setMax(sum * memory::gigabyte);
        }

        void Timeline::expandOTIOZ(const std::string& mediaPath,
                                   std::function<void(bool& aborted,
                                                      const std::string& title,
                                                      size_t done, size_t total) > progressCb)
        {
            TLRENDER_P();

            if (!p.zipReader)
                return;

            p.zipReader->saveMedia(mediaPath, progressCb);
        }

        void Timeline::_startThreads()
        {
            TLRENDER_P();

            p.thread.running = true;
            p.thread.logTimer = std::chrono::steady_clock::now();
            if (p.options.threaded)
            {
                p.startReadPool(p.options.readThreadCount);
                p.thread.thread = std::thread(
                    [this]
                        {
                            TLRENDER_P();
                            while (p.thread.running)
                            {
                                _tick();
                            }
                            _finishRequests();
                        });
            }
        }

        void Timeline::_stopThreads()
        {
            TLRENDER_P();

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.running = false;
            }
            p.thread.cv.notify_one();
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            p.stopReadPool();
        }

    } // namespace timeline
} // namespace tl
