

%pythoncode %{
# override the implementation of Fl_Widget.callback
# new version to be used with directors
def __Fl_WidgetCallback(self,*args):
        if len(args) == 1:
            new_args = (self, args[0], self)
        else:
            new_args = (self, args[0], self, args[1])
        #return apply(_fltk.Fl_Widget_callback,new_args)
        return _fltk.Fl_Widget_callback(*new_args)

Fl_Widget.callback = __Fl_WidgetCallback
# end of the Fl_Widget.callback wrapper
%}

%pythoncode %{
# override the implementation of Fl_Text_Buffer.add_modify_callback
def __Fl_Text_BufferAddModifyCallback(self,*args):
        if len(args) == 1:
            new_args = (self, args[0])
        else:
            new_args = (self, args[0], args[1])
        #return apply(_fltk.Fl_Text_Buffer_add_modify_callback,new_args)
        return _fltk.Fl_Text_Buffer_add_modify_callback(*new_args)

Fl_Text_Buffer.add_modify_callback = __Fl_Text_BufferAddModifyCallback
# end of the Fl_Text_Buffer.add_modify_callback wrapper
%}

%pythoncode %{
# override the implementation of Fl_Text_Buffer.remove_modify_callback
def __Fl_Text_BufferRemoveModifyCallback(self,*args):
        if len(args) == 1:
            new_args = (self, args[0], self)
        else:
            new_args = (self, args[0], self, args[1])
        #return apply(_fltk.Fl_Text_Buffer_remove_modify_callback,new_args)
        return _fltk.Fl_Text_Buffer_remove_modify_callback(*new_args)

Fl_Text_Buffer.remove_modify_callback = __Fl_Text_BufferRemoveModifyCallback
# end of the Fl_Text_Buffer.remove_modify_callback wrapper
%}

%pythoncode %{
# override the implementation of Fl_File_Chooser.callback
def __Fl_File_ChooserCallback(self,*args):
        if len(args) == 1:
            new_args = (self, args[0], self)
        else:
            new_args = (self, args[0], self, args[1])
        #return apply(_fltk.Fl_File_Chooser_callback,new_args)
        return _fltk.Fl_File_Chooser_callback(*new_args)

Fl_File_Chooser.callback = __Fl_File_ChooserCallback
# end of the Fl_File_Chooser.callback wrapper
%}

%pythoncode %{
# override the implementation of Fl_Help_View.link
def __Fl_Help_ViewLink(self,*args):
        if len(args) == 1:
            new_args = (self, args[0], self)
        else:
            new_args = (self, args[0], self, args[1])
        #return apply(_fltk.Fl_Help_View_link,new_args)
        return _fltk.Fl_Help_View_link(*new_args)

Fl_Help_View.link = __Fl_Help_ViewLink
# end of the Fl_Help_View.link wrapper
%}

%pythoncode %{
_fltk.add_timeout = staticmethod(Fl_add_timeout)
_fltk.repeat_timeout = staticmethod(Fl_repeat_timeout)
_fltk.remove_timeout = staticmethod(Fl_remove_timeout)
_fltk.add_check = staticmethod(Fl_add_check)
_fltk.remove_check = staticmethod(Fl_remove_check)
_fltk.add_handler = staticmethod(Fl_add_handler)
_fltk.remove_handler = staticmethod(Fl_remove_handler)
_fltk.add_fd = staticmethod(Fl_add_fd)
_fltk.remove_fd = staticmethod(Fl_remove_fd)
_fltk.get_font_sizes = staticmethod(Fl_get_font_sizes_tmp)
%}
