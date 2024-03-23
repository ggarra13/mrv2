// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Item.H>

#include "mrvPopupMenu.h"

#include "mrvFl/mrvPreferences.h"

extern Fl_Widget* fl_did_clipping;

namespace mrv
{

    static PopupMenu* pressed_menu_button_ = NULL;

    /*! The little down-arrow indicator can be replaced by setting a new
      glyph() function and making it draw whatever you want.
      If you don't want any glyph at all it is probably easiest to
      subclass and replace draw() with your own function.
    */
    void PopupMenu::draw()
    {
        // Fl_Menu_Button::draw();
        if (!box() || type())
            return;
        int H = 0;
        if (_enable_glyph)
            H = (labelsize() - 3) & -2;
        int X = x() + w() - H - Fl::box_dx(box()) - Fl::box_dw(box()) - 1;
        int Y = y() + (h() - H) / 2;
        if (Preferences::schemes.name == "Black" ||
            Preferences::schemes.name == "Dark Grey")
            color(Fl_Color(fl_rgb_color(75, 75, 75)));
        else
            color(Fl_Color(fl_rgb_color(140, 140, 140)));
        if (pressed_menu_button_ == this)
            draw_box(fl_down(box()), color());
        else
            draw_box(box(), color());
        draw_label(x() + Fl::box_dx(box()), y(), X - x() + 2, h());
        if (Fl::focus() == this)
            draw_focus();
        if (!_enable_glyph)
            return;
        fl_color(active_r() ? FL_DARK3 : fl_inactive(FL_DARK3));
        fl_line(X + H / 2, Y + H, X, Y, X + H, Y);
        fl_color(active_r() ? FL_LIGHT3 : fl_inactive(FL_LIGHT3));
        fl_line(X + H, Y, X + H / 2, Y + H);
    }

    const Fl_Menu_Item* PopupMenu::child(int i)
    {
        return &(menu()[i]);
    }

    bool PopupMenu::popped()
    {
        return (pressed_menu_button_ == this);
    }

    /**
      Act exactly as though the user clicked the button or typed the
      shortcut key.  The menu appears, it waits for the user to pick an item,
      and if they pick one it sets value() and does the callback or
      sets changed() as described above.  The menu item is returned
      or NULL if the user dismisses the menu.
      \note Since FLTK 1.4.0 Fl_Menu_::menu_end() is called before the menu
        pops up to make sure the menu array is located in private storage.

      \see Fl_Menu_::menu_end()
    */
    const Fl_Menu_Item* PopupMenu::popup()
    {
        menu_end();

        // If there's only one item (and the nullptr terminator) don't allow
        // picking.
        if (children() <= 2)
            return nullptr;

        const Fl_Menu_Item* m;
        pressed_menu_button_ = this;
        redraw();
        Fl_Widget_Tracker mb(this);
        if (!box() || type())
        {
            m = menu()->popup(
                Fl::event_x(), Fl::event_y(), label(), mvalue(), this);
        }
        else
        {
            m = menu()->pulldown(x(), y(), w(), h(), 0, this);
        }
        if (_disable_submenus && m && (m->flags & FL_SUBMENU))
            return nullptr;
        if (m && _enable_label)
        {
            const std::string& label = m->label();
            int numFound = 0;
            for (int t = 0; t < size(); t++)
            {
                const Fl_Menu_Item& item = menu()[t];
                if (!item.label())
                    continue;
                if (label == item.label())
                {
                    ++numFound;
                    if (numFound > 1)
                        break;
                }
            }

            if (numFound > 1)
            {
                char name[1024];
                int ok = item_pathname(name, sizeof(name) - 1, m);
                if (ok == 0)
                {
                    copy_label(name);
                }
            }
            else
            {
                copy_label(m->label());
            }
        }
        picked(m);
        pressed_menu_button_ = 0;
        if (mb.exists())
            redraw();
        return m;
    }

    int PopupMenu::handle(int e)
    {
        if (!menu() || !menu()->text)
            return 0;
        switch (e)
        {
        case FL_ENTER: /* FALLTHROUGH */
        case FL_LEAVE:
            return (box() && !type()) ? 1 : 0;
        case FL_PUSH:
            if (!box())
            {
                if (Fl::event_button() != 3)
                    return 0;
            }
            else if (type())
            {
                if (!(type() & (1 << (Fl::event_button() - 1))))
                    return 0;
            }
            if (Fl::visible_focus())
                Fl::focus(this);
            popup();
            return 1;
        case FL_KEYBOARD:
            if (!box())
                return 0;
            if (Fl::event_key() == ' ' &&
                !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META)))
            {
                popup();
                return 1;
            }
            else
                return 0;
        case FL_SHORTCUT:
            if (Fl_Widget::test_shortcut())
            {
                popup();
                return 1;
            }
            return test_shortcut() != 0;
        case FL_FOCUS: /* FALLTHROUGH */
        case FL_UNFOCUS:
            if (box() && Fl::visible_focus())
            {
                redraw();
                return 1;
            }
        default:
            return 0;
        }
    }

    void PopupMenu::value(int X)
    {
        if (X < 0 && X >= children())
            return;
        Fl_Menu_Button::value(X);
        const char* label = child(X)->label();
        if (_enable_label)
            copy_label(label);
        redraw_label();
    }

    PopupMenu::PopupMenu(int X, int Y, int W, int H, const char* l) :
        Fl_Menu_Button(X, Y, W, H, l),
        _enable_glyph(false),
        _enable_label(true),
        _disable_submenus(false)
    {
        align(FL_ALIGN_CENTER);
    }

} // namespace mrv
