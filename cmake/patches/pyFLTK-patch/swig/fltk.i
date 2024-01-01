/* File : fltk.i */
%define DOCSTRING
"pyFltk, the Python bindings to the FLTK GUI toolkit.
This is an easy to use and light-weight GUI toolkit
offering basic capabilities for the creation of
graphical user interfaces."
%enddef

%module(docstring=DOCSTRING, directors="1", package="fltk14._fltk14") fltk14


%feature("director");

%feature("nodirector") Fl_Valuator;

%feature("compactdefaultargs");

// print Python error message for director exceptions
//%feature("director:except") {
//    if ($error != NULL) {
//        PyErr_Print();
//    }
//}

%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}

%exception {
    try { $action }
    catch (Swig::DirectorException &e) { SWIG_fail; }
}


// ignore all variables -> no getters and setters
%rename("$ignore",%$isvariable) ""; 

%feature("autodoc", "1");


%{
// Ugly and potentially dangerous hack to enable compiling
// extensions with Python2.4 and later using MinGW. Library
// msvcr71 does not declare the below symbol anymore, and 
// msvcrt should not be linked with!
#ifdef __MINGW32_VERSION
short ** _imp___ctype = 0;
#endif
%}

%wrapper %{
// Patch 1767434, interactive pyFltk
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
#include <conio.h>
#else
static void _exit_wait_loop(int fd, void* data)
{
    int* stdin_ready = (int*)data;
    *stdin_ready = 1;
}
#endif

static int _run_loop_interactive(void)
{
  PyGILState_STATE gstate;
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
  gstate = PyGILState_Ensure();
  while (! _kbhit()) Fl::wait(0.1);
  PyGILState_Release(gstate);
#else
  int result;
  int stdin_ready = 0;
  const int fd = fileno(stdin);
  Fl::add_fd(fd, _exit_wait_loop, &stdin_ready);
  gstate = PyGILState_Ensure();
  while (!stdin_ready)
  {
      result = Fl::wait();
      if (!result) break;
  }
  PyGILState_Release(gstate);
  Fl::remove_fd(fd);
#endif
  return 0;
}
// End of patch 1767434, interactive pyFltk
%}

// To be put in SWIG_init
%init %{
// Patch 1767434, interactive pyFltk
if (PyOS_InputHook==NULL) 
  PyOS_InputHook = _run_loop_interactive;
else 
  PyErr_Warn(PyExc_RuntimeWarning, "PyOS_InputHook is not available for interactive use of pyFltk");
// End of patch 1767434, interactive pyFltk

// 
%}

%include "typemaps.i"
//Python3
#ifdef PYTHON3
%typemap(freearg) const char* {
  //printf("NewFreeArgTypemap");
}
%pythoncode 
{
import sys
if sys.version < '3':
  print("Python3 required!")
  exit(1)


sys_exit_original = sys.exit

def pyfltk_sys_exit(status = None):
    Fl.fltk_exit()
    return sys_exit_original(status)

sys.exit = pyfltk_sys_exit

sys.exit.__doc__ = \
r'''This is a sys.exit hooked by pyfltk. Discussion:

https://sourceforge.net/p/pyfltk/mailman/pyfltk-user/thread/87fsxgj3sh.fsf%40secretsauce.net/#msg37304779

 ''' + sys_exit_original.__doc__
	 }
#else
%pythoncode
{
import sys
if sys.version > '3':
  print("Python2 required!")
  exit(1)
}
#endif

// UTF8 typemap

 %typemap(in) Fl_CString %{
  {
  PyObject *tmp_obj = PyUnicode_FromEncodedObject($input, "utf-8", NULL);
  if (tmp_obj) {
    int buf_len = 0;
    char *buffer = 0;
    int result = SWIG_AsCharPtrAndSize(tmp_obj, &buffer, NULL, &buf_len);
    $1 = reinterpret_cast< Fl_CString >(buffer);
  }
  }
  %}

//#ifndef WIN32
#define FL_EXPORT
//#endif

%include fl_attr.i
%include fl_types.i
%include Enumerations.i

%include Fl_Image.i

%include Fl_Widget.i
%include Fl_Widget_Surface.i
%include Fl_Group.i
%include Fl_Browser_.i
%include Fl_Browser.i

%include Fl_File_Browser.i
%include Fl_File_Icon.i

%include Fl_File_Chooser.i

%include Fl.i

 //ToDo %include dirent.i
%include Enumerations.i
%include filename.i

// widgets
%include Fl_Valuator.i
%include Fl_Adjuster.i
%include fl_ask.i
%include Fl_Image.i
%include Fl_Bitmap.i
%include Fl_BMP_Image.i
%include Fl_Box.i
%include Fl_Button.i
%include Fl_Shortcut_Button.i
%include Fl_Chart.i
%include Fl_Check_Browser.i
%include Fl_Light_Button.i
%include Fl_Check_Button.i
%include Fl_Menu_.i
%include Fl_Menu.i
%include Fl_Choice.i
 //%include Fl_Copy_Surface.i  // @bug: linker errors on Windows
