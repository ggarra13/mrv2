#include "mrvWidgets/mrvTooltip.h"

#include <FL/Fl_Box.H>

namespace mrv
{
    Tooltip::Tooltip(int X, int Y, int W, int H, const char* L) :
        Fl_Double_Window(X, Y, W, H)
    {
        border(0);
        set_non_modal();
        
        begin();
        box = new Fl_Box(1, 1, W-2, H-2, L);
        box->box(FL_FLAT_BOX);
        box->color(FL_YELLOW);
        box->labelcolor(FL_BLACK);
        box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        end();
        resizable(0);
    }
    
    void Tooltip::copy_label(const std::string& label)
    {
        std::string t = label;
        if (t.size() > 10)
            t = t.substr(0, 10) + "...";
                                
        box->copy_label(t.c_str());
    }
    
    void Tooltip::hide_cb(Tooltip* o)
    {
        o->hide();
    }

    void Tooltip::show()
    {
        Fl_Double_Window::show();
        Fl::add_timeout(3.0, (Fl_Timeout_Handler)hide_cb, this);
    }
}
