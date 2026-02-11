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

# Create float32 image (will be saved as half-float)
img = np.zeros((height, width, 3), dtype=np.float32)

# Left patch: middle gray → ~14.5 nits
img[:, 0:patch_width] = 0.18

# Middle patch: reference white → ~106.6 nits
img[:, patch_width:2*patch_width] = 1.0

# Right patch: HDR peak → 1000 nits
img[:, 2*patch_width:3*patch_width] = 1000.0

# ------------------- Write EXR (half-float) -------------------
header = OpenEXR.Header(width, height)
pt = Imath.PixelType(Imath.PixelType.HALF)
header['channels'] = {'R': pt, 'G': pt, 'B': pt}

# Optional: mark it as ACEScg (many tools respect this)
# header['colorSpace'] = 'ACEScg'

out = OpenEXR.OutputFile(filename, header)

data = {
    'R': img[:, :, 0].astype(np.float16).tobytes(),
    'G': img[:, :, 1].astype(np.float16).tobytes(),
    'B': img[:, :, 2].astype(np.float16).tobytes()
}

out.writePixels(data)
out.close()

print(f"✅ Created {filename}")
print("   Left  patch (0–255)  → ACEScg 0.18  → ~14.5 nits")
print("   Middle patch (256–511) → ACEScg 1.0   → ~106.6 nits")
print("   Right patch (512–767) → ACEScg 1000.0 → 1000 nits")
