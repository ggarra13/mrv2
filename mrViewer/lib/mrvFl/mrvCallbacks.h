#pragma once

#include <vector>
#include <string>

class Fl_Widget;
class Fl_Group;
class Fl_Menu_Item;
class Fl_Menu_;
class ViewerUI;

namespace mrv
{
    class TimelineViewport;
    class MainWindow;

    //! File menu callbacks
    void open_files_cb( const std::vector< std::string >& files,
                        ViewerUI* ui  );
    void open_cb( Fl_Widget* w, ViewerUI* ui );
    void open_directory_cb( Fl_Widget* w, ViewerUI* ui );
    void open_separate_audio_cb( Fl_Widget* w, ViewerUI* ui );
    void open_recent_cb( Fl_Menu_* w, ViewerUI* ui );

    void close_current_cb( Fl_Widget* w, ViewerUI* ui );
    void close_all_cb( Fl_Widget* w, ViewerUI* ui );
    
    void previous_file_cb( Fl_Widget* w, ViewerUI* ui );
    void next_file_cb( Fl_Widget* w, ViewerUI* ui );

    void exit_cb( Fl_Widget* w, ViewerUI* ui );

    //! Display callbacks
    void minify_nearest_cb( Fl_Menu_* m, ViewerUI* ui );
    void minify_linear_cb( Fl_Menu_* m, ViewerUI* ui );
    void magnify_nearest_cb( Fl_Menu_* m, ViewerUI* ui );
    void magnify_linear_cb( Fl_Menu_* m, ViewerUI* ui );

    void mirror_x_cb( Fl_Menu_* m, ViewerUI* ui );
    void mirror_y_cb( Fl_Menu_* m, ViewerUI* ui );

    //! Channel callbacks
    void toggle_red_channel_cb( Fl_Menu_* m, ViewerUI* ui );
    void toggle_green_channel_cb( Fl_Menu_* m, ViewerUI* ui );
    void toggle_blue_channel_cb( Fl_Menu_* m, ViewerUI* ui );
    void toggle_alpha_channel_cb( Fl_Menu_* m, ViewerUI* ui );

    //! Compare callbacks
    void change_media_cb( Fl_Menu_* m, ViewerUI* ui );

    void A_media_cb( Fl_Menu_* m, ViewerUI* ui );
    void B_media_cb( Fl_Menu_* m, ViewerUI* ui );

    void compare_wipe_cb( Fl_Menu_* m, ViewerUI* ui );
    void compare_overlay_cb( Fl_Menu_* m, ViewerUI* ui );
    void compare_difference_cb( Fl_Menu_* m, ViewerUI* ui );
    void compare_horizontal_cb( Fl_Menu_* m, ViewerUI* ui );
    void compare_vertical_cb( Fl_Menu_* m, ViewerUI* ui );
    void compare_tile_cb( Fl_Menu_* m, ViewerUI* ui );

    //! Window callbacks
    void toggle_fullscreen_cb( Fl_Menu_* w, ViewerUI* ui );
    void toggle_float_on_top_cb( Fl_Menu_* w, ViewerUI* ui );
    void toggle_secondary_cb( Fl_Menu_* w, ViewerUI* ui );
    void toggle_secondary_float_on_top_cb( Fl_Menu_* w, ViewerUI* ui );
    
    void window_cb( Fl_Menu_* w, ViewerUI* ui );

    //! Masking callback
    void masking_cb( Fl_Menu_* w, ViewerUI* ui );

    //! HUD togle callback
    void hud_toggle_cb( Fl_Menu_* w, ViewerUI* ui );
    void hud_cb( Fl_Menu_* w, ViewerUI* ui );

    //! Auxiliary functions to remember what bars and what windows were
    //1 open in case of a fullscreen or presentation switch.
    void save_ui_state( ViewerUI* ui, Fl_Group* bar );
    void save_ui_state( ViewerUI* ui );
    void toggle_action_tool_bar( Fl_Menu_*, ViewerUI* ui );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar,
                        const int sizeX, const int sizeY );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size );
    void hide_ui_state( ViewerUI* ui );
    void restore_ui_state( ViewerUI* ui );

    // Playback callbacks
    void play_forwards_cb( Fl_Menu_*, ViewerUI* ui );
    void play_backwards_cb( Fl_Menu_*, ViewerUI* ui );
    void stop_cb( Fl_Menu_*, ViewerUI* ui );
    void toggle_playback_cb( Fl_Menu_*, ViewerUI* ui );

    void playback_loop_cb( Fl_Menu_*, ViewerUI* ui );
    void playback_once_cb( Fl_Menu_*, ViewerUI* ui );
    void playback_ping_pong_cb( Fl_Menu_*, ViewerUI* ui );

    // OCIO callbacks
    void attach_ocio_ics_cb( Fl_Menu_*, ViewerUI* ui );
    void attach_ocio_display_cb( Fl_Menu_*, ViewerUI* ui );
    void attach_ocio_view_cb( Fl_Menu_*, ViewerUI* ui );

    // Video levels callbacks
    void video_levels_from_file_cb( Fl_Menu_*, ViewerUI* ui );
    void video_levels_legal_range_cb( Fl_Menu_*, ViewerUI* ui );
    void video_levels_full_range_cb( Fl_Menu_*, ViewerUI* ui );
    
    // Alpha blend callbacks
    void alpha_blend_none_cb( Fl_Menu_*, ViewerUI* ui );
    void alpha_blend_straight_cb( Fl_Menu_*, ViewerUI* ui );
    void alpha_blend_premultiplied_cb( Fl_Menu_*, ViewerUI* ui );
}
