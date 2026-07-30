// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Audio.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>
#include <tlCore/Path.h>
#include <tlCore/ValueObserver.h>

#include <tlIO/Plugin.h>

#include <opentimelineio/timeline.h>
#include <opentimelineio/mediaReference.h>

#include <future>

namespace tl
{
    //! Timelines.
    namespace timeline
    {
        //! File sequence.
        enum class FileSequenceAudio {
            kNone,      //!< No audio
            BaseName,  //!< Search for an audio file with the same base name as
                       //!< the file sequence
            FileName,  //!< Use the given audio file name
            Directory, //!< Use the first audio file in the given directory

            Count,
            First = kNone
        };
        TLRENDER_ENUM(FileSequenceAudio);
        TLRENDER_ENUM_SERIALIZE(FileSequenceAudio);

        //! Spatial coordinate options.
        enum class Spatial
        {
            //! Ignore the OTIO spatial coordinates, laying out clips from their
            //! image sizes
            kNone,

            //! Use the OTIO spatial coordinates where clips provide them
            Coordinates,

            //! Use the OTIO spatial coordinates, and give clips without them
            //! the size of the first video clip, so that clips of differing
            //! resolutions are all displayed at the same size
            Normalize,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(Spatial);

        //! Timeline options.
        struct Options
        {
            FileSequenceAudio fileSequenceAudio = FileSequenceAudio::BaseName;

            //! Spatial coordinates.
            Spatial spatial = Spatial::Coordinates;

            std::string fileSequenceAudioFileName;
            std::string fileSequenceAudioDirectory;

            //! Enable workarounds for timelines that may not conform exactly
            //! to specification.
            bool compat = true;

            //! Maximum number of video requests.
            size_t videoRequestCount = 16;

            //! Maximum number of audio requests.
            size_t audioRequestCount = 16;

            //! Request timeout.
            std::chrono::milliseconds requestTimeout =
                std::chrono::milliseconds(5);

            //! I/O options.
            io::Options ioOptions;

            //! Path options.
            file::PathOptions pathOptions;

            bool operator==(const Options&) const;
            bool operator!=(const Options&) const;
        };

        //! Create a new timeline from a path. The path can point to an .otio
        //! file, .otioz file, movie file, or image sequence.
        otio::SerializableObject::Retainer<otio::Timeline> create(
            file::Path&, const std::shared_ptr<system::Context>&,
            const otime::RationalTime& = time::invalidTime,
            const Options& = Options());

        //! Create a new timeline from a path and audio path. The file name
        //! can point to an .otio file, .otioz file, movie file, or image
        //! sequence.
        otio::SerializableObject::Retainer<otio::Timeline> create(
            file::Path& path, const file::Path& audioPath,
            const std::shared_ptr<system::Context>&,
            const otime::RationalTime& = time::invalidTime,
            const Options& = Options());

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

        std::optional<math::Box2f> getSpatialBounds(
            const otio::Clip* otioClip,
            Spatial spatial,
            const math::Size2i& normalizeSize,
            double scale);

        //! Timeline.
        class Timeline : public std::enable_shared_from_this<Timeline>
        {
            TLRENDER_NON_COPYABLE(Timeline);

        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const std::shared_ptr<system::Context>&, const Options&);

            Timeline();

        public:
            ~Timeline();

            //! Create a new timeline.
            static std::shared_ptr<Timeline> create(
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const std::shared_ptr<system::Context>&,
                const Options& = Options());

            //! Create a new timeline from a file name. The file name can point
            //! to an .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                const std::string&, const std::shared_ptr<system::Context>&,
                const otime::RationalTime& = time::invalidTime,
                const Options& = Options());

            //! Create a new timeline from a path. The path can point to an
            //! .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                file::Path&, const std::shared_ptr<system::Context>&,
                const otime::RationalTime& = time::invalidTime,
                const Options& = Options());

            //! Create a new timeline from a file name and audio file name.
            //! The file name can point to an .otio file, movie file, or
            //! image sequence.
            static std::shared_ptr<Timeline> create(
                const std::string& fileName, const std::string& audioFilename,
                const std::shared_ptr<system::Context>&,
                const otime::RationalTime& = time::invalidTime,
                const Options& = Options());

            //! Create a new timeline from a path and audio path. The path can
            //! point to an .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                file::Path& path, const file::Path& audioPath,
                const std::shared_ptr<system::Context>&,
                const otime::RationalTime& = time::invalidTime,
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

            //! Tick the timeline.
            void tick();

        private:
            void _tick();
            void _requests();
            void _finishRequests();
            std::future<io::VideoData> _readVideo(
                const otio::Clip*, const otime::RationalTime&,
                const io::Options&);
            std::future<io::AudioData> _readAudio(
                const otio::Clip*, const otime::TimeRange&,
                const io::Options&);
            std::shared_ptr<io::IRead> _getRead(
                const otio::Clip*,
                const io::Options&);
            std::shared_ptr<io::IRead> _getRead(
                const otio::MediaReference*,
                const io::Options&);
            bool _getVideoInfo(const otio::Composable*);
            bool _getAudioInfo(const otio::Composable*);
            void _getCanvas();
            void _getMaxVideoSize();

            TLRENDER_PRIVATE();
        };
    } // namespace timeline
} // namespace tl
