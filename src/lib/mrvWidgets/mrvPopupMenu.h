// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "FL/Fl_Menu_Button.H"
#include "FL/Fl_Menu_Item.H"

namespace mrv
{

    class PopupMenu : public Fl_Menu_Button
    {
    public:
        PopupMenu(int, int, int, int, const char* = 0);
        virtual ~PopupMenu() {};

        unsigned children() { return size(); }
        const Fl_Menu_Item* child(int i) const;

        bool popped();

        const Fl_Menu_Item* popup();

        int value() const { return Fl_Menu_Button::value(); }
        void value(int x);

        // Undo the overrides of stuff from Group:
        void disable_glyph() { _enable_glyph = false; }
        void enable_glyph() { _enable_glyph = true; }
        void disable_label() { _enable_label = false; }
        void enable_label() { _enable_label = true; }
        void enable_character() { _enable_character = true; }
        void disable_submenus() { _disable_submenus = true; }

        virtual int handle(int e) override;

        virtual void draw() override;

    protected:
        bool _enable_glyph, _enable_label, _enable_character,
            _disable_submenus;
    };

} // namespace mrv
