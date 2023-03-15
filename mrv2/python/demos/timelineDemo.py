#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a movie clip, plays it for a second and then stops it.
#

from mrv2 import cmd, media, timeline

# Change root path to that of mrv2's root
ROOT = "."

MOVIE_DIR =  ROOT + "/tlRender/etc/SampleData"

def open_file():
    cmd.open( MOVIE_DIR + "/BART_2021-02-07.m4v")

if len(media.fileList()) == 0:
    open_file()

#
# Start the playback
#
timeline.playForwards()

#
# Wait in a loop updating the UI
#
for x in range(0,1000000):
    cmd.update()

#
# Stop the playback
#
timeline.stop()
