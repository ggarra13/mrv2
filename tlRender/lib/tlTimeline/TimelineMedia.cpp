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
#include <cmath>
#include <filesystem>

namespace tl
{
    namespace timeline
    {
        //! Resolve which media reference a clip should be read from.
        //!
        //! A key set for the clip alone takes precedence over the timeline
        //! wide key. Clips that do not have the requested key fall back to the
        //! default media key, and then to the media reference OTIO has active.
        OTIO_NS::MediaReference* resolveMediaReference(
            const OTIO_NS::Clip* otioClip,
            const std::string& key,
            const std::map<const OTIO_NS::Clip*, std::string>& clipKeys)
        {
            std::string clipKey = key;
            const auto i = clipKeys.find(otioClip);
            if (i != clipKeys.end() && !i->second.empty())
            {
                clipKey = i->second;
            }
            if (clipKey.empty())
            {
                // The common case, where no key has been set. Return early so
                // that the media reference map is not copied.
                return otioClip->media_reference();
            }
            const auto mediaReferences = otioClip->media_references();
            auto j = mediaReferences.find(clipKey);
            if (j == mediaReferences.end())
            {
                j = mediaReferences.find(OTIO_NS::Clip::default_media_key);
            }
            return j != mediaReferences.end() ?
                j->second :
                otioClip->media_reference();
        }

        std::vector<file::MemoryRead> Timeline::getMem(const OTIO_NS::MediaReference* otioRef)
        {
            TLRENDER_P();
            return *p.getMem(otioRef);
        }

        std::vector<file::Path> Timeline::getMediaPaths() const
        {
            TLRENDER_P();
            std::vector<file::Path> out;
            for (const auto& i : p.mediaByPath)
            {
                out.push_back(file::Path(i.first));
            }
            return out;
        }

        OTIO_NS::MediaReference* Timeline::_findMedia(const file::Path& path)
        {
            TLRENDER_P();
            const auto i = p.mediaByPath.find(path.get());
            if (i != p.mediaByPath.end())
            {
                return i->second;
            }
            // The media references are resolved when the timeline is read, so they
            // are usually absolute, while a caller asks with the path it was given.
            // Opening a file by a relative path would otherwise find none of its
            // own media.
            const auto j = p.mediaByNormalPath.find(normalMediaPath(path));
            return j != p.mediaByNormalPath.end() ? j->second : nullptr;
        }

        bool Timeline::getMediaInfo(
            const file::Path& path,
            io::Info& out,
            const io::Options& options)
        {
            TLRENDER_P();
            if (auto mediaReference = _findMedia(path))
            {
                return _getIOInfo(
                    mediaReference, io::merge(options, p.options.ioOptions), out);
            }
            return false;
        }

        std::optional<Timeline::MediaAt> Timeline::_mediaAt(
            const OTIO_NS::RationalTime& time)
        {
            TLRENDER_P();
            std::optional<MediaAt> out;
            // The same lookup the request thread makes: the timeline's own start is
            // taken off, and the first enabled video track holding that time wins.
            // Bisected rather than walked, because this is asked for the playhead
            // and for every ruler label that is drawn, and a sequence built out of
            // the runs of frames it has can be in a great many pieces.
            const OTIO_NS::RationalTime trackTime = time - p.timeRange.start_time();
            for (const auto& otioTrack : p.otioTimeline->video_tracks())
            {
                if (!otioTrack->enabled())
                {
                    continue;
                }
                for (const auto& otioChild :
                         p.getTrackChildrenAt(otioTrack, trackTime))
                {
                    auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioChild);
                    if (!otioClip)
                    {
                        continue;
                    }
                    const auto rangeInParent = p.getTrimmedRangeInParent(otioClip);
                    if (!rangeInParent.has_value() ||
                        !rangeInParent.value().contains(trackTime))
                    {
                        continue;
                    }

                    out = _mediaFrom(otioClip, rangeInParent.value());
                    if (out)
                    {
                        return out;
                    }
                }
            }
            return out;
        }

