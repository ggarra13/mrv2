/* File : Fl_Browser.i */
//%module Fl_Browser

%feature("docstring") ::Fl_Browser
"""
The Fl_Browser widget displays a scrolling list of text lines, and manages
all the storage for the text. This is not a text editor or spreadsheet! But
it is useful for showing a vertical list of named objects to the user.

Each line in the browser is identified by number. The numbers start at one
(this is so that zero can be reserved for 'no line' in the selective browsers).
Unless otherwise noted, the methods do not check to see if the passed line
number is in range and legal. It must always be greater than zero
and <= size().

Each line contains a null-terminated string of text and a data pointer.
The text string is displayed, the  pointer can be used by the callbacks
to reference the object the text describes.

The base class does nothing when the user clicks on it. The subclasses
Fl_Select_Browser, Fl_Hold_Browser, and Fl_Multi_Browser react to user
clicks to select lines in the browser and do callbacks.

The base class called Fl_Browser_ provides the scrolling and selection
mechanisms of this and all the subclasses, but the dimensions and appearance
of each item are determined by the subclass. You can use Fl_Browser_ to
display information other than text, or text that is dynamically produced
from your own data structures. If you find that loading the browser is a
lot of work or is inefficient, you may want to make a subclass of Fl_Browser_.
""" ;

%{
#include "FL/Fl_Browser.H"
%}


%ignore Fl_Browser::add(const char*, void* = 0);
%ignore Fl_Browser::insert(int, const char*, void* = 0);
%ignore Fl_Browser::data(int) const ;
%ignore Fl_Browser::data(int, void* v);
%ignore Fl_Browser::column_widths(const int*);


%include "macros.i"

CHANGE_OWNERSHIP(Fl_Browser)

%include "FL/Fl_Browser.H"




#ifdef PYTHON

%include typemaps.i



%extend Fl_Browser {
  void add(const char *text, PyObject *data = 0)
    {
      Py_XINCREF(data);
      if (data) {
	void *tmp = (void *) 0 ;
        if ((SWIG_ConvertPtr(data,(void **) &tmp, 0, SWIG_POINTER_EXCEPTION | 0 )) == -1) {
	  PyErr_Clear();
	  // not a C object, just add it as is
	  self->add( text, data );
	}
	else {
	  // found C object, add it instead of data
	  self->add( text, tmp );
	}
      }
      else
	self->add( text );
    }
  void insert(int index, const char *text, PyObject *data = 0)
    {
      Py_XINCREF(data);
      //self->insert( index, text, data);
      if (data) {
	void *tmp = (void *) 0 ;
	if ((SWIG_ConvertPtr(data,(void **) &tmp, 0, SWIG_POINTER_EXCEPTION | 0 )) == -1) {
	  PyErr_Clear();
	  // not a C object, just add it as is
	  self->insert( index, text, data );
	}
	else {
	  // found C object, add it instead of data
	  self->insert( index, text, tmp );
	}
      }
      else
	self->insert( index, text, 0 );
    }

  // we keep this for back compatibility
PyObject *get_data( int index)
{
	PyObject *data = (PyObject *)(self->data(index));
	Py_XINCREF(data);
	return (PyObject *)data;
}
//PyObject *data( int index)
//{
//	PyObject *data = (PyObject *)(self->data(index));
//	Py_XINCREF(data);
//	return (PyObject *)data;
//}

PyObject *data( int index, PyObject *data = 0)
   {
     //Py_XDECREF((PyObject *)(self->data(index)));
     Py_XINCREF(data);

     if (data) {
       void *tmp = (void *) 0 ;
       if ((SWIG_ConvertPtr(data,(void **) &tmp, 0, SWIG_POINTER_EXCEPTION | 0 )) == -1) {
	 PyErr_Clear();
	 // not a C object, just add it as is
	 self->data( index, data );
       }
       else {
	 // found C object, add it instead of data
	 self->data( index, tmp );
       }
     }
     else {
       PyObject *data = (PyObject *)(self->data(index));
       Py_XINCREF(data);
       return (PyObject *)data;
     }

     Py_XINCREF(Py_None);
     return Py_None;
   }

void column_widths( PyObject *widths )
{
    //static const int no_cols[] = { 0 };

    if (PyTuple_Check(widths))
    {
       // the following lines should not be necessary?
       // commented out by ah, June 15, 2003
       //if (self->column_widths())
       //{
       //   // FLTK has its own no_cols...
       //   if (*self->column_widths())
       //   {
       //      //delete [] self->column_widths();
       //      self->column_widths(no_cols);
       //   }
       //}

       Py_ssize_t sz = PyTuple_Size(widths);
       if (sz)
       {
          int* cw = new int[sz+1];
          cw[sz] = 0;
          for (int k = 0; k<sz; ++k)
          {
             PyObject *obj = PyTuple_GetItem(widths, k);
             if (!obj)
             {
                delete[] cw;
                return;
             }

             if (!PyLong_Check(obj))
             {
                delete[] cw;
                PyErr_SetString(PyExc_TypeError, "Integer needed");
                return;
             }

             cw[k] = PyLong_AsLong(obj);
          }

          self->column_widths(cw);
       }
       else
       {
          // nothing to do
       }
    }
    else
    {
       PyErr_SetString(PyExc_TypeError, "Not a tuple");
       return;
    }
}

}
#endif
