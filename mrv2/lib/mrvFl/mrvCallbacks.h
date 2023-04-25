// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include <string>
#include <memory>

#include <FL/Fl_Widget.H>

#include "mrvApp/mrvPlaylistsModel.h"

class Fl_Group;
class Fl_Button;
class Fl_Menu_Item;
class Fl_Menu_;
class HUDUI;
class ViewerUI;

namespace mrv
{
    struct WindowCallback
    {
        const char* name;
        Fl_Callback* callback;
    };

    extern WindowCallback kWindowCallbacks[];
    extern HUDUI* hud;

    //! File menu callbacks
    void open_files_cb(const std::vector< std::string >& files, ViewerUI* ui);
    void open_cb(Fl_Widget* w, ViewerUI* ui);
    void open_directory_cb(Fl_Widget* w, ViewerUI* ui);
    void open_separate_audio_cb(Fl_Widget* w, ViewerUI* ui);
    void open_recent_cb(Fl_Menu_* w, ViewerUI* ui);

    void save_movie_cb(Fl_Menu_* w, ViewerUI* ui);

    void close_current_cb(Fl_Widget* w, ViewerUI* ui);
    void close_all_cb(Fl_Widget* w, ViewerUI* ui);

    void previous_file_cb(Fl_Widget* w, ViewerUI* ui);
    void next_file_cb(Fl_Widget* w, ViewerUI* ui);

    void exit_cb(Fl_Widget* w, ViewerUI* ui);

    //! About menu callback
    void about_cb(Fl_Widget* w, ViewerUI* ui);

    //! Display callbacks
    void minify_nearest_cb(Fl_Menu_* m, ViewerUI* ui);
    void minify_linear_cb(Fl_Menu_* m, ViewerUI* ui);
    void magnify_nearest_cb(Fl_Menu_* m, ViewerUI* ui);
    void magnify_linear_cb(Fl_Menu_* m, ViewerUI* ui);

    void mirror_x_cb(Fl_Menu_* m, ViewerUI* ui);
    void mirror_y_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Channel callbacks
    void toggle_red_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_green_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_blue_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_alpha_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_color_channel_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Window callbacks
    void toggle_presentation_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_fullscreen_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_float_on_top_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_secondary_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_secondary_float_on_top_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Panel callbacks
    void toggle_one_panel_only_cb(Fl_Menu_* w, ViewerUI* ui);

    void window_cb(Fl_Menu_* w, ViewerUI* ui);
    void show_window_cb(const std::string& label, ViewerUI* ui);

    //! Safe Areas callback
    void safe_areas_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Masking callback
    void masking_cb(Fl_Menu_* w, ViewerUI* ui);

    //! HUD window callback
    void hud_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Auxiliary functions to remember what bars and what windows were
    // 1 open in case of a fullscreen or presentation switch.
    void save_ui_state(ViewerUI* ui, Fl_Group* bar);
    void save_ui_state(ViewerUI* ui);
    void toggle_action_tool_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_menu_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_top_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_pixel_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_bottom_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_status_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_ui_bar(
        ViewerUI* ui, Fl_Group* const bar, const int sizeX, const int sizeY);
    void toggle_ui_bar(ViewerUI* ui, Fl_Group* const bar, const int size = 0);
    void hide_ui_state(ViewerUI* ui);
    void restore_ui_state(ViewerUI* ui);

    // Playback callbacks
    void play_forwards_cb(Fl_Menu_*, ViewerUI* ui);
    void play_backwards_cb(Fl_Menu_*, ViewerUI* ui);
    void stop_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_playback_cb(Fl_Menu_*, ViewerUI* ui);

    // In/Out point callbacks
    void playback_set_in_point_cb(Fl_Menu_*, ViewerUI* ui);
    void playback_set_out_point_cb(Fl_Menu_*, ViewerUI* ui);

    // Loop callbacks
    void playback_loop_cb(Fl_Menu_*, ViewerUI* ui);
    void playback_once_cb(Fl_Menu_*, ViewerUI* ui);
    void playback_ping_pong_cb(Fl_Menu_*, ViewerUI* ui);

    void start_frame_cb(Fl_Menu_*, ViewerUI* ui);
    void end_frame_cb(Fl_Menu_*, ViewerUI* ui);

    void next_frame_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_frame_cb(Fl_Menu_*, ViewerUI* ui);

    void next_annotation_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_annotation_cb(Fl_Menu_*, ViewerUI* ui);

    void annotation_clear_cb(Fl_Menu_*, ViewerUI* ui);
    void annotation_clear_all_cb(Fl_Menu_*, ViewerUI* ui);

    // OCIO callbacks
    void attach_ocio_ics_cb(Fl_Menu_*, ViewerUI* ui);
    void attach_ocio_display_cb(Fl_Menu_*, ViewerUI* ui);
    void attach_ocio_view_cb(Fl_Menu_*, ViewerUI* ui);

    // Video levels callbacks
    void video_levels_from_file_cb(Fl_Menu_*, ViewerUI* ui);
    void video_levels_legal_range_cb(Fl_Menu_*, ViewerUI* ui);
    void video_levels_full_range_cb(Fl_Menu_*, ViewerUI* ui);

    // Alpha blend callbacks
    void alpha_blend_none_cb(Fl_Menu_*, ViewerUI* ui);
    void alpha_blend_straight_cb(Fl_Menu_*, ViewerUI* ui);
    void alpha_blend_premultiplied_cb(Fl_Menu_*, ViewerUI* ui);

    // Annotations
    void set_pen_color_cb(Fl_Button*, ViewerUI* ui);

    // Versioning
    void first_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void next_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void last_image_version_cb(Fl_Menu_*, ViewerUI* ui);

    //! .otio EDL creation from loaded files
    void create_playlist(
        ViewerUI* ui, const std::shared_ptr<Playlist>& playlst,
        const bool temp = true);

    void create_playlist(
        ViewerUI* ui, const std::shared_ptr<Playlist>& playlst,
        const std::string& fileName, const bool relative);

    void help_documentation_cb(Fl_Menu_*, ViewerUI* ui);

    void toggle_black_background_cb(Fl_Menu_* m, ViewerUI* ui);

    void toggle_sync_send_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_sync_receive_cb(Fl_Menu_* m, ViewerUI* ui);

} // namespace mrv
