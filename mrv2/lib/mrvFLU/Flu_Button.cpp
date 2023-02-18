// $Id: Flu_Button.cpp,v 1.13 2004/05/24 19:01:05 jbryan Exp $

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

#include <iostream>

#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include "mrvFLU/Flu_Button.h"

// taken explicitly from Fl_Return_Button.cpp
static int flu_return_arrow(int x, int y, int w, int h)
{
    int size = w;
    if (h < size)
        size = h;
    int d = (size + 2) / 4;
    if (d < 3)
        d = 3;
    int t = (size + 9) / 12;
    if (t < 1)
        t = 1;
    int x0 = x + (w - 2 * d - 2 * t - 1) / 2;
    int x1 = x0 + d;
    int y0 = y + h / 2;
    fl_color(FL_LIGHT3);
    fl_line(x0, y0, x1, y0 + d);
    fl_yxline(x1, y0 + d, y0 + t, x1 + d + 2 * t, y0 - d);
    fl_yxline(x1, y0 - t, y0 - d);
    fl_color(fl_gray_ramp(0));
    fl_line(x0, y0, x1, y0 - d);
    fl_color(FL_DARK3);
    fl_xyline(x1 + 1, y0 - t, x1 + d, y0 - d, x1 + d + 2 * t);
    return 1;
}

Flu_Button ::Flu_Button(int X, int Y, int W, int H, const char* l) :
    Fl_Button(X, Y, W, H, l)
{
    inactiveImg = NULL;
    color(FL_GRAY);
    selection_color(FL_GRAY);
    retBtn = linkBtn = false;
    hover            = false;
    eBox             = FL_NO_BOX;
}

Flu_Button ::~Flu_Button()
{
    if (inactiveImg)
        delete inactiveImg;
}

void Flu_Button ::checkLink()
{
    // change the cursor if the mouse is over the link
    // the 'hover' variable reduces the number of times fl_cursor needs to be
    // called (since it can be expensive)
    if (linkBtn)
    {
        if (Fl::event_inside(
                x() + labelSize[0], y() + labelSize[1], labelSize[2],
                labelSize[3]))
        {
            if (!hover)
                fl_cursor(FL_CURSOR_HAND);
            hover = true;
        } else
        {
            if (hover)
                fl_cursor(FL_CURSOR_DEFAULT);
            hover = false;
        }
    }
}

int Flu_Button ::handle(int event)
{
    if (!active_r())
        return Fl_Button::handle(event);
    switch (event)
    {
    case FL_MOVE: {
        checkLink();
        return 1;
    }
    case FL_ENTER: {
        if (active())
        {
            Fl_Button::color(fl_lighter(col));
            Fl_Button::selection_color(fl_lighter(sCol));
        }
        checkLink();
        redraw();
        return 1;
    }
    break;
    case FL_LEAVE: {
        Fl_Button::color(col);
        Fl_Button::selection_color(sCol);
        checkLink();
        redraw();
        return 1;
        break;
    }
    case FL_UNFOCUS:
    case FL_FOCUS:
        return 1;
    case FL_KEYBOARD:
    case FL_KEYUP:
    case FL_SHORTCUT: {
        if (retBtn)
        {
            if (Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter)
            {
                do_callback();
                return 1;
            }
        }

        return 1;
    }
    }
    return Fl_Button::handle(event);
}

// modified explicitly from Fl_Return_Button.cpp
void Flu_Button ::draw()
{
    if (type() == FL_HIDDEN_BUTTON)
        return;

    if (!active())
        Fl_Button::color(col);

    // draw the link text
    if (linkBtn)
    {
        fl_draw_box(box(), x(), y(), w(), h(), color());
        labelSize[0] = labelSize[1] = labelSize[2] = labelSize[3] = 0;
        fl_font(labelfont(), labelsize());
        fl_measure(label(), labelSize[2], labelSize[3], 1);

        labelSize[0] += 2;
        labelSize[1] += h() / 2 - labelsize() / 2 - 2;

        fl_color(labelcolor());
        fl_draw(
            label(), x() + labelSize[0], y() + labelSize[1], labelSize[2],
            labelSize[3], FL_ALIGN_LEFT);

        if (!overLink || (overLink && hover))
        {
            fl_line_style(FL_SOLID);
            fl_line(
                x() + labelSize[0], y() + labelSize[1] + labelSize[3] - 2,
                x() + labelSize[0] + labelSize[2],
                y() + labelSize[1] + labelSize[3] - 2);
            fl_line_style(0);
        }
        return;
    }

    const char* lbl = label();
    if (retBtn)
        label("");
    if (eBox != FL_NO_BOX && Fl::belowmouse() == this && active())
    {
        Fl_Boxtype oldbox = box();
        box(eBox);
        Fl_Button::draw();
        box(oldbox);
    } else
        Fl_Button::draw();
    if (retBtn)
    {
        int W = h();
        if (w() / 3 < W)
            W = w() / 3;
        flu_return_arrow(x() + w() - W - 4, y(), W, h());
        label(lbl);
        draw_label(x(), y(), w() - W + 4, h());
    }
}

void Flu_Button ::image(Fl_Image* i)
{
    if (inactiveImg)
        delete inactiveImg;
    inactiveImg = NULL;
    if (i)
    {
        if (labeltype() != FL_NORMAL_LABEL)
            labeltype(FL_NORMAL_LABEL);
        Fl_Button::image(i);
        inactiveImg = Fl_Button::image()->copy();
        inactiveImg->desaturate();
        Fl_Button::deimage(inactiveImg);
    }
}
