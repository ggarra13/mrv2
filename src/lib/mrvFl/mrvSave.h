// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlIO/IO.h>

#include "mrvSaveOptions.h"

class ViewerUI;

namespace mrv
{
    class TimelinePlayer;

    //! Waits for a frame to be in the cache
    void waitForFrame(
        const mrv::TimelinePlayer* player, const otime::RationalTime& time);

    //! Save single frame.  Returns 0 if successful, -1 if not.
    int save_single_frame(
        const std::string& file, const ViewerUI* ui,
        SaveOptions options = SaveOptions());

    //! Save multiple frames specified as a list of times.  Returns 0 if
    //! successful, -1 if not.
    int save_multiple_frames(
        const std::string& file, const std::vector<otime::RationalTime>& times,
        const ViewerUI* ui, SaveOptions options = SaveOptions());

    //! Save multiple frames specified as a list of times.  Returns 0 if
    //! successful, -1 if not.
    int save_multiple_annotation_frames(
        const std::string& file, const std::vector<otime::RationalTime>& times,
        const ViewerUI* ui, SaveOptions options = SaveOptions());

    //! Saves a movie or sequence.
    void save_movie(
        const std::string& file, const ViewerUI* ui,
        SaveOptions options = SaveOptions());

} // namespace mrv
