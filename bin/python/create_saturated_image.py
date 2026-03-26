import numpy as np
import OpenEXR
import Imath

# Create a small 256x256 image (easy to view)
width = 256
height = 256

# Create float32 arrays for each channel
# Red channel set to 5.0 (way above 1.0) → extremely saturated/bright red
# Green and Blue remain 0.0
red_channel   = np.full((height, width), 5.0, dtype=np.float32)
green_channel = np.zeros((height, width), dtype=np.float32)
blue_channel  = np.zeros((height, width), dtype=np.float32)

# Optional: you could also add an alpha channel if you want RGBA
# alpha_channel = np.ones((height, width), dtype=np.float32)

# Set up the OpenEXR header
header = OpenEXR.Header(width, height)
header["channels"] = {
    "R": Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT)),
    "G": Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT)),
    "B": Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT)),
    # "A": Imath.Channel(Imath.PixelType(Imath.PixelType.FLOAT)),  # uncomment for alpha
}

# Create the output file and write the pixel data
output = OpenEXR.OutputFile("oversaturated_red.exr", header)

# Write the channels (OpenEXR expects raw byte buffers)
output.writePixels({
    "R": red_channel.tobytes(),
    "G": green_channel.tobytes(),
    "B": blue_channel.tobytes(),
    # "A": alpha_channel.tobytes(),  # uncomment if using alpha
})

output.close()

print("✅ OpenEXR file 'oversaturated_red.exr' created successfully!")
print("   Red channel = 5.0 (saturation/intensity > 1.0 is fully supported)")
