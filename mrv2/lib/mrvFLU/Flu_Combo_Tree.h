// $Id: Flu_Combo_Tree.h,v 1.6 2004/03/24 02:49:00 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 *
 ***************************************************************/

#ifndef _FLU_COMBO_TREE_H
#define _FLU_COMBO_TREE_H

#include "FL/Fl_Tree.H"

#include "mrvFLU/Flu_Combo_Box.h"

//! Just like the Fl_Choice widget except the input area is editable and it can
//! display a tree instead of a list (using Flu_Tree_Browser)
class FLU_EXPORT Flu_Combo_Tree : public Flu_Combo_Box
{

public:
    //! Normal FLTK widget constructor
    Flu_Combo_Tree(int x, int y, int w, int h, const char* l = 0);

    //! Default destructor
    ~Flu_Combo_Tree();

    //! Publicly exposed tree widget (instance of Flu_Tree_Browser)
    Fl_Tree tree;

protected:
    bool _value(const char* v);
    const char* _next();
    const char* _previous();
    void _hilight(int event, int x, int y);

    inline static void _cb(Fl_Widget* w, void* arg)
    {
        ((Flu_Combo_Tree*)arg)->cb();
    }
    void cb();
};

#endif
