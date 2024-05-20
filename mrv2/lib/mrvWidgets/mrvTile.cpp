// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

/**
  \class Tile

  The Tile class lets you resize its children by dragging
  the border between them.

  \image html Tile.png
  \image latex Tile.png "Tile" width=5cm

  For the tiling to work correctly, the children of an Tile must
  cover the entire area of the widget, but not overlap.
  This means that all children must touch each other at their edges,
  and no gaps can be left inside the Tile.

  Tile does not normally draw any graphics of its own.
  The "borders" which can be seen in the snapshot above are actually
  part of the children. Their boxtypes have been set to FL_DOWN_BOX
  creating the impression of "ridges" where the boxes touch. What
  you see are actually two adjacent FL_DOWN_BOX's drawn next to each
  other. All neighboring widgets share the same edge - the widget's
  thick borders make it appear as though the widgets aren't actually
  touching, but they are. If the edges of adjacent widgets do not
  touch, then it will be impossible to drag the corresponding edges.

  Tile allows objects to be resized to zero dimensions.
  To prevent this you can use the resizable() to limit where
  corners can be dragged to. For more information see note below.

  Even though objects can be resized to zero sizes, they must initially
  have non-zero sizes so the Tile can figure out their layout.
  If desired, call position() after creating the children but before
  displaying the window to set the borders where you want.

  <b>Note on resizable(Fl_Widget &w):</b>
  The "resizable" child widget (which should be invisible) limits where
  the borders can be dragged to. All dragging will be limited inside the
  resizable widget's borders. If you don't set it, it will be possible
  to drag the borders right to the edges of the Tile widget, and thus
  resize objects on the edges to zero width or height. When the entire
  Tile widget is resized, the resizable() widget will keep its border
  distance to all borders the same (this is normal resize behavior), so
  that you can effectively set a border width that will never change.
  To ensure correct event delivery to all child widgets the resizable()
  widget must be the first child of the Tile widget group. Otherwise
  some events (e.g. FL_MOVE and FL_ENTER) might be consumed by the resizable()
  widget so that they are lost for widgets covered (overlapped) by the
  resizable() widget.

  \note
  You can still resize widgets \b inside the resizable() to zero width and/or
  height, i.e. box \b 2b above to zero width and box \b 3a to zero height.

  \see void Fl_Group::resizable(Fl_Widget &w)

  Example for resizable with 20 pixel border distance:
  \code
    int dx = 20, dy = dx;
    Tile tile(50,50,300,300);
    // create resizable() box first
    Fl_Box r(tile.x()+dx,tile.y()+dy,tile.w()-2*dx,tile.h()-2*dy);
    tile.resizable(r);
    // ... create widgets inside tile (see test/tile.cxx) ...
    tile.end();
  \endcode

  See also the complete example program in test/tile.cxx.
*/

#include <iostream>
#include <vector>

#include <FL/Fl_Window.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl.H>
#include <stdlib.h>

#include "mrvWidgets/mrvTile.h"
#include "mrvWidgets/mrvTimelineGroup.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrViewer.h"

namespace
{
    const int kDRAGV = 2;
    const int kGRABAREA = 4;
} // namespace

namespace mrv
{

    struct WidgetData
    {
        Fl_Widget* o;
        int X;
        int Y;
        int W;
        int H;
    };

    struct MoveData
    {
        Tile* t;
        std::vector<WidgetData> widgets;
    };

    static void move_cb(void* data)
    {
        MoveData* d = static_cast<MoveData*>(data);

        for (const auto& w : d->widgets)
        {
            Fl_Widget* o = w.o;
            int X = w.X;
            int Y = w.Y;
            int W = w.W;
            int H = w.H;
            o->damage_resize(X, Y, W, H);
        }
        d->t->init_sizes();
        delete d;
    }

