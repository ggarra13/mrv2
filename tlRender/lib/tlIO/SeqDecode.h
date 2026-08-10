// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/Decode.h>
#include <tlIO/SequenceIO.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace io
    {
        //! An image sequence, decoded one frame at a time.
        //!
        //! This is all that reading an image sequence needs: which file, or
        //! which range of bytes in a bundle, holds a frame, and a decoder to
        //! turn it into an image.
        //!
        //! It owns no thread and no request queue, and it is not written to
        //! after it is created, so the caller decides where reads happen: on
        //! a worker, on several at once, or in line.
        class SeqDecode : public std::enable_shared_from_this<SeqDecode>
        {
            TLRENDER_NON_COPYABLE(SeqDecode);

        protected:
            void _init(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const std::shared_ptr<IDecode>&,
                const io::Options&);

            SeqDecode();

        public:
            ~SeqDecode();

            //! Create a new sequence.
            //!
            //! This reads the first frame's header to find the image information,
            //! so it touches the file system once.
            static std::shared_ptr<SeqDecode> create(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const std::shared_ptr<IDecode>&,
                const io::Options& = io::Options());

            //! Get the path.
            const file::Path& getPath() const;

            //! Get the information for the sequence, including the time range
            //! that the individual files do not know about.
            const io::Info& getInfo() const;

            //! Decode one frame.
            //!
            //! Safe to call from several threads at once.
            VideoData readVideo(
                const otio::RationalTime&,
                const io::Options& = io::Options()) const;

        private:
            //! Read the image information from the first frame that is there.
            io::Info _probeInfo() const;

            //! The bytes for a frame, or null when the bundle does not hold it.
            const file::MemoryRead* _memFile(int64_t frame) const;

            //! The nearest frame at or before the given one that can be read, or
            //! the frame itself when there is none.
            int64_t _holdFrame(int64_t frame) const;

            VideoData _missingVideo(
                const otio::RationalTime&,
                MissingFrames) const;

            file::Path _path;
            std::vector<file::MemoryRead> _mem;
            std::shared_ptr<IDecode> _decode;
            io::Options _options;
            int64_t _startFrame = 0;
            int64_t _endFrame = 0;
            io::Info _info;
        };
    }
}
