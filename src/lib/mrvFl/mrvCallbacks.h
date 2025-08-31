// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvApp/mrvPlaylistsModel.h"

#include <FL/Fl_Widget.H> // For Fl_Callback

class Fl_Group;
class Fl_Button;
class Fl_Menu_Item;
class Fl_Menu_;
class HUDUI;
class OCIOPresetsUI;
class ViewerUI;

namespace mrv
{
    class Button;

    struct WindowCallback
    {
        const char* name;
        Fl_Callback* callback;
    };

    extern WindowCallback kWindowCallbacks[];
    extern HUDUI* hudClass;
    extern OCIOPresetsUI* OCIOPresetsClass;

    void open_files_cb(const std::vector< std::string >& files, ViewerUI* ui);
    void open_single_files_cb(const std::string& file, ViewerUI* ui);

    //! File menu callbacks
    void open_file_cb(const std::string& file, ViewerUI* ui);
    void open_cb(Fl_Widget* w, ViewerUI* ui);
    void open_single_image_cb(Fl_Widget* w, ViewerUI* ui);
    void open_directory_cb(Fl_Widget* w, ViewerUI* ui);
    void open_separate_audio_cb(Fl_Widget* w, ViewerUI* ui);
    void open_recent_cb(Fl_Menu_* w, ViewerUI* ui);

    void save_single_frame_to_folder_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_single_frame_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_annotations_only_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_annotations_as_json_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_movie_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_audio_cb(Fl_Menu_* w, ViewerUI* ui);
    void save_pdf_cb(Fl_Menu_* w, ViewerUI* ui);

    void close_current_cb(Fl_Widget* w, ViewerUI* ui);
    void close_all_cb(Fl_Widget* w, ViewerUI* ui);

    //! Previous/Next files
    void previous_file_cb(Fl_Widget* w, ViewerUI* ui);
    void next_file_cb(Fl_Widget* w, ViewerUI* ui);
    void previous_file_limited_cb(Fl_Widget* w, ViewerUI* ui);
    void next_file_limited_cb(Fl_Widget* w, ViewerUI* ui);

    //! Select A/B file callbacks
    void goto_file_cb(Fl_Widget* w, void* idx);
    void select_Bfile_cb(Fl_Widget* w, void* idx);

    //! Compare callbacks
    void compare_a_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_b_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_wipe_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_overlay_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_difference_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_horizontal_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_vertical_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_tile_cb(Fl_Widget* w, ViewerUI* ui);

    void exit_cb(Fl_Widget* w, ViewerUI* ui);

    //! Layer callbacks
    void previous_channel_cb(Fl_Widget* w, ViewerUI* ui);
    void next_channel_cb(Fl_Widget* w, ViewerUI* ui);

    //! About menu callback
    void about_cb(Fl_Widget* w, ViewerUI* ui);

    //! Display callbacks
    void minify_nearest_cb(Fl_Menu_* m, ViewerUI* ui);
    void minify_linear_cb(Fl_Menu_* m, ViewerUI* ui);
    void magnify_nearest_cb(Fl_Menu_* m, ViewerUI* ui);
    void magnify_linear_cb(Fl_Menu_* m, ViewerUI* ui);

    void mirror_x_cb(Fl_Menu_* m, ViewerUI* ui);
    void mirror_y_cb(Fl_Menu_* m, ViewerUI* ui);

    void rotate_plus_90_cb(Fl_Menu_* m, ViewerUI* ui);
    void rotate_minus_90_cb(Fl_Menu_* m, ViewerUI* ui);

        
    //! HDR callbacks
    void toggle_normalize_image_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_invalid_values_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_hdr_tonemap_cb(Fl_Menu_* w, ViewerUI* ui);
    void select_hdr_tonemap_cb(Fl_Menu_* w, ViewerUI* ui);
    void select_hdr_gamut_mapping_cb(Fl_Menu_* m, ViewerUI* ui);
    
    void hdr_data_from_file_cb(Fl_Menu_* m, ViewerUI* ui);
    void hdr_data_inactive_cb(Fl_Menu_* m, ViewerUI* ui);
    void hdr_data_active_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Channel callbacks
    void toggle_red_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_green_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_blue_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_alpha_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_lumma_channel_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_color_channel_cb(Fl_Menu_* m, ViewerUI* ui);

    //! Window callbacks
    void toggle_presentation_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_fullscreen_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_float_on_top_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_secondary_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_secondary_float_on_top_cb(Fl_Menu_* w, ViewerUI* ui);
    void toggle_click_through(Fl_Menu_* w, ViewerUI* ui);
    void more_ui_transparency(Fl_Menu_* w, ViewerUI* ui);
    void less_ui_transparency(Fl_Menu_* w, ViewerUI* ui);

    //! Panel callbacks
    void toggle_one_panel_only_cb(Fl_Menu_* w, ViewerUI* ui);

    void window_cb(Fl_Menu_* w, ViewerUI* ui);
    void show_window_cb(const std::string& label, ViewerUI* ui);

    //! Auto Frame view callback
    void frame_view_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Safe Areas callback
    void toggle_safe_areas_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Data Window callback
    void toggle_data_window_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Display Window callback
    void toggle_display_window_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Ignore Display Window callback
    void toggle_ignore_display_window_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Ignore Display Window callback
    void toggle_ignore_chromaticities_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Masking callback
    void masking_cb(Fl_Menu_* w, ViewerUI* ui);

    //! HUD window callback
    void hud_cb(Fl_Menu_* w, ViewerUI* ui);

