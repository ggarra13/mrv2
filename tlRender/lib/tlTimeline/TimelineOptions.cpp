// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/TimelineOptions.h>

#include <tlCore/Error.h>

#include <algorithm>
#include <sstream>
#include <thread>

namespace tl
{
    namespace timeline
    {

        TLRENDER_ENUM_IMPL(
            ImageSeqAudio, "None", "Extension", "FileName");
        TLRENDER_ENUM_SERIALIZE_IMPL(ImageSeqAudio);

        TLRENDER_ENUM_IMPL(
            Spatial,
            "None",
            "Coordinates",
            "Normalize");
        TLRENDER_ENUM_SERIALIZE_IMPL(Spatial);


        bool Options::operator==(const Options& other) const
        {
            return
                imageSeqAudio == other.imageSeqAudio &&
                spatial == other.spatial &&
                imageSeqAudioExts == other.imageSeqAudioExts &&
                imageSeqAudioFileName == other.imageSeqAudioFileName &&
                seqExpand == other.seqExpand &&
                compat == other.compat &&
                threaded == other.threaded &&
                readThreadCount == other.readThreadCount &&
                audioRequestMax == other.audioRequestMax &&
                readCacheMax == other.readCacheMax &&
                seqCacheMax == other.seqCacheMax &&
                ioOptions == other.ioOptions &&
                pathOptions == other.pathOptions;
        }

        bool Options::operator!=(const Options& other) const
        {
            return !(*this == other);
        }

        size_t getDefaultReadThreadCount()
        {
            return std::max(1u, std::thread::hardware_concurrency());
        }
    }
}
