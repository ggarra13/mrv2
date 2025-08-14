// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

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

    //! Handle move of clip (used in shifting clips around in tlRender).
    void edit_move_clip_annotations(
        const std::vector<tl::timeline::MoveData>& moves, ViewerUI* ui);

    //! Set the temporary EDL for a drag item callback.
    void toOtioFile(TimelinePlayer*, ViewerUI* ui);

    //! Make path relative to another fileName if possible.
    file::Path
    getRelativePath(const file::Path& path, const fs::path& fileName);

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

    //! Menu function to insert an audio clip at current time.
    void edit_insert_audio_clip_cb(ViewerUI* ui, const std::string& audioFile);

    //! Edit callbacks that open file requesters
    void insert_audio_clip_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Menu function to remove audio clip(s) at current time.
    void edit_remove_audio_clip_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to insert an audio gap at current time.
    void edit_insert_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Menu function to remove audio gap at current time.
    void edit_remove_audio_gap_cb(Fl_Menu_* m, ViewerUI* ui);

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

    //! Refresh file cache
    void refresh_file_cache_cb(Fl_Menu_* m, void* d);

    //! Create new timeline from selected clip.
    void create_new_timeline_cb(ViewerUI* ui);

    //! Add clip to otio timeline.
    void add_clip_to_new_timeline_cb(const int, ViewerUI* ui);

    //! Add clip to otio timeline.
    void add_clip_to_timeline_cb(const int, ViewerUI* ui);

    //! Save current OTIO timeline (EDL) to a permanent place on disk.
    void save_timeline_to_disk_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Save current OTIO timeline (EDL) to a filename.
    void save_timeline_to_disk(const std::string& otioFile);

    //! Replace current clip in otio timeline with newClipPath.
    bool replaceClipPath(tl::file::Path newClipPath, ViewerUI* ui);

    //! Get Active Tracks.
    bool getActiveTracks(
        std::vector<std::string>& tracks, std::vector<bool>& tracksActive,
        ViewerUI* ui);

    //! Toggle Track Enabled (active) state.
    bool toggleTrack(unsigned trackIndex, ViewerUI* ui);

    //
    // Set the edit mode height.
    //
    extern EditMode editMode;  // current edit mode
    extern EditMode previousEditMode;
    extern int editModeH;

    void save_edit_mode_state(ViewerUI* ui);
    void set_edit_button(EditMode mode, ViewerUI* ui);
    void set_edit_mode_cb(EditMode mode, ViewerUI* ui);
    int calculate_edit_viewport_size(ViewerUI* ui);
} // namespace mrv
