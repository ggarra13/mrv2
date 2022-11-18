#include <iostream>
#include "mrvButton.h"

#include <FL/fl_draw.H>
#include <FL/names.h>

namespace mrv
{


    Button::Button( int X, int Y, int W, int H, const char* L ) :
        Fl_Button( X, Y, W, H, L )
    {
        default_color = color();
    }

    int Button::handle( int e )
    {
        switch( e )
        {
        case FL_ENTER:
            default_color = color();
            if ( active_r() && !value() )
                color( fl_lighter( default_color ) );
            redraw();
            return 1;
        case FL_LEAVE:
            color( default_color );
            redraw();
            return 1;
        case FL_KEYBOARD:
            return 0;
        }
        int ret = Fl_Button::handle( e );
        return ret;
    }

    void Button::draw()
    {
        if (type() == FL_HIDDEN_BUTTON) return;
        Fl_Color col = value() ? selection_color() : color();
        draw_box( FL_FLAT_BOX, color() ); // needed to clear background on round
        if ( value() ) draw_box(down_box(), col);
        draw_backdrop();
        if ( value() ) labelcolor( FL_CYAN );
        else labelcolor( 28 );
        draw_label();
    }


    CheckButton::CheckButton( int X, int Y, int W, int H, const char* L ) :
        Fl_Check_Button( X, Y, W, H, L )
    {
    }

    void
    CheckButton::draw()
    {
        Fl_Check_Button::draw();
    }

    RadioButton::RadioButton( int X, int Y, int W, int H, const char* L ) :
        Fl_Radio_Button( X, Y, W, H, L )
    {
    }

    void
    RadioButton::draw()
    {
        Fl_Radio_Button::draw();
    }

    Toggle_Button::Toggle_Button( int X, int Y, int W, int H, const char* L ) :
        Fl_Toggle_Button( X, Y, W, H, L )
    {
        box( FL_FLAT_BOX ); // here it does not work
        down_box( FL_PLASTIC_ROUND_DOWN_BOX );
    }

    void Toggle_Button::draw()
    {
        if (type() == FL_HIDDEN_BUTTON) return;
        Fl_Color col = value() ? selection_color() : color();
        box( FL_FLAT_BOX );  // here it works
        draw_box(value() ? down_box() : box(), col);
        draw_backdrop();
        if ( value() ) labelcolor( FL_CYAN );
        else labelcolor( 28 );
        draw_label();
        if (Fl::focus() == this) draw_focus();
    }

}
