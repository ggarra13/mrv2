###########
Preferences
###########


User Interface
==============

.. image:: ./images/preferences-01.png
   :align: center
	   
Here is where you can set the preferences of the User Interface when starting mrv2.
The Window Behavior section allows you to set how the main and secondary window behave.

.. topic:: Always on Top and Secondary On Top

   This setting sets the behavior of the main and secondary windows to always remain on top of other windows on the system.

.. topic:: Single Instance

	   Currently unused.

.. topic:: Auto Refit Image

	   When this is on, any rescaling of the window will reposition and center the image withing the viewport.

.. topic:: Normal, Fullscreen and Presentation

	   With this setting, it allows mrv2 to start at the size of the loaded image (Normal), at Fullscreen or in Presentation mode.

UI Elements
+++++++++++

.. topic:: The UI bars
   
	   These allow you to start mrv2 without one of its toolbars.
	      
- Menu Bar
- Top Bar
- Pixel Toolbar
- Timeline
- Status Bar

.. topic:: macOS Menus

	   It allows you to use menus in the macOS bar instead of mrv2's menu bar (Setting for macOS only).

.. topic:: Tool Dock

	   This setting shows or hides the action and drawing tool dock.
	      
.. topic:: Only One Panel

	   When this is on, only one panel is shown in the docking section of the main UI.
		
	   
View Window
===========

.. image:: ./images/preferences-02.png
   :align: center
   

Here you can set the default settings for the view window.

.. topic:: Gain and Gamma

	   You can establish the default gain and gamma for the viewer.

.. topic:: Crop

	   You can set the default crop mask.

.. topic:: Safe Areas

	   You can establish that mrv2 starts with Safe Areas on.

.. topic:: Zoom Speed

	   This controls how fast the mousewheel zoom is.

.. topic:: HUD

	   The settings under this label set what parameters will show by default in the HUD.

Language and Colors
===================

.. image:: ./images/preferences-03.png
   :align: center


Here you can customize the colors and language used in mrv2.

.. topic:: Language

	   You can set the Natural Language used in the interface.

.. topic:: Scheme

	   You can set the default FLTK scheme used in the UI.
	   We recommend you stick with gtk+.

.. topic:: Color Theme

	   You can establish the color theme used.
	   We recommend you stick with the Black Color scheme.

.. topic:: View Colors

	   You can establish the colors for the background, the selection rectangle, the text overlay (unused) and the HUD.

Positioning
===========

.. image:: ./images/preferences-04.png
   :align: center


Here you can control the Window's size and positioning.

.. topic:: Always Save on Exit

	   When this is on, mrv2's positioning and size will be saved automatically.

.. topic:: Fixed Position

	   Here you can establish the position where mrv2's window will start.

.. topic:: Fixed Size

	   With this, you can establish the size of mrv2's window at start.

.. topic:: Take Current Window Values

	   When clicked on this button, the Fixed Position and Fixed Size values
	   will be filled with the current position and size of the mrv2 window.

File Requester
==============

.. image:: ./images/preferences-05.png
   :align: center

Here you can establish the behavior of mrv2's file requester, as well as use the OS native file requester.

.. topic:: Single Click to Travel Drawers

	   With this on, a single click on any folder will open it.  Otherwise,
	   you will need to click twice.

.. topic:: Thumbnails Active

	   With this on, mrv2's file requester will show thumbnails for those image, movie and asset formats that it recognizes.