    /**
      Drags the intersection at (\p oldx,\p oldy) to (\p newx,\p newy).
      This redraws all the necessary children.

      Pass zero as \p oldx or \p oldy to disable drag in that direction.
    */
    void Tile::move_intersection(int oldx, int oldy, int newx, int newy)
    {
        Fl_Widget* const* a = array();
        Fl_Rect* p = bounds();
        p += 2; // skip group & resizable's saved size
        MoveData* data = new MoveData;
        data->t = this;
        for (int i = children(); i--; p++)
        {
            Fl_Widget* o = *a++;
            if (o == resizable())
                continue;
            int X = o->x();
            int R = X + o->w();
            if (oldx)
            {
                int t = p->x();
                if (t == oldx || (t > oldx && X < newx) ||
                    (t < oldx && X > newx))
                    X = newx;
                t = p->r();
                if (t == oldx || (t > oldx && R < newx) ||
                    (t < oldx && R > newx))
                    R = newx;
            }
            int Y = o->y();
            int B = Y + o->h();
            if (oldy)
            {
                int t = p->y();
                if (t == oldy || (t > oldy && Y < newy) ||
                    (t < oldy && Y > newy))
                    Y = newy;
                t = p->b();
                if (t == oldy || (t > oldy && B < newy) ||
                    (t < oldy && B > newy))
                    B = newy;
            }
            WidgetData widget;
            widget.o = o;
            widget.X = X;
            widget.Y = Y;
            widget.W = R - X;
            widget.H = B - Y;
            data->widgets.push_back(widget);
        }
        Fl::add_timeout(0.0, (Fl_Timeout_Handler)move_cb, data);
    }

    void Tile::resize(int X, int Y, int W, int H)
    {
        Fl_Group::resize(X, Y, W, H);
    }

    void Tile::init_sizes()
    {
        Fl_Tile::init_sizes();
    }

    static Fl_Cursor cursors[5] = {
        FL_CURSOR_DEFAULT, FL_CURSOR_WE, FL_CURSOR_NS, FL_CURSOR_MOVE,
        FL_CURSOR_CROSS};

    static void tile_set_dragbar_color(Fl_Tile* t, Fl_Cursor c)
    {
        Fl_Widget* tg = t->child(2); // mrv::TimelineGroup
        if (c != cursors[kDRAGV])
        {
            tg->color(51); // default color;
        }
        else
        {
            tg->color(FL_WHITE);
        }
        tg->redraw();
    }

    static void tile_set_cursor(Fl_Tile* t, Fl_Cursor c)
    {
        static Fl_Cursor cursor = FL_CURSOR_CROSS;
        if (cursor == c || !t->window())
            return;
        cursor = c;
        tile_set_dragbar_color(t, c);
        t->window()->cursor(c);
    }

