// $Id: Flu_Wrap_Group.cpp,v 1.8 2004/01/27 21:44:24 jbryan Exp $

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

#include <cstring>
#include <cstdio>
#include <cmath>

#include <iostream>

#include <FL/Fl_Window.H>

#include "mrvFLU/Flu_Wrap_Group.h"
#include "mrvFLU/Flu_Entry.h"

#define SCROLL_SIZE 15

Flu_Wrap_Group::Scrollbar::Scrollbar(
    int x, int y, int w, int h, const char* l) :
    Fl_Scrollbar(x, y, w, h, l)
{
}

int Flu_Wrap_Group::Scrollbar::handle(int event)
{
    switch(event)
    {
    case FL_MOUSEWHEEL:
        handle_drag(clamp(value() + linesize() * Fl::e_dy));
        redraw();
        return 1;
    default:
        break;
    }
    return Fl_Scrollbar::handle(event);
}

Flu_Wrap_Group::Flu_Wrap_Group(int x, int y, int w, int h, const char* l) :
    Fl_Group(x, y, w, h, l),
    scrollbar(x + w - SCROLL_SIZE, y, SCROLL_SIZE, h),
    group(x, y, w - SCROLL_SIZE, h, "GROUP IN Flu_Wrap_Group")
{
    offset(0, 0);
    spacing(0, 0);
    _type = FL_VERTICAL;
    scrollTo = NULL;

    Fl_Group::add(&scrollbar);
    scrollbar.callback(_scrollCB, this);
    scrollbar.linesize(10);
    scrollbar.range(0, 100);
    scrollbar.show();

    Fl_Group::add(&group);
    Fl_Group::resizable(group);
    Fl_Group::end();
    group.begin();
}

void Flu_Wrap_Group::resize(int x, int y, int w, int h)
{
    group.resizable(NULL);
    Fl_Group::resize(x, y, w, h);
    if (type() == FL_VERTICAL)
    {
        scrollbar.resize(
            x + w - SCROLL_SIZE - Fl::box_dx(box()), y + Fl::box_dy(box()),
            SCROLL_SIZE, h - Fl::box_dh(box()));
        group.resize(x, y, w - SCROLL_SIZE - Fl::box_dx(box()), h);
    }
    else
    {
        scrollbar.resize(
            x + Fl::box_dx(box()), y + h - SCROLL_SIZE - Fl::box_dy(box()),
            w - Fl::box_dw(box()), SCROLL_SIZE);
        group.resize(x, y, w, h - SCROLL_SIZE - Fl::box_dh(box()));
    }
    Fl_Group::init_sizes();
    redraw();
}

void Flu_Wrap_Group::scroll_to(const Fl_Widget* w)
{
    scrollTo = w;
    redraw();
}

void Flu_Wrap_Group::scroll_to_beginning()
{
    ((Fl_Valuator*)&scrollbar)->value(scrollbar.minimum());
}

void Flu_Wrap_Group::scroll_to_end()
{
    ((Fl_Valuator*)&scrollbar)->value(scrollbar.maximum());
}

void Flu_Wrap_Group::type(int t)
{
    _type = t;
    resize(x(), y(), w(), h());
}

Fl_Widget* Flu_Wrap_Group::next(Fl_Widget* w)
{
    for (int i = 0; i < group.children() - 1; i++)
    {
        if (w == group.child(i))
            return group.child(i + 1);
    }
    return NULL;
}

Fl_Widget* Flu_Wrap_Group::previous(Fl_Widget* w)
{
    for (int i = 1; i < group.children(); i++)
    {
        if (w == group.child(i))
            return group.child(i - 1);
    }
    return NULL;
}

Fl_Widget* Flu_Wrap_Group::above(Fl_Widget* w)
{
    for (int i = 0; i < group.children(); i++)
    {
        if (w == group.child(i))
        {
            int measure[2];
            measure[0] = w->x() + w->w() / 2;
            measure[1] = w->y() - _spacing[1];
            int index = layout(scrollbar.visible(), false, measure);
            if (index >= 0)
                return group.child(index);
            else
                return group.child(0);
        }
    }
    return NULL;
}

Fl_Widget* Flu_Wrap_Group::below(Fl_Widget* w)
{
    for (int i = 0; i < group.children(); i++)
    {
        if (w == group.child(i))
        {
            int measure[2];
            measure[0] = w->x() + w->w() / 2;
            measure[1] = w->y() + w->h() + _spacing[1];
            int index = layout(scrollbar.visible(), false, measure);
            if (index >= 0)
                return group.child(index);
            else
                return group.child(group.children() - 1);
        }
    }
    return NULL;
}