.. topic:: USD Thumbnails

	   With it selected, USD (Pixar's Universal Scene Description) assets will show thumbnails.  Note that if your USD asset is complex, it may slow down the UI dramatically.

.. topic:: Use Native File Chooser

	   Instead of using mrv2's built-in file chooser, use the Native OS file chooser.

.. note::

   Some old versions of macOS would not allow opening private directories like Downloads, Documents and Desktop unless the Native File Chooser was used.
	   
Playback
========

.. image:: ./images/preferences-06.png
   :align: center

.. topic:: Auto Playback

	   With this setting, mrv2 will start playing the movie or file sequence as soon as it is loaded.

.. topic:: Looping Mode

	   Default looping mode.

.. topic:: Scrub Sensitivity

	   How fast or slow does dragging the mouse work when scrubbing. 
	   
Timeline
========

.. image:: ./images/preferences-07.png
   :align: center

.. topic:: Display

	   Whether to show the time as Frames, Seconds or Timecode.

.. topic:: Preview Thumbnails

	   Whether to show a thumbnail above the timeline when moving or dragging on the timeline.

Edit Viewport
-------------

.. topic:: Start in Edit mode

	   When selected, the UI will start in Edit mode by default.

.. topic:: Thumbnails

	   The size of the thumbnails or None for no thumbnails.

.. topic:: Show Transitions

	   Whether Transitions are shown as a bar in Edit mode.

.. topic:: Show Markers

	   Whether Markers are shown as a bar in Edit mode.

	   
Pixel Toolbar
=============

.. image:: ./images/preferences-08.png
   :align: center


The preferences in this section allow you to control how are the pixel values show in the pixel toolbar.

.. topic:: RGBA Display

	   Whether to show them as Float, Hex or Decimal values.

.. topic:: Pixel Values

	   Whether to show the pixel values with Full Lut, Gamma and Gain or
	   with the original values saved in the movie or image without
	   processing them through mrv2's color transformations.

.. topic:: Secondary Display

	   Whether to show the secondary display of pixel values as:

	   - HSV
	   - HSL
	   - CIE XYZ
	   - CIE xyY
	   - Lab CIELAB
	   - Luv CIELUV
	   - YUV (Analog PAL)
	   - YDbDr (Analog SECAM / PAL-N)
	   - YIQ (Analog NTSC)
	   - ITU-601 (Digital PAL/NTSC YCbCr)
	   - ITU-709 (Digital HDTV YCbCr)

.. topic:: Luminance

	   Whether to show Luminance as:

	   - Luminance (Y)
	   - Lumma (Y')
	   - Lightness
	     
OCIO
====

.. image:: ./images/preferences-09.png
   :align: center

	   
.. topic:: OCIO Config File

	   Configuration Setting for OCIO.
	   
OCIO Defaults
=============

.. image:: ./images/preferences-10.png
   :align: center


.. topic:: Use Active Views and Active Displays

	   When selected, if the OCIO config.ocio file has active views or active displays, these will be used (filtered).  Otherwise, they will be ignored.

.. topic:: Input Color Space

	   Establish the Input Color Space preferred for each image bit depth.
	   
Loading
=======

.. image:: ./images/preferences-11.png
   :align: center

Controls the behavior of loading movies and images.

.. topic:: Missing Frame

	   Sets what to do when a sequence or .otio file is missing a frame.
	   It can be set to Black Frame, Repeat Frame (the last available one)
	   or Scratched Frame (the last available one but with a red scratched
	   cross).

.. note::
   A setting other than Black Frame can make scrubbing slow if there are many missing frames.

.. topic:: Version Regex

	   mrv2 supports image and movie versioning by using a regular expression (regex).  The default is to use _v which will match any text that starts with _v and a number.  When a version is matched and a Next, Previous, First or Last version is searched, the regex matching is printed out to the terminal.

.. topic:: Maximum Images Apart

	   When searching for a previous or next image version, this setting controls how far apart the version numbers can be.
   
Path Mapping
============

.. image:: ./images/preferences-12.png
   :align: center


Path Mapping allows you to share images and movies on a network even when the drive and paths do not match.  In the example above, the Remote directory is /Users/gga (a macOS directory), that is mapped to /home/gga (a Unix one).

The path mapping paths are saved in a simple text file in your .filmaura home directory, as mrv2.paths.prefs.

.. topic:: Add Path

	   It allows you to add a new remote/local path mapping.

.. topic:: Remove Path

	   It removes the selected remote/local path from the list.

Network
=======

.. image:: ./images/preferences-13.png
   :align: center


The Network preferences allows you to set what settings are sent and received by the local machine when connected on a network to another server or client.

Errors
======

.. image:: ./images/preferences-14.png
   :align: center


The Errors preferences allows you to establish what to do in case of an error.  You can choose to Do Nothing, Open Logs on Dock or Open Logs on Window (Default).
