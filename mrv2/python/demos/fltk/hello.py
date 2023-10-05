#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a movie clip, start playing it and opens a window on top
# of mrv2's main window.
#
# For more info about pyFltk, go to:
#   https://pyfltk.sourceforge.io/
#
# For more examples, refer to the python directory:
#   python/fltk/
#

from mrv2 import cmd, media, timeline

from fltk14 import *

if len(media.list()) == 0:
    print("Please load a movie file or sequence.")
else:
    #
    # Start the playback
    #
    timeline.playForwards()


    # Find the main window of the UI
    main_window = None
    pw = Fl.first_window();
    while pw:
        if pw.parent() == None and main_window == None and pw.label() != None:
            main_window = pw
        pw = Fl.next_window(pw)


    
    window = Fl_Window(300,180, main_window.label())
    box = Fl_Box(20,40,260,100,"Hello, World!")
    box.box(FL_UP_BOX)
    box.labelsize(36)
    box.labelfont(FL_BOLD+FL_ITALIC)
    box.labeltype(FL_SHADOW_LABEL)
    window.end()
    window.show()  # don't use show(sys.argv) or it will break the UI.
    # No need to call Fl.run()
