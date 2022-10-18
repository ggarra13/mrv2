/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvCollapsibleGroup.h
 * @author gga
 * @date   Tue Aug  7 04:48:52 2007
 *
 * @brief  A group widget that has a button allowing it to collapse
 *         the contents.
 *
 *
 */

#ifndef mrvCollapsibleGroup_h
#define mrvCollapsibleGroup_h

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

#include "mrvPack.h"

namespace mrv {

  class CollapsibleGroup : public Fl_Group {
    public:
      CollapsibleGroup( const int x, const int y, const int w,
                        const int h, const char* l = 0 );
      ~CollapsibleGroup();
      void begin() {
          Fl_Group::begin();
          _contents->begin();
      }
      void end() {
          _contents->end();
          Fl_Group::end();
          layout();             // recalc our own layout
      }
      void add( Fl_Widget* w );
      void clear();
      void spacing( int x );
      void resize(int X,int Y,int W,int H);
      Pack* contents() { return _contents; }
      Fl_Button* button() { return _button; }
      void layout();

    protected:
      Fl_Button*  _button;
      Pack*     _contents;

      static void toggle_tab_cb( Fl_Button* w, void* data );
      void        toggle_tab( Fl_Button* w);
      void relabel_button();
      // virtual void draw();  // DEBUG

    public:     // added these -erco
      // Open/close the widget
      void open();
      void close();
      // Is widget open?
      bool is_open() const {
          return _contents->visible() ? true : false;
      }
  };

} // namespace mrv

#endif // mrvCollapsibleGroup_h
