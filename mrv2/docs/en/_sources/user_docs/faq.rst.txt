###
FAQ
###

All Platforms
=============

- My plaback is too slow.  How can I improve it?

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
