/* File : fl_draw.i */
//%module fl_draw

%include "typemaps.i"

%{
#include "FL/fl_draw.H"
%}

%typemap(in) const uchar * {
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
               $1[i] = (char)PyInt_AsLong(o);
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


%ignore fl_color(int c);
%ignore fl_draw_pixmap(const char* const* data, int x,int y,Fl_Color=FL_GRAY);
%ignore fl_measure_pixmap(const char* const* data, int &w, int &h);
%ignore fl_chord(int x, int y, int w, int h, double a1, double a2); 
%ignore fl_read_image(uchar *p, int x,int y, int w, int h, int alpha=0);
%ignore fl_draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=3);
%ignore fl_draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=1);
%ignore fl_measure;
//ToDo
%ignore fl_draw_check;

%apply int* OUTPUT { int& };

%include "FL/fl_draw.H"

%rename("") fl_measure; // Undo the %ignore

// Apply INOUT only to the 'x' parameter in fl_measure
%apply int* INOUT { int& x };
FL_EXPORT void fl_measure(const char *str, int &x, int &y, int draw_symbols = 1);

