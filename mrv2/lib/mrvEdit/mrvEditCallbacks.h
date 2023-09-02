
#pragma once

#include <string>

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

    //! Menu function to remove item(s) from the timeline, replacing them
    //! with gaps.
    void edit_remove_clip_with_gap_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Function to undo last edit.
    void edit_undo(ViewerUI* ui);

    //! Function to redo last edit.
    void edit_redo(ViewerUI* ui);

    //! Menu function to undo an edit or annotation.
    void edit_undo_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to redo an edit or annotation.
    void edit_redo_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Create empty timeline.
    void create_empty_timeline_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Refresh file cache
    void refresh_file_cache_cb(Fl_Menu_* m, void* d);

    //! Add clip to otio timeline.
    void add_clip_to_timeline(const std::string&, const size_t, ViewerUI* ui);

    //! Create new timeline from a clip.
    void create_new_timeline(ViewerUI* ui);

    //! Create new timeline from selected clip.
    void create_new_timeline_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Save current OTIO timeline (EDL) to a permanent place on disk.
    void save_timeline_to_disk_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Verify current OTIO timeline (EDL).

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
