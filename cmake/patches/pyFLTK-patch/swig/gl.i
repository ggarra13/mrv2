/* File : gl.i */
//%module gl

%{
#ifndef FL_gl_H
#define FL_gl_H

void gl_start();
void gl_finish();

void gl_color(Fl_Color);
void gl_color(int c);

void gl_rect(int x,int y,int w,int h);
void gl_rectf(int x,int y,int w,int h);

void gl_font(int fontid, int size);
int  gl_height();
int  gl_descent();
double gl_width(const char *);
double gl_width(const char *, int n);
double gl_width(uchar);

void gl_draw(const char*);
void gl_draw(const char*, int n);
void gl_draw(const char*, int x, int y);
void gl_draw(const char*, float x, float y);
void gl_draw(const char*, int n, int x, int y);
void gl_draw(const char*, int n, float x, float y);
void gl_draw(const char*, int x, int y, int w, int h, Fl_Align);
void gl_measure(const char*, int& x, int& y);

void gl_draw_image(const uchar *, int x,int y,int w,int h, int d=3, int ld=0);

//from opengl for testing
void  glLoadIdentity( void );
void  glViewport( int x, int y, int width, int height );
void  glClear( int mask );
void  glColor3f( float red, float green, float blue );
void  glBegin( int mode );
void  glEnd( void );
void  glVertex3f( float x, float y, float z );
#endif
%}

%ignore gl_color(Fl_Color);

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

%#ifndef DO_NOT_USE_OPENGL
%include "FL/gl.h"
%#else
%#define FL_gl_H
%#endif

%ignore gl_rectf;
%rename(gl_rectf) cond_gl_rectf;
void cond_gl_rectf(int x,int y,int w,int h)
{
%#ifdef DO_NOT_USE_OPENGL
      fprintf(stderr, "Warning: gl_rectf not supported!\n");
%#else
   glRecti(x,y,x+w,y+h);
%#endif
}

//from opengl for testing
void  glLoadIdentity( void );
void  glViewport( int x, int y, int width, int height );
void  glClear( int mask );
void  glColor3f( float red, float green, float blue );
void  glBegin( int mode );
void  glEnd( void );
void  glVertex3f( float x, float y, float z );
