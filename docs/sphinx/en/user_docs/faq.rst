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
	Or use mrv2 not on Wayland but on X11/Xorg.

	On Ubuntu 22.04.4 LTS, you should install the proprietary NVidia
	drivers 535 at least and do::

	  sudo apt install libnvidia-egl-wayland1

	Note that NVidia's EGL under Wayland under Jammy is buggy and may
	result on random stopping of the playback until you move the mouse.
	To fix it, install the deb package from Ubuntu Noble::

	  https://www.ubuntuupdates.org/package/core/noble/main/base/libnvidia-egl-wayland1
	
	If you run into that, use XWayland or log in onto X11.  To use
	XWayland, set::

	  export FLTK_BACKEND=x11
	  
- I get warnings when I run the pre-compiled mrv2 on the console under Wayland
  on a modern distro like Ubuntu 22.04.4 LTS, like::

    (mrv2:6869): GdkPixbuf-WARNING **: 09:23:50.243: Cannot open pixbuf loader module file '/usr/lib64/gdk-pixbuf-2.0/2.10.0/loaders.cache': No such file or directory

    This likely means that your installation is broken.
    Try running the command
    gdk-pixbuf-query-loaders > /usr/lib64/gdk-pixbuf-2.0/2.10.0/loaders.cache
    to make things work again for the time being.

    (mrv2:6869): Gtk-WARNING **: 09:23:50.244: Theme parsing error: gtk.css:1422:23: 'font-feature-settings' is not a valid property name

    (mrv2:6869): Gtk-WARNING **: 09:23:50.245: Theme parsing error: gtk.css:3308:25: 'font-feature-settings' is not a valid property name

    (mrv2:6869): Gtk-WARNING **: 09:23:50.246: Theme parsing error: gtk.css:3770:23: 'font-feature-settings' is not a valid property name


  Sadly, these cannot be avoided.  You will need to re-compile from source on
  your target platform or use mrv2 under XWayland or under X11.

- While repositioning or rescaling floating panels under Wayland, I get clipping
  redraws of the floating panel.

  Unfortunately, this is a current limitation of FLTK 1.4.

- When I have a long floating panel, like the Media Information Panel, I can
  drag them out of the screen, and cannot repositiong them.

  You can use ALT + F7 to reposition it.

- Using drag and drop from Chrome or Chromium does not work on Wayland.

  This is because mrv2 runs under Wayland while Chrome runs under XWayland.

  There's a good workaround, though, to work with Wayland: chromium can be made a Wayland client launching it as follows::

    chromium --ozone-platform-hint=wayland &

  With this, DnD from the chromium URL field to an FLTK Wayland client app works both with gnome/Mutter and KDE/Plasma.

  It's possible to configure chromium so it runs as a Wayland client when possible and an X11 client otherwise::

    run chromium
    type chrome://flags/#ozone-platform-hint in the URL field
    change Preferred Ozone platform setting to Auto
    close and re-launch chromium which will run as a Wayland client.

  Note that Chrome's Wayland support is currently very buggy.
  
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
