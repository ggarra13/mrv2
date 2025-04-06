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

/* \file
   Pack widget . */

#pragma once

#include <iostream>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>

/**
   This widget was designed to add the functionality of compressing and
   aligning widgets.
   <P>If type() is Pack::HORIZONTAL all the children are
   resized to the height of the Pack, and are moved next to
   each other horizontally. If type() is not Pack::HORIZONTAL
   then the children are resized to the width and are stacked below each
   other.  Then the Pack resizes itself to surround the child
   widgets.
   <P>This widget is needed for the Fl_Tabs.
   In addition you may want to put the Pack inside an
   Fl_Scroll.

   <P>The resizable for Pack is set to NULL by default.</p>
   <P>See also: Fl_Group::resizable()
*/

namespace mrv
{

    class Pack : public Fl_Group
    {
        int spacing_;

    public:
        enum { // values for type(int)
            VERTICAL = 0,
            HORIZONTAL = 1
        };

    protected:
        virtual void draw() override;

    public:
        Pack(int x, int y, int w, int h, const char* l = 0);
        virtual ~Pack();

        void end()
        {
            Fl_Group::end();
            layout();
        }

        void resize(int X, int Y, int W, int H) FL_OVERRIDE;

        /**
           Gets the number of extra pixels of blank space that are added
           between the children.
        */
        int spacing() const { return spacing_; }
        /**
           Sets the number of extra pixels of blank space that are added
           between the children.
        */
        void spacing(int i) { spacing_ = i; }
        /** Same as Fl_Group::type() */
        uchar horizontal() const { return type(); }
        void layout();
    };

} // namespace mrv
