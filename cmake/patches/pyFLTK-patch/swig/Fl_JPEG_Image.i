/* File : Fl_JPEG_Image.i */
//%module Fl_JPEG_Image

%feature("docstring") ::Fl_JPEG_Image
"""
The Fl_JPEG_Image class supports loading, caching, and drawing of Joint
Photographic Experts Group (JPEG) File Interchange Format (JFIF) images.
The class supports grayscale and color (RGB) JPEG image files.
""" ;

%{
#include "FL/Fl_JPEG_Image.H"
%}

//%include "macros.i"
//CHANGE_OWNERSHIP(Fl_JPEG_Image)

%typemap(in) const unsigned char *data {
    /* Check if the input support the buffer protocol */
  Py_buffer view;
  const void * buffer;
  int failure = PyObject_GetBuffer($input, &view, PyBUF_CONTIG_RO);
  if (!failure) {
    // work with array object
    buffer = view.buf;
    PyBuffer_Release(&view);
    $1 = (uchar *) buffer;
  } else {
    // work with list object
    // clear the error from PyObject_GetBuffer
    PyErr_Clear();
    buffer=0;
    /* Check if is a list */
    if (PyList_Check($input)) {
      Py_ssize_t size = PyList_Size($input);
      int i = 0;
      $1 = (uchar *) malloc((size+1)*sizeof(char));
      for (i = 0; i < size; i++) {
	PyObject *o = 	PyList_GetItem($input,i);
	if (PyLong_Check(o))
	  $1[i] = (uchar)PyLong_AsLong(o);
	else {
	  PyErr_SetString(PyExc_TypeError,"list must contain ints");
	  free($1);
	  return NULL;
	}
      }
      $1[i] = 0;
    } else {
      PyErr_SetString(PyExc_TypeError,"not a list or does not support single-segment readable buffer interface");
      return NULL;
    }
  }
}

%include "FL/Fl_JPEG_Image.H"
