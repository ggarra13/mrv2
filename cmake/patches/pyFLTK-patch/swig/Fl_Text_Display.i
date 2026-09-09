/* File : Fl_Text_Display.i */
//%module Fl_Text_Display

%feature("docstring") ::Fl_Text_Display
"""
This is the FLTK text display widget. It allows the user to view multiple
lines of text and supports highlighting and scrolling. The buffer that is
displayed in the widget is managed by the Fl_Text_Buffer class.
""" ;

// Redefine nested struct in global scope in order for SWIG to generate
// a proxy class. Only SWIG parses this definition.
struct Style_Table_Entry {
      Fl_Color	color;
      Fl_Font	font;
      int	size;
      unsigned	attr;
    };


%{
#include "FL/Fl_Text_Display.H"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Text_Display)

%ignore Fl_Text_Display::buffer(Fl_Text_Buffer& buf);
%ignore fl_text_drag_me(int pos, Fl_Text_Display* d);
// because of problems with Style_Table_Entry
%ignore Fl_Text_Display::highlight_data(Fl_Text_Buffer *styleBuffer,const Style_Table_Entry *styleTable,int nStyles, char unfinishedStyle,Unfinished_Style_Cb unfinishedHighlightCB,void *cbArg);
%ignore Fl_Text_Display::Style_Table_Entry;

//ToDo
%ignore fl_text_drag_prepare;

// delegate ownership of passed argument (Fl_Text_Buffer)
%pythonappend Fl_Text_Display::buffer %{
if len(args) > 0 and args[0] != None:
    #delegate ownership to C++
    args[0].this.disown()
%}

// typemap to convert a Python array to
// an array of Style_Table_Entry
%typemap(in) Fl_Text_Display::Style_Table_Entry const * {
  // Check if it is a list
  if (PyList_Check($input)) {
    Py_ssize_t size = PyList_Size($input);
    int i = 0;
    $1 = (Style_Table_Entry*) malloc((size)*sizeof(Style_Table_Entry));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyList_Check(o)) {
	Py_ssize_t item_size = PyList_Size(o);
	$1[i].color = (Fl_Color)PyLong_AsLong(PyList_GetItem(o,0));
	$1[i].font = (Fl_Font)PyLong_AsLong(PyList_GetItem(o,1));
	$1[i].size = PyLong_AsLong(PyList_GetItem(o,2));
	if (item_size > 3)
	  $1[i].attr = PyLong_AsLong(PyList_GetItem(o,3));
      }
    }
  }
  else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the Style_Table_Entry array we malloc'd before the function call
%typemap(freearg) Style_Table_Entry const * {
  //free((char *) $1);
}


// callback handling
%{
  static void UnfinishedStyleCB(int arg1, void *clientdata) {
    PyObject *func, *arglist;
    PyObject *result;

    // This is the function ....
    func = (PyObject *)( ((CallbackStruct *)clientdata)->func);
    if (((CallbackStruct *)clientdata)->data)
      {
	arglist = Py_BuildValue("(iO)", arg1, (PyObject *)(((CallbackStruct *)clientdata)->data) );
      }
    else
      {
	arglist = Py_BuildValue("(iO)", arg1 );
      }

    result =  PyObject_CallObject(func, arglist);

    Py_DECREF(arglist);                           // Trash arglist
      Py_XDECREF(result);
      if (PyErr_Occurred())
	{
	  PyErr_Print();
	}

      return /*void*/;

  }
%}





%{
// SWIG thinks that Style_Table_Entry is a global class, so we need to trick the C++
// compiler into understanding this so called global type.
typedef Fl_Text_Display::Style_Table_Entry Style_Table_Entry;
%}

%include "FL/Fl_Text_Display.H"

%extend Fl_Text_Display {
  %rename(highlight_data) highlight_data_new;
    void highlight_data_new(Fl_Text_Buffer *styleBuffer,
                        //const Fl_Text_Display::Style_Table_Entry *styleTable,
			const Style_Table_Entry *styleTable,
                        int nStyles, char unfinishedStyle,
                        PyObject *func,
                        PyObject *cbArg) {
      if (!PyCallable_Check(func)) {
	PyErr_SetString(PyExc_TypeError, "Need a callable object!");
      }
      else {
	CallbackStruct *cb = new CallbackStruct( func , cbArg, (PyObject*)0 );
	// add reference

	Py_INCREF(func);
	Py_XINCREF(cbArg);
	self->highlight_data(styleBuffer, styleTable, nStyles, unfinishedStyle, UnfinishedStyleCB, (void*)cb);
      }

  }
  //%rename(highlight_data) highlight_data_new;
}

%typemap(in) PyObject *PyFunc {
  if (!PyCallable_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    return NULL;
  }
  $1 = $input;
}
