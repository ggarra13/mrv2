// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/Image.h>
#include <tlCore/Time.h>

#include <optional>

namespace tl
{
    //! Audio and video I/O.
    namespace io
    {
        //! File types.
        enum class FileType {
            Unknown = 0,
            Movie = 1,
            Sequence = 2,
            Audio = 4,

            Count,
            First = Unknown
        };

        //! I/O information.
        struct Info
        {
            //! Video layer information.
            std::vector<image::Info> video;

            //! Video time range.
            std::optional<otime::TimeRange> videoTime = time::invalidTimeRange;

            //! Audio information.
            audio::Info audio;

            //! Audio time range.
            std::optional<otime::TimeRange> audioTime = time::invalidTimeRange;

            //! Metadata tags.
            image::Tags tags;

            bool operator==(const Info&) const;
            bool operator!=(const Info&) const;
        };

        //! Add the tags describing the video, which are derived from the rest
        //! of the information rather than read from the file.
        void addVideoTags(io::Info&);

        //! Add the OTIO tags.
        void addOtioTags(
            image::Tags& tags, const std::string& clipName,
            const otime::RationalTime& time);

        //! Merge the video half of the information with the audio half. Video
        //! and audio come from separate readers; this is how the two halves are
        //! put back together.
        io::Info merge(const io::Info& video, const io::Info& audio);

        //! Video I/O data.
        struct VideoData
        {
            VideoData();
            VideoData(
                const otime::RationalTime&, uint16_t layer,
                const std::shared_ptr<image::Image>&);

            otime::RationalTime time = time::invalidTime;
            uint16_t layer = 0;
            std::shared_ptr<image::Image> image;

            //! Whether the image stands in for a frame the media does not have,
            //! rather than being the frame that was asked for. Reported
            //! whatever the policy, so that showing it is a display choice and
            //! costs no reading.
            bool                        missing = false;

            //! The frame repeated in place of it, when there was one to repeat.
            std::optional<int64_t>      heldFrom;

            bool operator==(const VideoData&) const;
            bool operator!=(const VideoData&) const;
            bool operator<(const VideoData&) const;
        };

        //! Audio I/O data.
        struct AudioData
        {
            AudioData();
            AudioData(
                const otime::RationalTime&,
                const std::shared_ptr<audio::Audio>&);

            otime::RationalTime time = time::invalidTime;
            std::shared_ptr<audio::Audio> audio;

            bool operator==(const AudioData&) const;
            bool operator!=(const AudioData&) const;
            bool operator<(const AudioData&) const;
        };

        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        Options merge(const Options&, const Options&);
    } // namespace io
} // namespace tl

#include <tlIO/IOInline.h>
