
import mrv2
from mrv2 import cmd, math, imaging, media, timeline

PATH = 'D:/movies/bunny_v001.0001.exr'

def open_file():
    print("Opening",PATH)
    cmd.open(PATH)

def clone():
    items = media.fileList()
    path = items[0].path.get()
    layers = cmd.getLayers()
    for i in range(0,len(layers)):
        print("Opening",path,"for layer",layers[i])
        cmd.open(path)
    return layers

def set_layers():
    items = media.fileList()
    for i in range(0,len(items)):
        media.setLayer(items[i], i)

def contact_sheet(layers):
    media.setA(0)
    media.setB(1, False)
    opts = cmd.compareOptions()
    opts.mode = media.CompareMode.Tile
    cmd.setCompareOptions(opts)
    for i in range(1,len(layers)):
        media.setB(i, True)

if len(media.fileList()) < 1:
    open_file()

layers = clone()
print("Layers=", layers)
set_layers()
contact_sheet(layers)

print("BIndices=")
print(media.BIndexes())




