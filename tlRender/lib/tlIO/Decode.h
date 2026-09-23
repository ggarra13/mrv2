// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/LogSystem.h>
#include <tlCore/FileIO.h>

namespace tl
{
    namespace io
    {
        //! Base class for image decoders.
        //!
        //! A decoder turns one file, or one range of bytes within one, into one
        //! image. It has no thread, no request queue, and no position in a
        //! file, so one decoder serves any number of callers at once and can
        //! be driven synchronously.
        //!
        //! Which frame a file holds, and how files make up a sequence, is the
        //! caller's business. A format whose reader has to carry state, such
        //! as a movie with a demuxer position, is not decoded this way.
        class IDecode : public std::enable_shared_from_this<IDecode>
        {
            TLRENDER_NON_COPYABLE(IDecode);

        protected:
            void _init(const std::shared_ptr<log::System>&);

            IDecode();

        public:
            virtual ~IDecode() = 0;

            //! Get information about one file.
            //!
            //! The time range is left empty: one file does not know the
            //! sequence it belongs to.
            virtual io::Info getInfo(
                const std::string& fileName,
                const file::MemoryRead* = nullptr) = 0;

            //! Decode one file to an image.
            virtual io::VideoData readVideo(
                const std::string& fileName,
                const file::MemoryRead*,
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) = 0;

            //! Get the speed the file declares, or the given default when the
            //! format has no way to declare one.
            virtual double getSpeed(const io::Info&, double defaultSpeed) const;

        protected:
            std::weak_ptr<log::System> _logSystem;
        };
    }
}
