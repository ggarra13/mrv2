
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

#include <mrvWidgets/mrvTile.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Rect.H>
#include <stdlib.h>

namespace mrv
{

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
            o->damage_resize(X, Y, R - X, B - Y);
        }
    }

    void Tile::resize(int X, int Y, int W, int H)
    {
        Fl_Group::resize(X, Y, W, H);
        return;
    }

    int Tile::handle(int event)
    {

        int ret = Fl_Tile::handle(event);
        if (event == FL_RELEASE)
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
