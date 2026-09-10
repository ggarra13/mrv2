/* File : Fl_File_Browser.i */
//%module Fl_File_Browser

%feature("docstring") ::Fl_File_Browser
"""
The Fl_File_Browser widget displays a list of filenames, optionally with
file-specific icons.
""" ;

%{
#ifdef ANY
#undef ANY
#endif

#include "FL/Fl_File_Browser.H"

enum DIR_SEARCH {
    FL_ALPHASORT,
    FL_CASEALPHASORT,
    FL_CASENUMERICSORT,
    FL_NUMERICSORT
  };
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_File_Browser)

%ignore Fl_File_Browser::load(const char* directory, Fl_File_Sort_F* sort);

%include "FL/Fl_File_Browser.H"

//%inline %{
  enum DIR_SEARCH {
    FL_ALPHASORT,
    FL_CASEALPHASORT,
    FL_CASENUMERICSORT,
    FL_NUMERICSORT
  };
//  %}

%extend Fl_File_Browser {
  %rename(load) load_new;

  int load_new(PyObject* dObj, PyObject* sObj) {
    //char *directory=SWIG_Python_str_AsChar(dObj);
    const char *directory=PyUnicode_AsUTF8(dObj);
    int sort=PyLong_AsLong(sObj);
    int result = -1;

    switch (sort) {
    case FL_ALPHASORT:
      result = self->load(directory, fl_alphasort);
      break;
    case FL_CASEALPHASORT:
      result = self->load(directory, fl_casealphasort);
      break;
    case FL_CASENUMERICSORT:
      result = self->load(directory, fl_casenumericsort);
      break;
    case FL_NUMERICSORT:
      result = self->load(directory, fl_numericsort);
      break;
    default:
      break;
    }

    return result;
  }
}
