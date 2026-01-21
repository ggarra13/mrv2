/* File : Fl.i */
//%module Fl

%include "typemaps.i"

// hack to convince SWIG that Fl_Color is something different than it really is!
%apply unsigned int { enum Fl_Color};
%apply const unsigned int& { const enum Fl_Color&};
// end hack

// used for get_color
%apply uchar* OUTPUT { uchar& };

%feature("docstring") ::Fl
"""The Fl class is the FLTK global (static) class containing state 
information and global methods for the current application.""" ;

// The following lines will create getter and setter for the named variables
%rename("e_number", %$isvariable) "e_number"; 
%rename("e_x", %$isvariable) "e_x"; 
%rename("e_y", %$isvariable) "e_y";
%rename("e_x_root", %$isvariable) "e_x_root";
%rename("e_y_root", %$isvariable) "e_y_root";
%rename("e_dx", %$isvariable) "e_dx";
%rename("e_dy", %$isvariable) "e_dy";
%rename("e_state", %$isvariable) "e_state";
%rename("e_clicks", %$isvariable) "e_click";
%rename("e_is_click", %$isvariable) "e_is_click";
%rename("e_keysym", %$isvariable) "e_keysym";


%{
#include "FL/Fl.H"
#include <signal.h>
%}


%ignore Fl::grab(Fl_Window &);

%ignore Fl::has_check(Fl_Timeout_Handler, void* = 0);
%ignore Fl::set_labeltype(Fl_Labeltype, Fl_Labeltype from);
%ignore Fl::add_timeout(double t, Fl_Timeout_Handler,void* = 0);
%ignore Fl::repeat_timeout(double t, Fl_Timeout_Handler,void* = 0);
%ignore Fl::remove_timeout(Fl_Timeout_Handler,void* = 0);
%ignore Fl::add_check(Fl_Timeout_Handler, void* = 0);
%ignore Fl::add_idle(void (*cb)(void*), void* = 0);
%ignore Fl::set_idle(void (*cb)());
%ignore Fl::remove_idle(void (*cb)(void*), void* = 0);
%ignore Fl::get_color(Fl_Color);
%ignore Fl::add_fd;
%ignore Fl::remove_fd;
%ignore Fl::add_handler;
%ignore Fl::remove_handler;
%ignore Fl::remove_check;
%ignore Fl::gl_visual;
%ignore Fl::get_awake_handler_;
%ignore Fl::copy_image;

//%wrapper %{
//#ifdef __APPLE__ 
//  // empty implementation
//  void Fl::free_color(Fl_Color i, int overlay) 
//    {
//    }
//#endif 
//  %}

/* ignore multithreading, but see bottom for actual support */
//%ignore Fl::lock();
//%ignore Fl::unlock();
//%ignore Fl::awake(void* message = 0);
//%ignore Fl::thread_message();

/* missing wrappings */
//%ignore Fl::add_fd;
//%ignore Fl::remove_fd;
/* end missing wrappings */

// font related stuff
//%apply int* OUTPUT { int*& sizep };
%apply int* OUTPUT { int* attributes };
%apply int& OUTPUT { int& X };
%apply int& OUTPUT { int& Y };
%apply int& OUTPUT { int& W };
%apply int& OUTPUT { int& H };

%ignore Fl::get_font_sizes;

%include "FL/Fl.H"

void pyFLTK_controlIdleCallbacks(int enable);

%{
// called by FLTK when idle
PyObject *registeredDoIDle = 0;
void pyFLTK_idleCallback(void *data)
{
        PyObject *arglist;
        PyObject *result;
        arglist = Py_BuildValue("()");       // Build argument list
        result = PyObject_CallObject(registeredDoIDle, arglist);   // Call Python
        Py_DECREF(arglist);                           // Trash arglist
        //result = PyObject_CallObject(func,(PyObject *)0);     // Call Python
        Py_XDECREF(result);
		if (PyErr_Occurred())
		{
			PyErr_Print();
		}
}


// turn on/off idle callback into the Python interpreter	
void pyFLTK_controlIdleCallbacks(int enable)
{
	if (enable)
	{
		Fl::add_idle(pyFLTK_idleCallback, 0);
	}
	else
	{
		Fl::remove_idle(pyFLTK_idleCallback);
	}
}
%}

%native(pyFLTK_registerDoIdle) PyObject * registerDoIdle(PyObject  *self,
	PyObject *args); 

