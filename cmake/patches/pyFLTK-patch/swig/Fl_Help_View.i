/* File : Fl_Help_View.i */
//%module Fl_Help_View

%feature("docstring") ::Fl_Help_View
"""
The Fl_Help_View widget displays HTML text. Most HTML 2.0 elements are
supported, as well as a primitive implementation of tables. GIF, JPEG,
and PNG images are displayed inline.
""" ;

%{
#include "FL/Fl_Help_View.H"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Help_View)

%{
#include "CallbackStruct.h"
#include <FL/Fl_Button.H>

  //static PyObject *my_pycallback = NULL;

  static const char* PythonLink(Fl_Widget *widget, const char *uri)
    {
      PyObject *func, *arglist;
      PyObject *result;
      PyObject *obj;

      CallbackStruct *cb = (CallbackStruct *)(widget->user_data());

      if (cb == NULL)
	return NULL;

      // This is the function ....
      func = (PyObject *)( cb->link);
      if (func == NULL) {
	PyErr_SetString(PyExc_NotImplementedError, "Callable link not found!");
	return NULL;
      }

      // always of the same type: Fl_Help_View
      //obj = SWIG_NewPointerObj(widget, SWIGTYPE_p_Fl_Help_View, 0);
      // the parent widget
      obj = (PyObject *)( cb->widget);

      // build arument list
      arglist = Py_BuildValue("(Os)", obj, uri );

      // call the callback
      result =  PyObject_CallObject(func, arglist);


      Py_DECREF(arglist);                           // Trash arglist
      Py_XDECREF(result);
      if (PyErr_Occurred())
	{
	  PyErr_Print();
	}
#ifndef PYTHON3
      if (result == Py_None)
	return NULL;
      else
	return PyBytes_AsString(result); /*void*/;
#else
     return NULL;
#endif
    }
  %}

%include "FL/Fl_Help_View.H"


%extend Fl_Help_View {
//#include <FL/Fl_Window.H>

  void
    link(PyObject *PyFunc, PyObject *PyWidget)
    {
      // get existing callback structure
      CallbackStruct *cb = (CallbackStruct*)self->user_data();

      // use the link member to hold the callback
      if (cb) {
	cb->link = PyFunc;
      }
      else {
	cb = new CallbackStruct( 0, 0, PyWidget, PyFunc );
	self->user_data((void*)cb);
      }

      // Add a reference to new callback
      Py_INCREF(PyFunc);
      Py_XINCREF(PyWidget);

      self->link(&PythonLink);


    }

}


%typemap(in) PyObject *PyFunc {
  if (!PyCallable_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    return NULL;
  }
  $1 = $input;
}