        std::optional<Timeline::MediaAt> Timeline::_mediaFrom(
            const OTIO_NS::Clip* otioClip,
            const OTIO_NS::TimeRange& rangeInParent)
        {
            TLRENDER_P();
            std::optional<MediaAt> out;
            const io::Options optionsMerged = p.options.ioOptions;
            auto mediaReference = p.mediaReference(otioClip);
            MediaAt mediaAt;
            mediaAt.seq = _getSeqDecode(mediaReference, optionsMerged);
            io::Info ioInfo;
            if (mediaAt.seq)
            {
                ioInfo = mediaAt.seq->getInfo();
            }
            else if (auto read = _getVideoRead(mediaReference, optionsMerged))
            {
                ioInfo = read->getInfo().get();
            }
            else
            {
                return out;
            }

            if (!ioInfo.videoTime.has_value())
            {
                // No video in the media, so there is no rate to convert times
                // with and nothing to say where the clip sits.
                return out;
            }
            // The same as _readVideo(), so that both agree on which media time
            // a timeline time means.
            const OTIO_NS::TimeRange trimmedRange = getMediaTrimmedRange(
                otioClip->trimmed_range(),
                otioClip->available_range(),
                getSequenceOffset(mediaReference),
                ioInfo.videoTime,
                p.options.compat);
            mediaAt.rangeInParent = rangeInParent;
            mediaAt.trimmedRange = trimmedRange;
            mediaAt.rate = ioInfo.videoTime->duration().rate();
            out = mediaAt;
            return out;
        }

