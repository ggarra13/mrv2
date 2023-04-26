// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cassert>

/* fltk includes */
#include <FL/Fl.H>

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvUtil.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvResizableBar.h"
#include "mrvPanelGroup.h"
#include "mrvDropWindow.h"
#include "mrvDockGroup.h"
#include "mrvCollapsibleGroup.h"

#include "mrViewer.h"

// On macOS, the buttons go to the left of the window.
#ifdef __APPLE__
#    define LEFT_BUTTONS 1
#endif

// #define DEBUG_COORDS

namespace
{
    const char* kIcon = "@-4circle";
}

namespace mrv
{
    void cb_dock(Fl_Button* o, void* v)
    {
        PanelGroup* gp = (PanelGroup*)v;
        if (!gp->docked())
        {
            gp->dock_grp(v);
        }
        else
        {
            gp->undock_grp(v);
        }
    }

    // function to handle the dock actions
    void PanelGroup::dock_grp(void* v)
    { // dock CB
        PanelGroup* gp = (PanelGroup*)v;
        DockGroup* dock = gp->get_dock();

        // we can only dock a group that's not already docked
        // and only if a dock exists for it
        if ((!gp->docked()) && (dock))
        {
            // Make sure we turn off the paenlgroup scroller, as we are going
            // to handle it with the dockgroup scroller
            gp->end();
            Pack* pack = gp->get_pack();
            Fl_Scroll* scroll = gp->get_scroll();
            int W = pack->w();
            if (pack->h() > dock->h())
            {
                W -= scroll->scrollbar.w();
            }
            pack->size(W, pack->h());
            scroll->size(W, pack->h());
            scroll->scroll_to(0, 0);
            dock->add(gp);    // move the toolgroup into the dock
            gp->docked(true); // toolgroup is docked...
            // so we no longer need the tool window.
            tw->hide();
            delete tw;
            tw = nullptr;
            layout();
            dock->redraw();
        }
    }

    // static CB to handle the undock actions
    void PanelGroup::undock_grp(void* v)
    { // undock CB
        PanelGroup* gp = (PanelGroup*)v;
        DockGroup* dock = gp->get_dock();

        if (gp->docked())
        { // undock the group into its own non-modal tool window
            int W = gp->w() + 3;
            int H = gp->h() + 3;
            int X = Fl::event_x_root() - 10;
            int Y = Fl::event_y_root() - 35;
            Fl_Group::current(0);
            tw = new PanelWindow(X, Y, W, H);
            tw->end();
            gp->docked(false); // toolgroup is no longer docked
            gp->end();         // needed to adjust pack and scroll
            dock->remove(gp);
            tw->add(gp);        // move the tool group into the floating window
            gp->position(1, 1); // align group in floating window
            tw->resizable(gp);
            auto settingsObject = App::ui->app->settingsObject();
            auto dragger = gp->get_dragger();
            std::string label = dragger->label();
            std::string prefix = "gui/" + label;
            std::string key;

            std_any value;

            key = prefix + "/WindowW";
            value = settingsObject->value(key);
            W = std_any_empty(value) ? W : std_any_cast<int>(value);

            key = prefix + "/WindowH";
            value = settingsObject->value(key);

            int H2 = std_any_empty(value) ? H : std_any_cast<int>(value);
            if (H2 != 0)
                H = H2;
            assert(H != 0);

            tw->resize(X, Y, W, H);
            tw->show();     // show floating window
            dock->redraw(); // update the dock, to show the group has gone...
        }
    }

    // static CB to handle the dismiss action
    void PanelGroup::cb_dismiss(Fl_Button*, void* v)
    {
        PanelGroup* gp = (PanelGroup*)v;
        DockGroup* dock = gp->get_dock();

        if (gp->docked())
        { // remove the group from the dock
            dock->remove(gp);
            gp->docked(false);
            dock->redraw(); // update the dock, to show the group has gone...
            Fl::delete_widget(gp);
        }
        else
        { // remove the group from the floating window,
            // and remove the floating window
            PanelWindow* tw = gp->get_window();
            tw->remove(gp);
            // we no longer need the tool window.
            Fl::delete_widget(tw);
            Fl::delete_widget(gp);
        }
    }

