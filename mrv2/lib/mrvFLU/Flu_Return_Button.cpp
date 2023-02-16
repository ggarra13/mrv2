// $Id: Flu_Return_Button.cpp,v 1.2 2003/08/20 16:29:47 jbryan Exp $

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

#include "mrvFLU/Flu_Return_Button.h"

Flu_Return_Button ::Flu_Return_Button(int X, int Y, int W, int H, const char *l)
    : Flu_Button(X, Y, W, H, l) {
  retBtn = true;
}

Flu_Return_Button ::~Flu_Return_Button() {}
