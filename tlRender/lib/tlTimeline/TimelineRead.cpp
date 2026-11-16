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
#include <filesystem>

namespace tl
{
    namespace timeline
    {

        namespace
        {
            //! The rate an image sequence reference gives its frames, or zero
            //! for any other reference or one that does not say.
            double getSeqRate(const OTIO_NS::MediaReference* mediaReference)
            {
                double out = 0.0;
                if (auto imageSeqReference =
                    dynamic_cast<const OTIO_NS::ImageSequenceReference*>(mediaReference))
                {
                    out = std::max(imageSeqReference->rate(), 0.0);
                }
                return out;
            }

            std::string getKey(const file::Path& path, double seqRate)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                if (seqRate > 0.0)
                {
                    // A reader's rate is fixed when it is made, so two
                    // references to the same files at different rates need a
                    // reader each.
                    out.push_back(string::Format("{0}").arg(seqRate, 17));
                }
                return string::join(out, ';');
            }
        }


        template<typename T>
        std::shared_ptr<T> Timeline::Private::getCached(
            memory::LRUCache<std::string, std::shared_ptr<T> >& cache,
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            const std::function<std::shared_ptr<T>(
                   const std::shared_ptr<system::Context>&,
                   const file::Path&,
                   const std::vector<file::MemoryRead>&,
                   const io::Options&)>& create)
        {
            std::shared_ptr<T> out;
            if (mediaUnavailable(mediaReference))
            {
                // Named by the bundle but not inside it. Reading it from its
                // path would be reading a different file than the bundle
                // describes.
                return out;
            }
            const auto mediaPath = timeline::getPath(
                mediaReference,
                path.getDirectory(),
                options.pathOptions);
            const double seqRate = getSeqRate(mediaReference);
            const std::string key = getKey(mediaPath, seqRate);
            std::vector<std::shared_ptr<T> > evicted;
            std::unique_lock<std::mutex> lock(readCacheMutex);
            if (!cache.get(key, out))
            {
                auto context = this->context.lock();
                if (!context)
                {
                    return out;
                }
                try
                {
                    const auto mem = getMem(mediaReference);
                    if (mediaUnavailable(mediaReference))
                    {
                        // Resolving its byte ranges said the bundle does not
                        // hold it; reading it from its path is not the same
                        // file.
                        return out;
                    }
                    io::Options readOptions = ioOptions;
                    // Written with every digit: six significant ones made
                    // 23.976024627685547 into 23.976, which at frame 363808 (an
                    // edit starting at four hours of timecode) is a third of a
                    // frame out, and past six hours reads the wrong file.
                    readOptions["SequenceIO/DefaultSpeed"] =
                        string::Format("{0}").arg(timeRange.duration().rate(),
                                                  17);
                    if (seqRate > 0.0)
                    {
                        // The reference's rate is what its frames are numbered
                        // in, and what the clip's times were written against.
                        // The timeline's rate is not: a 24 fps sequence in a 30
                        // fps timeline read at 30 ran out of frames with a
                        // fifth of the clip still to play (#128).
                        readOptions["SeqIO/Speed"] =
                            string::Format("{0}").arg(seqRate, 17);
                    }
                    if (auto imageSeqReference =
                        dynamic_cast<const OTIO_NS::ImageSequenceReference*>(mediaReference))
                    {
                        // The reference says what to do about frames it does
                        // not have, and it is more specific than the options
                        // the timeline was opened with. A reference this
                        // timeline built for a file opened directly carries
                        // those options already.
                        readOptions["SequenceIO/MissingFrames"] = to_string(
                            fromOTIO(imageSeqReference->missing_frame_policy()));
                    }
                    out = create(context, mediaPath, *mem, readOptions);
                }
                catch (const std::exception& e)
                {
                    if (auto log = context->getLogSystem())
                    {
                        log->print(
                            "tl::Timeline",
                            string::Format("Cannot read \"{0}\": {1}").
                            arg(mediaPath.get()).arg(e.what()),
                            log::Type::Error);
                    }
                    return std::shared_ptr<T>();
                }
                if (out)
                {
                    // A reader the cache is about to let go of is let go of
                    // after the lock, not under it: its destructor joins its
                    // thread, and that thread can be waiting on a request the
                    // timeline hands out from under this same lock. Two readers
                    // kept, on a timeline of three clips, deadlocked here.
                    while (cache.getMax() > 0 && cache.getCount() >= cache.getMax())
                    {
                        const auto keys = cache.getKeys();
                        const auto values = cache.getValues();
                        if (keys.empty() || values.empty())
                            break;
                        evicted.push_back(values.back());
                        cache.remove(keys.back());
                    }
                    cache.add(key, out);
                }
            }
            return out;
        }

