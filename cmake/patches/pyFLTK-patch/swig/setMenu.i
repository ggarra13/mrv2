#ifdef PYTHON
/* File : setMenu.i */
//%module setMenu

%native(setMenu) PyObject *setMenu(PyObject *self, PyObject *args);



%{

Fl_Menu_Item *createFl_Menu_Item_Array(PyObject *self, PyObject *pyMenuList)
{
	Py_ssize_t numItems = PyTuple_Size(pyMenuList);
	//if a list (mutable) and not a tuple (immutable) is passed,
	// 'numItems' == -1, so if so, assume a list and covert it to a tuple
	if (PyList_Check(pyMenuList))
	{
		//try a list
		pyMenuList = PyList_AsTuple(pyMenuList);
		numItems = PyTuple_Size(pyMenuList);
	}
	Fl_Menu_Item *arrayOfFMI = 0;
	arrayOfFMI = new Fl_Menu_Item[numItems+1];
	int index=0;
	for (; index<numItems; index++)
	{
		PyObject *nextItem = PyTuple_GetItem( pyMenuList, index );
		char *pyText=0;
		int shortcut=0;
		PyObject *callback=0;
		PyObject *userData=0;
		int flags=0;
		unsigned char labelType=0;
		unsigned char labelFont=0;
		unsigned char labelSize=0;
		unsigned char labelColor=0;
		int ok = PyArg_ParseTuple( nextItem, "z|iOOibbbb",
			&pyText, &shortcut, &callback, &userData, &flags,
			&labelType, &labelFont, &labelSize, &labelColor);

		Fl_Menu_Item *p = arrayOfFMI+index;
		if (ok)
		{
			//have all the components, now set the values....

			//// Yes, this is a memory leak
			//// I don't know away around it, since strings from
			//// the scripting language are all dynamically allocated
			if ( !pyText )
			{
				p->text = 0;
			}
			else
			{
					p->text = strdup(pyText);
			}

			p->shortcut_ = shortcut;

			if (callback && PyCallable_Check(callback))
			{
				CallbackStruct *cb = new CallbackStruct( callback, userData, SWIGTYPE_p_Fl_Menu_Item );
				Py_INCREF(callback);
				Py_XINCREF(userData);
				//self->callback(PythonCallBack, (void *)cb);
				p->callback_ = (Fl_Callback *)Fl_Menu__PythonCallBack;
				p->user_data_ = (void *)cb;
			}
			else
			{
				p->callback_ = (Fl_Callback *)0;
			}


			p->flags = flags;
			p-> labeltype_ = labelType;
			p-> labelfont_ = labelFont;
			p-> labelsize_ = labelSize;
			p-> labelcolor_ = labelColor;
		}
		else
		{
			fprintf(stderr, "Could not convert menu item %d\n", index);
			PyObject_Print(nextItem, stderr, 0);
			fprintf(stderr, "\n");
			p->text = 0;
			delete [] arrayOfFMI;
			return NULL;
		}
	}
	arrayOfFMI[index].text = 0;
	return arrayOfFMI;
}

// this is deprecated
PyObject *setMenu(PyObject *self, PyObject *args)
{
	PyObject *targetObject, *menuList;
	printf("Warning: setMenu is deprecated, use Fl_Menu_.copy() instead!\n");
	if (!PyTuple_Check(args))
	{
		printf("setMenuError: not a tup\n");
		return NULL;
	}

	if (!PyArg_ParseTuple( args, "OO", &targetObject, &menuList))
	{
		printf("no conv args\n");
		return NULL;
	}

	PyObject *thisPtrString = PyObject_GetAttrString( targetObject, "this");
#if PY_VERSION_HEX>=0x03000000
	if (!PyUnicode_Check(thisPtrString))
#else
        if (!PyBytes_Check(thisPtrString))
#endif
	{
			printf( "no get this str\n");
			return NULL;
	}

	Fl_Menu_ *theMenu;
	//char *thisPtrAsCString = PyBytes_AsString(thisPtrString);
	//SWIG_GetPtr( thisPtrAsCString, (void **)&theMenu, "_Fl_Menu_p");
	SWIG_ConvertPtr(thisPtrString, (void **)&theMenu, SWIGTYPE_p_Fl_Menu_, 0);

	Fl_Menu_Item *theMenuItems = createFl_Menu_Item_Array( NULL, menuList);

	// call the C++ object to add the menu
	theMenu->copy( theMenuItems, NULL );

	delete [] theMenuItems;

	Py_INCREF(Py_None);
	return Py_None;

}

%}



#endif
