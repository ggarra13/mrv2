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
    print("Please load a movie file or sequence with at least 100 frames.")
else:
    #
    # Start the playback
    #
    timeline.playForwards()

    main_window = None

    pw = Fl.first_window();
    while pw:
        print("Window label",pw.label())
        pw = Fl.next_window();
    
    window = Fl_Window(300,180)
    box = Fl_Box(20,40,260,100,"Hello, World!")
    box.box(FL_UP_BOX)
    box.labelsize(36)
    box.labelfont(FL_BOLD+FL_ITALIC)
    box.labeltype(FL_SHADOW_LABEL)
    window.end()
    window.show()  # don't use show(sys.argv) or it will break the UI.
    # No need to call Fl.run()