Fl_Widget* Flu_Wrap_Group::left(Fl_Widget* w)
{
    for (int i = 0; i < group.children(); i++)
    {
        if (w == group.child(i))
        {
            int measure[2];
            measure[0] = w->x() - _spacing[0];
            measure[1] = w->y() + w->h() / 2;
            int index = layout(scrollbar.visible(), false, measure);
            if (index >= 0)
                return group.child(index);
            else
                return group.child(0);
        }
    }
    return NULL;
}

Fl_Widget* Flu_Wrap_Group::right(Fl_Widget* w)
{
    for (int i = 0; i < group.children(); i++)
    {
        if (w == group.child(i))
        {
            int measure[2];
            measure[0] = w->x() + w->w() + _spacing[0] + 1;
            measure[1] = w->y() + w->h() / 2;
            int index = layout(scrollbar.visible(), false, measure);
            if (index >= 0)
                return group.child(index);
            else
                return group.child(group.children() - 1);
        }
    }
    return NULL;
}

int Flu_Wrap_Group::layout(bool sbVisible, bool doScrollTo, int* measure)
{
    int xx = x() + Fl::box_dx(box()), yy = y() + Fl::box_dy(box()),
        ww = w() - Fl::box_dw(box()), hh = h() - Fl::box_dh(box());

    if (type() == FL_VERTICAL)
    {
        int i, X, Y, maxH, H, col, row, maxW, scrollY;
        Fl_Widget* c;

        scrollbar.type(FL_VERTICAL);

    BEGIN_H:

        X = xx + _offset[0];
        Y = yy + _offset[1] - (sbVisible ? scrollbar.value() : 0);
        maxH = _offset[1];
        H = 0;
        col = 0;
        row = 0;
        scrollY = 0;
        maxW = xx + ww - (sbVisible ? scrollbar.w() : 0);

        for (i = 0; i < group.children(); i++)
        {
            c = group.child(i);
            if (!c->visible())
                continue;
            H = std::max(H, c->h());
            if (col == 0)
                maxH += H + _spacing[1];
            if ((X + c->w()) > maxW)
            {
                Y += H + _spacing[1];
                scrollY += H + _spacing[1];
                if (i == group.children() - 1)
                    maxH += H + _spacing[1];

                if (measure)
                {
                    if (xx + _offset[0] <= measure[0] &&
                        measure[0] <= xx + c->w() + _offset[0] + _spacing[0] &&
                        Y <= measure[1] &&
                        measure[1] <= Y + c->h() + _spacing[1])
                        return i;
                }
                else
                    c->position(xx + _offset[0], Y);

                col = 0;
                row++;
                H = 0;
                X = xx + c->w() + _offset[0] + _spacing[0];
            }
            else
            {
                if (measure)
                {
                    if (X <= measure[0] &&
                        measure[0] <= X + c->w() + _spacing[0] &&
                        Y <= measure[1] &&
                        measure[1] <= Y + c->h() + _spacing[1])
                        return i;
                }
                else
                    c->position(X, Y);
                X += c->w() + _spacing[0];
                col++;
            }

            if (doScrollTo && (c == scrollTo))
            {
                if (scrollY > scrollbar.maximum())
                    scrollY = (int)scrollbar.maximum();
                ((Fl_Valuator*)&scrollbar)->value(scrollY);
                scrollTo = NULL;
                goto BEGIN_H;
            }

            // if we exceed the height and the scrollbar is not visible,
            // then it will soon become visible so we don't need to process
            // anymore
            if (!measure && !sbVisible && maxH > hh)
                return 1;
        }

        if (measure)
            return -1;
        else if (maxH > hh)
        {
            scrollbar.range(0, maxH - hh);
            scrollbar.slider_size(std::max(
                float(scrollbar.h() - (maxH - hh)) / float(scrollbar.h()),
                0.08f));
            return 1;
        }
        else
            return 0;
    }
    else
    {
        int i, X, Y, W, maxW, maxH, col, row, scrollX;
        Fl_Widget* c;

        scrollbar.type(FL_HORIZONTAL);

    BEGIN_W:

        X = xx + _offset[0] - (sbVisible ? scrollbar.value() : 0);
        Y = yy + _offset[1];
        maxW = _offset[0];
        W = 0;
        col = 0;
        row = 0;
        scrollX = 0;
        maxH = yy + hh - (sbVisible ? scrollbar.h() : 0);

        for (i = 0; i < group.children(); i++)
        {
            c = group.child(i);
            if (!c->visible())
                continue;

            W = std::max(W, c->w());

            if (row == 0)
                maxW += W + _spacing[0];

            if ((Y + c->h()) > maxH)
            {
                X += W + _spacing[0];
                scrollX += W + _spacing[0];
                if (i == group.children() - 1)
                    maxW += W + _spacing[0];

                if (measure)
                {
                    if (X <= measure[0] &&
                        measure[0] <= X + c->w() + _spacing[0] &&
                        yy + _offset[1] <= measure[1] &&
                        measure[1] <= yy + c->h() + _offset[1] + _spacing[1])
                        return i;
                }
                else
                    c->position(X, yy + _offset[1]);

                row = 0;
                col++;
                W = 0;
                Y = yy + c->h() + _offset[1] + _spacing[1];
            }
            else
            {
                if (measure)
                {
                    if (X <= measure[0] &&
                        measure[0] <= X + c->w() + _spacing[0] &&
                        Y <= measure[1] &&
                        measure[1] <= Y + c->h() + _spacing[1])
                        return i;
                }
                else
                    c->position(X, Y);
                Y += c->h() + _spacing[1];
                row++;
            }

            if (doScrollTo && (c == scrollTo))
            {
                if (scrollX > scrollbar.maximum())
                    scrollX = (int)scrollbar.maximum();
                ((Fl_Valuator*)&scrollbar)->value(scrollX);
                scrollTo = NULL;
                goto BEGIN_W;
            }

            // if we exceed the width and the scrollbar is not visible,
            // then it will soon become visible so we don't need to process
            // anymore
            if (!measure && !sbVisible && maxW > ww)
                return 1;
        }

        if (measure)
            return -1;
        else if (maxW > ww)
        {
            scrollbar.range(0, maxW - ww);
            scrollbar.slider_size(std::max(
                float(scrollbar.w() - (maxW - ww)) / float(scrollbar.w()),
                0.08f));
            return 1;
        }
        else
            return 0;
    }
}

