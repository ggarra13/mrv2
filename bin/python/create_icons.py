#!/usr/bin/env python

#
# This simple script uses Python's pillow (PIL) module to convert icons to
# the different platforms.
#

import os

try:
    from PIL import Image
except ImportError as e:
    print('Please run python -m pip install pillow')
    exit(1)

PNG_ORIG_ICON='icons/original/mrv2_icon_1024x1024.png'

# Linux directory to store Freedesktop.org icons
PNG_DIR='mrv2/share/application'
PNG_ICON='mrv2.png'

# Windows ICO file
WINDOWS_ICON='mrv2/src/app.ico'

# MacOS icns file
MACOS_ICON='mrv2/etc/macOS/mrv2.icns'

print(f'Loading original icon {PNG_ORIG_ICON}')
image = Image.open(PNG_ORIG_ICON)

print('Saving macOS .icns with multiple embedded sizes.')
image.save(MACOS_ICON,
           sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128),
                  (256, 256), (512, 512), (1024, 1024)])

print('Saving Windows .ico with multiple embedded sizes.')
resized_image = image.resize((256, 256))
resized_image.save(WINDOWS_ICON, sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])

#
# Verified to work.
#
print('Saving Linux multiple sizes for adhering to Freedesktop.org.')
for i in [16, 32, 48, 64, 128, 256]:
    resized_image = image.resize((i,i))
    png_dir=f'mrv2/share/icons/hicolor/{i}x{i}/apps/'
    os.makedirs(png_dir, exist_ok=True)
    png_icon = png_dir + PNG_ICON
    print(f'\t{png_icon}')
    resized_image.save(png_icon)

    if i == 48:
        png_dir=f'mrv2/etc/'
        png_icon = png_dir + PNG_ICON
        resized_image.save(png_icon)
    
    if i == 16:
        png_dir=f'mrv2/icons/'
        png_icon = png_dir + PNG_ICON
        resized_image.save(png_icon)
        
        