        std::shared_ptr<io::SeqDecode> Timeline::_getSeqDecode(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return p.getCached<io::SeqDecode>(
                p.seqCache,
                mediaReference,
                ioOptions,
                [](const std::shared_ptr<system::Context>& context,
                   const file::Path& path,
                   const std::vector<file::MemoryRead>& mem,
                   const io::Options& options)
                    {
                        std::shared_ptr<io::SeqDecode> out;
                        const auto readSystem = context->getSystem<io::ReadSystem>();
                        if (const auto plugin = readSystem->getPlugin(path))
                        {
                            // Null for a format that has to be read statefully,
                            // which leaves the caller to fall back to a reader.
                            if (const auto decode = plugin->decode(options))
                            {
                                out = io::SeqDecode::create(path, mem, decode, options);
                            }
                        }
                        return out;
                    });
        }

                bool Timeline::_getVideoIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            if (auto seq = _getSeqDecode(mediaReference, ioOptions))
            {
                out = seq->getInfo();
                return true;
            }
            if (auto videoRead = _getVideoRead(mediaReference, ioOptions))
            {
                out = videoRead->getInfo().get();
                return true;
            }
            return false;
        }

        bool Timeline::_getAudioIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            // Audio is never a sequence of stateless files.
            if (auto audioRead = _getAudioRead(mediaReference, ioOptions))
            {
                out = audioRead->getInfo().get();
                return true;
            }
            return false;
        }

        bool Timeline::_getIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            if (auto seq = _getSeqDecode(mediaReference, ioOptions))
            {
                out = seq->getInfo();
                return true;
            }
            // Both requests go out before either is waited on, so that the two
            // readers open the file at the same time.
            auto videoRead = _getVideoRead(mediaReference, ioOptions);
            auto audioRead = _getAudioRead(mediaReference, ioOptions);
            std::future<io::Info> videoFuture;
            std::future<io::Info> audioFuture;
            if (videoRead)
            {
                videoFuture = videoRead->getInfo();
            }
            if (audioRead)
            {
                audioFuture = audioRead->getInfo();
            }
            if (!videoFuture.valid() && !audioFuture.valid())
            {
                return false;
            }
            io::Info videoInfo;
            if (videoFuture.valid())
            {
                videoInfo = videoFuture.get();
            }
            io::Info audioInfo;
            if (audioFuture.valid())
            {
                audioInfo = audioFuture.get();
            }
            out = io::merge(videoInfo, audioInfo);
            return true;
        }

        std::shared_ptr<io::IVideoRead> Timeline::_getVideoRead(
            const OTIO_NS::Clip* clip,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return _getVideoRead(p.mediaReference(clip), ioOptions);
        }

        std::shared_ptr<io::IVideoRead> Timeline::_getVideoRead(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            const file::Path mediaPath = timeline::getPath(
                mediaReference, p.path.getDirectory(), p.options.pathOptions);
            auto frameCache = p.frameCache;
            return p.getCached<io::IVideoRead>(
                p.videoReadCache,
                mediaReference,
                ioOptions,
                [frameCache](const std::shared_ptr<system::Context>& context,
                             const file::Path& path,
                             const std::vector<file::MemoryRead>& mem,
                             const io::Options& options)
                    {
                        auto read = context->getSystem<io::ReadSystem>()->videoRead(
                            path, mem, options);
                        if (read)
                            read->setCache(frameCache);
                        return read;
                    });
        }

        std::shared_ptr<io::IAudioRead> Timeline::_getAudioRead(
            const OTIO_NS::Clip* clip,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return _getAudioRead(p.mediaReference(clip), ioOptions);
        }

        std::shared_ptr<io::IAudioRead> Timeline::_getAudioRead(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            const file::Path mediaPath = timeline::getPath(
                mediaReference, p.path.getDirectory(), p.options.pathOptions);
            auto frameCache = p.frameCache;
            return p.getCached<io::IAudioRead>(
                p.audioReadCache,
                mediaReference,
                ioOptions,
                [frameCache](const std::shared_ptr<system::Context>& context,
                   const file::Path& path,
                   const std::vector<file::MemoryRead>& mem,
                   const io::Options& options)
                    {
                        auto read = context->getSystem<io::ReadSystem>()->audioRead(
                            path, mem, options);

                        if (read)
                            read->setCache(frameCache);  // no-op for non-FFmpeg readers
                        return read;
                    });
        }

        std::future<io::VideoData> Timeline::_readVideo(
            const OTIO_NS::Clip* clip,
            const OTIO_NS::RationalTime& time,
            const io::Options& options,
            std::string* path,
            std::shared_ptr<io::IVideoRead>* readOut,
            OTIO_NS::RationalTime* mediaTimeOut)
        {
            TLRENDER_P();
            std::future<io::VideoData> out;
            io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            optionsMerged["USD/CameraName"] = clip->name();
            const auto mediaReference = p.mediaReference(clip);
            // A sequence is decoded on the timeline's pool; anything that has to
            // be read statefully keeps its own reader.
            auto seq = _getSeqDecode(mediaReference, optionsMerged);
            auto read = seq ? nullptr : _getVideoRead(mediaReference, optionsMerged);
            if (readOut)
            {
                *readOut = read;
            }
            if (path)
            {
                // The reader's own path rather than one derived again from the
                // media reference, so the layer names exactly what was read.
                if (seq)
                {
                    *path = seq->getPath().get();
                }
                else if (read)
                {
                    *path = read->getPath().get();
                }
            }
            const auto timeRangeOpt = p.getTrimmedRangeInParent(clip);
            if ((seq || read) && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = seq ? seq->getInfo() : read->getInfo().get();
                if (!ioInfo.videoTime.has_value())
                {
                    // No video in the media, so there is no frame to read and no
                    // rate to convert the time with.
                    return out;
                }
                const OTIO_NS::TimeRange trimmedRange = getMediaTrimmedRange(
                    clip->trimmed_range(),
                    clip->available_range(),
                    getSequenceOffset(mediaReference),
                    ioInfo.videoTime,
                    p.options.compat);
                const auto mediaTime = toVideoMediaTime(
                    time,
                    timeRangeOpt.value(),
                    trimmedRange,
                    ioInfo.videoTime->duration().rate());
                if (mediaTimeOut)
                {
                    *mediaTimeOut = mediaTime;
                }
                out = seq ?
                      p.submitRead(
                          [seq, mediaTime, optionsMerged]
                              {
                                  return seq->readVideo(mediaTime, optionsMerged);
                              },
                          time + p.timeRange.start_time()) :
                      read->readVideo(mediaTime, optionsMerged);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::_readAudio(
            const OTIO_NS::Clip* clip,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            std::future<io::AudioData> out;
            io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            auto read = _getAudioRead(clip, optionsMerged);
            const auto timeRangeOpt = p.getTrimmedRangeInParent(clip);
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                if (p.options.compat &&
                    ioInfo.audioTime.has_value() &&
                    trimmedRange.start_time() < ioInfo.audioTime->start_time())
                {
                    //! \bug If the trimmed range is less than the media time,
                    //! assume the media time is wrong (e.g., ALab trailer) and
                    //! compensate for it.
                    trimmedRange = OTIO_NS::TimeRange(
                        ioInfo.audioTime->start_time() + trimmedRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaRange = toAudioMediaTime(
                    timeRange,
                    timeRangeOpt.value(),
                    trimmedRange,
                    ioInfo.audio.sampleRate);
                out = read->readAudio(mediaRange, optionsMerged);
            }
            return out;
        }

        bool Timeline::_getVideoInfo(const OTIO_NS::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    io::Info ioInfo;
                    if (_getVideoIOInfo(
                            p.mediaReference(clip), p.options.ioOptions, ioInfo))
                    {
                        p.ioInfo.video = ioInfo.video;
                        p.ioInfo.videoTime = ioInfo.videoTime;
                        p.ioInfo.videoSource = ioInfo.videoSource;
                        p.ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());

                        // Find the largest resolution among the clip's media
                        // references, so that the canvas can hold the highest
                        // resolution one rather than only the reference that is
                        // active now. The readers opened here stay in the read
                        // cache, which also makes the first switch faster.
                        //
                        // The information reported by getIOInfo() is left as that
                        // of the active reference, since that is the media being
                        // played.
                        p.maxVideoSize = math::Size2i();
                        if (!p.ioInfo.video.empty())
                        {
                            p.maxVideoSize.w = p.ioInfo.video[0].size.w;
                            p.maxVideoSize.h = p.ioInfo.video[0].size.h;
                        }
                        p.videoInfoClip = clip;
                        for (const auto& i : clip->media_references())
                        {
                            io::Info mediaReferenceInfo;
                            if (_getVideoIOInfo(
                                    i.second, p.options.ioOptions, mediaReferenceInfo))
                            {
                                // Kept so that getIOInfo() can report the media
                                // that is actually being read; completed with the
                                // timeline level information once it is known.
                                p.videoInfoByReference[i.second] = mediaReferenceInfo;

                                if (!mediaReferenceInfo.video.empty())
                                {
                                    const image::Size& size =
                                        mediaReferenceInfo.video[0].size;
                                    if (size.w * size.h >
                                        p.maxVideoSize.w * p.maxVideoSize.h)
                                    {
                                        p.maxVideoSize.w = size.w;
                                        p.maxVideoSize.h = size.h;
                                    }
                                }
                            }
                        }
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (_getVideoInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Timeline::_getAudioInfo(const OTIO_NS::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first audio clip defines the audio information for the timeline.
                    io::Info ioInfo;
                    if (_getAudioIOInfo(
                            p.mediaReference(clip), p.options.ioOptions, ioInfo))
                    {
                        p.ioInfo.audio = ioInfo.audio;
                        p.ioInfo.audioTime = ioInfo.audioTime;
                        p.ioInfo.audioSource = ioInfo.audioSource;
                        p.ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (_getAudioInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Timeline::Private::mediaUnavailable(
            const OTIO_NS::MediaReference* mediaReference)
        {
            std::unique_lock<std::mutex> lock(memFilesMutex);
            return unavailableMediaReferences.find(mediaReference) !=
                unavailableMediaReferences.end();
        }

    } // namespace timeline
} // namespace tl
