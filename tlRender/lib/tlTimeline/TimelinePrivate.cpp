// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const std::chrono::milliseconds timeout(5);
        }

        bool Timeline::Private::getVideoInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first video clip defines the video information for
                    // the timeline.
                    if (auto read = getRead(clip, options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
                        this->ioInfo.video = ioInfo.video;
                        this->ioInfo.videoTime = ioInfo.videoTime;
                        this->ioInfo.tags.insert(
                            ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition =
                    dynamic_cast<const otio::Composition*>(composable))
            {
                if (!composition->enabled())
                    return false;

                for (const auto& child : composition->children())
                {
                    if (getVideoInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Timeline::Private::getAudioInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first audio clip defines the audio information for
                    // the timeline.
                    if (auto read = getRead(clip, options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
                        this->ioInfo.audio = ioInfo.audio;
                        this->ioInfo.audioTime = ioInfo.audioTime;
                        this->ioInfo.tags.insert(
                            ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition =
                    dynamic_cast<const otio::Composition*>(composable))
            {
                if (!composition->enabled())
                    return false;

                for (const auto& child : composition->children())
                {
                    if (getAudioInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        float Timeline::Private::transitionValue(
            double frame, double in, double out) const
        {
            return (frame - in) / (out - in);
        }

        void Timeline::Private::tick()
        {
            const auto t0 = std::chrono::steady_clock::now();

            requests();

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - thread.logTimer;
            if (diff.count() > 10.F)
            {
                thread.logTimer = t1;
                if (auto context = this->context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        videoRequestsSize = mutex.videoRequests.size();
                        audioRequestsSize = mutex.audioRequests.size();
                    }
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}, {2} in-progress, {3} "
                            "max\n"
                            "    Audio requests: {4}, {5} in-progress, {6} max")
                            .arg(path.get())
                            .arg(videoRequestsSize)
                            .arg(thread.videoRequestsInProgress.size())
                            .arg(options.videoRequestCount)
                            .arg(audioRequestsSize)
                            .arg(thread.audioRequestsInProgress.size())
                            .arg(options.audioRequestCount));
                }
                t1 = std::chrono::steady_clock::now();
            }

            // Sleep for a bit.
            time::sleep(timeout, t0, t1);
        }

        void Timeline::Private::requests()
        {
            // Gather requests.
            std::list<std::shared_ptr<VideoRequest> > newVideoRequests;
            std::list<std::shared_ptr<AudioRequest> > newAudioRequests;
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                thread.cv.wait_for(
                    lock, options.requestTimeout,
                    [this]
                    {
                        return mutex.otioTimeline.value ||
                               !mutex.videoRequests.empty() ||
                               !thread.videoRequestsInProgress.empty() ||
                               !mutex.audioRequests.empty() ||
                               !thread.audioRequestsInProgress.empty();
                    });
                if (mutex.otioTimeline.value)
                {
                    thread.otioTimeline = mutex.otioTimeline;
                    mutex.otioTimeline = nullptr;
                    mutex.otioTimelineChanged = true;
                }
                while (!mutex.videoRequests.empty() &&
                       (thread.videoRequestsInProgress.size() +
                        newVideoRequests.size()) < options.videoRequestCount)
                {
                    newVideoRequests.push_back(mutex.videoRequests.front());
                    mutex.videoRequests.pop_front();
                }
                while (!mutex.audioRequests.empty() &&
                       (thread.audioRequestsInProgress.size() +
                        newAudioRequests.size()) < options.audioRequestCount)
                {
                    newAudioRequests.push_back(mutex.audioRequests.front());
                    mutex.audioRequests.pop_front();
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                try
                {
                    for (const auto& otioTrack :
                         thread.otioTimeline->video_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem =
                                    dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto requestTime =
                                    request->time - timeRange.start_time();
                                otio::ErrorStatus errorStatus;
                                const auto range =
                                    otioItem->trimmed_range_in_parent(
                                        &errorStatus);
                                if (range.has_value() &&
                                    range.value().contains(requestTime))
                                {
                                    VideoLayerData videoData;
                                    if (auto otioClip =
                                            dynamic_cast<const otio::Clip*>(
                                                otioItem))
                                    {
                                        videoData.image = readVideo(
                                            otioClip, requestTime,
                                            request->options);
                                    }
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.second.value))
                                    {
                                        if (requestTime >
                                            range.value().end_time_inclusive() -
                                                otioTransition->in_offset())
                                        {
                                            videoData.transition = toTransition(
                                                otioTransition
                                                    ->transition_type());
                                            videoData.transitionValue =
                                                transitionValue(
                                                    requestTime.value(),
                                                    range.value()
                                                            .end_time_inclusive()
                                                            .value() -
                                                        otioTransition
                                                            ->in_offset()
                                                            .value(),
                                                    range.value()
                                                            .end_time_inclusive()
                                                            .value() +
                                                        otioTransition
                                                            ->out_offset()
                                                            .value() +
                                                        1.0);
                                            const auto transitionNeighbors =
                                                otioTrack->neighbors_of(
                                                    otioTransition,
                                                    &errorStatus);
                                            if (const auto otioClipB =
                                                    dynamic_cast<otio::Clip*>(
                                                        transitionNeighbors
                                                            .second.value))
                                            {
                                                videoData.imageB = readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                            }
                                        }
                                    }
                                    if (auto otioTransition =
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.first.value))
                                    {
                                        if (requestTime <
                                            range.value().start_time() +
                                                otioTransition->out_offset())
                                        {
                                            std::swap(
                                                videoData.image,
                                                videoData.imageB);
                                            videoData.transition = toTransition(
                                                otioTransition
                                                    ->transition_type());
                                            videoData.transitionValue =
                                                transitionValue(
                                                    requestTime.value(),
                                                    range.value()
                                                            .start_time()
                                                            .value() -
                                                        otioTransition
                                                            ->in_offset()
                                                            .value() -
                                                        1.0,
                                                    range.value()
                                                            .start_time()
                                                            .value() +
                                                        otioTransition
                                                            ->out_offset()
                                                            .value());
                                            const auto transitionNeighbors =
                                                otioTrack->neighbors_of(
                                                    otioTransition,
                                                    &errorStatus);
                                            if (const auto otioClipB =
                                                    dynamic_cast<otio::Clip*>(
                                                        transitionNeighbors
                                                            .first.value))
                                            {
                                                videoData.image = readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                            }
                                        }
                                    }
                                    request->layerData.push_back(
                                        std::move(videoData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                thread.videoRequestsInProgress.push_back(request);
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& otioTrack :
                         thread.otioTimeline->audio_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem =
                                    dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto rangeOptional =
                                    otioItem->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const otime::TimeRange clipTimeRange(
                                        rangeOptional.value()
                                            .start_time()
                                            .rescaled_to(1.0),
                                        rangeOptional.value()
                                            .duration()
                                            .rescaled_to(1.0));
                                    const double start = request->seconds -
                                                         timeRange.start_time()
                                                             .rescaled_to(1.0)
                                                             .value();
                                    const otime::TimeRange requestTimeRange =
                                        otime::TimeRange(
                                            otime::RationalTime(start, 1.0),
                                            otime::RationalTime(1.0, 1.0));
                                    otime::TimeRange transitionRange =
                                        clipTimeRange;

                                    otio::ErrorStatus errorStatus;
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.first.value))
                                    {
                                        const auto inOffset =
                                            otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                        transitionRange = otime::TimeRange(
                                            transitionRange.start_time() -
                                                inOffset,
                                            transitionRange.duration() +
                                                inOffset);
                                    }

                                    if (auto otioTransition =
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.second.value))
                                    {
                                        const auto outOffset =
                                            otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                        transitionRange = otime::TimeRange(
                                            transitionRange.start_time(),
                                            transitionRange.duration() +
                                                outOffset);
                                    }

                                    if (requestTimeRange.intersects(
                                            transitionRange))
                                    {
                                        AudioLayerData audioData;
                                        audioData.seconds = request->seconds;
                                        //! \bug Why is
                                        //! otime::TimeRange::clamped() not
                                        //! giving us the result we expect?
                                        // audioData.timeRange =
                                        // requestTimeRange.clamped(clipTimeRange);
                                        const double start = std::max(
                                            transitionRange.start_time()
                                                .value(),
                                            requestTimeRange.start_time()
                                                .value());
                                        const double end = std::min(
                                            transitionRange.start_time()
                                                    .value() +
                                                transitionRange.duration()
                                                    .value(),
                                            requestTimeRange.start_time()
                                                    .value() +
                                                requestTimeRange.duration()
                                                    .value());
                                        audioData.timeRange = otime::TimeRange(
                                            otime::RationalTime(start, 1.0),
                                            otime::RationalTime(
                                                end - start, 1.0));

                                        if (auto otioClip =
                                                dynamic_cast<otio::Clip*>(
                                                    otioItem))
                                        {
                                            audioData.audio = readAudio(
                                                otioClip, audioData.timeRange,
                                                request->options);
                                        }

                                        if (auto otioTransition =
                                                dynamic_cast<otio::Transition*>(
                                                    neighbors.second.value))
                                        {
                                            const auto pad =
                                                otime::RationalTime(1.0, 1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                    .rescaled_to(1.0);
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                    .rescaled_to(1.0);
                                            auto transitionRange =
                                                otime::TimeRange(
                                                    clipTimeRange
                                                            .end_time_inclusive() -
                                                        inOffset,
                                                    inOffset + outOffset + pad);
                                            if (audioData.timeRange.intersects(
                                                    transitionRange))
                                            {
                                                audioData.clipTimeRange =
                                                    clipTimeRange;
                                                audioData.outTransition =
                                                    otioTransition;
                                            }
                                        }

                                        if (auto otioTransition =
                                                dynamic_cast<otio::Transition*>(
                                                    neighbors.first.value))
                                        {
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                    .rescaled_to(1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                    .rescaled_to(1.0);
                                            auto transitionRange =
                                                otime::TimeRange(
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

                thread.audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = thread.videoRequestsInProgress.begin();
            while (videoRequestIt != thread.videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*videoRequestIt)->layerData)
                {
                    if (i.image.valid())
                    {
                        valid &= i.image.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                    if (i.imageB.valid())
                    {
                        valid &= i.imageB.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                }
                if (valid)
                {
                    VideoData data;
                    if (!ioInfo.video.empty())
                    {
                        data.size = ioInfo.video.front().size;
                    }
                    data.time = (*videoRequestIt)->time;
                    try
                    {
                        for (auto& j : (*videoRequestIt)->layerData)
                        {
                            VideoLayer layer;
                            if (j.image.valid())
                            {
                                layer.image = j.image.get().image;
                            }
                            if (j.imageB.valid())
                            {
                                layer.imageB = j.imageB.get().image;
                            }
                            layer.transition = j.transition;
                            layer.transitionValue = j.transitionValue;
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*videoRequestIt)->promise.set_value(data);
                    videoRequestIt =
                        thread.videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = thread.audioRequestsInProgress.begin();
            while (audioRequestIt != thread.audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                }
                if (valid)
                {
                    AudioData data;
                    data.seconds = (*audioRequestIt)->seconds;
                    try
                    {
                        for (auto& j : (*audioRequestIt)->layerData)
                        {
                            AudioLayer layer;
                            if (j.audio.valid())
                            {
                                const auto audioData = j.audio.get();
                                if (audioData.audio)
                                {
                                    layer.audio = padAudioToOneSecond(
                                        audioData.audio, j.seconds,
                                        j.timeRange);
                                    layer.clipTimeRange = j.clipTimeRange;
                                    layer.inTransition = j.inTransition;
                                    layer.outTransition = j.outTransition;
                                }
                            }
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*audioRequestIt)->promise.set_value(data);
                    audioRequestIt =
                        thread.audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        void Timeline::Private::finishRequests()
        {
            {
                std::list<std::shared_ptr<Private::VideoRequest> >
                    videoRequests;
                std::list<std::shared_ptr<Private::AudioRequest> >
                    audioRequests;
                {
                    std::unique_lock<std::mutex> lock(mutex.mutex);
                    mutex.stopped = true;
                    videoRequests = std::move(mutex.videoRequests);
                    audioRequests = std::move(mutex.audioRequests);
                }
                videoRequests.insert(
                    videoRequests.begin(),
                    thread.videoRequestsInProgress.begin(),
                    thread.videoRequestsInProgress.end());
                thread.videoRequestsInProgress.clear();
                audioRequests.insert(
                    audioRequests.begin(),
                    thread.audioRequestsInProgress.begin(),
                    thread.audioRequestsInProgress.end());
                thread.audioRequestsInProgress.clear();
                for (auto& request : videoRequests)
                {
                    VideoData data;
                    data.time = request->time;
                    for (auto& i : request->layerData)
                    {
                        VideoLayer layer;
                        if (i.image.valid())
                        {
                            layer.image = i.image.get().image;
                        }
                        if (i.imageB.valid())
                        {
                            layer.imageB = i.imageB.get().image;
                        }
                        layer.transition = i.transition;
                        layer.transitionValue = i.transitionValue;
                        data.layers.push_back(layer);
                    }
                    request->promise.set_value(data);
                }
                for (auto& request : audioRequests)
                {
                    AudioData data;
                    data.seconds = request->seconds;
                    for (auto& i : request->layerData)
                    {
                        AudioLayer layer;
                        if (i.audio.valid())
                        {
                            layer.audio = i.audio.get().audio;
                            layer.inTransition = i.inTransition;
                            layer.outTransition = i.outTransition;
                        }
                        data.layers.push_back(layer);
                    }
                    request->promise.set_value(data);
                }
            }
        }

        namespace
        {
            std::string getKey(const file::Path& path)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                return string::join(out, ';');
            }
        } // namespace

        std::shared_ptr<io::IRead> Timeline::Private::getRead(
            const otio::Clip* clip, const io::Options& ioOptions)
        {
            std::shared_ptr<io::IRead> out;
            const auto path = timeline::getPath(
                clip->media_reference(), this->path.getDirectory(),
                options.pathOptions);
            const std::string key = getKey(path);
            if (!readCache.get(key, out))
            {
                if (auto context = this->context.lock())
                {
                    const auto memoryRead =
                        getMemoryRead(clip->media_reference());
                    io::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] =
                        string::Format("{0}").arg(timeRange.duration().rate());
                    otio::ErrorStatus error;
                    otime::RationalTime startTime = time::invalidTime;
                    const otime::TimeRange availableRange =
                        clip->available_range(&error);
                    if (!otio::is_error(error))
                    {
                        startTime = availableRange.start_time();
                    }
                    else if (clip->source_range().has_value())
                    {
                        startTime = clip->source_range().value().start_time();
                    }
                    options["FFmpeg/StartTime"] =
                        string::Format("{0}").arg(startTime);
                    const auto ioSystem = context->getSystem<io::System>();
                    out = ioSystem->read(path, memoryRead, options);
                    readCache.add(key, out);
                }
            }
            return out;
        }

        std::future<io::VideoData> Timeline::Private::readVideo(
            const otio::Clip* clip, const otime::RationalTime& time,
            const io::Options& options)
        {
            std::future<io::VideoData> out;
            io::Options optionsMerged =
                io::merge(options, this->options.ioOptions);
            optionsMerged["USD/cameraName"] = clip->name();
            auto read = getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                const auto mediaTime = timeline::toVideoMediaTime(
                    time, timeRangeOpt.value(), clip->trimmed_range(),
                    ioInfo.videoTime.duration().rate());
                out = read->readVideo(mediaTime, optionsMerged);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::Private::readAudio(
            const otio::Clip* clip, const otime::TimeRange& timeRange,
            const io::Options& options)
        {
            std::future<io::AudioData> out;
            io::Options optionsMerged =
                io::merge(options, this->options.ioOptions);
            auto read = getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                const auto mediaRange = timeline::toAudioMediaTime(
                    timeRange, timeRangeOpt.value(), clip->trimmed_range(),
                    ioInfo.audio.sampleRate);
                out = read->readAudio(mediaRange, optionsMerged);
            }
            return out;
        }

        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio, double seconds,
            const otime::TimeRange& timeRange)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s =
                seconds - this->timeRange.start_time().rescaled_to(1.0).value();
            if (timeRange.start_time().value() > s)
            {
                const otime::RationalTime t =
                    timeRange.start_time() - otime::RationalTime(s, 1.0);
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence =
                    audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (timeRange.end_time_exclusive().value() < s + 1.0)
            {
                const otime::RationalTime t =
                    otime::RationalTime(s + 1.0, 1.0) -
                    timeRange.end_time_exclusive();
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence =
                    audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            size_t sampleCount = audio::getSampleCount(list);
            auto out = audio::Audio::create(audio->getInfo(), sampleCount);
            audio::move(list, out->getData(), out->getByteCount());
            return out;
        }
    } // namespace timeline
} // namespace tl
