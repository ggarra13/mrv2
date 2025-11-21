/* File : x.i */
//%module x

// typemap to map output of fl_xid from Window to long (Fl_Window*)
%typemap(out) Window {
    $result = PyLong_FromVoidPtr( (Fl_Window*)$1);
}	

%{
#include "FL/platform.H"
%}

%ignore fl_xid_;

%include "FL/platform.H"

// clear the output typemap
%typemap(out) Window;

