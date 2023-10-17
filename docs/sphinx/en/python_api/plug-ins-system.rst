##############
Plug-in System
##############

Plug-ins
--------

mrv2 supports python plug-ins to add menu entries to the main menus of mrv2 or
even create new entries.
This allows you to add commands and actual classes to mrv2, going farther than
what the Python console allows.

To use the plug-ins, you must define the environment variable::

     MRV2_PYTHON_PLUGINS

with a list of colon (Linux or macOS) or semi-colon (Windows) paths where the
plug-ins reside.

In there, python files (.py) that have this basic structure::

    import mrv2
    from mrv2 import plugin, timeline
  
    class HelloPlugin(plugin.Plugin):
        def __init__(self):
	    super().__init__()
	    pass
	    
        def hello(self):
            print("Hello from a python plugin")

	"""
	This function creates a new Menu called Menu with two entries.
	One called Hello with a divider line that calls the self.hello method
	and one called Play that triggers playback of the current clip.
	"""
        def menus(self):
            menu = { "Menu/Hello" : ( self.hello, "__divider__" ),
                     "Menu/Play"  : timeline.playForwards}
            return menu

You can define more than one plug-in per file.
