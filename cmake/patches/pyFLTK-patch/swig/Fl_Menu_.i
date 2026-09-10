/* File : Fl_Menu_.i */
//%module Fl_Menu_

%feature("docstring") ::Fl_Menu_
"""
All widgets that have a menu in FLTK are subclassed off of this class.
Currently FLTK provides you with Fl_Menu_Button, Fl_Menu_Bar, and Fl_Choice .

The class contains a pointer to an array of structures of type Fl_Menu_Item.
The array may either be supplied directly by the user program, or it may be
'private': a dynamically allocated array managed by the Fl_Menu_.
""" ;

%{
#include "FL/Fl_Menu_.H"
#include <menu_item.h>
%}

%include "macros.i"

CHANGE_OWNERSHIP(Fl_Menu_)


%ignore Fl_Menu_::global();
%ignore Fl_Menu_::menu(const Fl_Menu_Item *m);
//%ignore Fl_Menu_::menu();
%ignore Fl_Menu_::add(const char* label, const char* shortcut, Fl_Callback*, void *user_data, int flags);
%ignore Fl_Menu_::menu(const Fl_Menu_Item*);
%ignore Fl_Menu_::copy(const Fl_Menu_Item*);
%ignore Fl_Menu_::value(const Fl_Menu_Item*);

DEFINE_CALLBACK(Fl_Menu_)

%include "FL/Fl_Menu_.H"


%extend Fl_Menu_ {

	void copy(PyObject *args, PyObject *user_data = 0) {
		PyObject *menuList;
		if (!PyTuple_Check(args))
		{
			printf("Fl_Menu_.menu: not a tuple\n");
			return;
		}

		menuList = args;
		Fl_Menu_Item *theMenuItems = createFl_Menu_Item_Array( NULL, menuList);

		// call the C++ object to add the menu
		self->copy(theMenuItems, user_data);

		delete [] theMenuItems;

	}

	void menu(PyObject *args) {
		PyObject *menuList;
		if (!PyTuple_Check(args))
		{
			printf("Fl_Menu_.menu: not a tuple\n");
			return;
		}

		menuList = args;
		Fl_Menu_Item *theMenuItems = createFl_Menu_Item_Array( NULL, menuList);

		// call the C++ object to add the menu
		self->copy(theMenuItems);

		delete [] theMenuItems;

	}

	int add(PyObject *lObj, PyObject *sObj, PyObject *cObj, PyObject* uObj=0, PyObject* fObj=0) {

		//char *pyLabel=PyBytes_AsString(lObj);
                const char* pyLabel = PyUnicode_AsUTF8(lObj);

		int shortcut=PyLong_AsLong(sObj);
		PyObject *callback=cObj;
		PyObject *userData=uObj;
		int flags=0;
		if (fObj)
		   flags=PyLong_AsLong(fObj);
		Fl_Callback *callback_=(Fl_Callback*)0;
		void *user_data_=0;


		// got all the values
		if (callback && PyCallable_Check(callback))
		{
			CallbackStruct *cb = new CallbackStruct( callback, userData, SWIGTYPE_p_Fl_Menu_Item  );
			Py_INCREF(callback);
			//self->callback(PythonCallBack, (void *)cb);
			callback_ = (Fl_Callback *)Fl_Menu__PythonCallBack;
			user_data_ = (void *)cb;
		}

		return self->add(pyLabel, shortcut, callback_, user_data_, flags);
	}

	PyObject* menu() {
	    // returns the Fl_Menu_Item structure of the menu
	    // as a Python list
	    const Fl_Menu_Item* items = self->menu();
	    int length = self->size();
	    PyObject *result = PyList_New(length);
	    for (int i = 0; i < length; i++) {
		PyObject *o = Py_None;
		if (items[i].text != NULL) {
		    if (items[i].callback_) {
			// registered callback,
			// caution, callback information is in user_data
			CallbackStruct *cb = (CallbackStruct*)items[i].user_data_;
			o = Py_BuildValue("zlOOl", items[i].text, items[i].shortcut_, cb->func, cb->data, items[i].flags);
		    }
		    else {
			// no callback, return Py_None
			o = Py_BuildValue("zlOOl", items[i].text, items[i].shortcut_, Py_None, Py_None, items[i].flags);
		    }
		}
		else {
		    // empty item
		    o = Py_BuildValue("OOOOO", Py_None, Py_None, Py_None, Py_None, Py_None);
		}
		PyList_SetItem(result, i, o);
	    }
	    return result;
	}
}