%include Fl_Clock.i
%include Fl_Value_Input.i
%include Fl_Color_Chooser.i
%include Fl_Counter.i
%include Fl_Dial.i
%include Fl_Window.i
%include Fl_Double_Window.i
%include fl_draw.i
%include Fl_Input_.i
%include Fl_Input.i
%include Fl_File_Input.i
%include Fl_Fill_Dial.i
%include Fl_Slider.i
%include Fl_Fill_Slider.i
%include Fl_Flex.i
%include Fl_Float_Input.i
%include Fl_FormsBitmap.i
%include Fl_FormsPixmap.i
%include Fl_Free.i
%include Fl_Pixmap.i
 //TODO %include Fl_GIF_Image.i
%include Fl_Anim_GIF_Image.i
%include Fl_ICO_Image.i
 //%include Fl_Image_Surface.i  // @bug: linker errors on Windows
 //%include Fl_Int_Vector.i
 //%include Fl_String.i
 //%include fl_string_functions.i
#ifndef DO_NOT_USE_OPENGL
%include Fl_Gl_Window.i
#endif
%include Fl_Help_Dialog.i
%include Fl_Help_View.i
%include Fl_Hold_Browser.i
%include Fl_Hor_Fill_Slider.i
%include Fl_Hor_Nice_Slider.i
%include Fl_Hor_Slider.i
%include Fl_Value_Slider.i
%include Fl_Hor_Value_Slider.i
%include Fl_Int_Input.i
%include Fl_Input_Choice.i
%include Fl_JPEG_Image.i
%include Fl_Line_Dial.i
%include Fl_Menu_Bar.i
%include Fl_Menu_Button.i
%include Fl_Menu_Item.i
%include Fl_Single_Window.i
%include Fl_Menu_Window.i
%include fl_message.i
%include Fl_Multiline_Input.i
%include Fl_Output.i
%include Fl_Multiline_Output.i
%include Fl_Multi_Browser.i
%include Fl_Multi_Label.i
%include Fl_Nice_Slider.i
 //TODO %include Fl_Native_File_Chooser.i
%include Fl_Object.i
%include Fl_Overlay_Window.i
%include Fl_Pack.i
%include Fl_PNG_Image.i
%include Fl_PNM_Image.i
%include Fl_Positioner.i
%include Fl_Preferences.i
%include Fl_Progress.i
%include Fl_Radio_Button.i
%include Fl_Radio_Light_Button.i
%include Fl_Round_Button.i
%include Fl_Radio_Round_Button.i
%include Fl_Repeat_Button.i
%include Fl_Return_Button.i
%include Fl_Rect.i
%include Fl_RGB_Image.i
%include Fl_SVG_Image.i
%include Fl_SVG_File_Surface.i
%include Fl_Roller.i
%include Fl_Round_Clock.i
%include Fl_Scroll.i
 //%include Fl_Scheme.i         // @bug:
 //%include Fl_Scheme_Choice.i  // results in link error on Windows
%include Fl_Scrollbar.i
%include Fl_Secret_Input.i
%include Fl_Select_Browser.i
%include Fl_Shared_Image.i
%include Fl_Spinner.i
%include fl_show_colormap.i
%include fl_show_input.i
%include Fl_Simple_Counter.i
//%include Fl_Simple_Terminal.i
//%include Fl_Sys_Menu_Bar.i
%include Fl_Tabs.i
%include Fl_Text_Buffer.i
%include Fl_Text_Display.i
%include Fl_Text_Editor.i
%include Fl_Tile.i
%include Fl_Tiled_Image.i
%include Fl_Timer.i
%include Fl_Toggle_Button.i
%include Fl_Toggle_Light_Button.i
%include Fl_Toggle_Round_Button.i
%include Fl_Tooltip.i
%include Fl_Value_Output.i
%include Fl_Wizard.i
%include Fl_XBM_Image.i
%include Fl_XPM_Image.i
%include x.i

// tree support
%include Fl_Tree_Prefs.i
%include Fl_Tree_Item_Array.i
%include Fl_Tree_Item.i
%include Fl_Tree.i

// printer support
//%include Fl_Plugin.i
//%include Fl_Device.i
%include Fl_Paged_Device.i
//%include Fl_PostScript.i
%include Fl_Printer.i

 // utf8
 %include fl_utf8.i

// contributions
%include ListSelect.i
 //ToDo %include Fl_Table.i
 //ToDo %include Fl_Table_Row.i
//%include Fl_Thread.i

// misc
//%include forms.i
%include gl.i
//%include gl2opengl.i
//%include glut.i
//%include gl_draw.i

%include math.i
%include setMenu.i
//%include widget_casts.i
%include py_idle.i

// user defined widgets
%include UserDefinedWidgets.i


// polymorphism patches (python code)
%include pyFinalize.i
