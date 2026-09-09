/* File : Fl_Tree_Item.i */
//%module Fl_Slider

%feature("docstring") ::Fl_Tree_Item
"""
This class is a single tree item, and manages all of the item's attributes. Fl_Tree_Item is used by Fl_Tree, which is comprised of many instances of Fl_Tree_Item.
Fl_Tree_Item is hierarchical; it dynamically manages an Fl_Tree_Item_Array of children that are themselves instances of Fl_Tree_Item. Each item can have zero or more children. When an item has children, close() and open() can be used to hide or show them. Items have their own attributes; font size, face, color. Items maintain their own hierarchy of children. When you make changes to items, you'll need to tell the tree to redraw() for the changes to show up.
""" ;

%{
#include "FL/Fl_Tree_Item.H"
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Tree_Item)


// typemap to map output of Fl_Button.valueFl_Tree_Item.is_selected from char to int
%typemap(out) char {
    $result = PyLong_FromLong( (long)$1);
}



%ignore Fl_Tree_Item::user_data(void *);
%ignore Fl_Tree_Item::user_data() const;

%include "FL/Fl_Tree_Item.H"

ADD_USERDATA(Fl_Tree_Item)

// clear the typemap for char
%typemap(out) char;
