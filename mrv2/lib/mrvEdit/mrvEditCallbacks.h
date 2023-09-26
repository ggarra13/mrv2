// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include <string>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Util.h>

#include "mrvEdit/mrvEditMode.h"

class Fl_Menu_;
class ViewerUI;

namespace mrv
{
    using namespace tl;

    class TimelinePlayer;
    using otio::Timeline;

    //@{
    //! Store timeline in undo queue.

    void edit_store_undo(TimelinePlayer*, ViewerUI* ui);
    void edit_clear_redo(ViewerUI* ui);
    //@}

    //! Return whether edit has undo.
    bool edit_has_undo();

    //! Return whether edit has redo.
    bool edit_has_redo();

    //! Handle insert of clip (used in shifting clips around in tlRender).
    void edit_insert_clip(
        const std::vector<tl::timeline::InsertData>& inserts, ViewerUI* ui);

    //! Set the temporary EDL for a drag item callback.
    void toOtioFile(TimelinePlayer*, ViewerUI* ui);

    //! Make paths of an otio::Timeline absolute.
    void makePathsAbsolute(TimelinePlayer* player, ViewerUI* ui);

    //! Menu function to copy one frame to the buffer.
    void edit_copy_frame_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to cut one frame from the timeline.
    void edit_cut_frame_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to paste one frame from the buffer.
    void edit_paste_frame_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to insert one frame from the buffer.
    void edit_insert_frame_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to slice the timeline.
    void edit_slice_clip_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to remove item(s) from the timeline.
    void edit_remove_clip_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Set Action Mode to trim
    void edit_trim_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Set Action Mode to slip
    void edit_slip_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Set Action Mode to slide
    void edit_slide_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Set Action Mode to ripple
    void edit_ripple_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Set Action Mode to roll
    void edit_roll_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to undo an edit or annotation.
    void edit_undo_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to redo an edit or annotation.
    void edit_redo_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Edit function to shift annotations in a timeRange to a start time
    void shiftAnnotations(
        const otime::TimeRange& range, const otime::RationalTime& startTime,
        ViewerUI* ui);

    //! Create empty timeline.
    void create_empty_timeline_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Refresh file cache
    void refresh_file_cache_cb(Fl_Menu_* m, void* d);

    //! Add clip to otio timeline.
    void add_clip_to_timeline(const std::string&, const int, ViewerUI* ui);

    //! Create new timeline from a clip.
    void create_new_timeline(ViewerUI* ui);

    //! Create new timeline from selected clip.
    void create_new_timeline_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Save current OTIO timeline (EDL) to a permanent place on disk.
    void save_timeline_to_disk_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Save current OTIO timeline (EDL) to a filename.  Timeline and EDL
    //! must be verified first.
    void save_timeline_to_disk(otio::Timeline*, const std::string& fileName);

    //
    // Set the edit mode height.
    //
    extern EditMode editMode;
    extern int editModeH;

    void save_edit_mode_state(ViewerUI* ui);
    void set_edit_mode_cb(EditMode mode, ViewerUI* ui);
    int calculate_edit_viewport_size(ViewerUI* ui);
} // namespace mrv
