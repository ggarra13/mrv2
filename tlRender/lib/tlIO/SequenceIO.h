// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/Write.h>

namespace tl
{
    namespace io
    {
        //! What to do about a frame a sequence does not have.
        //!
        //! The first three match OTIO's image sequence reference policies,
        //! which is where the value comes from when the sequence is described
        //! by a timeline.
        //! They are decided per read: the sequence keeps its length and each of
        //! them says what fills a gap.
        //!
        //! The last two are different in kind. They do not fill anything; they
        //! say what the timeline is built out of, so a clip covers each run of
        //! frames that are there and no read ever asks for a frame that is
        //! not. That makes them structural: they are settled when the sequence
        //! is opened, and changing one means opening it again. Because the
        //! frames are found in order to build the clips, they only apply to
        //! an image sequence opened directly -- a timeline that was authored
        //! already says what its clips are, and those are not ours to rewrite.
        enum class MissingFrames
        {
            Error,  //!< The frame does not read.
            Hold,   //!< Repeat the nearest frame before it.
            Black,  //!< A blank frame.
            Skip,   //!< Leave it out; only the frames that are there play.
            Gaps,   //!< Leave a hole, so the frames keep the times they had.

            Count,
            First = Error
        };
        TLRENDER_ENUM(MissingFrames);

        //! Get whether a policy is settled when the sequence is opened, by
        //! deciding the clips, rather than per read.
        bool isStructural(MissingFrames);

        //! Timeout for requests.
        const std::chrono::milliseconds sequenceRequestTimeout(5);

        //! Get whether a policy is settled when the sequence is opened, by
        //! deciding the clips, rather than per read.
        bool isStructural(MissingFrames);

        //! Sequence I/O options.
        struct SeqOptions
        {
            SeqOptions();

            double        defaultSpeed  = 24.0;
            MissingFrames missingFrames = MissingFrames::Error;

            bool operator == (const SeqOptions&) const;
            bool operator != (const SeqOptions&) const;
        };

        //! Get sequence I/O options.
        io::Options getOptions(const SeqOptions&);

        //! Get the missing frame policy from the options.
        MissingFrames getMissingFrames(const io::Options&);

        //! Base class for image sequence writers.
        class ISequenceWrite : public IWrite
        {
        protected:
            void _init(
                const file::Path&, const Info&, const Options&,
                const std::shared_ptr<log::System>&);

            ISequenceWrite();

        public:
            virtual ~ISequenceWrite();

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const Options& = Options()) override;

        protected:
            virtual void _writeVideo(
                const std::string& fileName, const OTIO_NS::RationalTime&,
                const std::shared_ptr<image::Image>&, const Options&) = 0;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace io
} // namespace tl
