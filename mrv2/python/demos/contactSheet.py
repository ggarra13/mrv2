# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script creates a contact sheet from all the layers in an OpenEXR file.
# 
import mrv2
from mrv2 import cmd, math, imaging, media, timeline


def clone():
    items = media.list()
    path = items[0].path.get()
    layers = cmd.getLayers()
    for i in range(0,len(layers)):
        print("Opening",path,"for layer",layers[i])
        cmd.open(path)
    return layers

def set_layers():
    items = media.list()
    for i in range(0,len(items)):
        media.setLayer(items[i], i)

def contact_sheet(layers):
    media.setA(0)
    media.setB(0, False)
    opts = cmd.compareOptions()
    opts.mode = media.CompareMode.Tile
    cmd.setCompareOptions(opts)
    for i in range(1,len(layers)):
        media.setB(i, True)

if len(media.list()) < 1:
    print("This script needs an EXR file or sequence with many layers.")
else:
    layers = clone()
    print("Layers=", layers)
    set_layers()
    contact_sheet(layers)
