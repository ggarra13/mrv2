// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/TimelineOptions.h>

#include <tlTimeline/Audio.h>
#include <tlTimeline/PlayerOptions.h>
#include <tlTimeline/Video.h>

#include <tlIO/Read.h>
#include <tlIO/SeqDecode.h>

#include <tlCore/Context.h>
#include <tlCore/Path.h>
#include <tlCore/ValueObserver.h>

#include <opentimelineio/timeline.h>
#include <opentimelineio/mediaReference.h>

#include <future>

namespace tl
{
    namespace io
    {
        class SeqDecode;
    }

    //! Timelines.
    namespace timeline
    {

        //! Video request.
        struct VideoRequest
        {
            uint64_t id = 0;
            std::future<VideoFrame> future;
        };

        //! Audio request.
        struct AudioRequest
        {
            uint64_t id = 0;
            std::future<AudioFrame> future;
        };

        //! Timeline.
        class Timeline : public std::enable_shared_from_this<Timeline>
        {
            TLRENDER_NON_COPYABLE(Timeline);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                file::Path& inOutPath,
                file::Path& inOutAudioPath,
                const Options&);
            void _init(
                const std::shared_ptr<system::Context>&,
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const Options&);

            Timeline();

        public:
            ~Timeline();

            //! Create a new timeline.
            static std::shared_ptr<Timeline> create(
                const std::shared_ptr<system::Context>&,
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const Options& = Options());

            //! Create a new timeline from a path. The path can point to an
            //! .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                const std::shared_ptr<system::Context>&,
                file::Path& inOutPath,
                const Options& = Options());

