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

      - Thumbnails

	Preview Thumbnails Above Timeline off.

      - Playback

	Auto Hide Pixel Bar checked
	or optionally hide the pixel bar or set the UI to presentation mode.

    * Panel->Settings
      
      - Cache Gigabytes

	Set to 4GB at least (the default is half your memory)

      - FFmpeg Color Precision

	If you are playing a YUV420_P8 video with Color Space "undefined"
	this setting will provide a more accurate color conversion, but it
	can make the reading of the movie slow.

Linux
=====

All protocols
-------------

- Playback of screen captures don't play at their actual speed.

  Sadly, this is a limitation of mrv2.  Screen captures such as those created
  by Ubuntu may rely on Variable Frame Rates (VFR), meaning that the speed
  changes between each frame of the video.
  mrv2 needs a constant frame rate throughout for playback, for seeking and
  for showing the timeline.

X11
---

- I see some tearing of the video during playback.  How can I fix it?

  This is likely an issue with your configuration of your graphics card.
  For NVidia graphic cards, you can fix it by:
  
    * Edit /etc/X11/xorg.conf with sudo permissions.

    * Go to the "Device" section of your NVIDIA graphics card.

    * Add or modify::
	
	Option "TripleBuffer" "True"
	Option "SwapInterval" "1"

Wayland
-------

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
	Or use mrv2 not on Wayland but on XWayland.
	
	If you run into that, use XWayland or log in onto X11.  To use
	XWayland, set::

	  export FLTK_BACKEND=x11

- When I have a long floating panel, like the Media Information Panel, I can
  drag them out of the screen, and cannot repositiong them.

  You can use ALT + F7 to reposition it.

  
Windows
=======

- After an install with file associations all icons appear with the mrv2 logo.
  How can I display the picture thumbnails once again?

  * It is a bug in Windows, but it can be worked around.  You manually should
    select one file extension that you want thumbnails for and select::
    
      Open with->Choose another app

    Then select "Photos" from the menu and "Always".  This will restore the
    thumbnail, but it will remove the association to mrv2.  To once again
    associate mrv2 to the file, go again to::

      Open with->Choose another app

    But this time select "mrv2 Media Player Latest" and "Always".  This will
    associate mrv2 back to the file, but leave the thumbnails.  You will see
    a small icon of mrv2 on the right bottom corner of the thumbnail.
