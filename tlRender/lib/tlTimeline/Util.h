// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlCore/FileIO.h>

#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Get the timeline file extensions.
        std::vector<std::string>
        getExtensions(const std::shared_ptr<system::Context>&, int type);

        //! Convert frames to ranges.
        std::vector<OTIO_NS::TimeRange>
            toRanges(std::vector<OTIO_NS::RationalTime>);

        //! Loop a time.
        OTIO_NS::RationalTime loop(
            const OTIO_NS::RationalTime&, const OTIO_NS::TimeRange&,
            bool* looped = nullptr);

        //! Cache direction.
        enum class CacheDirection {
            Forward,
            Reverse,

            Count,
            First = Forward
        };
        TLRENDER_ENUM(CacheDirection);
        TLRENDER_ENUM_SERIALIZE(CacheDirection);

        //! Loop the cache time range.
        std::vector<OTIO_NS::TimeRange> loopCache(
            const OTIO_NS::TimeRange&, const OTIO_NS::TimeRange&, CacheDirection);

        //! Get the root (highest parent).
        const OTIO_NS::Composable* getRoot(const OTIO_NS::Composable*);

        //! Get the parent of the given type.
        template <typename T> const T* getParent(const OTIO_NS::Item*);

        //! Get the duration of all tracks of the same kind.
        std::optional<OTIO_NS::RationalTime>
        getDuration(const OTIO_NS::Timeline*, const std::string& kind);

        //! Get the time range of a timeline. Unset when no track has a duration.
        std::optional<OTIO_NS::TimeRange> getTimeRange(const OTIO_NS::Timeline*);

        //! Get a list of paths to open from the given path.
        std::vector<file::Path> getPaths(
            const file::Path&, const file::PathOptions&,
            const std::shared_ptr<system::Context>&);

        //! Get an absolute path. The directory may include a protocol, so
        //! that media next to a remote timeline resolves remote. A file URL
        //! gives the path it names.
        file::Path getPath(
            const std::string& url, const std::string& directory,
            const file::PathOptions&);

        //! Get a path for a media reference.
        file::Path getPath(
            const OTIO_NS::MediaReference*, const std::string& directory,
            file::PathOptions);

        //! Convert from an OTIO missing frame policy.
        io::MissingFrames fromOTIO(
            OTIO_NS::ImageSequenceReference::MissingFramePolicy);

        //! Convert to an OTIO missing frame policy.
        OTIO_NS::ImageSequenceReference::MissingFramePolicy toOTIO(
            io::MissingFrames);

        //! Get how far an image sequence's file numbers are from its media time.
        //!
        //! OTIO puts the reference's start frame at the start of its available
        //! range, and the two need not be the same number: files 0001 to 0048
        //! with an available range from zero is a valid reference, whose time 5
        //! is file 0006. A reader counts in file numbers, so read as the media
        //! time alone it is file 0005, and every frame is one file early with
        //! nothing failing but the first. None for anything but an image sequence
        //! reference with an available range.
        std::optional<OTIO_NS::RationalTime> getSequenceOffset(
            const OTIO_NS::MediaReference*);

        //! Get a clip's trimmed range in the time its media's reader counts in:
        //! file numbers for an image sequence (the offset above), and otherwise
        //! the media's own time, less the available range when compat is set and
        //! the available range starts after the media does -- an OTIO file
        //! written against a movie's timecode with the movie counting from zero
        //! (e.g., Picchu).
        OTIO_NS::TimeRange getMediaTrimmedRange(
            const OTIO_NS::TimeRange& trimmedRange,
            const OTIO_NS::TimeRange& availableRange,
            const std::optional<OTIO_NS::RationalTime>& sequenceOffset,
            const std::optional<OTIO_NS::TimeRange>& mediaTime,
            bool compat);

        //! Get a memory read for a media reference.
        std::vector<file::MemoryRead>
        getMemoryRead(const OTIO_NS::MediaReference*);

        //! Convert to memory references.
        enum class ToMemoryReference {
            Shared,
            Raw,

            Count,
            First = Shared
        };
        TLRENDER_ENUM(ToMemoryReference);
        TLRENDER_ENUM_SERIALIZE(ToMemoryReference);

        //! Convert media references to memory references for testing.
        void toMemoryReferences(
            OTIO_NS::Timeline*, const std::string& directory, ToMemoryReference,
            const file::PathOptions& = file::PathOptions());

        //! Transform track time to video media time: the frame that is showing
        //! then, which at a different rate is the one whose time it falls in
        //! rather than the nearest. Within a millionth of a frame counts as on
        //! it, for the arithmetic's sake.
        OTIO_NS::RationalTime toVideoMediaTime(
            const OTIO_NS::RationalTime&,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange, double rate);

        //! Transform track time to audio media time.
        OTIO_NS::TimeRange toAudioMediaTime(
            const OTIO_NS::TimeRange&,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange, double sampleRate);

        //! Write a timeline to an .otioz file.
        bool writeOTIOZ(
            const std::string& fileName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const std::string& directory = std::string());
    } // namespace timeline
} // namespace tl

#include <tlTimeline/UtilInline.h>
