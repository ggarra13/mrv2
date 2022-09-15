#pragma once


class Fl_Widget;
class Fl_Menu_Item;
class Fl_Menu_;
class ViewerUI;

namespace mrv
{
    class TimelineViewport;

    void open_cb( Fl_Widget* w, ViewerUI* ui );
    void exit_cb( Fl_Widget* w, ViewerUI* ui );
    void display_options_cb( Fl_Menu_* w, TimelineViewport* view );
}