    int Tile::handle(int event)
    {
        static int sdrag;
        static int sdx, sdy;
        static int sx, sy;

        int mx = Fl::event_x();
        int my = Fl::event_y();

        // \@bug: Window does not properly refresh the OpenGL timeline.
        //        So we need to the resize on a timeout.
#if defined(_WIN32)

        switch (event)
        {

        case FL_MOVE:
        case FL_ENTER:
        case FL_PUSH:
            // don't potentially change the mouse cursor if inactive:
            if (!active())
                break; // will cascade inherited handle()
            {
                int mindx = 100;
                int mindy = 100;
                int oldx = 0;
                int oldy = 0;
                Fl_Widget* const* a = array();
                Fl_Rect* q = bounds();
                Fl_Rect* p = q + 2;
                for (int i = children(); i--; p++)
                {
                    Fl_Widget* o = *a++;
                    if (o == resizable())
                        continue;
                    if (p->r() < q->r() && o->y() <= my + kGRABAREA &&
                        o->y() + o->h() >= my - kGRABAREA)
                    {
                        int t = mx - (o->x() + o->w());
                        if (abs(t) < mindx)
                        {
                            sdx = t;
                            mindx = abs(t);
                            oldx = p->r();
                        }
                    }
                    if (p->b() < q->b() && o->x() <= mx + kGRABAREA &&
                        o->x() + o->w() >= mx - kGRABAREA)
                    {
                        int t = my - (o->y() + o->h());
                        if (abs(t) < mindy)
                        {
                            sdy = t;
                            mindy = abs(t);
                            oldy = p->b();
                        }
                    }
                }
                sdrag = 0;
                sx = sy = 0;
                if (mindy <= kGRABAREA)
                {
                    sdrag |= kDRAGV;
                    sy = oldy;
                }

                Fl_Cursor cursor = cursors[sdrag];
                tile_set_cursor(this, cursor);
                if (sdrag)
                    return 1;
                return Fl_Group::handle(event);
            }

        case FL_LEAVE:
            tile_set_cursor(this, cursors[sdrag]);
            break;

        case FL_DRAG:
            // This is necessary if CONSOLIDATE_MOTION in Fl_x.cxx is turned
            // off: if (damage()) return 1; // don't fall behind
            {
                set_edit_button(mrv::EditMode::kSaved, App::ui);
                tile_set_cursor(this, cursors[sdrag]);
            }
        case FL_RELEASE:
        {
            if (!sdrag)
                break;
            Fl_Widget* r = resizable();
            if (!r)
                r = this;
            int newx = sx;
            int newy;
            if (sdrag & kDRAGV)
            {
                newy = Fl::event_y() - sdy;
                if (newy < r->y())
                    newy = r->y();
                else if (newy > r->y() + r->h())
                    newy = r->y() + r->h();
            }
            else
                newy = sy;
            move_intersection(sx, sy, newx, newy);
            if (event == FL_DRAG)
            {
                set_changed();
                do_callback(FL_REASON_DRAGGED);
            }
            else
            {
                do_callback(FL_REASON_CHANGED);
            }
            set_edit_button(mrv::EditMode::kSaved, App::ui);
            return 1;
        }
        }
#else

        switch (event)
        {

        case FL_MOVE:
        case FL_ENTER:
        case FL_PUSH:
            // don't potentially change the mouse cursor if inactive:
            if (!active())
                break; // will cascade inherited handle()
            {
                int mindy = 100;
                int oldy = 0;
                Fl_Widget* const* a = array();
                Fl_Rect* q = bounds();
                Fl_Rect* p = q + 2;
                for (int i = children(); i--; p++)
                {
                    Fl_Widget* o = *a++;
                    if (o == resizable())
                        continue;
                    if (p->b() < q->b() && o->x() <= mx + kGRABAREA &&
                        o->x() + o->w() >= mx - kGRABAREA)
                    {
                        int t = my - (o->y() + o->h());
                        if (abs(t) < mindy)
                        {
                            sdy = t;
                            mindy = abs(t);
                            oldy = p->b();
                        }
                    }
                }
                sdrag = 0;
                sx = sy = 0;
                if (mindy <= kGRABAREA)
                {
                    sdrag |= kDRAGV;
                    sy = oldy;
                }

                Fl_Cursor cursor = cursors[sdrag];
                tile_set_cursor(this, cursor);
            }

        case FL_LEAVE:
            break;

        case FL_DRAG:
            set_edit_button(mrv::EditMode::kSaved, App::ui);
            tile_set_cursor(this, cursors[sdrag]);
            break;
        case FL_RELEASE:
            set_edit_button(mrv::EditMode::kSaved, App::ui);
            break;
        default:
            break;
        }
#endif
        int ret = Fl_Tile::handle(event);
        if (ret && event == FL_RELEASE)
            init_sizes();
        return ret;
    }

    /**
  Creates a new Tile widget using the given position, size,
  and label string. The default boxtype is FL_NO_BOX.

  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code. A kludge has been done so the
  Tile and all of its children can be automatic (local)
  variables, but you must declare the Tile <I>first</I>, so
  that it is destroyed last.

  \see class Fl_Group
*/

    Tile::Tile(int X, int Y, int W, int H, const char* L) :
        Fl_Tile(X, Y, W, H, L)
    {
    }

} // namespace mrv
