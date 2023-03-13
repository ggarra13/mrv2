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

# Change root path to that of mrv2's root
ROOT = "."

MOVIE_DIR =  ROOT + "/tlRender/etc/SampleData"

def open_files():
    cmd.open( MOVIE_DIR + "/BART_2021-02-07.m4v")
    cmd.open( MOVIE_DIR + "/Dinky_2015-06-11.m4v")

def compare_files():
    files = media.fileList()
    cmd.compare( files[0], files[1] )

def change_wipe():
    o = cmd.compareOptions()
    o.wipeCenter.x = 0.25
    o.wipeRotation = 15.0
    cmd.setCompareOptions(o)

def change_saturation():
    o = cmd.displayOptions()
    o.colorEnabled = True
    o.color.saturation = math.Vector3f( 4, 4, 4)
    cmd.setDisplayOptions(o)

if len(media.fileList()) < 2:
    open_files()
compare_files()
change_wipe()
change_saturation()
