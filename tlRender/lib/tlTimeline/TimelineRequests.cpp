// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/TimelinePrivate.h>

#include <filesystem>
#include <iterator>

#include <tlTimeline/Util.h>
#include <tlTimeline/ZipPrivate.h>

#include <tlIO/SequenceIO.h>
#include <tlIO/System.h>

#include <tlCore/URL.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>
#include <tlCore/LogSystem.h>
#include <tlCore/OS.h>
#include <tlCore/Path.h>
#include <tlCore/Time.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

#include <algorithm>
#include <set>

namespace tl
{
    namespace
    {
        // The order a request is served in, given where the playhead is:
        // lowest first. At and ahead of it in the direction of playback,
        // nearest first; then those behind it, in order of time, so that a
        // movie reader, which starts its decoder again whenever a time is
        // not the next, reads them in one run; then those with no time.
        double getRequestKey(
            const OTIO_NS::RationalTime* priorityTime,
            bool reverse,
            const std::optional<OTIO_NS::RationalTime>& time)
        {
            constexpr double behind = 1.0e12;
            if (!priorityTime || !time.has_value())
            {
                return 2.0 * behind;
            }
            const double d =
                time->rescaled_to(priorityTime->rate()).value() -
                priorityTime->value();
            const double ahead = reverse ? -d : d;
            return ahead >= 0.0 ? ahead : (behind + d);
        }

        const std::chrono::milliseconds timeout(5);

        // How often an otherwise idle timeline wakes to log itself, and so
        // how long it will wait for a request before looking again.
        const std::chrono::seconds logInterval(10);

        // How long a timeline without a thread waits for one of its own
        // requests before giving up on it.
        const std::chrono::seconds syncRequestTimeout(60);

        template<typename T>
        void eraseRequest(std::list<std::shared_ptr<T> >& list, const T* request)
        {
            for (auto i = list.begin(); i != list.end(); ++i)
            {
                if (i->get() == request)
                {
                    list.erase(i);
                    return;
                }
            }
        }
    }

    namespace timeline
    {

#if 0   // this improves with a new file::MemoryRead
        std::shared_ptr<std::vector<file::MemoryRead> > Timeline::Private::getMem(
            const OTIO_NS::MediaReference* otioRef)
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

            // First use of this reference: work out where each of its files lives
            // inside the bundle.
            //
            // A sequence member is placed at its offset from the first frame
            // rather than packed against the previous one, so that the result is
            // indexed by frame number. Packing them would misplace every frame
            // after a gap, and every frame at all when the step is greater than
            // one.
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
                file::MemoryRead memFile;
                if (mapped)
                {
                    memFile = file::MemoryRead(
                        fileIO,
                        fileIO->getMemoryStart() + entry->offset,
                        entry->size);
                    // A read-only mapping of the bundle, which the decoders may
                    // let go of once a frame is read.
                    memFile.mapped = true;
                }
                else
                {
                    memFile.size = entry->size;
                }
                // The disk address too: the bytes are STORED, so a reader
                // that cannot take memory -- the FFmpeg command line, or the
                // library reading a window -- can reach the same range
                // through the bundle file.
                memFile.path = file::fromFileSystem(fileIO->getPath());
                memFile.offset = entry->offset;
                (*out)[mediaFileName.first] = memFile;
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
#endif

        OTIO_NS::MediaReference* Timeline::Private::mediaReference(
            const OTIO_NS::Clip* otioClip) const
        {
            return resolveMediaReference(
                otioClip,
                thread.mediaReferenceKey,
                thread.clipMediaReferenceKeys);
        }

        std::optional<OTIO_NS::TimeRange>
        Timeline::Private::getTrimmedRangeInParent(
            const OTIO_NS::Composable* otioComposable) const
        {
            if (const auto i = trimmedRangeInParent.find(otioComposable);
                i != trimmedRangeInParent.end())
            {
                return i->second;
            }
            if (auto otioItem = dynamic_cast<const OTIO_NS::Item*>(otioComposable))
            {
                return otioItem->trimmed_range_in_parent();
            }
            return std::nullopt;
        }

