/* File : Fl_Pixmap.i */
//%module Fl_Pixmap

%feature("docstring") ::Fl_Pixmap
"""
The Fl_Pixmap class supports caching and drawing of colormap (pixmap) images, including transparency.
""" ;

%{
#include "FL/Fl_Pixmap.H"
%}

//%include "macros.i"
//CHANGE_OWNERSHIP(Fl_Pixmap)

// This tells SWIG to treat char ** as a special case
//%typemap(python,in) const char* const * pixmapData {
%typemap(in) char** {
  /* Check if it is a list */
  if (PyList_Check($input)) {
    Py_ssize_t size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);

%#if PY_VERSION_HEX>=0x03000000
      if (PyUnicode_Check(o))
%#else
      if (PyBytes_Check(o))
%#endif
      {
	 //$1[i] = PyBytes_AsString(PyList_GetItem($input,i));
	 //$1[i] = SWIG_Python_str_AsChar(PyList_GetItem($input,i));
         $1[i] = const_cast<char*>(PyUnicode_AsUTF8(PyList_GetItem($input,i)));
      }
      else {
	 PyErr_SetString(PyExc_TypeError,"list must contain strings");
	 free($1);
	 return NULL;
      }

    }
    $1[i] = NULL;

  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}


%typemap(freearg) char**  {
  // this is a memory leak, but what the heck!
  // free((char*)$1);
}

//%ignore Fl_Pixmap::Fl_Pixmap(const char* const * D);
%ignore Fl_Pixmap::Fl_Pixmap(const uchar* const * D);
%ignore Fl_Pixmap::Fl_Pixmap(char * const * D);
%ignore Fl_Pixmap::Fl_Pixmap(uchar * const * D);
%ignore Fl_Pixmap::id;
%ignore Fl_Pixmap::mask;


%include "FL/Fl_Pixmap.H"
