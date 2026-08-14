// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <tlIO/Plugin.h>


namespace tl
{
    //! Timelines.
    namespace timeline
    {
        //! File sequence.
        enum class ImageSeqAudio {
            kNone,     //!< No audio
            Ext,
            FileName,  //!< Use the given audio file name

            Count,
            First = kNone
        };
        TLRENDER_ENUM(ImageSeqAudio);
        TLRENDER_ENUM_SERIALIZE(ImageSeqAudio);

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
        TLRENDER_ENUM_SERIALIZE(Spatial);

        //! Get the default number of sequence decoding threads.
        size_t getDefaultReadThreadCount();

        //! Timeline options.
        struct Options
        {
            ImageSeqAudio imageSeqAudio = ImageSeqAudio::Ext;

            //! Spatial coordinates.
            Spatial spatial = Spatial::Coordinates;

            //! Image sequence audio extensions.
            std::vector<std::string> imageSeqAudioExts = {
                ".mp3", ".wav",
                ".ogg", ".vorbis"
            };

            //! Image sequence audio file name.
            std::string imageSeqAudioFileName;

            //! Find the frames of an image sequence on disk when the timeline
            //! is opened.
            //!
            //! Turn this off when the path already states the range to use. A
            //! path cannot say so itself: a range of one frame looks exactly
            //! like the frame parsed out of a file name, so a sequence stated
            //! as a single frame would be expanded back to whatever is on disk.
            bool seqExpand = true;

            //! Find the frames of an image sequence on disk when the timeline
            //! is opened.
            //!
            //! Turn this off when the path already states the range to use. A
            //! path cannot say so itself: a range of one frame looks exactly
            //! like the frame parsed out of a file name, so a sequence stated
            //! as a single frame would be expanded back to whatever is on disk.
            bool compat = true;

            //! Run the timeline on its own thread.
            //!
            //! When this is false the timeline has no thread and no read pool:
            //! a request is filled by the call that makes it, and the future it
            //! returns is already resolved. That is what a caller that only
            //! wants a frame or two wants, and it is what lets a thumbnail be
            //! taken without a thread per file.
            bool threaded = true;

            //! How many sequence frames the timeline decodes at once.
            //!
            //! This is the one place the decoding concurrency is set. How many
            //! video requests are in flight follows from it, so that raising it
            //! is not silently undone by a separate limit.
            size_t readThreadCount = getDefaultReadThreadCount();

            //! Maximum number of video requests.
            size_t videoRequestMax = 16;

            //! Maximum number of audio requests in flight.
            //!
            //! Unlike video, this does not follow readThreadCount: audio is not
            //! decoded by the timeline's pool. It comes from a reader that does
            //! its own threading, so there is no thread count here for it to
            //! follow and this is the only limit on how much audio is being
            //! assembled at once.
            size_t audioRequestMax = 16;

            //! Maximum number of stateful readers held per half.
            //!
            //! Video and audio are cached separately, so this many of each: a
            //! movie with sound occupies one entry in both. That is the same
            //! budget as before the halves were split apart, when a single
            //! reader for such a movie ran a thread for each of them anyway.
            //!
            //! What it bounds is decoder state and threads, not parallelism --
            //! readThreadCount is the knob for that. Its right value follows
            //! from how many distinct references a timeline plays across, which
            //! is a property of the timeline rather than of the machine.
            size_t readCacheMax = 10;

            //! Maximum number of image sequences held.
            //!
            //! Far larger than readCacheMax because a sequence holds no thread
            //! and no queue: only where each frame lives, so evicting one costs
            //! a header read. A timeline with a clip per shot has hundreds.
            //!
            //! Note that inside a bundle an entry carries a byte range per
            //! frame, so a long sequence is not free; a count is the wrong
            //! bound if that ever starts to matter.
            size_t seqCacheMax = 1000;

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

    } // namespace timeline
} // namespace tl
