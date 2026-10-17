// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlCore/Path.h>

#include <vector>

namespace tl
{
    namespace io
    {
        //! Watches one image sequence for frames that were missing and have
        //! since appeared on disk.
        //!
        //! This is read-only and runs on nobody's thread of its own:
        //! something else decides when poll() is called and on what
        //! schedule. tl::timeline::Timeline is the one caller today,
        //! polling one SeqWatch at a time from its own background thread
        //! (see Timeline::Private::pollSeqWatches()), but nothing here
        //! assumes that; it is just as usable from a standalone poll loop,
        //! or a test.
        //!
        //! It only makes sense for a sequence read with
        //! MissingFrames::Hold or MissingFrames::Black: those are the
        //! policies that quietly stand something else in for a frame that
        //! is not there rather than failing the read (Error), or leaving
        //! the structure of the sequence to say what is missing (Skip,
        //! Gaps). Nothing here enforces that; it is the caller's job to
        //! decide a watch is worth having, the same way Timeline does.
        class SeqWatch
        {
        public:
            SeqWatch() = default;

            //! \p path should already carry the declared range to watch,
            //! set with Path::setFrames() -- the whole asset for a plain
            //! open sequence, or just the window a clip trims out of a
            //! larger one. Missing frames outside that range are not this
            //! watch's business, the same way getRuns() in Timeline.cpp
            //! only looks within the range it is given.
            explicit SeqWatch(
                const file::Path& path,
                const file::PathOptions& = file::PathOptions());

            //! Look on disk for frames that were missing and have appeared
            //! since the last poll (or since construction, the first
            //! time). This is the one place that touches the file system;
            //! callers poll on their own schedule rather than every tick.
            //!
            //! Returns the newly found frames, sorted and without
            //! duplicates. Empty on most polls: nothing changed since the
            //! last one, or the sequence was already complete.
            std::vector<int64_t> poll();

            //! Whether there is still something missing within the
            //! declared range. Once this is false, polling again cannot
            //! find more without a larger declared range (setFrames()
            //! again, which this class has no way to do to itself), so a
            //! caller can retire the watch.
            bool hasMissing() const;

            //! Get the path this is watching.
            const file::Path& getPath() const;

        private:
            file::Path _path;
            file::PathOptions _options;
            // What is still believed missing, as of the last poll (or the
            // construction-time poll). Sorted and merged, same as
            // file::getMissing() returns it.
            std::vector<file::FrameSeq> _missing;
        };
    } // namespace io
} // namespace tl
