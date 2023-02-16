// $Id: Flu_Separator.cpp,v 1.2 2003/08/20 16:29:47 jbryan Exp $

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

#include "mrvFLU/Flu_Separator.h"

Flu_Separator ::Flu_Separator(int X, int Y, int W, int H, const char *l)
    : Fl_Widget(X, Y, W, H, l) {
  type(HORIZONTAL);
  box(FL_EMBOSSED_BOX);
}

void Flu_Separator ::draw() {
  if (_type == HORIZONTAL)
    draw_box(box(), x() + 3, y() + h() / 2, w() - 6, 2, color());
  else
    draw_box(box(), x() + w() / 2, y() + 3, 2, h() - 6, color());
}
