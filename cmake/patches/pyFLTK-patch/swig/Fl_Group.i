/* File : Fl_Group.i */
//%module Fl_Group

%feature("docstring") ::Fl_Group
"""
The Fl_Group class is the FLTK container widget. It maintains an array of 
child widgets. These children can themselves be any widget including Fl_Group. 
The most important subclass of Fl_Group  is Fl_Window, however groups can 
also be used to control radio buttons or to enforce resize behavior.
""" ;

%{
#include "FL/Fl_Group.H"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Group)

REVERT_OWNERSHIP(Fl_Group::add)

REVERT_OWNERSHIP(Fl_Group::remove)

%ignore Fl_Group::find(const Fl_Widget*) const;
%ignore Fl_Group::add(Fl_Widget&);
%ignore Fl_Group::remove(Fl_Widget&);
%ignore Fl_Group::resizable(Fl_Widget& o);

%ignore Fl_Group::on_insert(Fl_Widget*, int idx);
%ignore Fl_Group::on_move(int a, int b);
%ignore Fl_Group::on_remove(int idx);

%rename(insert_before) Fl_Group::insert(Fl_Widget& o, Fl_Widget* before);

// needed for getting directors to work
%ignore Fl_Group::array() const;

// typemap for child method
%include <std_string.i>

%typemap(out) Fl_Widget * Fl_Group::child {
  const std::string lookup_typename = std::string("_p_") + std::string(reinterpret_cast<CallbackStruct*>($1->user_data())->type_name);
  //printf("Type = %s\n", lookup_typename.c_str());
  swig_type_info * const outtype = SWIG_TypeQuery(lookup_typename.c_str());
  //if (outtype == 0)
  //  printf("SWIG Type not found\n");
  //else
  //  printf("Swig Type = %s\n", outtype->str);
  $result = SWIG_NewPointerObj(SWIG_as_voidptr($1), outtype, $owner);
}


%include "FL/Fl_Group.H"


