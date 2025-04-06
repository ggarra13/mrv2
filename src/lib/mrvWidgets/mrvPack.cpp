//
// "$Id$"
//
// Copyright 1998-2006 by Bill Spitzak and others.
// Port to fltk1.4 by Gonzalo Garramuño.
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
//

//
// This is a nabbed copy of fltk1.3.x Fl_Pack, modified to allow the
// layout code to be called directly, instead of hidden in draw(). -erco
// 03/19/19
//

// Based on code by Curtis Edwards
// Group that compresses all it's children together and resizes to surround
// them on each redraw (only if box() is zero)
// Bugs: ?

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

#include "mrvUI/mrvDesktop.h"

#include "mrvWidgets/mrvPack.h"

namespace mrv
{

    /**
       Creates a new Pack widget using the given position, size,
       and label string. The default boxtype is FL_NO_BOX.
       <P>The destructor <I>also deletes all the children</I>. This allows a
       whole tree to be deleted at once, without having to keep a pointer to
       all the children in the user code. A kludge has been done so the
       Pack and all of it's children can be automatic (local)
       variables, but you must declare the Pack<I>first</I>, so
       that it is destroyed last.
    */
    Pack::Pack(int X, int Y, int W, int H, const char* l) :
        Fl_Group(X, Y, W, H, l)
    {
        resizable(0);
        box(FL_NO_BOX);
        spacing_ = 0;
        // type(VERTICAL); // already set like this
    }

    Pack::~Pack() {}

    /**
       Recalculate the layout of the widget based on children.

       NOTE:
       In FLTK's Fl_Pack, this logic is normally done during draw() only,
       making it uncallable outside of a drawing context.

       The logic from draw() was copied here, with all drawing code removed
       so we can provide a way to pre-calculate the widget's size BEFORE
       the widget is drawn.
    */
    void Pack::layout()
    {
        int tx = x() + Fl::box_dx(box());
        int ty = y() + Fl::box_dy(box());
        int tw = w() - Fl::box_dw(box());
        int th = h() - Fl::box_dh(box());
        int rw, rh;
        int current_position = horizontal() ? tx : ty;
        int maximum_position = current_position;
        int last_child = 0;
        Fl_Widget* const* a = array();
        if (horizontal())
        {
            rw = -spacing_;
            rh = th;

            for (int i = children(); i--;)
                if (child(i)->visible())
                {
                    // last_child = i;
                    if (child(i) != this->resizable())
                        rw += child(i)->w();
                    rw += spacing_;
                }
        }
        else
        {
            rw = tw;
            rh = -spacing_;

            for (int i = children(); i--;)
                if (child(i)->visible())
                {
                    // last_child = i;
                    if (child(i) != this->resizable())
                        rh += child(i)->h();
                    rh += spacing_;
                }
        }
        for (int i = children(); i--;)
        {
            Fl_Widget* o = *a++;
            if (!o->visible())
                continue;
            int X, Y, W, H;
            if (horizontal())
            {
                X = current_position;
                W = o->w();
                Y = ty;
                H = th;
            }
            else
            {
                X = tx;
                W = tw;
                Y = current_position;
                H = o->h();
            }
            // Last child, if resizable, takes all remaining room
            if (i == last_child && o == this->resizable())
            {
                if (horizontal())
                    W = tw - rw;
                else
                    H = th - rh;
            }
            if (X != o->x() || Y != o->y() || W != o->w() || H != o->h())
            {
                o->resize(X, Y, W, H);
                o->clear_damage(FL_DAMAGE_ALL);
            }
            current_position += (horizontal() ? o->w() : o->h());
            if (current_position > maximum_position)
                maximum_position = current_position;
            current_position += spacing_;
        }

        if (horizontal())
        {
            tw = maximum_position - tx;
        }
        else
        {
            th = maximum_position - ty;
        }

        tw += Fl::box_dw(box());
        if (tw <= 0)
            tw = 1;
        th += Fl::box_dh(box());
        if (th <= 0)
            th = 1;
        if (tw != w() || th != h())
        {
            Fl_Widget::resize(x(), y(), tw, th);
        }
    }

