// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include <FL/Fl_Group.H>

#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvDragButton.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvPanelButton.h"
#include "mrvWidgets/mrvPanelWindow.h"
#include "mrvWidgets/mrvScroll.h"

namespace mrv
{

    class PanelGroup : public Fl_Group
    {
    private:
        // control variables
        bool _docked = true;
        DockGroup* dock = nullptr;

        // constructor helper function
        void create_dockable_group(bool docked, const char* lbl);
        void create_docked(DockGroup* d, const char* lbl);
        void create_floating(
            DockGroup* d, int x, int y, int w, int h, const char* l);

        void set_Fl_Group();
        
    protected:
        // Widgets used by the toolbar
        DragButton* dragger = nullptr;
        PanelButton* docker = nullptr;
        PanelButton* dismiss = nullptr;
        PanelWindow* tw = nullptr;
        Scroll* scroll = nullptr;
        Pack* pack = nullptr;
        Fl_Group* group = nullptr;
        std::string _label;

        //! Defines which dock the group can dock into
        inline void set_dock(DockGroup* w) { dock = w; }

        //! Sets whether window is docked or not.
        void docked(bool r);

    public:
        // Constructors for docked/floating window
        PanelGroup(
            DockGroup* d, int f, int x, int y, int w, int h, const char* l = 0);

        // Get the toolwindow or null if docked
        DragButton* get_dragger() const { return dragger; }
        Fl_Group* get_group() const { return group; }
        Pack* get_pack() const { return pack; }
        Scroll* get_scroll() const { return scroll; }
        PanelWindow* get_window() const { return tw; }
        
        // get the dock group ID
        inline DockGroup* get_dock(void) { return dock; }

        inline void bind_image(Fl_Image* img) { dragger->bind_image(img); }

        // Recalculate the sizes
        void layout();

        // methods for hiding/showing *all* the floating windows
        static void show_all(void);
        static void hide_all(void);

        // Tests whether window is docked or not.
        bool docked() { return _docked | !tw; }

        // generic callback function for the dock/undock checkbox
        void dock_grp();
        void undock_grp(bool button = true);

        // generic callback function for the dismiss button
        static void cb_dismiss(Fl_Button*, void* v);

        inline void callback(Fl_Callback* c, void* d)
        {
            dismiss->callback(c, d);
        }

        inline void setLabel(const std::string& l) { _label = l; }
        inline std::string label() const { return _label; }
        inline const char* translatedLabel() const { return dragger->label(); }
        // wrap some basic Fl_Group functions to access the enclosed pack
        inline void clear() { pack->clear(); }
        inline void begin() { pack->begin(); }
        void end();
        void resize(int X, int Y, int W, int H) FL_OVERRIDE;
        inline void resizable(Fl_Widget* box) { pack->resizable(box); }
        inline void resizable(Fl_Widget& box) { pack->resizable(box); }
        inline Fl_Widget* resizable() const { return pack->resizable(); }
        inline void add(Fl_Widget* w) { pack->add(w); }
        inline void add(Fl_Widget& w) { add(&w); }
        inline void insert(Fl_Widget& w, int n) { pack->insert(w, n); }
        inline void insert(Fl_Widget& w, Fl_Widget* beforethis)
        {
            pack->insert(w, beforethis);
        }
        inline void remove(Fl_Widget& w) { pack->remove(w); }
        inline void remove(Fl_Widget* w) { pack->remove(w); }
        inline int children() const { return pack->children(); }
    };

} // namespace mrv
