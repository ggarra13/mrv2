// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

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
        if (d->t)
            d->t->init_sizes();
        delete d;
    }

    /**
      Drags the intersection at (\p oldx,\p oldy) to (\p newx,\p newy).
      This redraws all the necessary children.

      Pass zero as \p oldx or \p oldy to disable drag in that direction.
    */
    void Tile::move_intersection(
        int oldx, int oldy, int newx, int newy, bool release)
    {
        Fl_Widget* const* a = array();
        Fl_Rect* p = bounds();
        p += 2; // skip group & resizable's saved size
        MoveData* data = new MoveData;
        if (release)
            data->t = this;
        else
            data->t = nullptr;
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
        static int sdy;
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
            if (event == FL_DRAG)
            {
                move_intersection(sx, sy, newx, newy, false);
                set_changed();
                do_callback(FL_REASON_DRAGGED);
            }
            else
            {
                move_intersection(sx, sy, newx, newy, true);
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