        std::vector<Timeline::MediaAt> Timeline::_mediaAll()
        {
            TLRENDER_P();
            std::vector<MediaAt> out;
            for (const auto& otioTrack : p.otioTimeline->video_tracks())
            {
                if (!otioTrack->enabled())
                {
                    continue;
                }
                // Every clip, not just the one at some time: this is for finding
                // which clip holds a frame that was asked for by number. The ranges
                // still come from the index rather than from OTIO.
                for (const auto& otioChild : otioTrack->children())
                {
                    auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioChild.value);
                    if (!otioClip)
                    {
                        continue;
                    }
                    const auto rangeInParent = p.getTrimmedRangeInParent(otioClip);
                    if (!rangeInParent.has_value())
                    {
                        continue;
                    }
                    if (const auto mediaAt =
                        _mediaFrom(otioClip, rangeInParent.value()))
                    {
                        out.push_back(*mediaAt);
                    }
                }
            }
            return out;
        }

        OTIO_NS::RationalTime Timeline::_toMediaTime(
            const MediaAt& mediaAt,
            const OTIO_NS::RationalTime& time) const
        {
            TLRENDER_P();
            return toVideoMediaTime(
                time - p.timeRange.start_time(),
                mediaAt.rangeInParent,
                mediaAt.trimmedRange,
                mediaAt.rate);
        }

        OTIO_NS::RationalTime Timeline::_fromMediaTime(
            const MediaAt& mediaAt,
            int64_t frame) const
        {
            TLRENDER_P();

            // The inverse of toVideoMediaTime(), with the timeline's own start put
            // back on. Which frames a clip covers is the caller's business: this
            // just moves a frame number into the clip that holds it. That one
            // takes the frame showing, so this takes the first timeline frame it
            // shows on, at the timeline's rate: at the clip's, a seek would round
            // it down onto the frame before.
            const OTIO_NS::RationalTime mediaTime(
                static_cast<double>(frame), mediaAt.rate);
            const double rate = p.timeRange.duration().rate();
            const double value = (mediaTime
                                  - mediaAt.trimmedRange.start_time()
                                  + mediaAt.rangeInParent.start_time()
                                  + p.timeRange.start_time()).
                                 rescaled_to(rate).value();
            const double nearest = std::round(value);
            return OTIO_NS::RationalTime(
                std::abs(value - nearest) < 1.0e-6 ? nearest : std::ceil(value),
                rate);
        }


        std::optional<OTIO_NS::RationalTime> Timeline::getMediaTime(
            const OTIO_NS::RationalTime& time)
        {
            std::optional<OTIO_NS::RationalTime> out;
            if (const auto mediaAt = _mediaAt(time))
            {
                out = _toMediaTime(*mediaAt, time);
            }
            return out;
        }

        std::optional<int64_t> Timeline::getMediaFrame(
            const OTIO_NS::RationalTime& time)
        {
            std::optional<int64_t> out;
            if (const auto mediaTime = getMediaTime(time))
            {
                // Already whole, at the media's rate.
                out = static_cast<int64_t>(mediaTime->value());
            }
            return out;
        }

        std::optional<OTIO_NS::RationalTime> Timeline::getMediaFrameTime(
            const OTIO_NS::RationalTime& time,
            int64_t frame)
        {
            std::optional<OTIO_NS::RationalTime> out;
            if (const auto mediaAt = _mediaAt(time))
            {
                out = _fromMediaTime(*mediaAt, frame);
            }
            return out;
        }

        bool Timeline::isMediaTimeContinuous() const
        {
            TLRENDER_P();
            std::optional<std::string> path;
            std::optional<OTIO_NS::RationalTime> end;
            size_t count = 0;
            for (const auto& otioTrack : p.otioTimeline->video_tracks())
            {
                if (!otioTrack->enabled())
                {
                    continue;
                }
                for (const auto& otioChild : otioTrack->children())
                {
                    auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioChild.value);
                    if (!otioClip)
                    {
                        continue;
                    }
                    const std::string clipPath = timeline::getPath(
                        p.mediaReference(otioClip),
                        p.path.getProtocol() + p.path.getDirectory(),
                        p.options.pathOptions).get();
                    if (path.has_value() && clipPath != path.value())
                    {
                        return false;
                    }
                    path = clipPath;
                    const OTIO_NS::TimeRange range = otioClip->trimmed_range();
                    if (end.has_value() && range.start_time() < end.value())
                    {
                        return false;
                    }
                    end = range.end_time_exclusive();
                    ++count;
                }
            }
            return count > 0;
        }

        std::optional<OTIO_NS::RationalTime> Timeline::getTimelineTime(
            const OTIO_NS::RationalTime& time,
            const OTIO_NS::RationalTime& mediaTime)
        {
            std::optional<OTIO_NS::RationalTime> out;
            const auto at = _mediaAt(time);
            if (!at)
            {
                return out;
            }
            const OTIO_NS::RationalTime frame =
                mediaTime.rescaled_to(at->rate).round();

            // The clip being looked at, when it is the one holding the frame asked
            // for. This is the whole answer for a timeline whose clips cover their
            // media without a break.
            if (at->trimmedRange.contains(frame))
            {
                out = _fromMediaTime(
                    *at, static_cast<int64_t>(frame.value()));
                return out;
            }

            // Otherwise the frame belongs to one of the other clips over the same
            // media, which is what a sequence with frames left out looks like. Only
            // clips over that same media are considered, so a frame number means
            // the same thing it does in the clip it was typed against rather than
            // being matched against some other file that happens to number its
            // frames the same way.
            std::optional<MediaAt> snap;
            for (const auto& i : _mediaAll())
            {
                if (i.seq != at->seq)
                {
                    continue;
                }
                if (i.trimmedRange.contains(frame))
                {
                    out = _fromMediaTime(i, static_cast<int64_t>(frame.value()));
                    return out;
                }
                const bool before = i.trimmedRange.end_time_inclusive() < frame;
                if (before &&
                    (!snap ||
                     snap->trimmedRange.end_time_inclusive() <
                     i.trimmedRange.end_time_inclusive()))
                {
                    snap = i;
                }
            }

            // A frame that is not there at all snaps to the last one before it, or
            // to the first frame when it is before them all, so that typing a
            // number always lands somewhere.
            if (snap)
            {
                out = _fromMediaTime(
                    *snap,
                    static_cast<int64_t>(
                        snap->trimmedRange.end_time_inclusive().value()));
            }
            else
            {
                out = _fromMediaTime(
                    *at,
                    static_cast<int64_t>(at->trimmedRange.start_time().value()));
            }
            return out;
        }

        std::future<io::VideoData> Timeline::readMedia(
            const file::Path& path,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            std::future<io::VideoData> out;
            const io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            if (auto mediaReference = _findMedia(path))
            {
                if (auto seq = _getSeqDecode(mediaReference, optionsMerged))
                {
                    out = p.submitRead(
                        [seq, time, optionsMerged]
                            {
                                return seq->readVideo(time, optionsMerged);
                            });
                }
                else if (auto videoRead = _getVideoRead(mediaReference, optionsMerged))
                {
                    out = videoRead->readVideo(time, optionsMerged);
                }
            }
            return out;
        }

        std::future<io::AudioData> Timeline::readMediaAudio(
            const file::Path& path,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            std::future<io::AudioData> out;
            const io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            if (auto mediaReference = _findMedia(path))
            {
                // Audio is never a sequence of stateless files.
                if (auto audioRead = _getAudioRead(mediaReference, optionsMerged))
                {
                    out = audioRead->readAudio(timeRange, optionsMerged);
                }
            }
            return out;
        }

        std::vector<std::string> Timeline::getMediaReferenceKeys() const
        {
            TLRENDER_P();
            std::set<std::string> keys;
            for (const auto& otioClip :
                     p.otioTimeline.value->find_children<OTIO_NS::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    keys.insert(i.first);
                }
            }
            return std::vector<std::string>(keys.begin(), keys.end());
        }

        std::string Timeline::getMediaReferenceKey() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            return p.mutex.mediaReferenceKey;
        }

        void Timeline::setMediaReferenceKey(const std::string& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (value != p.mutex.mediaReferenceKey)
            {
                p.mutex.mediaReferenceKey = value;
                p.mutex.mediaReferenceKeysChanged = true;
            }
        }

        std::string Timeline::getMediaReferenceKey(
            const OTIO_NS::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            const auto i = p.mutex.clipMediaReferenceKeys.find(otioClip);
            return i != p.mutex.clipMediaReferenceKeys.end() ?
                i->second :
                std::string();
        }

        void Timeline::setMediaReferenceKey(
            const OTIO_NS::Clip* otioClip,
            const std::string& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (value.empty())
            {
                if (p.mutex.clipMediaReferenceKeys.erase(otioClip) > 0)
                {
                    p.mutex.mediaReferenceKeysChanged = true;
                }
            }
            else
            {
                auto& key = p.mutex.clipMediaReferenceKeys[otioClip];
                if (value != key)
                {
                    key = value;
                    p.mutex.mediaReferenceKeysChanged = true;
                }
            }
        }

        OTIO_NS::MediaReference* Timeline::getMediaReference(
            const OTIO_NS::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            return resolveMediaReference(
                otioClip,
                p.mutex.mediaReferenceKey,
                p.mutex.clipMediaReferenceKeys);
        }


        void Timeline::Private::indexTimeline()
        {
            // Built into fresh maps and swapped in at the end, rather than
            // cleared and refilled in place: these key on raw pointers into
            // otioTimeline, and setTimeline() can replace it with an
            // unrelated tree, which would otherwise leave entries pointing
            // at composables and media references that have already been
            // freed. Building into fresh maps first also means a
            // readMedia()/readMediaAudio() call on another thread never
            // finds mediaByPath/mediaByNormalPath briefly empty mid-rebuild;
            // it sees either the old timeline's entries or the new one's.
            std::map<const OTIO_NS::Composable*, OTIO_NS::TimeRange>
                newTrimmedRangeInParent;
            std::map<const OTIO_NS::Track*, std::vector<TrackItem> >
                newTrackItems;
            std::map<std::string, OTIO_NS::MediaReference*> newMediaByPath;
            std::map<std::string, OTIO_NS::MediaReference*> newMediaByNormalPath;
            if (otioTimeline.value)
            {
                for (const auto& otioTrack :
                         otioTimeline.value->find_children<OTIO_NS::Track>())
                {
                    OTIO_NS::ErrorStatus errorStatus;
                    const auto ranges =
                        otioTrack->range_of_all_children(&errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        continue;
                    }
                    auto& items = newTrackItems[otioTrack];
                    for (const auto& i : ranges)
                    {
                        if (const auto trimmed =
                                otioTrack->trim_child_range(i.second))
                        {
                            newTrimmedRangeInParent[i.first] = trimmed.value();
                            if (auto otioItem =
                                    dynamic_cast<OTIO_NS::Item*>(i.first))
                            {
                                items.push_back({ otioItem, trimmed.value() });
                            }
                        }
                    }
                    std::sort(
                        items.begin(),
                        items.end(),
                        [](const TrackItem& a, const TrackItem& b)
                            {
                                return a.range.start_time() <
                                       b.range.start_time();
                            });
                }
                for (const auto& otioClip :
                         otioTimeline.value->find_children<OTIO_NS::Clip>())
                {
                    for (const auto& i : otioClip->media_references())
                    {
                        if (i.second)
                        {
                            const file::Path mediaPath = timeline::getPath(
                                i.second,
                                path.getDirectory(),
                                options.pathOptions);
                            newMediaByPath[mediaPath.get()] = i.second;
                            newMediaByNormalPath[normalMediaPath(mediaPath)] =
                                i.second;
                        }
                    }
                }
            }
            // trimmedRangeInParent/trackItems are only read without locking
            // by the request thread, which indexTimeline()'s caller has
            // already stopped, so no lock is needed here to replace them.
            trimmedRangeInParent = std::move(newTrimmedRangeInParent);
            trackItems = std::move(newTrackItems);
            {
                std::unique_lock<std::mutex> lock(readCacheMutex);
                mediaByPath = std::move(newMediaByPath);
                mediaByNormalPath = std::move(newMediaByNormalPath);
            }
        }

    } // namespace timeline
} // namespace tl
