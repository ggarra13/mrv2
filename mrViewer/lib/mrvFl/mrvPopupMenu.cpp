// "$Id: PopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php

/*! \class mrv::PopupMenu

This subclass pops up a menu in response to a user click. The menu is
popped up positioned so that the mouse is pointing at the last-selected
item, even if it in a nested submenu (To turn off this behaivor do
value(-1) after each item is selected)

\image html menu_button.gif

Normally any mouse button will pop up a menu and it is lined up above
the button, or below it when there is no previous selected value as
shown in the picture.

However a mrvPopupMenu can also have type() set to POPUP1, POPUP2,
POPUP12, POPUP3, POPUP13, POPUP23, or POPUP123. It then becomes invisible
and ignores all mouse buttons other than the ones named in the popup
type. You can then resize it to cover another widget (or many widgets)
so that pressing that button pops up the menu.

The menu will also pop up in response to shortcuts indicated by
the shortcut() or by putting '&x' in the label().

Typing the Fl_Widget::shortcut() of any menu items will cause it
to be picked. The callback will be done but there will be no visible
effect to the widget.

*/

#include "mrvPopupMenu.h"

#define __STDC_CONSTANT_MACROS

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Item.H>

#include "mrvFl/mrvPreferences.h"

extern Fl_Widget* fl_did_clipping;

namespace mrv {


static mrv::PopupMenu* pressed_menu_button_ = NULL;


/*! The little down-arrow indicator can be replaced by setting a new
  glyph() function and making it draw whatever you want.
  If you don't want any glyph at all it is probably easiest to
  subclass and replace draw() with your own function.
*/
void PopupMenu::draw() {
    //Fl_Menu_Button::draw();
  if (!box() || type()) return;
  int H = 0;
  if ( _enable_glyph ) H = (labelsize()-3)&-2;
  int X = x()+w()-H-Fl::box_dx(box())-Fl::box_dw(box())-1;
  int Y = y()+(h()-H)/2;
  if ( Preferences::schemes.name == "Black" ||
       Preferences::schemes.name == "Dark Grey"  )
      color( Fl_Color( fl_rgb_color( 75, 75, 75 ) ) );
  else
      color( Fl_Color( fl_rgb_color( 140, 140, 140 ) )  );
  if ( pressed_menu_button_ == this ) draw_box( fl_down(box()), color());
  else draw_box( FL_ROUNDED_BOX, color() );
  // draw_box(pressed_menu_button_ == this ? fl_down(box()) : box(), color());
  draw_label(x()+Fl::box_dx(box()), y(), X-x()+2, h());
  if (Fl::focus() == this) draw_focus();
  if ( !_enable_glyph ) return;
  fl_color(active_r() ? FL_DARK3 : fl_inactive(FL_DARK3));
  fl_line(X+H/2, Y+H, X, Y, X+H, Y);
  fl_color(active_r() ? FL_LIGHT3 : fl_inactive(FL_LIGHT3));
  fl_line(X+H, Y, X+H/2, Y+H);
}

const Fl_Menu_Item* PopupMenu::child(int i) {
    return &(menu()[i]);
}


bool PopupMenu::popped() {
    return ( pressed_menu_button_ == this );
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
const Fl_Menu_Item* PopupMenu::popup() {
    menu_end();
    const Fl_Menu_Item* m;
    pressed_menu_button_ = this;
    redraw();
    Fl_Widget_Tracker mb(this);
    if (!box() || type()) {
        m = menu()->popup(Fl::event_x(), Fl::event_y(), label(), mvalue(), this);
    } else {
        m = menu()->pulldown(x(), y(), w(), h(), 0, this);
    }
    if ( m && _enable_label ) copy_label( m->text );
    picked(m);
    pressed_menu_button_ = 0;
    if (mb.exists()) redraw();
    return m;
}

int PopupMenu::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  switch (e) {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
    return (box() && !type()) ? 1 : 0;
  case FL_PUSH:
    if (!box()) {
      if (Fl::event_button() != 3) return 0;
    } else if (type()) {
      if (!(type() & (1 << (Fl::event_button()-1)))) return 0;
    }
    if (Fl::visible_focus()) Fl::focus(this);
    popup();
    return 1;
  case FL_KEYBOARD:
    if (!box()) return 0;
    if (Fl::event_key() == ' ' &&
        !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
      popup();
      return 1;
    } else return 0;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) {popup(); return 1;}
    return test_shortcut() != 0;
  case FL_FOCUS: /* FALLTHROUGH */
  case FL_UNFOCUS:
    if (box() && Fl::visible_focus()) {
      redraw();
      return 1;
    }
  default:
    return 0;
  }
}

PopupMenu::PopupMenu(int X,int Y,int W,int H,const char *l)
    : Fl_Menu_Button(X,Y,W,H,l),
      _enable_glyph( false ),
      _enable_label( true )
{
    align(FL_ALIGN_CENTER);
}

} // namespace mrv

//
// End of "$Id: mrvPopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
