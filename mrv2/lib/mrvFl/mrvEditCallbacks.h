
#pragma once

#include <opentimelineio/composition.h>
#include <opentimelineio/item.h>

#include "mrvFl/mrvEditMode.h"

class Fl_Menu_;
class ViewerUI;

namespace mrv
{

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

    //! Menu function to undo last edit.
    void edit_undo_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to redo last edit.
    void edit_redo_cb(Fl_Menu_* m, ViewerUI* ui);

    //
    // Set the edit mode height.
    //

    extern EditMode editMode;

    void save_edit_mode_state(ViewerUI* ui);
    void set_edit_mode_cb(EditMode mode, ViewerUI* ui);
    int calculate_edit_viewport_size(ViewerUI* ui);
} // namespace mrv