%{
// called by the initialization code in the module - 
// not meant for user consumption
PyObject *registerDoIdle(PyObject *self, PyObject *args) 
{
	PyArg_ParseTuple( args, "O", &registeredDoIDle);
	if (!PyCallable_Check(registeredDoIDle)) 
	{
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
	}
	else
	{
		Py_INCREF(registeredDoIDle);
	}
	Py_INCREF(Py_None);
	return Py_None;
}

%}

//implement time out processing

%native(Fl_add_timeout) PyObject * Fl_add_timeout(PyObject  *self,
	PyObject *args); 
%native(Fl_repeat_timeout) PyObject * Fl_repeat_timeout(PyObject  *self,
	PyObject *args);
%native(Fl_remove_timeout) PyObject * Fl_remove_timeout(PyObject  *self,
	PyObject *args); 


%{


     
#include "CallbackStruct.h"

struct timeout_link {
  CallbackStruct *handle;
  timeout_link *next;
};

static timeout_link *py_timeout_funcs = NULL;


void pyFLTK_timeoutCallback(void *data)
{
  CallbackStruct *cb =  (CallbackStruct *)data;
  PyObject *result;

  if (py_timeout_funcs != NULL) {
    timeout_link *l, *p;
    for (l = py_timeout_funcs, p = 0; l && l->handle != cb; p = l, l = l->next);
    
    if (l) {
      // Found it, so remove it from the list...
      if (p) 
	p->next = l->next;
      else 
	py_timeout_funcs = l->next;
      
      // And free the record...
      delete l;
    }
  }

  // check for NULL arguments
  PyObject *args = NULL;
  if (cb->data) {
    args = Py_BuildValue("(O)", cb->data);
    if (PyErr_Occurred())
      {  
	PyErr_Print();
      }
  }	
  result = PyObject_CallObject(cb->func, args);     // Call Python
  if (PyErr_Occurred())
    {
      PyErr_Print();
    }
  Py_DECREF(cb->func); 
  Py_XDECREF(cb->data);
  Py_XDECREF(args);
  Py_XDECREF(result);
  delete cb;
  if (PyErr_Occurred())
    {
      PyErr_Print();
    }
}

PyObject *Fl_add_timeout(PyObject *self, PyObject *args) 
{
  float numSeconds; 
  PyObject *func = 0;
  PyObject *data = 0;
  
  PyArg_ParseTuple( args, "fO|O", &numSeconds, &func, &data);
  if (!PyCallable_Check(func)) 
    {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    }
  else
    {
      CallbackStruct *cb = new CallbackStruct( func, data, (PyObject*)0 );
      // add reference to list
      timeout_link *t = new timeout_link;
      t->handle = cb;
      t->next = py_timeout_funcs;
      py_timeout_funcs = t;
      // function call
      Py_INCREF(func);
      Py_XINCREF(data);
      Fl::add_timeout( numSeconds, pyFLTK_timeoutCallback, cb);
    }
  
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *Fl_repeat_timeout(PyObject *self, PyObject *args) 
{
  float numSeconds; 
  PyObject *func = 0;
  PyObject *data = 0;
  
  PyArg_ParseTuple( args, "fO|O", &numSeconds, &func, &data);
  if (!PyCallable_Check(func)) 
    {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    }
  else
    {
      CallbackStruct *cb = new CallbackStruct( func, data, (PyObject*)0 );
      // add reference to list
      timeout_link *t = new timeout_link;
      t->handle = cb;
      t->next = py_timeout_funcs;
      py_timeout_funcs = t;
      // function call
      Py_INCREF(func);
      Py_XINCREF(data);
      Fl::repeat_timeout( numSeconds, pyFLTK_timeoutCallback, cb);
    }
  
  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *Fl_remove_timeout(PyObject *self, PyObject *args) 
{
  PyObject *func = 0;
  PyObject *data = 0;
  PyArg_ParseTuple( args, "O|O", &func, &data);
  if (!PyCallable_Check(func)) 
    {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    }
  else
    {
      //printf("Fl_Remove_timeout callable found\n");
      //CallbackStruct *cb = new CallbackStruct( func, data, (PyObject*)0 );
      //Fl::remove_timeout( pyFLTK_timeoutCallback, cb);
      //Py_DECREF(cb->func); 
      //Py_XDECREF(cb->data);

      if (py_timeout_funcs != NULL) 
      {
	timeout_link *l, *p;

	l = py_timeout_funcs;
	p = 0;
	while (l != NULL)
	{
	  // compare the functions
	  if( PyObject_RichCompareBool( l->handle->func, func, Py_EQ ) == 1 )
	    {
	      // if needed, also compare data
	      if (!(data == NULL || ( PyObject_RichCompareBool( l->handle->data, data, Py_EQ ) == 1)))
		{
		  //printf("Fl_Remove_timeout incomplete match\n");
		  p = l;
		  l = l->next;
		  continue;
		}
	      //printf("Removing: l = %xd, handle = %xd; func = %xd; data = %xd\n", l, l->handle, l->handle->func, l->handle->data);
	      // Found it, so remove it from the list...
	      if (p) 
		p->next = l->next;
	      else {
		py_timeout_funcs = l->next;
	      }
	      Fl::remove_timeout( pyFLTK_timeoutCallback, l->handle);
	      Py_DECREF(l->handle->func);
	      Py_XDECREF(l->handle->data);

	      // And free the record...
	      l->handle = NULL;
	      l->next = NULL;
	      delete l;
	      l = NULL;
	    }
	  if (l) {
	    p = l;
	    l = l->next;
	  }
	  else {
	    l = py_timeout_funcs;
	    p = 0;
	  }
	}
      }
    }
  Py_INCREF(Py_None);
  return Py_None;
}

%}

// implement file descriptors
%native(Fl_add_fd) PyObject * Fl_add_fd(PyObject *self, PyObject *args);
%native(Fl_remove_fd) PyObject * Fl_remove_fd(PyObject *self, PyObject *args);

%{

struct fd_link {
       int fd;
       PyObject *func;
       PyObject *data;
       fd_link *next;
};

static fd_link *py_fd_funcs = NULL;

void pyFLTK_fdCallback(int fd, void* data) {
     // check for NULL arguments
     PyObject *args = NULL;

     // loop through Python handler
     for (const fd_link *hl = py_fd_funcs; hl; hl = hl->next) {
       if (hl->fd == fd) {
         if (hl->data == NULL)
     	   args = Py_BuildValue("(i)", fd);
         else
           args = Py_BuildValue("(iO)", fd, hl->data);
         PyObject_CallObject(hl->func, args);     // Call Python
         if (PyErr_Occurred())
	 {
	   PyErr_Print();
	 }
         break;
       }	 
     }

     Py_XDECREF(args);
   }

PyObject *Fl_add_fd(PyObject *self, PyObject *args) 
   {
     int fd;
     int when;
     bool long_signature = true;
     PyObject *func = 0;
     PyObject *data = 0;

     PyArg_ParseTuple( args, "iiO|O", &fd, &when, &func, &data);
     if (PyErr_Occurred())
     {
       PyErr_Clear();
       PyArg_ParseTuple( args, "iO|O", &fd, &func, &data);
       long_signature = false;
       if (PyErr_Occurred())
       {
         PyErr_Print();
       }
     }
     if (!PyCallable_Check(func)) 
       {
	       PyErr_SetString(PyExc_TypeError, "Need a callable object!");
       }
     else
       {
         fd_link *cb = py_fd_funcs;
	 while (cb != NULL) {
	   if (cb->fd == fd) {
	     Py_INCREF(cb->func);
	     Py_XINCREF(cb->data);
	     cb->func = func;
	     cb->data = data;
	     break;
	   }
	   cb = cb->next;
	 }
	 if (cb == NULL) {
	   // only add one callback, actual callback list is kept in Python
     if (long_signature)
	     Fl::add_fd(fd, when, (Fl_FD_Handler)pyFLTK_fdCallback);
     else
       Fl::add_fd(fd, (Fl_FD_Handler)pyFLTK_fdCallback);

	   // add the python callback
	   fd_link *cb = new fd_link;
	   cb->next = py_fd_funcs;
	   cb->fd = fd;
	   cb->func = func;
	   cb->data = data;
	   py_fd_funcs = cb;
	   Py_INCREF(func);
	   Py_XINCREF(data);
	 }
       }

     Py_INCREF(Py_None);
     return Py_None;
   }

PyObject * Fl_remove_fd(PyObject *self, PyObject *args) 
   {
     int fd;
     PyArg_ParseTuple( args, "i", &fd);
     if (PyErr_Occurred())
     {
       PyErr_Print();
     }

     if (py_fd_funcs != NULL) {
       fd_link *l, *p;

       // Search for the handler in the list...
       for (l = py_fd_funcs, p = 0; l && l->fd != fd; p = l, l = l->next);

       if (l) {
	 // Found it, so remove it from the list...
	 if (p) 
	   p->next = l->next;
	 else 
	   py_fd_funcs = l->next;

	 // reference count
	 Py_DECREF(l->func);
	 Py_XDECREF(l->data);

	 // And free the record...
	 delete l;

	 // remove the fltk handler to avoid memory leaks
	 Fl::remove_fd(fd);
       }
     }
     Py_INCREF(Py_None);
     return Py_None;
   }
%}

// implement global handler
%native(Fl_add_handler) PyObject * Fl_add_handler(PyObject  *self,
	PyObject *args);

%native(Fl_remove_handler) PyObject * Fl_remove_handler(PyObject  *self,
	PyObject *args); 

%{

struct handler_link {
  //int (*handle)(int);
  PyObject *handle;
  handler_link *next;
};

static handler_link *py_handler_funcs = NULL;

int pyFLTK_handlerCallback(int data)
   {
     PyObject *result = NULL;

     // check for NULL arguments
     PyObject *args = NULL;
     args = Py_BuildValue("(i)", data);
     if (PyErr_Occurred())
       {  
	 PyErr_Print();
       }

     // loop through Python handler
     int c_result = 0;
     for (const handler_link *hl = py_handler_funcs; hl; hl = hl->next) {
       result = PyObject_CallObject(hl->handle, args);     // Call Python
       if (PyErr_Occurred())
	 {
	   PyErr_Print();
	 }

       
       if (result){
	 SWIG_AsVal_int(result, &c_result);
       }
       if (c_result) break;
     }

     Py_XDECREF(args);
     Py_XDECREF(result);

     return c_result;
   }

PyObject *Fl_add_handler(PyObject *self, PyObject *args) 
   {
     PyObject *func = 0;

     PyArg_ParseTuple( args, "O", &func);
     if (!PyCallable_Check(func)) 
       {
	 PyErr_SetString(PyExc_TypeError, "Need a callable object!");
       }
     else
       {
	 if (py_handler_funcs == NULL) {
	   // only add one callback, actual callback list is kept in Python
	   Fl::add_handler(pyFLTK_handlerCallback);
	 }
	 // add the python callback
	 handler_link *cb = new handler_link;
	 cb->next = py_handler_funcs;
	 cb->handle = func;
	 py_handler_funcs = cb;
	 Py_INCREF(func);
       }

     Py_INCREF(Py_None);
     return Py_None;
   }

PyObject * Fl_remove_handler(PyObject *self, PyObject *args) 
   {
     PyObject *func = 0;
     PyArg_ParseTuple( args, "O", &func);

     if (py_handler_funcs != NULL) {
       handler_link *l, *p;

       // Search for the handler in the list...
       for (l = py_handler_funcs, p = 0; l && l->handle != func; p = l, l = l->next);

       if (l) {
	 // Found it, so remove it from the list...
	 if (p) 
	   p->next = l->next;
	 else 
	   py_handler_funcs = l->next;

	 // reference count
	 Py_DECREF(l->handle);

	 // And free the record...
	 delete l;
       }
       // was this the last one?
       if (py_handler_funcs == NULL) {
	 // remove the fltk handler to avoid memory leaks
	 Fl::remove_handler(pyFLTK_handlerCallback);
       }
     }
     Py_INCREF(Py_None);
     return Py_None;
   }




 %}

// implement global check
%native(Fl_add_check) PyObject * Fl_add_check(PyObject  *self,
	PyObject *args);

%native(Fl_remove_check) PyObject * Fl_remove_check(PyObject  *self,
	PyObject *args); 

%{

struct check_link {
  //int (*handle)(int);
  PyObject *handle;
  PyObject *args;
  check_link *next;
};

static check_link *py_check_funcs = NULL;

void pyFLTK_checkCallback(void *data)
   {
     PyObject *result = NULL;

     

     // loop through Python handler
     for (const check_link *hl = py_check_funcs; hl; hl = hl->next) {
       // check for NULL arguments
       PyObject *args = NULL;
       if (hl->args) {
	 args = Py_BuildValue("(O)", hl->args);
       }
       else {
       	 args = Py_BuildValue("()");
       }
       if (PyErr_Occurred()) {  
	   PyErr_Print();
       }
       result = PyObject_CallObject(hl->handle, args);     // Call Python
       if (PyErr_Occurred())
	 {
	   PyErr_Print();
	 }
       Py_XDECREF(args);
       Py_XDECREF(result);
     }
   }

PyObject *Fl_add_check(PyObject *self, PyObject *args) 
   {
     PyObject *func = 0;
     PyObject *data = 0;

     PyArg_ParseTuple( args, "O|O", &func, &data);
     if (!PyCallable_Check(func)) 
       {
	 PyErr_SetString(PyExc_TypeError, "Fl.add_check: need a callable object!");
       }
     else
       {
	 if (py_check_funcs == NULL) {
	   // only add one callback, actual callback list is kept in Python
	   Fl::add_check(pyFLTK_checkCallback);
	 }
	 // add tye python callback
	 check_link *cb = new check_link;
	 cb->next = py_check_funcs;
	 cb->handle = func;
	 cb->args = data;
	 py_check_funcs = cb;
	 Py_INCREF(func);
	 Py_XINCREF(data);
       }

     Py_INCREF(Py_None);
     return Py_None;
   }

PyObject * Fl_remove_check(PyObject *self, PyObject *args) 
   {
     PyObject *func = 0;
     PyObject *data = 0;
     PyArg_ParseTuple( args, "O|0", &func, &data);

     if (py_check_funcs != NULL) {
       check_link *l, *p;

       // Search for the handler in the list...
       for (l = py_check_funcs, p = 0; l && l->handle != func; p = l, l = l->next);

       if (l) {
	 // Found it, so remove it from the list...
	 if (p) 
	   p->next = l->next;
	 else 
	   py_check_funcs = l->next;

	 // reference count
	 Py_DECREF(l->handle);
	 Py_XDECREF(l->args);

	 // And free the record...
	 delete l;
       }
       // was this the last one?
       if (py_check_funcs == NULL) {
	 // remove the fltk handler to avoid memory leaks
	 Fl::remove_check(pyFLTK_checkCallback);
       }
     }
     Py_INCREF(Py_None);
     return Py_None;
   }




 %}

// clean up typemaps
%clear uchar&;
%clear int& X;
%clear int& Y;
%clear int& W;
%clear int& H;

 %{


 %}

%extend Fl {
   #include <signal.h>
#define BARF(fmt, ...) PyErr_Format(PyExc_RuntimeError, "%s:%d %s(): " fmt, __FILE__, __LINE__, __func__)
// Python is silly. There's some nuance about signal handling where it sets a
// SIGINT (ctrl-c) handler to just set a flag, and the python layer then reads
// this flag and does the thing. Here I'm running C code, so SIGINT would set a
// flag, but not quit, so I can't interrupt the capture. Thus I reset the SIGINT
// handler to the default, and put it back to the python-specific version when
// I'm done
#define SET_SIGINT() 		\
    struct sigaction sigaction_new = {};                                \
    sigaction_new.sa_handler = SIG_DFL;                                 \
    if( 0 != sigaction(SIGINT,                                          \
                       &sigaction_new,                                  \
                       &sigaction_old) )                                \
    {                                                                   \
        BARF("sigaction() failed");                                     \
    }                                                                   
   
   #define RESET_SIGINT()                                            \
    if( 0 != sigaction(SIGINT,                                          \
                       &sigaction_old, NULL ))                          \
        BARF("sigaction-restore failed"); 

   static int run_with_default_sigint_handler()
   {
     int result = 1;
#if defined(LINUX)
     struct sigaction sigaction_old = {};
       SET_SIGINT();
#endif
     result = Fl::run();
#if defined(LINUX)
       RESET_SIGINT();
#endif
     return result;
   }

   static int wait_with_default_sigint_handler(double time)
   {
     int result = 1;
#if defined(LINUX)
     struct sigaction sigaction_old = {};
       SET_SIGINT();
#endif
     result = Fl::wait(time);
#if defined(LINUX)
       RESET_SIGINT();
#endif
     return result;
   }
   
  static void fltk_exit()
  {
    while(Fl::first_window())
      {
	Fl::first_window()->hide();
	Fl::check();
      }
  }
}

%pythoncode %{
def Fl_mt_run(window):
    import time
    while window.visible():
        time.sleep(0.1)
        Fl.check()

_fltk.mt_run = staticmethod(Fl_mt_run)
%}

%native(Fl_get_font_sizes_tmp) PyObject * Fl_get_font_sizes_tmp(PyObject  *self,
	PyObject *args); 

%{	
PyObject *Fl_get_font_sizes_tmp(PyObject *self, PyObject *args) 
{
  int fontName; 
  
  PyArg_ParseTuple( args, "i", &fontName);

  int *fontSizes = NULL;
  int size = Fl::get_font_sizes(fontName, fontSizes);

  PyObject *result = PyList_New(size);
  for (int i = 0; i < size; i++) {
    PyObject *o = PyLong_FromLong(fontSizes[i]);
    PyList_SetItem(result,i,o);
  }

  Py_INCREF(result);
  return result;
}
 %}


