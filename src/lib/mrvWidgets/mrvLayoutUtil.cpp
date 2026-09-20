// mrvWidgets/mrvLayoutUtil.cpp
#include "mrvWidgets/mrvLayoutUtil.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvPanelGroup.h"
#include "mrvWidgets/mrvAnnotationGroup.h"

#include <FL/Fl_Window.H>

namespace mrv
{
    void relayout(Fl_Widget* w)
    {
        Fl_Widget* p = w->parent();
        while (p)
        {
            if (auto* pack = dynamic_cast<Pack*>(p))
                pack->layout();
            else if (auto* ag = dynamic_cast<AnnotationGroup*>(p))
                ag->layout();
            else if (auto* pg = dynamic_cast<PanelGroup*>(p))
            {
                pg->layout(); // also fires the dock-relayout callback, see below
                break;        // PanelGroup is the top of the layout-aware chain
            }
            p = p->parent();
        }
        if (w->window())
            w->window()->redraw();
    }
}
