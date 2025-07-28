#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads two movie clips, compares them with a wipe,
# changes the wipe angle and center and finally, changes saturation.
#

import mrv2
from mrv2 import cmd, math, media

def compare_files():
    cmd.compare( 0, 1 )

def change_wipe():
    o = cmd.compareOptions()
    o.wipeCenter.x = 0.25
    o.wipeRotation = 15.0
    cmd.setCompareOptions(o)
    #
    # Wait in a loop updating the UI for 0.1 seconds
    # We need to do this so the comparison takes effect.
    #
    time = 0.0
    while time < 0.1:
        time += cmd.update()

def change_saturation():
    o = cmd.displayOptions()
    o.color.enabled = True
    o.color.saturation = math.Vector3f(4, 4, 4)
    cmd.setDisplayOptions(o)

if len(media.list()) < 2:
    print("Please load two videos or sequences to compare.")
else:
    compare_files()
    change_wipe()
    change_saturation()