    //! Auxiliary functions to remember what bars and what windows were
    //! 1 open in case of a fullscreen or presentation switch.
    void save_ui_state(ViewerUI* ui, Fl_Group* bar);
    void save_ui_state(ViewerUI* ui);
    void toggle_action_tool_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_menu_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_top_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_pixel_bar(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_bar(Fl_Menu_*, ViewerUI* ui);
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
    void toggle_otio_clip_in_out_cb(Fl_Menu_*, ViewerUI* ui);

    // Loop callbacks
    void playback_loop_cb(Fl_Menu_*, ViewerUI* ui);
    void playback_once_cb(Fl_Menu_*, ViewerUI* ui);
    void playback_ping_pong_cb(Fl_Menu_*, ViewerUI* ui);

    // Navigation
    void start_frame_cb(Fl_Menu_*, ViewerUI* ui);
    void end_frame_cb(Fl_Menu_*, ViewerUI* ui);

    void next_frame_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_frame_cb(Fl_Menu_*, ViewerUI* ui);

    void next_annotation_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_annotation_cb(Fl_Menu_*, ViewerUI* ui);

    void next_clip_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_clip_cb(Fl_Menu_*, ViewerUI* ui);

    // Timeline view
    void toggle_timeline_markers_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_transitions_cb(Fl_Menu_*, ViewerUI* ui);
    void timeline_thumbnails_none_cb(Fl_Menu_*, ViewerUI* ui);
    void timeline_thumbnails_small_cb(Fl_Menu_*, ViewerUI* ui);
    void timeline_thumbnails_medium_cb(Fl_Menu_*, ViewerUI* ui);
    void timeline_thumbnails_large_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_editable_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_edit_associated_clips_cb(Fl_Menu_*, ViewerUI* ui);
    void timeline_frame_view_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_scroll_to_current_frame_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_track_info_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_clip_info_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_timeline_active_track_cb(Fl_Menu_*, ViewerUI* ui);

    // OCIO callbacks

    // (From Media Info Panel)
    void attach_ocio_ics_cb(Fl_Menu_*, ViewerUI* ui);

    // From OCIO Menus
    void ocio_presets_cb(Fl_Menu_* w, ViewerUI* ui);

    void toggle_ocio_cb(Fl_Menu_*, ViewerUI* ui);
    void toggle_ocio_topbar_cb(Fl_Menu_*, ViewerUI* ui);

    void current_ocio_ics_cb(Fl_Menu_*, ViewerUI* ui);
    void current_ocio_look_cb(Fl_Menu_*, ViewerUI* ui);

    void monitor_ocio_view_cb(Fl_Menu_*, ViewerUI* ui);
    void all_monitors_ocio_view_cb(Fl_Menu_*, ViewerUI* ui);

    // Video levels callbacks
    void video_levels_from_file_cb(Fl_Menu_*, ViewerUI* ui);
    void video_levels_legal_range_cb(Fl_Menu_*, ViewerUI* ui);
    void video_levels_full_range_cb(Fl_Menu_*, ViewerUI* ui);

    // Alpha blend callbacks
    void alpha_blend_none_cb(Fl_Menu_*, ViewerUI* ui);
    void alpha_blend_straight_cb(Fl_Menu_*, ViewerUI* ui);
    void alpha_blend_premultiplied_cb(Fl_Menu_*, ViewerUI* ui);

    // Auxiliary color function
    image::Color4f from_fltk_color(const Fl_Color& color);
    Fl_Color to_fltk_color(const image::Color4f& color);
    image::Color4f get_color_cb(Fl_Color c, ViewerUI* ui);

    // Annotations
    void set_pen_color_cb(Fl_Button*, ViewerUI* ui);
    void flip_pen_color_cb(Fl_Button*, ViewerUI* ui);
    void toggle_annotation_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_visible_annotation_cb(Fl_Menu_* m, ViewerUI* ui);
    void annotation_clear_cb(Fl_Menu_*, ViewerUI* ui);
    void annotation_clear_all_cb(Fl_Menu_*, ViewerUI* ui);

    // Versioning
    void first_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void previous_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void next_image_version_cb(Fl_Menu_*, ViewerUI* ui);
    void last_image_version_cb(Fl_Menu_*, ViewerUI* ui);

    //! Call the browser with documentation.
    void help_documentation_cb(Fl_Menu_*, ViewerUI* ui);

    // Network toggles
    void toggle_sync_send_cb(Fl_Menu_* m, ViewerUI* ui);
    void toggle_sync_receive_cb(Fl_Menu_* m, ViewerUI* ui);

    // Session callbacks
    void save_session_as_cb(Fl_Menu_* m, ViewerUI* ui);
    void save_session_cb(Fl_Menu_* m, ViewerUI* ui);
    void load_session_cb(Fl_Menu_* m, ViewerUI* ui);

    // Note annotations
    void clear_note_annotation_cb(ViewerUI* ui);
    void add_note_annotation_cb(ViewerUI* ui, const std::string& text);

    // Text shapes
    void edit_text_shape_cb(ViewerUI* ui);

    // Panel callbacks
    void refresh_file_cache_cb(Fl_Menu_* m, void* d);
    void clone_file_cb(Fl_Menu_* m, void* d);
    void update_video_frame_cb(Fl_Menu_* m, void* d);

    //! Reload media, replacing current one.
    void refresh_media_cb(Fl_Menu_* m, void* d);

    //! If a movie, refresh it with current ffmpeg settings.
    void refresh_movie_cb(Fl_Menu_* m, void* d);

    void set_stereo_cb(Fl_Menu_* m, void* d);
    void copy_filename_cb(Fl_Menu_* m, void* d);
    void file_manager_cb(Fl_Menu_* m, void* d);

    // Python
    void run_python_method_cb(Fl_Menu_* m, void* d);

} // namespace mrv
