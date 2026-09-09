/* File : Fl_Gl_Window.i */
//%module Fl_Gl_Window

%feature("docstring") ::Fl_Gl_Window
"""
The Fl_Gl_Window widget sets things up so OpenGL works, and also keeps an
OpenGL 'context' for that window, so that changes to the lighting and
projection may be reused between redraws. Fl_Gl_Window also flushes the
OpenGL streams and swaps buffers after draw()  returns.

OpenGL hardware typically provides some overlay bit planes, which are very
useful for drawing UI controls atop your 3D graphics. If the overlay hardware
is not provided, FLTK tries to simulate the overlay, This works pretty well
if your graphics are double buffered, but not very well for single-buffered.
""" ;

%feature("nodirector") Fl_Gl_Window::show;

%{
#include "FL/Fl_Gl_Window.H"
#include "FL/gl.h"
#include "string.h"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Gl_Window)

%include "WindowShowTypemap.i"
%include typemaps.i

// override method show
%extend Fl_Gl_Window {
	MACRO_WINDOW_SHOW
}

// typemap to map output of Fl_Gl_Window.valid from char to int
%typemap(out) char {
    $result = PyLong_FromLong( (long)$1);
}

// ignore original declaration
%ignore Fl_Gl_Window::show();
%ignore Fl_Gl_Window::show(int argc, char** argv);

%include "FL/Fl_Gl_Window.H"
%include "FL/gl.h"

%feature("docstring") Fl_Gl_Window::drawPixels
"Writes a raw RGB string to the canvas.

Arguments:
    - rgb - a string of width * height * 3 bytes, comprising
      the raw image in RGB format
"

%extend Fl_Gl_Window {

void Fl_Gl_Window::drawPixels(PyObject *rgb_py) {

    char *rgb;
    size_t len;
    int i, height=self->h(), halfheight=self->h()/2, rowsize=self->w()*3;
    char *row0, *row1;

    char *tmp = new char [rowsize];
    SWIG_AsCharPtrAndSize(rgb_py, &rgb, &len, 0);

    row0 = rgb;
    row1 = rgb + (height-1) * rowsize;
    for (i=0; i<halfheight; i++) {
        memcpy(tmp, row0, rowsize);
        memcpy(row0, row1, rowsize);
        memcpy(row1, tmp, rowsize);
        row0 += rowsize;
        row1 -= rowsize;
    }

    glDrawPixels(self->w(), self->h(), GL_RGB, GL_UNSIGNED_BYTE, rgb);

    delete [] tmp;
}

};

// clear the typemap for char
%typemap(out) char;