            //! Create a new timeline from a path and audio path. The path can
            //! point to an .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                const std::shared_ptr<system::Context>&,
                file::Path& inOutPath,
                file::Path& inOutAudioPath,
                const Options& = Options());

            //! Create a new timeline from a file name. The file name can point
            //! to an .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                const std::shared_ptr<system::Context>&,
                const std::string&,
                const Options& = Options());

            //! Create a new timeline from a file name and audio file name.
            //! The file name can point to an .otio file, movie file, or
            //! image sequence.
            static std::shared_ptr<Timeline> create(
                const std::shared_ptr<system::Context>&,
                const std::string& fileName,
                const std::string& audioFilename,
                const Options& = Options());

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;

            //! Get the timeline.
            const otio::SerializableObject::Retainer<otio::Timeline>&
            getTimeline() const;

            //! Observe timeline changes.
            std::shared_ptr<observer::IValue<bool> >
            observeTimelineChanges() const;

            //! Set the timeline.
            void setTimeline(
                const otio::SerializableObject::Retainer<otio::Timeline>&);

            //! Get the file path.
            const file::Path& getPath() const;

            //! Get the audio file path.
            const file::Path& getAudioPath() const;

            //! Get the timeline options.
            const Options& getOptions() const;

            //! Get the memory for the given media reference.
            std::vector<file::MemoryRead> getMem(
                const OTIO_NS::MediaReference*);

            //! Get how many video requests the timeline keeps in flight.
            //!
            //! Twice the decoding threads: enough that a thread finishing a frame
            //! always has another waiting, without queueing work that a seek
            //! would only throw away.
            size_t getVideoRequestMax() const;

            //! Get how many sequence frames the timeline decodes at once.
            size_t getReadThreadCount() const;

            //! Get the paths of the media in the timeline.
            //!
            //! A bundle's media are byte ranges rather than files on disk, so a
            //! caller that wants one of them read cannot open its path. These
            //! name the media to getMediaInfo() and readMedia() instead, which
            //! keeps the reading on the side that knows where the bytes are.
            std::vector<file::Path> getMediaPaths() const;

            //! Get the media path (if any) at the current mediaTime.
            file::Path getMediaPath(otio::RationalTime& mediaTime);

            //! Get the information for one of the media in the timeline.
            bool getMediaInfo(
                const file::Path&,
                io::Info&,
                const io::Options& = io::Options());

            //! Read one frame of one of the media in the timeline.
            //!
            //! On a timeline with no thread the future comes back resolved.
            std::future<io::VideoData> readMedia(

                const file::Path&,
                const otio::RationalTime&,
                const io::Options& = io::Options());

            //! Read audio from one of the media in the timeline.
            std::future<io::AudioData> readMediaAudio(
                const file::Path&,
                const otio::TimeRange&,
                const io::Options& = io::Options());

            //! \name Media References
            ///
            //! Clips may carry several media references, for example a proxy
            //! and a full resolution version of the same media, and one of
            //! them is active at a time. Which one is active is tracked here
            //! rather than written back to the OTIO timeline, so that the
            //! timeline can be read by the request thread without locking.
            ///@{

            //! Get the media reference keys used anywhere in the timeline,
            //! sorted and without duplicates.
            std::vector<std::string> getMediaReferenceKeys() const;

            //! Get the media reference key applied to the whole timeline. An
            //! empty key, the default, leaves every clip on the media
            //! reference that OTIO has active.
            std::string getMediaReferenceKey() const;

            //! Set the media reference key for the whole timeline. Clips that
            //! have no media reference with this key fall back to
            //! otio::Clip::default_media_key, and then to the media reference
            //! OTIO has active.
            //!
            //! The change applies to media read after it; the caller is
            //! responsible for discarding anything already read, for example
            //! with Player::clearCache().
            void setMediaReferenceKey(const std::string&);

            //! Get the media reference key applied to the given clip, which
            //! may be empty. This is the key set for the clip alone, not the
            //! timeline wide key it falls back to.
            std::string getMediaReferenceKey(const otio::Clip*) const;

            //! Set the media reference key for a single clip, overriding the
            //! timeline wide key. An empty key returns the clip to the timeline
            //! wide key.
            void setMediaReferenceKey(
                const otio::Clip*,
                const std::string&);

            //! Get the media reference a clip is read from, honoring the keys
            //! set above.
            otio::MediaReference* getMediaReference(
                const otio::Clip*) const;

            ///@}

            //! \name Information
            ///@{

            //! Get the time range.
            const otime::TimeRange& getTimeRange() const;

            //! Get the I/O information. This information is retrieved from
            //! the first clip in the timeline.
            const io::Info& getIOInfo() const;

            ///@}

            //! \name Video and Audio Data
            ///@{

            //! Get video data.
            VideoRequest getVideo(
                const otime::RationalTime&, const io::Options& = io::Options());

            //! Get audio data.
            AudioRequest
            getAudio(double seconds, const io::Options& = io::Options());

            //! Cancel requests.
            void cancelRequests(const std::vector<uint64_t>&);

            ///@}

            //! Stats
            size_t getObjectCount();

            //! Tick the timeline.
            void tick();

            //! Set the cache options.
            void setCacheOptions(const PlayerCacheOptions&);

        private:
            void _tick();
            void _requests();
            void _finishRequests();

            //! What is needed to convert between timeline time and media time for
            //! the clip at a time.
            struct MediaAt
            {
                std::shared_ptr<io::SeqDecode> seq;
                otio::TimeRange rangeInParent;
                otio::TimeRange trimmedRange;
                double rate = 0.0;
            };
            std::optional<MediaAt> _mediaAt(const otio::RationalTime&);
            std::optional<MediaAt> _mediaFrom(
                const otio::Clip*,
                const otio::TimeRange& rangeInParent);
            std::vector<MediaAt> _mediaAll();
            otio::RationalTime _toMediaTime(
                const MediaAt&,
                const otio::RationalTime&) const;
            otio::RationalTime _fromMediaTime(const MediaAt&, int64_t frame) const;

            // Find a media reference by its resolved path.
            otio::MediaReference* _findMedia(const file::Path&);
            // Get the sequence for a media reference, or null when the format is
            // not read as a sequence of stateless files: a movie carries a
            // demuxer position and keeps its own reader.
            std::shared_ptr<io::SeqDecode> _getSeqDecode(
                const otio::MediaReference*,
                const io::Options&);
            // Get one half of the information for a media reference, from
            // whichever of the decoder or the reader provides it.
            bool _getVideoIOInfo(
                const otio::MediaReference*,
                const io::Options&,
                io::Info&);
            bool _getAudioIOInfo(
                const otio::MediaReference*,
                const io::Options&,
                io::Info&);
            // Get both halves, merged. Callers that want only one half should
            // ask for it: asking for both opens both readers.
            bool _getIOInfo(
                const otio::MediaReference*,
                const io::Options&,
                io::Info&);
            // Get the reader for one half of a media reference. Video and audio
            // are separate readers and separately cached, so a reference that
            // is read for only one of them costs only that one.
            std::shared_ptr<io::IVideoRead> _getVideoRead(
                const otio::Clip*,
                const io::Options&);
            std::shared_ptr<io::IVideoRead> _getVideoRead(
                const otio::MediaReference*,
                const io::Options&);
            std::shared_ptr<io::IAudioRead> _getAudioRead(
            const otio::Clip*,
            const io::Options&);
            std::shared_ptr<io::IAudioRead> _getAudioRead(
                const otio::MediaReference*,
                const io::Options&);
            std::future<io::VideoData> _readVideo(
                const otio::Clip*, const otime::RationalTime&,
                const io::Options&);
            std::future<io::AudioData> _readAudio(
                const otio::Clip*, const otime::TimeRange&,
                const io::Options&);
            bool _getVideoInfo(const otio::Composable*);  // OK
            bool _getAudioInfo(const otio::Composable*);  // OK
            void _getCanvas();  // OK
            void _getMaxVideoSize(); // OK
            void _timelineUpdate();  // OK
            float _transitionValue(double frame, double in, double out) const;


            TLRENDER_PRIVATE();
        };
    } // namespace timeline
} // namespace tl
