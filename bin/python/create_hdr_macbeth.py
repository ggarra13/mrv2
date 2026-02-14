#!/usr/bin/env python

import numpy as np
import OpenEXR
import Imath

def srgb_to_linear(srgb_array):
    normalized = srgb_array / 255.0
    return np.where(normalized <= 0.04045, 
                    normalized / 12.92, 
                    ((normalized + 0.055) / 1.055) ** 2.4)

# 1. Standard 24 Macbeth Patches (SDR range)
macbeth_srgb = [
    [115, 82, 68],   [194, 150, 130], [98, 122, 157],  [87, 108, 67],   [133, 128, 177], [103, 189, 170],
    [214, 126, 44],  [80, 91, 166],   [193, 90, 99],   [94, 60, 108],   [157, 188, 64],  [224, 163, 46],
    [56, 61, 150],   [70, 148, 73],   [175, 54, 60],   [231, 199, 31],  [187, 86, 149],  [8, 133, 161],
    [243, 243, 242], [200, 200, 200], [160, 160, 160], [122, 122, 121], [85, 85, 85],    [52, 52, 52]
]
print('Creating macbeth linear')
macbeth_linear = srgb_to_linear(np.array(macbeth_srgb))

# 2. Define HDR Strip (Mapping 1.0 Linear = 100 nits)
# Values: 4000nits (40.0), 1000nits (10.0), 100nits (1.0), 18% Gray (0.18)
# We fill the remaining two slots with absolute black (0.0) and a "super-white" (100.0 / 10,000 nits)
print('creating HDR strip')
hdr_strip = [
    [40.0, 40.0, 40.0],  # 4000 nits
    [10.0, 10.0, 10.0],  # 1000 nits
    [1.0, 1.0, 1.0],     # 100 nits (Diffuse White)
    [0.18, 0.18, 0.18],  # 18% Mid-Gray (Reference Gray)
    [0.01, 0.01, 0.01],  # Near Black
    [100.0, 100.0, 100.0]# 10,000 nits (Peak HDR10/Dolby Vision)
]

# Settings
patch_size, padding = 200, 20
cols, rows = 6, 5 # Increased rows to 5
width = (cols * patch_size) + ((cols + 1) * padding)
height = (rows * patch_size) + ((rows + 1) * padding)

print(f"Size: {width}x{height}")
image_data = np.zeros((height, width, 3), dtype=np.float32)

# Fill Standard Macbeth Rows (0-3)
for i in range(24):
    r, c = divmod(i, 6)
    y_s = padding + r * (patch_size + padding)
    x_s = padding + c * (patch_size + padding)
    image_data[y_s:y_s+patch_size, x_s:x_s+patch_size] = macbeth_linear[i]

# Fill HDR Strip Row (Row 4)
for c in range(6):
    y_s = padding + 4 * (patch_size + padding) # 5th row
    x_s = padding + c * (patch_size + padding)
    image_data[y_s:y_s+patch_size, x_s:x_s+patch_size] = hdr_strip[c]

# --- Save to EXR ---
R, G, B = [image_data[:,:,i].tobytes() for i in range(3)]
header = OpenEXR.Header(width, height)
header['channels'] = {c: Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT)) for c in "RGB"}

output_file = "hdr_macbeth_reference.exr"
exr = OpenEXR.OutputFile(output_file, header)
exr.writePixels({'R': R, 'G': G, 'B': B})
exr.close()

print(f"Created HDR Chart: {output_file} with peak value 100.0 (10k nits).")
