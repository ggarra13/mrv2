#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This demo loads a sequence or movie and saves a movie out.  It is mainly to
# be used to bake OCIO into a movie file.
# It should be run from the command-line like:
#
# mrv2 -ics "ACEScg" -od "sRGB - Display" -pythonScript bakeOCIO.py -pythonArgs 'out.mov' <sequence.exr>
# vmrv2 -ics 'ACEScg' -od 'ST2084-P3-D65 - Display' -pythonScript bakeOCIO.py -pa 'out.mov' <sequence.exr>
#

import mrv2
from mrv2 import cmd

args = sys.argv
if len(args) != 2:
    print("Normal usage for baking SDR:")
    print()
    print("mrv2 -ics 'ACEScg' -od 'sRGB - Display' -ps bakeOCIO.py -pa 'out.mkv' <sequence.exr>")
    print()
    print("Normal usage for baking HDR:")
    print()
    print("vmrv2 -ics 'ACEScg' -od 'ST2084-P3-D65 - Display' -ps bakeOCIO.py -pa 'out.mkv' <sequence.exr>")
    print()
else:
    out      = sys.argv[1]

    #
    # Select Profile (codec). 
    #
    profile = mrv2.io.Profile(mrv2.io.Profile.VP9)

    #
    # Select SaveOptions (like profile and presets)
    #
    options = mrv2.io.SaveOptions()
    
    options.ffmpegProfile = profile
    options.ffmpegPreset = cmd.rootPath() + '/presets/vp9-good.pst'
    
    cmd.save(fileName=out, options=options)
    print('Saving done.')
