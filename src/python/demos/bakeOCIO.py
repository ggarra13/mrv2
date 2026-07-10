#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a sequence or movie and saves a movie out.  It is mainly to
# be used to bake OCIO into a movie file.
# It should be run from the command-line like:
#
# mrv2 -ics "ACEScg" -od "sRGB - Display" -pythonScript bakeOCIO.py -pythonArgs 'out.mov'"
#

import mrv2
from mrv2 import cmd

args = sys.argv
if len(args) != 2:
    print("bakeOCIO.py 'outmov'")
    exit(1)

out      = sys.argv[1]
cmd.save(fileName=out)

print('Saving done.')
