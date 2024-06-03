#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a sequence or movie, applies an ICS and a Display/View
# and saves out a new movie or sequence.  It is to be run from the command-line
# like:
#
# mrv2 -pythonScript bakeOCIO.py -pythonArgs "'sequence.exr' 'ACEScg'
#                                            'ACES 1.0 - SDR Video'"
#

import mrv2
from mrv2 import cmd, image

args = cmd.args()

if len(args) != 4:
    print("bakeOCIO.py 'insequence' 'ics' 'display/view' 'outmov'")
    exit(1)

sequence = args[0]
ics      = args[1]
view     = args[2]
out      = args[3]

image.setOcioIcs(ics)
image.setOcioView(view)

cmd.open(filename=sequence)
cmd.save(filename=out)
print('Saving done.')
