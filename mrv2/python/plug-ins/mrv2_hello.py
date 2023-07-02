# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# For this example, we will just use the timeline module.
#
import mrv2
from mrv2 import timeline, plugin

#
# Hello Plugin class.  Must derive from plugin.Plugin
#
class HelloPlugin(plugin.Plugin):
    """
    Constructor.  Init your variables here.
    """
    def __init__(self):
        super().__init__()
        pass

    """
    Status of the plug-in.  Must be True of False.
    Change this to False to not activate the plug-in.
    If active is missing from the class, it is assumed to be True.
    """
    def active(self):
        return True
    
    """
    The actual plugin execution for Python/Hello.
    """
    def run(self):
        print("Hello from python plugin!")

        
    """
    Return the menus to add to mrv2's main toolbar.
    You can add your commands to mrv2 menus or create a new
    menu entry. Each Submenu is separated by '/'.
    Currently, you cannot remove a menu entry.

    Returns:
    dict: a dictionary of key for menu entries and values as methods.
    """
    def menus(self):
        menus = {
            "Python/Hello" : self.run,
        }
        return menus
        

#
# Playback Plugin class. Must derive from plugin.Plugin
#
class PlaybackPlugin(plugin.Plugin):
    """
    Constructor.  Init your variables here.
    """
    def __init__(self):
        super().__init__()
        pass

    """
    Status of the plug-in.  Must be True of False.
    Change this to False to not activate the plug-in.
    If active is missing from the class, it is assumed to be True.
    """
    def active(self):
        return True

    """
    The actual plugin execution for Python/Playback/Forwards.
    """
    def play(self):
        timeline.playForwards()
        
    """
    Return the menus to add to mrv2's main toolbar.
    You can add your commands to mrv2 menus or create a new
    menu entry. Each Submenu is separated by '/'.
    Currently, you cannot remove a menu entry.

    Returns:
    dict: a dictionary of key for menu entries and values as methods.
    """
    def menus(self):
        menus = {
            "Python/Play/Forwards" : self.play,              # call a method
            "Python/Play/Backwards" : timeline.playBackwards # call a function
        }
        return menus
        
