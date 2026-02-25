#!/usr/bin/env python
import numpy as np
import OpenEXR
import Imath
import sys

# ------------------- Configuration -------------------
filename = "ACEScg_OCIO_test_1000nits.exr" if len(sys.argv) <= 1 else sys.argv[1]

width = 768
height = 256
patch_width = 256

# Create float32 image
img = np.zeros((height, width, 3), dtype=np.float32)

# Left patch: middle gray
img[:, 0:patch_width] = 0.18

# Middle patch: reference white
img[:, patch_width:2*patch_width] = 1.0

# Right patch: HDR peak
img[:, 2*patch_width:3*patch_width] = 1000.0

# ------------------- Write EXR (half-float) -------------------
header = OpenEXR.Header(width, height)
# Even though we want a HALF file, the Python wrapper needs FLOAT buffers
pt = Imath.PixelType(Imath.PixelType.FLOAT)
header['channels'] = {
    'R': Imath.Channel(pt),
    'G': Imath.Channel(pt),
    'B': Imath.Channel(pt)
}

# Optional: Add chromaticities or colorSpace if needed
# header['colorSpace'] = 'ACEScg'

out = OpenEXR.OutputFile(filename, header)

# CRITICAL FIX: Use float32 tobytes(). 
# The library converts this to HALF based on the header definition.
data = {
    'R': img[:, :, 0].astype(np.float32).tobytes(),
    'G': img[:, :, 1].astype(np.float32).tobytes(),
    'B': img[:, :, 2].astype(np.float32).tobytes()
}

out.writePixels(data)
out.close()

print(f"✅ Created {filename}")
print("   Left  patch (0–255)  → ACEScg 0.18  → ~14.5 nits")
print("   Middle patch (256–511) → ACEScg 1.0   → ~106.6 nits")
print("   Right patch (512–767) → ACEScg 1000.0 → 1000 nits")
