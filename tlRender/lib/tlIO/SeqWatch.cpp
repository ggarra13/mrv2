// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SeqWatch.h>

#include <algorithm>
#include <iterator>

namespace tl
{
    namespace io
    {
        SeqWatch::SeqWatch(
            const file::Path& path,
            const file::PathOptions& options) :
            _path(path),
            _options(options)
        {
            if (_path.getFrames().has_value())
            {
                // Eager rather than pessimistic: if this started out
                // believing everything was missing, the first poll() would
                // report every already-present frame as "newly found",
                // which would send a caller off invalidating cache entries
                // that were never wrong.
                _missing = file::getMissing(
                    file::findSeq(_path, _options),
                    _path.getFrames().value());
            }
        }

        std::vector<int64_t> SeqWatch::poll()
        {
            std::vector<int64_t> out;
            if (!_path.getFrames().has_value())
            {
                return out;
            }

            const std::vector<file::FrameSeq> missingNow = file::getMissing(
                file::findSeq(_path, _options),
                _path.getFrames().value());

            // Frames that were missing before and are not missing now.
            // Both sides are already sorted: getMissing() sweeps in
            // increasing order and toFrames() just expands that in place.
            const std::vector<int64_t> was = file::toFrames(_missing);
            const std::vector<int64_t> now = file::toFrames(missingNow);
            std::set_difference(
                was.begin(), was.end(),
                now.begin(), now.end(),
                std::back_inserter(out));

            _missing = missingNow;
            return out;
        }

        bool SeqWatch::hasMissing() const
        {
            return !_missing.empty();
        }

        const file::Path& SeqWatch::getPath() const
        {
            return _path;
        }
    } // namespace io
} // namespace tl
