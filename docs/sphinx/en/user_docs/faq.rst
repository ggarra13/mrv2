###
FAQ
###

All Platforms
=============

- My playback is too slow.  How can I improve it?
	
  Check that you have:
  
    * Window->Preferences

      - OpenGL
	
	Color Buffers' Accuracy set to Automatic (preferred) or Fast.

      - Timeline

	Preview Thumbnails off.

      - Playback

	Auto Hide Pixel Bar checked
	or optionally hide the pixel bar or set the UI to presentation mode.

    * Panel->Settings
      
      - Cache Gigabytes

	Set to 4GB at least (the default is half your memory)


Linux
=====

- My playback is too slow.  How can I improve it?	

  Check that you have:

    * Help->About

      - Go to the HW tab.

	Check the GPU information and make sure that Vendor/Renderer is not
	Mesa, but the appropriate for your graphics card.  For example:
	
	Vendor:     NVIDIA Corporation
	Renderer:   NVIDIA GeForce RTX 3080/PCIe/SSE2
	Version:    4.6.0 NVIDIA 535.171.04

	Max. Texture Size:32768 x 32768

	If it is using Mesa, you may need to configure XWayland/Wayland
	correctly for your OS and graphics card.
	Or use mrv2 not on Wayland but on X11/Xorg.

	On Ubuntu 22.04.4 LTS, you should install the proprietary NVidia
	drivers 535 at least and do::

	  sudo apt install libnvidia-egl-wayland1


-  Seeking on 4K or 8K movies is too slow.  How can I improve it?

   Use a small Settings Cache Read Ahead and Read Behind with 0 Gigabytes.
   You will not be able to scrub as nicely, but jumping between frames will
   be really fast.

  
Windows
=======

- After an install with file associations all icons appear with the mrv2 logo.
  How can I display the picture thumbnails once again?

  * It is a bug in Windows, but it can be worked around.  You manually should
    select one file extension that you want thumbnails from and select::
    
      Open with->Choose another app

    Then select "Photos" from the menu and "Always".  This will restore the
    thumbnail, but it will remove the association to mrv2.  To once again
    associate mrv2 to the file, go again to::

      Open with->Choose another app

    But this time select "mrv2 Media Player Latest" and "Always".  This will
    associate mrv2 back to the file, but leave the thumbnails.  You will see
    a small icon of mrv2 on the right bottom corner of the thumbnail.