void Flu_Wrap_Group::draw_child(Fl_Widget& widget) const
{
    const int not_clipped =
        fl_not_clipped(widget.x(), widget.y(), widget.w(), widget.h());
    Flu_Entry* e = static_cast<Flu_Entry*>(&widget);
    if (widget.visible() && widget.type() < FL_WINDOW && not_clipped)
    {
        e->startRequest();
        widget.damage(FL_DAMAGE_ALL);
        widget.draw();
        widget.clear_damage();
    }
    else
    {
        if (not_clipped == 0 || !widget.visible())
            e->cancelRequest();
    }
}

void Flu_Wrap_Group::update_child(Fl_Widget& widget) const
{
    const int not_clipped =
        fl_not_clipped(widget.x(), widget.y(), widget.w(), widget.h());
    Flu_Entry* e = static_cast<Flu_Entry*>(&widget);
    if (widget.damage() && widget.visible() && widget.type() < FL_WINDOW &&
        not_clipped)
    {
        e->startRequest();
        widget.draw();
        widget.clear_damage();
    }
    else
    {
        if (not_clipped == 0 || !widget.visible())
            e->cancelRequest();
    }
}

void Flu_Wrap_Group::draw()
{
    // we first try to fit all children assuming no scrollbar. if they do not
    // all fit, we have to turn the scrollbar on and try again
    if (layout(false, false))
    {
        scrollbar.show();
        layout(true, false);
    }
    else
        scrollbar.hide();

    // hack to look right when resizing smaller
    if (scrollbar.value() > scrollbar.maximum())
    {
        ((Fl_Valuator*)&scrollbar)->value(scrollbar.maximum());
        layout(scrollbar.visible(), scrollTo != NULL);
    }
    else if (scrollTo)
        layout(scrollbar.visible(), true);

    scrollTo = NULL;

    if (damage() & ~FL_DAMAGE_CHILD)
    {
        draw_box();
        draw_label();
    }

    group.init_sizes();
    
    Fl_Widget*const* a = array();

    
    fl_push_clip(x() + Fl::box_dx(box()),
                 y() + Fl::box_dy(box()),
                 w() - Fl::box_dw(box()),
                 h() - Fl::box_dh(box()));

    int c = children();
    
    if (damage() & ~FL_DAMAGE_CHILD)
    { // redraw the entire thing:
        for (int i=c; i--;)
        {
            Fl_Widget& o = **a++;
            draw_child(o);
            draw_outside_label(o);
        }
    }
    else
    {
        // only redraw the children that need it:
        for (int i=c; i--;) update_child(**a++);
    }

    if (scrollbar.visible())
    {
        scrollbar.damage(FL_DAMAGE_ALL);
        scrollbar.draw();
        scrollbar.clear_damage();
    }
    
    fl_pop_clip();

}

int Flu_Wrap_Group::handle(int event)
{
    int ret = Fl_Group::handle(event);
    
    switch (event)
    {
    case FL_ENTER:
    case FL_LEAVE:
    case FL_MOVE:
        redraw();
        ret = 1;
        break;
    default:
        break;
    }
    return ret;
}
