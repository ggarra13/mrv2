#pragma once


class Fl_Widget;
class Fl_Group;
class Fl_Menu_Item;
class Fl_Menu_;
class ViewerUI;

namespace mrv
{
    class TimelineViewport;
    class MainWindow;

    void open_cb( Fl_Widget* w, ViewerUI* ui );
    void open_directory_cb( Fl_Widget* w, ViewerUI* ui );
    void exit_cb( Fl_Widget* w, ViewerUI* ui );

    void display_options_cb( Fl_Menu_* m, TimelineViewport* view );

    void mirror_x_cb( Fl_Menu_* m, TimelineViewport* view );
    void mirror_y_cb( Fl_Menu_* m, TimelineViewport* view );

    void toggle_red_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_green_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_blue_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_alpha_channel_cb( Fl_Menu_* m, TimelineViewport* view );

    void change_media_cb( Fl_Menu_* m, MainWindow* w );

    void wipe_cb( Fl_Menu_* m, MainWindow* w );

    void A_media_cb( Fl_Menu_* m, MainWindow* w );
    void B_media_cb( Fl_Menu_* m, MainWindow* w );

    void window_cb( Fl_Menu_* w, ViewerUI* ui );

    // HUD togle callbakc
    void hud_cb( Fl_Menu_* w, ViewerUI* ui );

    // Auxiliary functions to remember what bars and what windows were
    // open in case of a fullscreen or presentation switch.
    void save_ui_state( ViewerUI* ui );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar,
                        const int sizeX, const int sizeY );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size );
    void hide_ui_state( ViewerUI* ui );
    void restore_ui_state( ViewerUI* ui );
}
