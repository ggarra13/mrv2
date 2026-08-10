// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Audio.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl.H>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const std::chrono::milliseconds timeout(5);
        }

        void Timeline::_tick()
        {
            TLRENDER_P();

            const auto t0 = std::chrono::steady_clock::now();

            _requests();

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - p.thread.logTimer;
            if (diff.count() > 10.F)
            {
                p.thread.logTimer = t1;
                if (auto context = p.context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        videoRequestsSize = p.mutex.videoRequests.size();
                        audioRequestsSize = p.mutex.audioRequests.size();
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
                            .arg(p.path.get())
                            .arg(videoRequestsSize)
                            .arg(p.thread.videoRequestsInProgress.size())
                            .arg(p.options.videoRequestCount)
                            .arg(audioRequestsSize)
                            .arg(p.thread.audioRequestsInProgress.size())
                            .arg(p.options.audioRequestCount));
                }
                t1 = std::chrono::steady_clock::now();
            }

            // Sleep for a bit.
            time::sleep(timeout, t0, t1);
        }


        VideoFrame Timeline::Private::videoFrame(PendingVideoRequest& request)
        {
            VideoFrame frame;
            frame.canvasSize = canvasSize;
            frame.size.w = maxVideoSize.w;
            frame.size.h = maxVideoSize.h;
            frame.time = request.time;
            for (auto& i : request.layerData)
            {
                VideoLayer layer;
                try
                {
                    if (i.image.valid())
                    {
                        layer.image = i.image.get().image;
                    }
                    if (i.imageB.valid())
                    {
                        layer.imageB = i.imageB.get().image;
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto context = this->context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
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
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto context = this->context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
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

        std::optional<otio::TimeRange>
        Timeline::Private::getTrimmedRangeInParent(
            const otio::Composable* otioComposable) const
        {
            if (const auto i = trimmedRangeInParent.find(otioComposable);
                i != trimmedRangeInParent.end())
            {
                return i->second;
            }
            if (auto otioItem = dynamic_cast<const otio::Item*>(otioComposable))
            {
                return otioItem->trimmed_range_in_parent();
            }
            return std::nullopt;
        }

        std::vector<otio::Composable*> Timeline::Private::getTrackChildrenAt(
            const otio::Track* otioTrack,
            const otio::RationalTime& time) const
        {
            std::vector<otio::Composable*> out;
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
                [](const otio::RationalTime& value, const TrackItem& item)
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


        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio, double seconds,
            const otime::TimeRange& range)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s =
                seconds - this->timeRange.start_time().rescaled_to(1.0).value();
            if (range.start_time().value() > s)
            {
                const otime::RationalTime t =
                    range.start_time() - otime::RationalTime(s, 1.0);
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (range.end_time_exclusive().value() < s + 1.0)
            {
                const otime::RationalTime t =
                    otime::RationalTime(s + 1.0, 1.0) - range.end_time_exclusive();
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
