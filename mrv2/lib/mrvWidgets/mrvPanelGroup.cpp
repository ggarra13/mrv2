// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

//#define DEBUG_CLIPPING 1

#include <cassert>

#include <FL/Fl.H>

#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvDropWindow.h"
#include "mrvWidgets/mrvPanelConstants.h"
#include "mrvWidgets/mrvPanelGroup.h"
#include "mrvWidgets/mrvResizableBar.h"

#include "mrvUI/mrvDesktop.h"
#include "mrvUI/mrvUtil.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

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
            gp->dock_grp();
        }
        else
        {
            gp->undock_grp();
        }
    }

    void PanelGroup::dock_grp()
    {
        assert(dock);

        // we can only dock a group that's not already docked
        // and only if a dock exists for it
        if (!docked())
        {
            // Make sure we turn off the panelgroup scroller, as we are going
            // to handle it with the dockgroup scroller
            end();

            auto settings = App::app->settings();
            auto dragger = get_dragger();
            const std::string& label = dragger->label();
            std::string prefix = "gui/" + label;
            std::string key;

            // Store window X and Y values.
            key = prefix + "/WindowX";
            settings->setValue(key, tw->x_root());

            key = prefix + "/WindowY";
            settings->setValue(key, tw->y_root());

            Pack* pack = get_pack();
            Fl_Scroll* scroll = get_scroll();
            int W = pack->w();
            if (pack->h() > dock->h())
            {
                W -= scroll->scrollbar.w();
            }
            pack->size(W, pack->h());
            scroll->size(W, pack->h());
            scroll->scroll_to(0, 0);
            dock->add(this);    // move the toolgroup into the dock
            docked(true); // toolgroup is docked...
            // so we no longer need the tool window.
            tw->hide();
            delete tw;
            tw = nullptr;
            layout();
            dock->redraw();
        }
    }

    void PanelGroup::set_Fl_Group()
    {
        Fl_Group::current(0);
        if (desktop::Wayland())
        {
            Fl_Group::current(dock->top_window());
        }
    }


    
    void PanelGroup::avoid_clipping(int& X, int& Y, int& W, int& H)
    {
        if (desktop::Wayland())
        {
            Fl_Window* top = dock->top_window();
            int maxW = top->w() - kMargin * 2;
            int maxH = top->h() - kMargin;

#ifdef DEBUG_CLIPPING
            std::cerr << "max     WxH=" << maxW << "x" << maxH << std::endl;
            std::cerr << "orig =" << X << " " << Y << " "
                      << W << "x" << H << std::endl;
#endif

            bool clippedX = false, clippedY = false;

            // Check clipping to the right and adjust X
            if (X < maxW && X + W > maxW)
            {
                clippedX = true;
                X = maxW - W;
            }
            
            // Check clipping to the bottom and adjust Y
            if (Y < maxH && Y + H > maxH)
            {
                clippedY = true;
                Y = maxH - H;
            }
            
#ifdef DEBUG_CLIPPING
            std::cerr << "first X,Y=" << X << "," << Y << std::endl;
#endif
            // Check clipping to the right again
            if (X < maxW && X + W > maxW)
            {
                clippedX = true;
                W = maxW - X;
            }
            
            // Check clipiping to the bottom again
            if (Y < maxH && Y + H > maxH)
            {
                clippedY = true;
                H = maxH - Y;
            }
            
#ifdef DEBUG_CLIPPING
            std::cerr << "clamped WxH=" << W << "x" << H << std::endl;
#endif

            // Check if clipping to the left and change X
            if (X < 0 && X + W >= 0)
            {
                clippedX = true;
                X = 0;
            }
            
            // Check if clipping to the top and change Y
            if (Y < 0 && Y + H >= 0)
            {
                clippedY = true;
                Y = 0;
            }
            
#ifdef DEBUG_CLIPPING
            std::cerr << "  non min WxH=" << W << "x" << H << std::endl;
#endif
            // Finally, check maximum sizes.  The clippedX/Y checks must
            // be reversed.
            if (clippedY)
                W = std::min(W, maxW);

            if (clippedX)
                H = std::min(H, maxH);

#ifdef DEBUG_CLIPPING
            std::cerr << "  final WxH=" << W << "x" << H << std::endl;
#endif
        }
    }
    
    void PanelGroup::undock_grp()
    {
        if (docked())
        { // undock the group into its own non-modal tool window
            int W = w() + kMargin;
            int H = h() + kMargin;
            int X = Fl::event_x_root() - 10;
            int Y = Fl::event_y_root() - 35;
            docked(false); // toolgroup is no longer docked

            auto settings = App::app->settings();
            auto dragger = get_dragger();
            std::string label = dragger->label();
            if (label == "Python" || label == "Logs")
                tw->size_range(640, 400);
            std::string prefix = "gui/" + label;
            std::string key;

            std_any value;

            // If we have window X, Y, W, and H values, use them.
            key = prefix + "/WindowX";
            value = settings->getValue<std::any>(key);
            X = std_any_empty(value) ? X : std_any_cast<int>(value);

            key = prefix + "/WindowY";
            value = settings->getValue<std::any>(key);
            Y = std_any_empty(value) ? Y : std_any_cast<int>(value);

            key = prefix + "/WindowW";
            value = settings->getValue<std::any>(key);
            W = std_any_empty(value) ? W : std_any_cast<int>(value);

            key = prefix + "/WindowH";
            value = settings->getValue<std::any>(key);

            int H2 = std_any_empty(value) ? H : std_any_cast<int>(value);
            if (H2 != 0)
                H = H2;
            assert(H != 0);

            avoid_clipping(X, Y, W, H);

            set_Fl_Group();

            tw = new PanelWindow(X, Y, W, H);
            tw->end();
            dock->remove(this);
            tw->add(this);  // move the tool group into the floating window
            position(1, 1); // align group in floating window (needed)
            size(W - kMargin, H - kMargin);     // resize to fit (needed)
            tw->resizable(this);
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
        int GH = group && group->visible() ? group->h() : 0;
        assert(GH >= 0);
        int DH = docker->h();

        pack->size(W, pack->h());

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
            assert(maxH > 0);

            assert(tw->h() >= DH);
            H = tw->h() - GH - DH;
            assert(H >= 0);

            if (tw->y() + H > minY + maxH)
            {
                H = maxH;
            }
            assert(H >= 0);

            if (group)
                group->size(W, group->h());

            scroll->resize(kMargin, scroll->y(), pack->w(), H - kMargin);
            if (pack->h() < H - kTitleBar - kMargin)
                pack->size(W, H - kTitleBar - kMargin);
            scroll->init_sizes(); // needed? to reset scroll size init size
        }

        Fl_Group::resize(X, Y, W, pack->h() + DH + GH);

        // Make sure buttons don't stretch
        W = w() - kButtonW * 2;
#ifdef LEFT_BUTTONS
        X = x() + kButtonW * 2;
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
        if (!docked())
        {
            int X = tw->x();
            int Y = tw->y();
            int W = tw->w();
            int H = tw->h();
        
            avoid_clipping(X, Y, W, H);
            tw->resizable(0);
            tw->resize(X, Y, W + kMargin, H + kMargin);
            tw->resizable(this);
        }
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

            int maxYH = tw->y() + maxH - kMargin;
            int twYH = tw->y() + H - kMargin;

            if (twYH > maxYH)
                H = maxH - kMargin;

            tw->resizable(0);
            tw->size(W + kMargin, H + kMargin);
            tw->resizable(this);

            H = tw->h() - GH - DH;

            if (H > maxH)
                H = maxH;

            scroll->size(pack->w(), H);
            scroll->init_sizes(); // needed to reset scroll size init size
        }
    }

    // Constructors for docked/floating window
    // WITH x, y co-ordinates
    PanelGroup::PanelGroup(
        DockGroup* dk, int floater, int X, int Y, int W, int H,
        const char* lbl) :
        Fl_Group(0, 0, W, H),
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
    void PanelGroup::create_dockable_group(bool docked, const char* lbl)
    {
        int X = x();
        int Y = y();

#ifdef LEFT_BUTTONS
        // Create a group to enclose the buttons and make it
        // not resizable on macOS.
        Fl_Group* g = new Fl_Group(X, Y, kButtonW * 2, 20);
        dismiss = new PanelButton(X, Y, kButtonW, 20, kIcon);
        X += kButtonW;
        docker = new PanelButton(X, Y, kButtonW, 20, kIcon);

        g->end();
        g->resizable(0);

        X += kButtonW;

        const int dragW = w() - kButtonW * 2;
        dragger = new DragButton(X, Y, dragW, 20, lbl);
#else
        dragger = new DragButton(X, Y, w() - kButtonW * 2, 20, lbl);
        X += dragger->w();
        docker = new PanelButton(X, Y, kButtonW, 20, kIcon);
        X += kButtonW;
        dismiss = new PanelButton(X, Y, kButtonW, 20, kIcon);
#endif
        dismiss->labelcolor(FL_RED);
        docker->labelcolor(FL_YELLOW);

        dismiss->box(FL_FLAT_BOX);
        dismiss->tooltip(_("Dismiss"));
        dismiss->clear_visible_focus();
        dismiss->callback((Fl_Callback*)cb_dismiss, (void*)this);

        docker->box(FL_FLAT_BOX);
        docker->tooltip(_("Dock"));
        docker->clear_visible_focus();
        docker->callback((Fl_Callback*)cb_dock, (void*)this);

        dragger->type(FL_TOGGLE_BUTTON);
        dragger->box(FL_ENGRAVED_BOX);
        dragger->tooltip(_("Drag Box"));
        dragger->clear_visible_focus();
        dragger->align(
            FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_IMAGE_NEXT_TO_TEXT);
        dragger->color(fl_lighter(dragger->color()));
        dragger->when(FL_WHEN_CHANGED);

        // Group is used for non scrolling widgets in the panel, like the
        // Search box in Media Info Panel.
        if (docked)
        {
            X = 0;
        }
        else
        {
            X = kMargin;
        }
        group = new Fl_Group(
            X, dragger->y() + dragger->h(), w() - kMargin, 30, "Group");
        group->labeltype(FL_NO_LABEL);
        group->hide();
        group->end();
        int GH = group->visible() ? group->h() : 0;

        // Scroll will contain a pack with this panel's contents.
        scroll = new Fl_Scroll(
            X, Y + dragger->h(), w() - kMargin, h() - dragger->h() - kMargin,
            "Scroll");
        scroll->labeltype(FL_NO_LABEL);
        scroll->type(Fl_Scroll::BOTH);
        scroll->begin();

        pack = new Pack(X, scroll->y(), scroll->w(), 1, "Pack");
        pack->labeltype(FL_NO_LABEL);
        pack->end();

        scroll->end();
        Fl_Group::resizable(scroll);
    }

    void PanelGroup::create_docked(DockGroup* dk, const char* lbl)
    {

        set_dock(dk); // define where the toolgroup is allowed to dock
        // Create the group itself
        create_dockable_group(true, lbl);
        // place it in the dock
        dk->add(this);
        docked(true); // docked
    }

    void PanelGroup::create_floating(
        DockGroup* dk, int X, int Y, int W, int H, const char* lbl)
    {

        // create the group itself
        create_dockable_group(false, lbl);

        set_dock(dk); // define where the toolgroup is allowed to dock

        // Check if it would clip (if so, offset on X/Y).
        avoid_clipping(X, Y, W, H);
        
        // create a floating toolbar window
        // Ensure the window is not created as a child of its own inner group!
        set_Fl_Group();
        tw = new PanelWindow(X, Y, W, H);
        tw->end();
        docked(false); // NOT docked
        tw->add(this); // move the tool group into the floating window
        this->position(1, 1);
        this->size(W - kMargin, H - kMargin);
        tw->resizable(0);
        tw->resize(X, Y, W, H);
        tw->resizable(this);
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