        std::vector<OTIO_NS::Composable*> Timeline::Private::getTrackChildrenAt(
            const OTIO_NS::Track* otioTrack,
            const OTIO_NS::RationalTime& time) const
        {
            std::vector<OTIO_NS::Composable*> out;
            const auto i = trackItems.find(otioTrack);
            if (i == trackItems.end())
            {
                for (const auto& otioChild : otioTrack->children())
                {
                    out.push_back(otioChild.value);
                }
                return out;
            }
            const auto& items = i->second;
            auto j = std::upper_bound(
                items.begin(),
                items.end(),
                time,
                [](const OTIO_NS::RationalTime& value, const TrackItem& item)
                    {
                        return value < item.range.start_time();
                    });
            if (j != items.begin())
            {
                --j;
                if (j->range.contains(time))
                {
                    out.push_back(j->item);
                }
            }
            return out;
        }

        VideoRequest Timeline::getVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::PendingVideoRequest>();
            request->id = p.requestId;
            request->time = time;
            request->options = options;
            VideoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoFrame());
            }
            if (!p.options.threaded)
            {
                // Nothing else is going to run this request. Bounded because a
                // caller blocking on the future would have no way to find out
                // that it is never going to resolve; reaching the bound is a
                // bug here rather than a slow read.
                std::unique_lock<std::mutex> driver(p.driverMutex);
                const auto start = std::chrono::steady_clock::now();
                while (out.future.valid() &&
                       out.future.wait_for(std::chrono::seconds(0)) !=
                       std::future_status::ready)
                {
                    if (std::chrono::steady_clock::now() - start > syncRequestTimeout)
                    {
                        p.abandon(request);
                        break;
                    }
                    _tick();
                }
            }
            return out;
        }

        AudioRequest Timeline::getAudio(
            double seconds,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::PendingAudioRequest>();
            request->id = p.requestId;
            request->seconds = seconds;
            request->options = options;
            AudioRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(AudioFrame());
            }
            if (!p.options.threaded)
            {
                // Nothing else is going to run this request. Bounded because a
                // caller blocking on the future would have no way to find out
                // that it is never going to resolve; reaching the bound is a
                // bug here rather than a slow read.
                std::unique_lock<std::mutex> driver(p.driverMutex);
                const auto start = std::chrono::steady_clock::now();
                while (out.future.valid() &&
                       out.future.wait_for(std::chrono::seconds(0)) !=
                       std::future_status::ready)
                {
                    if (std::chrono::steady_clock::now() - start > syncRequestTimeout)
                    {
                        p.abandon(request);
                        break;
                    }
                    _tick();
                }
            }
            return out;
        }

        void Timeline::setRequestPriority(
            const OTIO_NS::RationalTime& time,
            bool reverse,
            bool playing)
        {
            TLRENDER_P();
            const Private::RequestPriority priority{ time, reverse, playing };
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.requestPriority = priority;
            }
            {
                std::unique_lock<std::mutex> lock(p.readPool.mutex);
                p.readPool.priority = priority;
            }
        }

        void Timeline::cancelRequests(const std::vector<uint64_t>& ids)
        {
            TLRENDER_P();

            std::list<std::shared_ptr<Private::PendingVideoRequest> > cancelledVideo;
            std::list<std::shared_ptr<Private::PendingAudioRequest> > cancelledAudio;

            auto cancel = [&ids](auto& requests, auto& cancelled)
                {
                    auto i = requests.begin();
                    while (i != requests.end())
                    {
                        if (std::find(ids.begin(), ids.end(), (*i)->id) != ids.end())
                        {
                            cancelled.push_back(std::move(*i));
                            i = requests.erase(i);
                        }
                        else
                        {
                            ++i;
                        }
                    }
                };
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                cancel(p.mutex.videoRequests, cancelledVideo);
                cancel(p.mutex.audioRequests, cancelledAudio);
                p.mutex.cancelledIds.insert(
                    p.mutex.cancelledIds.end(), ids.begin(), ids.end());
            }
            p.thread.cv.notify_one();

            // Complete them so no promise is destroyed unfulfilled. Done
            // outside the lock in case a continuation touches the timeline.
            for (auto& request : cancelledVideo)
            {
                request->promise.set_value(VideoFrame());
            }
            for (auto& request : cancelledAudio)
            {
                request->promise.set_value(AudioFrame());
            }
        }

        void Timeline::closeReaders()
        {
            TLRENDER_P();
            // Taken out under the lock and let go of after it: a reader's
            // destructor cancels what it has queued and waits for the frame in
            // its decoder, and every read of this timeline waits on the lock.
            // A read already under way holds its own reference, and finishes.
            std::vector<std::shared_ptr<io::IVideoRead> > video;
            std::vector<std::shared_ptr<io::IAudioRead> > audio;
            {
                std::unique_lock<std::mutex> lock(p.readCacheMutex);
                video = p.videoReadCache.getValues();
                audio = p.audioReadCache.getValues();
                p.videoReadCache.clear();
                p.audioReadCache.clear();
            }
        }

        float Timeline::_transitionValue(double frame, double in, double out) const
        {
            return (frame - in) / (out - in);
        }

        void Timeline::_tick()
        {
            TLRENDER_P();

            const auto t0 = std::chrono::steady_clock::now();

            _requests();

            if (const auto found = p.pollSeqWatches(); !found.empty())
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.foundFrames.insert(
                    p.mutex.foundFrames.end(), found.begin(), found.end());
            }

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - p.thread.logTimer;
            if (diff > logInterval)
            {
                p.thread.logTimer = t1;
                if (auto logSystem = p.logSystem.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        videoRequestsSize = p.mutex.videoRequests.size();
                        audioRequestsSize = p.mutex.audioRequests.size();
                    }
                    logSystem->print(
                        string::Format("tl::Timeline {0}").arg(this),
                        string::Format(
                            "\n"
                            "    * Path: {0}\n"
                            "    * Video requests: {1}, {2} in-progress, {3} max\n"
                            "    * Audio requests: {4}, {5} in-progress, {6} max").
                        arg(p.path.get()).
                        arg(videoRequestsSize).
                        arg(p.thread.videoRequestsInProgress.size()).
                        arg(getVideoRequestMax()).
                        arg(audioRequestsSize).
                        arg(p.thread.audioRequestsInProgress.size()).
                        arg(p.options.audioRequestMax));
                }
                t1 = std::chrono::steady_clock::now();
            }

            // Sleep for a bit, unless the caller is driving this itself and is
            // waiting on the very request that just finished.
            if (p.options.threaded)
            {
                //file_wrong::sleep(timeout, t0, t1);
                time::sleep(timeout, t0, t1);
            }
        }

                void Timeline::_requests()
        {
            TLRENDER_P();

            // Gather requests.
            std::list<std::shared_ptr<Private::PendingVideoRequest> > newVideoRequests;
            std::list<std::shared_ptr<Private::PendingAudioRequest> > newAudioRequests;
            std::vector<uint64_t> cancelledIds;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.cv.wait_for(
                    lock,
                    logInterval,
                    [this]
                        {
                            return
                                !_p->thread.running ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->thread.videoRequestsInProgress.empty() ||
                                !_p->mutex.audioRequests.empty() ||
                                !_p->thread.audioRequestsInProgress.empty() ||
                                !_p->mutex.cancelledIds.empty();
                        });
                cancelledIds.swap(p.mutex.cancelledIds);
                // Nearest the playhead first, when there is one to be near: the
                // oldest request is not the most urgent once the playhead has
                // moved on from it. A stable sort, so that requests the same
                // distance away keep their order.
                if (p.mutex.requestPriority.has_value() && p.mutex.videoRequests.size() > 1)
                {
                    const OTIO_NS::RationalTime priorityTime = p.mutex.requestPriority->time;
                    const bool reverse = p.mutex.requestPriority->reverse;
                    p.mutex.videoRequests.sort(
                        [&priorityTime, reverse](
                            const std::shared_ptr<Private::PendingVideoRequest>& a,
                            const std::shared_ptr<Private::PendingVideoRequest>& b)
                            {
                                return
                                    getRequestKey(&priorityTime, reverse, a->time) <
                                    getRequestKey(&priorityTime, reverse, b->time);
                            });
                }
                if (p.mutex.otioTimeline.value)
                {
                    p.thread.otioTimeline = p.mutex.otioTimeline;
                    p.mutex.otioTimeline = nullptr;
                    p.mutex.otioTimelineChanged = true;
                }
                while (!p.mutex.videoRequests.empty() &&
                       (p.thread.videoRequestsInProgress.size() + newVideoRequests.size()) <
                       getVideoRequestMax())
                {
                    newVideoRequests.push_back(p.mutex.videoRequests.front());
                    p.mutex.videoRequests.pop_front();
                }
                while (!p.mutex.audioRequests.empty() &&
                       (p.thread.audioRequestsInProgress.size() + newAudioRequests.size()) < p.options.audioRequestMax)
                {
                    newAudioRequests.push_back(p.mutex.audioRequests.front());
                    p.mutex.audioRequests.pop_front();
                }
                // Take a copy of the media reference keys so that the rest of the
                // traversal can resolve media references without locking.
                if (p.mutex.mediaReferenceKeysChanged)
                {
                    p.thread.mediaReferenceKey = p.mutex.mediaReferenceKey;
                    p.thread.clipMediaReferenceKeys = p.mutex.clipMediaReferenceKeys;
                    p.mutex.mediaReferenceKeysChanged = false;
                }
            }

            // Requests cancelled while in progress have their reads queued at
            // the readers, and a reader works through its queue in order:
            // scrubbing a long GOP movie, every seek left each reader six stale
            // frames to decode, a seek and a walk from the keyframe among them,
            // before the frame the playhead had moved to, and no frame reached
            // the screen while the finger moved. A reader's queue is cancelled
            // once nothing in progress at it is wanted any more; the frame in
            // its decoder finishes. The cancelled requests complete as usual,
            // with what the reads return, since nobody is waiting on them.
            if (!cancelledIds.empty())
            {
                std::set<std::shared_ptr<io::IVideoRead> > cancelledReads;
                std::set<std::shared_ptr<io::IVideoRead> > wantedReads;
                for (const auto& request : p.thread.videoRequestsInProgress)
                {
                    if (!request->cancelled &&
                        std::find(cancelledIds.begin(), cancelledIds.end(), request->id) != cancelledIds.end())
                    {
                        request->cancelled = true;
                    }
                    auto& reads = request->cancelled ? cancelledReads : wantedReads;
                    for (const auto& layer : request->layerData)
                    {
                        if (layer.read)
                        {
                            reads.insert(layer.read);
                        }
                        if (layer.readB)
                        {
                            reads.insert(layer.readB);
                        }
                    }
                }
                for (const auto& read : cancelledReads)
                {
                    if (wantedReads.find(read) == wantedReads.end())
                    {
                        read->cancelRequests();
                    }
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                for (const auto& otioTrack : p.otioTimeline->video_tracks())
                {
                    if (otioTrack->enabled())
                    {
                        // Only the item covering the requested time can add a
                        // layer, so bisect for it rather than asking every child of
                        // the track.
                        for (const auto& otioChild : p.getTrackChildrenAt(
                                 otioTrack, request->time - p.timeRange.start_time()))
                        {
                            if (auto otioItem = dynamic_cast<OTIO_NS::Item*>(otioChild))
                            {
                                const auto requestTime = request->time - p.timeRange.start_time();
                                OTIO_NS::ErrorStatus errorStatus;
                                const auto range = p.getTrimmedRangeInParent(otioItem);
                                if (range.has_value() && range.value().contains(requestTime))
                                {
                                    Private::VideoLayerData videoLayerData;
                                    try
                                    {
                                        if (auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioItem))
                                        {
                                            videoLayerData.image = _readVideo(otioClip, requestTime, request->options, &videoLayerData.path, &videoLayerData.read, &videoLayerData.mediaTime);
                                            videoLayerData.bounds = getCanvasBox(
                                                getMediaReferenceBounds(p.mediaReference(otioClip)),
                                                p.options.spatial,
                                                p.normalizeSize,
                                                p.boundsScale,
                                                p.canvasOffset);
                                        }
                                        const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                        if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.second.value))
                                        {
                                            if (requestTime > range.value().end_time_inclusive() - otioTransition->in_offset())
                                            {
                                                videoLayerData.transition = toTransition(otioTransition->transition_type());
                                                videoLayerData.transitionValue = _transitionValue(
                                                    requestTime.value(),
                                                    range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                    range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                                const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                                if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.second.value))
                                                {
                                                    videoLayerData.imageB = _readVideo(otioClipB, requestTime, request->options, &videoLayerData.pathB, &videoLayerData.readB, &videoLayerData.mediaTimeB);
                                                    videoLayerData.boundsB = getCanvasBox(
                                                        getMediaReferenceBounds(p.mediaReference(otioClipB)),
                                                        p.options.spatial,
                                                        p.normalizeSize,
                                                        p.boundsScale,
                                                        p.canvasOffset);
                                                }
                                            }
                                        }
                                        if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.first.value))
                                        {
                                            if (requestTime < range.value().start_time() + otioTransition->out_offset())
                                            {
                                                std::swap(videoLayerData.image, videoLayerData.imageB);
                                                std::swap(videoLayerData.bounds, videoLayerData.boundsB);
                                                std::swap(videoLayerData.path, videoLayerData.pathB);
                                                videoLayerData.transition = toTransition(otioTransition->transition_type());
                                                videoLayerData.transitionValue = _transitionValue(
                                                    requestTime.value(),
                                                    range.value().start_time().value() - otioTransition->in_offset().value() - 1.0,
                                                    range.value().start_time().value() + otioTransition->out_offset().value());
                                                const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                                if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.first.value))
                                                {
                                                    videoLayerData.image = _readVideo(otioClipB, requestTime, request->options, &videoLayerData.path, &videoLayerData.read, &videoLayerData.mediaTime);
                                                    videoLayerData.bounds = getCanvasBox(
                                                        getMediaReferenceBounds(p.mediaReference(otioClipB)),
                                                        p.options.spatial,
                                                        p.normalizeSize,
                                                        p.boundsScale,
                                                        p.canvasOffset);
                                                }
                                            }
                                        }
                                    }
                                    catch (const std::exception&)
                                    {
                                        //! \todo How should this be handled?
                                    }
                                    request->layerData.push_back(std::move(videoLayerData));
                                }
                            }
                        }
                    }
                }

                p.thread.videoRequestsInProgress.push_back(request);
            }

            // Tell the readers where the playhead is, each in its own time,
            // through a request in progress at it:
            // see io::IVideoRead::setPlayhead().
            {
                std::optional<Private::RequestPriority> priority;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    priority = p.mutex.requestPriority;
                }
                if (priority != p.thread.readersPriority || !newVideoRequests.empty())
                {
                    p.thread.readersPriority = priority;
                    std::set<io::IVideoRead*> told;
                    for (const auto& request : p.thread.videoRequestsInProgress)
                    {
                        const auto tell = [&priority, &request, &told](
                            const std::shared_ptr<io::IVideoRead>& read,
                            const OTIO_NS::RationalTime& mediaTime)
                            {
                                if (read && told.insert(read.get()).second)
                                {
                                    std::optional<io::Playhead> playhead;
                                    if (priority.has_value())
                                    {
                                        playhead = io::Playhead{
                                            mediaTime + (priority->time - request->time).rescaled_to(mediaTime.rate()),
                                            priority->reverse,
                                            priority->playing };
                                    }
                                    read->setPlayhead(playhead);
                                }
                            };
                        for (const auto& layer : request->layerData)
                        {
                            tell(layer.read, layer.mediaTime);
                            tell(layer.readB, layer.mediaTimeB);
                        }
                    }
                }
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& otioTrack : p.thread.otioTimeline->audio_tracks())
                    {
                        if (!otioTrack->enabled())
                            continue;
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<OTIO_NS::Item*>(otioChild.value))
                            {
                                const auto rangeOptional = otioItem->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const OTIO_NS::TimeRange clipTimeRange(
                                        rangeOptional.value().start_time().rescaled_to(1.0),
                                        rangeOptional.value().duration().rescaled_to(1.0));
                                    const double start = request->seconds - p.timeRange.start_time().rescaled_to(1.0).value();
                                    const OTIO_NS::TimeRange requestTimeRange =
                                        OTIO_NS::TimeRange(
                                            OTIO_NS::RationalTime(start, 1.0),
                                            OTIO_NS::RationalTime(1.0, 1.0));
                                    OTIO_NS::TimeRange transitionRange = clipTimeRange;

                                    OTIO_NS::ErrorStatus errorStatus;
                                    const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                    if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.first.value))
                                    {
                                        const auto inOffset = otioTransition->in_offset().rescaled_to(1.0);
                                        transitionRange = OTIO_NS::TimeRange(
                                            transitionRange.start_time() - inOffset,
                                            transitionRange.duration() + inOffset);
                                    }

                                    if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.second.value))
                                    {
                                        const auto outOffset = otioTransition->out_offset().rescaled_to(1.0);
                                        transitionRange = OTIO_NS::TimeRange(
                                            transitionRange.start_time(),
                                            transitionRange.duration() + outOffset);
                                    }

                                    if (requestTimeRange.intersects(transitionRange))
                                    {
                                        Private::AudioLayerData audioData;
                                        audioData.seconds = request->seconds;
                                        //! \bug Why is
                                        //! OTIO_NS::TimeRange::clamped() not
                                        //! giving us the result we expect?
                                        // audioData.timeRange =
                                        // requestTimeRange.clamped(clipTimeRange);
                                        const double start = std::max(
                                            transitionRange.start_time().value(),
                                            requestTimeRange.start_time().value());
                                        const double end = std::min(
                                            transitionRange.start_time().value() +
                                            transitionRange.duration().value(),
                                            requestTimeRange.start_time().value() +
                                            requestTimeRange.duration().value());
                                        audioData.timeRange = OTIO_NS::TimeRange(
                                            OTIO_NS::RationalTime(start, 1.0),
                                            OTIO_NS::RationalTime(end - start, 1.0));

                                        if (auto otioClip = dynamic_cast<OTIO_NS::Clip*>(otioItem))
                                        {
                                            audioData.audio = _readAudio(otioClip, audioData.timeRange, request->options);
                                        }

                                        if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.second.value))
                                        {
                                            const auto pad = OTIO_NS::RationalTime(1.0, 1.0);
                                            const auto inOffset =  otioTransition->in_offset().rescaled_to(1.0);
                                            const auto outOffset = otioTransition->out_offset().rescaled_to(1.0);
                                            auto transitionRange =
                                                OTIO_NS::TimeRange(
                                                    clipTimeRange.end_time_inclusive() - inOffset,
                                                    inOffset + outOffset + pad);
                                            if (audioData.timeRange.intersects(transitionRange))
                                            {
                                                audioData.clipTimeRange = clipTimeRange;
                                                audioData.outTransition = otioTransition;
                                            }
                                        }

                                        if (auto otioTransition =
                                            dynamic_cast<OTIO_NS::Transition*>(
                                                neighbors.first.value))
                                        {
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                            auto transitionRange =
                                                OTIO_NS::TimeRange(
                                                    clipTimeRange.start_time() -
                                                    inOffset,
                                                    outOffset + inOffset);
                                            if (audioData.timeRange.intersects(
                                                    transitionRange))
                                            {
                                                audioData.clipTimeRange =
                                                    clipTimeRange;
                                                audioData.inTransition =
                                                    otioTransition;
                                            }
                                        }
                                        request->layerData.push_back(
                                            std::move(audioData));
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                p.thread.audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = p.thread.videoRequestsInProgress.begin();
            while (videoRequestIt != p.thread.videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*videoRequestIt)->layerData)
                {
                    if (i.image.valid())
                    {
                        valid &= i.image.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                    if (i.imageB.valid())
                    {
                        valid &= i.imageB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    const auto frame = p.videoFrame(**videoRequestIt);
                    //p.updateReadErrors();
                    (*videoRequestIt)->promise.set_value(frame);
                    videoRequestIt = p.thread.videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = p.thread.audioRequestsInProgress.begin();
            while (audioRequestIt != p.thread.audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    const auto frame = p.audioFrame(**audioRequestIt);
                    //p.updateReadErrors();
                    (*audioRequestIt)->promise.set_value(frame);
                    audioRequestIt = p.thread.audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        void Timeline::_finishRequests()
        {
            TLRENDER_P();
            {
                std::list<std::shared_ptr<Private::PendingVideoRequest> > videoRequests;
                std::list<std::shared_ptr<Private::PendingAudioRequest> > audioRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.stopped = true;
                    videoRequests = std::move(p.mutex.videoRequests);
                    audioRequests = std::move(p.mutex.audioRequests);
                }
                videoRequests.insert(
                    videoRequests.begin(),
                    p.thread.videoRequestsInProgress.begin(),
                    p.thread.videoRequestsInProgress.end());
                p.thread.videoRequestsInProgress.clear();
                audioRequests.insert(
                    audioRequests.begin(),
                    p.thread.audioRequestsInProgress.begin(),
                    p.thread.audioRequestsInProgress.end());
                p.thread.audioRequestsInProgress.clear();
                for (auto& request : videoRequests)
                {
                    const auto frame = p.videoFrame(*request);
                    //p.updateReadErrors();
                    request->promise.set_value(frame);
                }
                for (auto& request : audioRequests)
                {
                    const auto frame = p.audioFrame(*request);
                    //p.updateReadErrors();
                    request->promise.set_value(frame);
                }
            }
        }

        void Timeline::Private::startReadPool(size_t threadCount)
        {
            readPool.stopped = false;
            for (size_t i = 0; i < std::max(threadCount, size_t(1)); ++i)
            {
                readPool.threads.push_back(std::thread(
                                               [this, i]
                                                   {
                                                       //ftk::setCurrentThreadName(string::Format("tl:read{0}").arg(i));
                                                       while (true)
                                                       {
                                                           ReadPool::Task task;
                                                           {
                                                               std::unique_lock<std::mutex> lock(readPool.mutex);
                                                               readPool.cv.wait(
                                                                   lock,
                                                                   [this]
                                                                       {
                                                                           return readPool.stopped || !readPool.tasks.empty();
                                                                       });
                                                               if (readPool.tasks.empty())
                                                               {
                                                                   // Stopped and drained.
                                                                   return;
                                                               }
                                                               // The one nearest the playhead, reckoned now rather
                                                               // than when it was queued: after a seek the frames
                                                               // at the new time are wanted before those queued
                                                               // ahead of them. The first, when all are equal.
                                                               auto next = readPool.tasks.begin();
                                                               if (readPool.priority.has_value())
                                                               {
                                                                   const OTIO_NS::RationalTime* priorityTime = &readPool.priority->time;
                                                                   const bool reverse = readPool.priority->reverse;
                                                                   double nextKey = getRequestKey(priorityTime, reverse, next->time);
                                                                   for (auto i = std::next(next); i != readPool.tasks.end(); ++i)
                                                                   {
                                                                       const double key = getRequestKey(priorityTime, reverse, i->time);
                                                                       if (key < nextKey)
                                                                       {
                                                                           next = i;
                                                                           nextKey = key;
                                                                       }
                                                                   }
                                                               }
                                                               task = std::move(*next);
                                                               readPool.tasks.erase(next);
                                                           }
                                                           try
                                                           {
                                                               task.promise.set_value(task.f());
                                                           }
                                                           catch (const std::exception&)
                                                           {
                                                               // Passed on rather than delivered empty: the
                                                               // frame still comes out blank, since videoFrame()
                                                               // catches this and carries on, but it is counted
                                                               // and logged instead of going by in silence.
                                                               task.promise.set_exception(std::current_exception());
                                                           }
                                                       }
                                                   }));
            }
        }

        void Timeline::Private::stopReadPool()
        {
            std::list<ReadPool::Task> dropped;
            {
                std::unique_lock<std::mutex> lock(readPool.mutex);
                readPool.stopped = true;
                // Whatever has not started decoding is not going to be looked at,
                // so give the frames back empty rather than making the close wait
                // for a queue of them.
                dropped = std::move(readPool.tasks);
                readPool.tasks.clear();
            }
            for (auto& task : dropped)
            {
                task.promise.set_value(io::VideoData());
            }
            readPool.cv.notify_all();
            for (auto& thread : readPool.threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
            readPool.threads.clear();
        }

        std::future<io::VideoData> Timeline::Private::submitRead(
            std::function<io::VideoData()> f,
            const std::optional<OTIO_NS::RationalTime>& time)
        {
            ReadPool::Task task;
            task.f = std::move(f);
            task.time = time;
            auto out = task.promise.get_future();
            if (readPool.threads.empty())
            {
                // No pool: the caller is the worker.
                try
                {
                    task.promise.set_value(task.f());
                }
                catch (const std::exception&)
                {
                    task.promise.set_exception(std::current_exception());
                }
                return out;
            }
            bool queued = false;
            {
                std::unique_lock<std::mutex> lock(readPool.mutex);
                if (!readPool.stopped)
                {
                    readPool.tasks.push_back(std::move(task));
                    queued = true;
                }
            }
            if (queued)
            {
                readPool.cv.notify_one();
            }
            else
            {
                task.promise.set_value(io::VideoData());
            }
            return out;
        }

        void Timeline::Private::abandon(
            const std::shared_ptr<PendingVideoRequest>& request)
        {
            if (auto log = logSystem.lock())
            {
                log->print("tl::Timeline", string::Format(
                               "Video request {0} did not complete: \"{1}\"").
                           arg(request->id).arg(path.get()),
                           log::Type::Error);
            }
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                eraseRequest(mutex.videoRequests, request.get());
            }
            eraseRequest(thread.videoRequestsInProgress, request.get());
            request->promise.set_value(VideoFrame());
        }

        void Timeline::Private::abandon(
            const std::shared_ptr<PendingAudioRequest>& request)
        {
            if (auto log = logSystem.lock())
            {
                log->print("tl::Timeline", string::Format(
                               "Audio request {0} did not complete: \"{1}\"").
                           arg(request->id).arg(path.get()),
                           log::Type::Error);
            }
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                eraseRequest(mutex.audioRequests, request.get());
            }
            eraseRequest(thread.audioRequestsInProgress, request.get());
            request->promise.set_value(AudioFrame());
        }

        VideoFrame Timeline::Private::videoFrame(PendingVideoRequest& request)
        {
            VideoFrame frame;
            if (!ioInfo.video.empty())
            {
                frame.size = ioInfo.video.front().size;
            }
            frame.canvasSize = canvasSize;
            frame.time = request.time;
            for (auto& i : request.layerData)
            {
                VideoLayer layer;
                try
                {
                    if (i.image.valid())
                    {
                        const io::VideoData data = i.image.get();
                        layer.image = data.image;
                        layer.missing = data.missing;
                        layer.heldFrom = data.heldFrom;
                        layer.declined = data.declined;
                    }
                    if (i.imageB.valid())
                    {
                        const io::VideoData dataB = i.imageB.get();
                        layer.imageB = dataB.image;
                        layer.declined |= dataB.declined;
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto logSystemLocked = logSystem.lock())
                    {
                        logSystemLocked->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
                layer.path = i.path;
                layer.pathB = i.pathB;
                layer.bounds = i.bounds;
                layer.boundsB = i.boundsB;
                layer.transition = i.transition;
                layer.transitionValue = i.transitionValue;
                frame.layers.push_back(layer);
            }
            return frame;
        }

        AudioFrame Timeline::Private::audioFrame(PendingAudioRequest& request)
        {
            AudioFrame frame;
            frame.seconds = request.seconds;
            for (auto& i : request.layerData)
            {
                AudioLayer layer;
                try
                {
                    if (i.audio.valid())
                    {
                        const auto audioData = i.audio.get();
                        if (audioData.audio)
                        {
                            layer.audio = padAudioToOneSecond(audioData.audio, i.seconds, i.timeRange);
                        }
                        layer.clipTimeRange = i.clipTimeRange;
                        layer.inTransition = i.inTransition;
                        layer.outTransition = i.outTransition;
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto logSystemLocked = logSystem.lock())
                    {
                        logSystemLocked->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
                frame.layers.push_back(layer);
            }
            if (frame.layers.empty())
            {
                auto audio = audio::Audio::create(ioInfo.audio, ioInfo.audio.sampleRate);
                audio->zero();
                frame.layers.push_back({ audio });
            }
            return frame;
        }

        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio,
            double seconds,
            const OTIO_NS::TimeRange& range)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s = seconds - timeRange.start_time().rescaled_to(1.0).value();
            if (range.start_time().value() > s)
            {
                const OTIO_NS::RationalTime t =
                    range.start_time() - OTIO_NS::RationalTime(s, 1.0);
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(),
                                                    t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (range.end_time_exclusive().value() < s + 1.0)
            {
                const OTIO_NS::RationalTime t =
                    OTIO_NS::RationalTime(s + 1.0, 1.0) - range.end_time_exclusive();
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(),
                                                    t2.value());
                silence->zero();
                list.push_back(silence);
            }
            // Exactly one second, as the name says. The silence either side is
            // worked out in seconds, and rescaling a fraction of a second to
            // samples can land just under a whole one -- 0.45s of it came to
            // 21599 rather than 21600 -- which left the layer a sample short of
            // the ones it is mixed with. The length comes from the sample rate
            // rather than from what the pieces happen to add up to, and
            // whatever they do not fill stays silent.
            const size_t sampleCount = audio->getInfo().sampleRate;
            auto out = audio::Audio::create(audio->getInfo(), sampleCount);
            out->zero();
            audio::move(list, out->getData(), sampleCount);
            return out;
        }

    }
}
