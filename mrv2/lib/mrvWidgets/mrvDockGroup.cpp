// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

#include <tlCore/StringFormat.h>

#include <FL/Fl.H>

#include "mrvPanelGroup.h"

#include "mrvDockGroup.h"
#include "mrvDropWindow.h"

namespace mrv
{

    // basic fltk constructors
    DockGroup::DockGroup(int x, int y, int w, int h, const char* l) :
        Fl_Group(x, y, w, h, l)
    {
        scroll = new Fl_Scroll(x, y, w, h);
        scroll->type(Fl_Scroll::BOTH);
        scroll->begin();

        pack = new Pack(x, y, w, h);
        pack->type(Pack::VERTICAL);
        pack->end();
        children = 0;
        scroll->resizable(pack);
        scroll->end();
        resizable(scroll);
        end();
    }

    void DockGroup::add(Fl_Widget* grp)
    {
        int wd = w();
        int ht = h();

        // if the dock is "closed", open it back up
        if (!parent()->visible())
        {
            DropWindow* dw = (DropWindow*)win;
            parent()->show();
            dw->workspace->layout();
        }
        pack->add(grp);
        children++;
    }

    void DockGroup::remove(Fl_Widget* grp)
    {
        int wd = w();
        int ht = h();
        pack->remove(grp);
        children--;

        // If the dock is empty, close it down
        if (children <= 0)
        {
            DropWindow* dw = (DropWindow*)win;
            children = 0; // just in case...!
            parent()->hide();
            dw->workspace->layout();
        }
    }

    std::vector<std::string> DockGroup::getPanelList() const
    {
        std::vector<std::string> out;
        for (int i = 0; i < pack->children(); ++i)
        {
            Fl_Widget*  w = pack->child(i);
            PanelGroup* g = dynamic_cast<PanelGroup*>(w);
            if (!g)
            {
                const std::string lbl = w->label() ? w->label() : "unknown";
                const std::string msg =
                    tl::string::Format("Not a Panel {0}").arg(lbl);
                out.push_back(msg);
            }
            else if (!g->label().empty())
                out.push_back(g->label().c_str());
            else
                out.push_back("Unknown Panel Label");
        }
        return out;
    }

} // namespace mrv
