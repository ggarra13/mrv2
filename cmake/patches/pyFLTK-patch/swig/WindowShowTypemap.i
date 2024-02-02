// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
  // Check if is a list
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
%#if PY_VERSION_HEX>=0x03000000
      if (PyUnicode_Check(o))
%#else  
      if (PyString_Check(o))
%#endif
	//$1[i] = PyString_AsString(PyList_GetItem($input,i));
        $1[i] = const_cast<char*>(PyUnicode_AsUTF8(PyList_GetItem($input,i)));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}


%define MACRO_WINDOW_SHOW
void show(PyObject *count = 0, PyObject *data = 0)
{
  Py_XINCREF(count);
  Py_XINCREF(data);
  if (!count)
    self->show();
  else if (!data) {
    if (PyList_Check(count)) {
      int size = PyList_Size(count);
      int i = 0;
      char** tmp = (char **) malloc((size+1)*sizeof(char *));
      for (i = 0; i < size; i++) {
          PyObject *o = PyList_GetItem(count,i);
%#if PY_VERSION_HEX>=0x03000000
        if (PyUnicode_Check(o))
%#else  
        if (PyString_Check(o))
%#endif
     tmp[i] = const_cast<char*>(PyUnicode_AsUTF8(PyList_GetItem(count,i)));
	  //tmp[i] = PyString_AsString(PyList_GetItem(count,i));
	else {
	  PyErr_SetString(PyExc_TypeError,"list must contain strings");
	  free(tmp);
	  return;
	}
      }
      tmp[i] = 0;
      self->show();
      free(tmp);
    }
  }
  else {
    if (PyList_Check(data)) {
      int size = PyInt_AsLong(count);
      int i = 0;
      char** tmp = (char **) malloc((size+1)*sizeof(char *));
      for (i = 0; i < size; i++) {
	PyObject *o = PyList_GetItem(data,i);
%#if PY_VERSION_HEX>=0x03000000
        if (PyUnicode_Check(o))
%#else  
        if (PyString_Check(o))
%#endif
     tmp[i] = const_cast<char*>(PyUnicode_AsUTF8(PyList_GetItem(data,i)));
	  //tmp[i] = PyString_AsString(PyList_GetItem(data,i));
	else {
	  PyErr_SetString(PyExc_TypeError,"list must contain strings");
	  free(tmp);
	  return;
	}
      }
      tmp[i] = 0;
      self->show();
      free(tmp);
    } else {
      PyErr_SetString(PyExc_TypeError,"not a list");
      self->show();
    }
		
  }
}
%enddef
