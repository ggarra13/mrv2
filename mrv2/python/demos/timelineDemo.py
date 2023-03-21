#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a movie clip, plays it for a second and then stops it.
#

from mrv2 import cmd, media, timeline

if len(media.list()) == 0:
    print("Please load a movie file or sequence with at least 120 frames.") 
    exit()

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

#
# Seek to frame 120
#
timeline.seek(120)