    void PanelGroup::docked(bool r)
    {
        _docked = r;
        if (r)
            docker->tooltip(_("Undock"));
        else
            docker->tooltip(_("Dock"));
    }

    void PanelGroup::resize(int X, int Y, int W, int H)
    {

        // W must be -3 to leave some headroom
        pack->size(W, pack->h());

        int GH = group && group->visible() ? group->h() : 0;
        assert(GH >= 0);
        int DH = docker->h();

        if (docked())
        {
            scroll->size(pack->w(), pack->h());
        }
        else
        {
            int screen = Fl::screen_num(tw->x(), tw->y(), tw->w(), tw->h());
            int minX, minY, maxW, maxH;
            Fl::screen_work_area(minX, minY, maxW, maxH, screen);

            // leave some headroom for topbar
            maxH = maxH - docker->h(); // 20 of offset

            H = tw->h() - GH - DH;

            if (H > maxH)
                H = maxH;

            if (group)
                group->size(W, group->h());

            scroll->size(pack->w(), H - 3);
            if (pack->h() < H - 20 - 3)
                pack->size(W - 3, H - 20 - 3);
            scroll->init_sizes(); // needed? to reset scroll size init size
        }

        assert(H > 0);
        Fl_Group::resize(X, Y, W, pack->h() + DH + GH);
        debug("RESIzE");

        // Make sure buttons don't stretch
        W = w() - 40 - 3;
#ifdef LEFT_BUTTONS
        X = x() + 40 + 3;
        dragger->resize(X, dragger->y(), W, dragger->h());
#else
        X = x();
        dragger->resize(X, dragger->y(), W, dragger->h());
        X = dragger->x() + dragger->w();
        docker->resize(X, docker->y(), 20, 20);
        X = docker->x() + docker->w();
        dismiss->resize(X, dismiss->y(), 20, 20);
#endif
    }

    void PanelGroup::end()
    {
        assert(h() > 0);
        pack->end();
        assert(h() > 0);
        Fl_Group::end();
        assert(h() > 0);
        layout();
        assert(h() > 0);
    }

    void PanelGroup::debug(const char* lbl) const
    {
#ifdef DEBUG_COORDS
        std::cerr << lbl << " ---------------------------------" << std::endl
                  << "       H=" << h() << std::endl
                  << "  pack H=" << pack->h() << std::endl
                  << "scroll H=" << scroll->h() << std::endl if (tw) std::cerr
                  << "tw     H=" << tw->h() << std::endl;
        std::cerr << "=============================================="
                  << std::endl;
        assert(h() > 0);
        assert(pack->h() > 0);
        assert(scroll->h() > 0);
        assert(tw->h() > 0);
#endif
    }

    void PanelGroup::layout()
    {
        pack->layout();
        Fl_Group* g = group;
        int GH = g && g->visible() ? g->h() : 0;
        int GY = g && g->visible() ? g->y() : 0;
        int DH = dragger->h();
        int W = w();
        int H = GH + DH + pack->h();
        assert(GH >= 0);

        Fl_Group::resizable(0);
        Fl_Group::size(W, H);
        Fl_Group::init_sizes();
        // Fl_Group::resizable(scroll);

        if (!docked())
        {
            // group->resize(group->x(), GY, group->w(), GH);

            int screen = Fl::screen_num(tw->x(), tw->y(), tw->w(), tw->h());
            int minX, minY, maxW, maxH;
            Fl::screen_work_area(minX, minY, maxW, maxH, screen);

            // leave some headroom for topbar
            maxH = maxH - DH; // 20 of offset

            int maxY = tw->y() + maxH - 3;
            int twY = tw->y() + H - 3;

            if (twY > maxY)
                H = maxH - 3;

            tw->size(W + 3, H + 3);

            H = tw->h() - GH - DH;

            if (H > maxH)
                H = maxH;

            scroll->size(pack->w(), H);
            scroll->init_sizes(); // needed? to reset scroll size init size
            debug("LAYOUT");
        }
    }

