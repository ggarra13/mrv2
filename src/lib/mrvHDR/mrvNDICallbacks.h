#pragma once

class HDRUI;
class Fl_Menu_;

namespace mrv
{
    void select_ndi_source_cb(Fl_Menu_* m, HDRUI* ui);
    void apply_metadata_cb(Fl_Menu_* m, HDRUI* ui);
    void toggle_fullscreen_cb(Fl_Menu_* m, HDRUI* ui);
}