    void Pack::resize(int X, int Y, int W, int H)
    {
        Fl_Widget::resize(X, Y, W, H);
        redraw();
    }

    void Pack::draw()
    {
        int tx = x() + Fl::box_dx(box());
        int ty = y() + Fl::box_dy(box());
        int tw = w() - Fl::box_dw(box());
        int th = h() - Fl::box_dh(box());
        int rw, rh;
        int current_position = horizontal() ? tx : ty;
        int maximum_position = current_position;
        uchar d = damage();
        int last_child = 0;
        Fl_Widget* const* a = array();
        if (horizontal())
        {
            rw = -spacing_;
            rh = th;

            for (int i = children(); i--;)
                if (child(i)->visible())
                {
                    last_child = i;
                    if (child(i) != this->resizable())
                        rw += child(i)->w();
                    rw += spacing_;
                }
        }
        else
        {
            rw = tw;
            rh = -spacing_;

            for (int i = children(); i--;)
                if (child(i)->visible())
                {
                    last_child = i;
                    if (child(i) != this->resizable())
                        rh += child(i)->h();
                    rh += spacing_;
                }
        }
        for (int i = children(); i--;)
        {
            Fl_Widget* o = *a++;
            if (o->visible())
            {
                int X, Y, W, H;
                if (horizontal())
                {
                    X = current_position;
                    W = o->w();
                    Y = ty;
                    H = th;
                }
                else
                {
                    X = tx;
                    W = tw;
                    Y = current_position;
                    H = o->h();
                }
                // Last child, if resizable, takes all remaining room
                if (i == last_child && o == this->resizable())
                {
                    if (horizontal())
                        W = tw - rw;
                    else
                        H = th - rh;
                }
                if (spacing_ && current_position > maximum_position && box() &&
                    (X != o->x() || Y != o->y() || d & FL_DAMAGE_ALL))
                {
                    fl_color(color());
                    if (horizontal())
                        fl_rectf(maximum_position, ty, spacing_, th);
                    else
                        fl_rectf(tx, maximum_position, tw, spacing_);
                }
                if (X != o->x() || Y != o->y() || W != o->w() || H != o->h())
                {
                    if (desktop::Wayland())
                    {
                        if (!o->as_window())
                            o->resize(X, Y, W, H);
                    }
                    else
                    {
                        o->resize(X, Y, W, H);
                        o->clear_damage(FL_DAMAGE_ALL);
                    }
                }
                if (d & FL_DAMAGE_ALL)
                {
                    draw_child(*o);
                    draw_outside_label(*o);
                }
                else
                    update_child(*o);
                // child's draw() can change it's size, so use new size:
                current_position += (horizontal() ? o->w() : o->h());
                if (current_position > maximum_position)
                    maximum_position = current_position;
                current_position += spacing_;
            }
        }

        if (horizontal())
        {
            if (maximum_position < tx + tw && box())
            {
                fl_color(color());
                fl_rectf(maximum_position, ty, tx + tw - maximum_position, th);
            }
            tw = maximum_position - tx;
        }
        else
        {
            if (maximum_position < ty + th && box())
            {
                fl_color(color());
                fl_rectf(tx, maximum_position, tw, ty + th - maximum_position);
            }
            th = maximum_position - ty;
        }

        tw += Fl::box_dw(box());
        if (tw <= 0)
            tw = 1;
        th += Fl::box_dh(box());
        if (th <= 0)
            th = 1;
        if (tw != w() || th != h())
        {
            Fl_Widget::resize(x(), y(), tw, th);
            d = FL_DAMAGE_ALL;
        }
        if (d & FL_DAMAGE_ALL)
        {
            draw_box();
            draw_label();
        }
    }
} // namespace mrv