    // Constructors for docked/floating window
    // WITH x, y co-ordinates
    PanelGroup::PanelGroup(
        DockGroup* dk, int floater, int X, int Y, int W, int H,
        const char* lbl) :
        Fl_Group(1, 1, W, H),
        tw(nullptr)
    {
        assert(H > 0);
        if ((floater) && (dk)) // create floating
        {
            create_floating(dk, X, Y, W, H, lbl);
        }
        else if (dk) // create docked
        {
            create_docked(dk, lbl);
        }
        //	else //do nothing...
    }

    // construction function
    void PanelGroup::create_dockable_group(const char* lbl)
    {
        // Create a group to enclose the buttons and make it
        // not resizable.
        int W = 20;

#ifdef LEFT_BUTTONS
        int X = 3;
        Fl_Group* g = new Fl_Group(X, 3, W * 2, 20);
        dismiss = new PanelButton(X, 3, W, 20, kIcon);
        X += W;
        docker = new PanelButton(X, 3, W, 20, kIcon);

        g->end();
        g->resizable(0);

        X += W;
        W = w() - W * 2 - 3;

        dragger = new DragButton(X, 3, W, 20, lbl);
#else
        int X = x() + w() - W * 2 - 3;
        // Fl_Group* g = new Fl_Group(X, 3, W * 2, 20);
        docker = new PanelButton(X, 3, W, 20, kIcon);
        X = x() + w() - W - 3;
        dismiss = new PanelButton(X, 3, W, 20, kIcon);
        // g->end();

        dragger = new DragButton(3, 3, w() - W * 2 - 3, 20, lbl);
#endif
        dismiss->labelcolor(FL_RED);
        docker->labelcolor(FL_YELLOW);

        dismiss->box(FL_NO_BOX);
        dismiss->tooltip("Dismiss");
        dismiss->clear_visible_focus();
        dismiss->callback((Fl_Callback*)cb_dismiss, (void*)this);

        docker->box(FL_NO_BOX);
        docker->tooltip("Dock");
        docker->clear_visible_focus();
        docker->callback((Fl_Callback*)cb_dock, (void*)this);

        dragger->type(FL_TOGGLE_BUTTON);
        dragger->box(FL_ENGRAVED_BOX);
        dragger->tooltip("Drag Box");
        dragger->clear_visible_focus();
        dragger->align(
            FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_IMAGE_NEXT_TO_TEXT);
        dragger->when(FL_WHEN_CHANGED);

        group = new Fl_Group(x(), 23, w(), 30, "Group");
        group->labeltype(FL_NO_LABEL);
        group->hide();
        group->end();
        int GH = group->visible() ? group->h() : 0;

        scroll = new Fl_Scroll(3, 23, w() - 3, h() - 23, "Scroll");
        scroll->labeltype(FL_NO_LABEL);
        scroll->type(Fl_Scroll::BOTH);
        scroll->begin();

        pack = new Pack(3, 23, w() - 3, 1, "Pack");
        pack->labeltype(FL_NO_LABEL);
        pack->end();

        scroll->end();
        Fl_Group::resizable(scroll);
    }

    void PanelGroup::create_docked(DockGroup* dk, const char* lbl)
    {

        set_dock(dk); // define where the toolgroup is allowed to dock
        // Create the group itself
        create_dockable_group(lbl);
        // place it in the dock
        dk->add(this);
        docked(true); // docked
    }

    void PanelGroup::create_floating(
        DockGroup* dk, int X, int Y, int W, int H, const char* lbl)
    {

        // create the group itself
        create_dockable_group(lbl);
        // create a floating toolbar window
        // Ensure the window is not created as a child of its own inner group!
        Fl_Group::current(0);
        tw = new PanelWindow(X, Y, W + 3, H + 3);
        tw->end();
        set_dock(dk);  // define where the toolgroup is allowed to dock
        docked(false); // NOT docked
        tw->add(this); // move the tool group into the floating window
        this->position(1, 1);
        tw->resizable(this);
        tw->resize(X, Y, w() + 3, h() + 3);
        tw->show();
        // leave this group open when we leave the constructor...
        Fl_Group::current(pack);
    }

    // methods for hiding/showing *all* the floating windows
    // show all the active floating windows
    void PanelGroup::show_all(void)
    {
        PanelWindow::show_all();
    }

    //! hide all the active floating windows
    void PanelGroup::hide_all(void)
    {
        PanelWindow::hide_all();
    }

} // namespace mrv
