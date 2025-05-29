// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlCore/FileIO.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Get the timeline file extensions.
        std::vector<std::string>
        getExtensions(int types, const std::shared_ptr<system::Context>&);

        //! Convert frames to ranges.
        std::vector<otime::TimeRange>
            toRanges(std::vector<otime::RationalTime>);

        //! Loop a time.
        otime::RationalTime loop(
            const otime::RationalTime&, const otime::TimeRange&,
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
        std::vector<otime::TimeRange> loopCache(
            const otime::TimeRange&, const otime::TimeRange&, CacheDirection);

        //! Get the root (highest parent).
        const otio::Composable* getRoot(const otio::Composable*);

        //! Get the parent of the given type.
        template <typename T> const T* getParent(const otio::Item*);

        //! Get the duration of all tracks of the same kind.
        std::optional<otime::RationalTime>
        getDuration(const otio::Timeline*, const std::string& kind);

        //! Get the time range of a timeline.
        otime::TimeRange getTimeRange(const otio::Timeline*);

        //! Get a list of paths to open from the given path.
        std::vector<file::Path> getPaths(
            const file::Path&, const file::PathOptions&,
            const std::shared_ptr<system::Context>&);

        //! Get an absolute path.
        file::Path getPath(
            const std::string& url, const std::string& directory,
            const file::PathOptions&);

        //! Get a path for a media reference.
        file::Path getPath(
            const otio::MediaReference*, const std::string& directory,
            file::PathOptions);

        //! Get a memory read for a media reference.
        std::vector<file::MemoryRead>
        getMemoryRead(const otio::MediaReference*);

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
            otio::Timeline*, const std::string& directory, ToMemoryReference,
            const file::PathOptions& = file::PathOptions());

        //! Transform track time to video media time.
        otime::RationalTime toVideoMediaTime(
            const otime::RationalTime&,
            const otime::TimeRange& trimmedRangeInParent,
            const otime::TimeRange& trimmedRange, double rate);

        //! Transform track time to audio media time.
        otime::TimeRange toAudioMediaTime(
            const otime::TimeRange&,
            const otime::TimeRange& trimmedRangeInParent,
            const otime::TimeRange& trimmedRange, double sampleRate);

        //! Write a timeline to an .otioz file.
        bool writeOTIOZ(
            const std::string& fileName,
            const otio::SerializableObject::Retainer<otio::Timeline>&,
            const std::string& directory = std::string());
    } // namespace timeline
} // namespace tl

#include <tlTimeline/UtilInline.h>
