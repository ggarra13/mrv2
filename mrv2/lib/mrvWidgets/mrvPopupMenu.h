// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "FL/Fl_Menu_Button.H"
#include "FL/Fl_Menu_Item.H"

namespace mrv {

class PopupMenu : public Fl_Menu_Button
{
public:
    PopupMenu(int,int,int,int,const char* =0);
    virtual ~PopupMenu() {};

    unsigned children() { return size(); }
    const Fl_Menu_Item* child(int i);

    bool popped();

    const Fl_Menu_Item* popup();

    // Undo the overrides of stuff from Group:
    void disable_glyph() {
        _enable_glyph = false;
    }
    void enable_glyph() {
        _enable_glyph = true;
    }
    void disable_label() {
        _enable_label = false;
    }
    void enable_label() {
        _enable_label = true;
    }

    virtual int handle(int e) override;

protected:
    virtual void draw() override;

    bool _enable_glyph, _enable_label;
};

}
