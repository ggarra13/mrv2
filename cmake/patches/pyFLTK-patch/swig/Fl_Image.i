/* File : Fl_Image.i */
//%module Fl_Image

%feature("docstring") ::Fl_Image
"""
Fl_Image is the base class used for caching and drawing all kinds of images 
in FLTK. This class keeps track of common image data such as the pixels, 
colormap, width, height, and depth. Virtual methods are used to provide 
type-specific image handling.

Since the Fl_Image class does not support image drawing by itself, calling 
the draw() method results in a box with an X in it being drawn instead.
""" ;

%{
#include "FL/Fl_Image.H"
%}

//%include "macros.i"
//CHANGE_OWNERSHIP(Fl_Image)
//CHANGE_OWNERSHIP(Fl_RGB_Image)


%typemap(in) const uchar *bits {
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
	if (PyInt_Check(o))
	  $1[i] = (uchar)PyInt_AsLong(o);
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

// Typemap for converting const char *const * (image data) to Python bytes
%typemap(out) const char *const * {
    int height = arg1->data_h();
    int width = arg1->data_w();  // Get the image width
    int depth = arg1->d();       // Get the image depth (e.g., 3 for RGB)
    int size  = width * height * depth;

    const char *const *img_data = $1;  // Access the image data

    // Create a bytes object
    PyObject *pyBytes = PyBytes_FromStringAndSize(NULL, size);  

    if (img_data != NULL) {
        // Copy the data into the bytes object
        memcpy(PyBytes_AsString(pyBytes), img_data[0], size);
    }

    $result = pyBytes;  // Set the output result to the new bytes object
}

%ignore Fl_RGB_Image::id;
%ignore Fl_RGB_Image::mask;

%newobject Fl_Image::copy;

%include "FL/Fl_Image.H"



