#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a sequence or movie and saves a movie out.  It is mainly to
# be used to bake OCIO into a movie file.
# It should be run from the command-line like:
#
# mrv2 -ics "ACEScg" -od "sRGB - Display" -pythonScript bakeOCIO.py -pythonArgs "'sequence.exr' 'out.mov'"
#

import mrv2
from mrv2 import cmd

args = cmd.args()

if len(args) != 2:
    print("bakeOCIO.py 'insequence' 'outmov'")
    exit(1)

sequence = args[0]
out      = args[1]

cmd.open(fileName=sequence)
cmd.save(fileName=out)
print('Saving done.')
