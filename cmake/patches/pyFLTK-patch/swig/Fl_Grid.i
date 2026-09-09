/* File : Fl_Grid.i */
//%module Fl_Grid
%feature("docstring") ::Fl_Grid
"""

""" ;

// Ignore the following methods:
//%rename("$ignore", regextarget=1, fullname=1) operator==;
//%rename("$ignore", regextarget=1, fullname=1) operator!=;

%{
#include "FL/Fl_Grid.H"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Grid)

%typemap(in,numinputs=1) (const int *value, size_t size) {
  size_t i;
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expecting a list");
    return NULL;
  }
  $2 = PyList_Size($input);
  $1 = (int *) malloc(($2)*sizeof(int));
  for (i = 0; i < $2; i++) {
    PyObject *s = PyList_GetItem($input,i);
    if (!PyLong_Check(s)) {
        free($1);
        PyErr_SetString(PyExc_ValueError, "List items must be integers");
        return NULL;
    }
    $1[i] = PyLong_AsLong(s);
  }
 }

%typecheck(SWIG_TYPECHECK_POINTER) (const int *value, size_t size) {
  $1 = PyList_Check($input) ? 1 : 0;
}

%typemap(freearg) (const int *value, size_t size) {
   if ($1) free($1);
}

%include "FL/Fl_Grid.H"
