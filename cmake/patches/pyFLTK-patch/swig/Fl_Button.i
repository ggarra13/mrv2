/* File : Fl_Button.i */
//%module Fl_Button

%feature("docstring") ::Fl_Button
"""
Buttons generate callbacks when they are clicked by the user. You control
exactly when and how by changing the values for type()  and when().

Buttons can also generate callbacks in response to FL_SHORTCUT events. The
button can either have an explicit shortcut() value or a letter shortcut
can be indicated in the label() with an '&' character before it. For the
label shortcut it does not matter if Alt is held down, but if you have an
input field in the same window, the user will have to hold down the Alt key
so that the input field does not eat the event first as an FL_KEYBOARD event.
""" ;

%{
#include "FL/Fl_Button.H"
%}

%include "macros.i"

// typemap to map output of Fl_Button.value from char to int
%typemap(out) char {
    $result = PyLong_FromLong( (long)$1);
}

CHANGE_OWNERSHIP(Fl_Button)

%include "FL/Fl_Button.H"

// clear the typemap for char
%typemap(out) char;
