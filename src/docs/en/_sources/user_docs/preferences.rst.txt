###########
Preferences
###########

Preferences are changed by going to **Window->Preferences** (F10 by default).
The preferences of mrv2 are stored in your home directory, inside the .filmaura directory.  That is:

Linux::

  /home/<username>/.filmaura

macOS::

  /Users/<username>/.filmaura

Windows::

  C:/Users/<username>/.filmaura

There are several files.  The main preferences file that holds all the windows settings and user preferences is called::

  mrv2.prefs

The mrv2.prefs for the windows settings can usually be reset if you start mrv2 with -resetSettings or go to **Panel->Settings->Default** Settings.
  
The hotkeys file that holds all your keyboard shortcuts is called::

  mrv2.keys.prefs
  
The paths preferences file that holds old the path mappings is called::

  mrv2.paths.prefs

The mrv2 file browser favorite directories is called::

  mrv2.favorites

It's important to know each file in case something does not work right or you wan to share the preferences between several different machines or users (you can usually copy each file or the whole .filmaura folder).

User Interface
==============

.. image:: ./images/preferences/en_UI.png
   :align: center
	   
Here is where you can set the preferences of the User Interface when starting mrv2.
The Window Behavior section allows you to set how the main and secondary window behave.

.. topic:: Always on Top and Secondary On Top

   This setting sets the behavior of the main and secondary windows to always remain on top of other windows on the system.

.. topic:: Single Instance

	   When selected, mrv2 will behave as a single instance.  If you open
	   other movies with another mrv2, these will be redirected to the
	   single instance of mrv2.  Note that you can toggle this setting at
	   any time in the preferences to switch the behavior of mrv2.

	   This works with a TCP port in 55120, so if you are on Windows, it may
	   bring up a popup for allowing the connection to go through the
	   Firewall.  On Linux or macOS, this is not necessary.
	   

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

.. image:: ./images/preferences/en_view.png
   :align: center
   

Here you can set the default settings for the view window.

.. topic:: Gain and Gamma

	   You can establish the default gain and gamma for the viewer.

.. topic:: Crop

	   You can set the default crop mask.

.. topic:: Safe Areas

	   You can establish that mrv2 starts with Safe Areas on.

.. topic:: OCIO in Top Bar

	   When set, OCIO ICS, View and Look will be shown in the Top Bar of
	   the UI.
	   When not set, Gain (Exposure), Saturation and Gamma controls will
	   be shown.

.. topic:: Zoom Speed

	   This controls how fast the mousewheel zoom is.

.. topic:: HUD

	   The settings under this label set what parameters will show by default in the HUD.

Fonts
=====

.. image:: ./images/preferences/en_fonts.png
   :align: center
	   
.. topic:: Menus

	   Font for menus.

.. topic:: Panels

	   Not functional.


File Requester
==============

.. image:: ./images/preferences/en_filereq.png
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
	   
Language and Colors
===================

.. image:: ./images/preferences/en_language.png
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
	   
Pixel Toolbar
=============

.. image:: ./images/preferences/en_pixelbar.png
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
	     
Positioning
===========

.. image:: ./images/preferences/en_position.png
   :align: center


Here you can control the Window's size and positioning.

.. topic:: Always Save on Exit

	   When this is on, mrv2's positioning and size will be saved automatically.

.. topic:: Fixed Position

	   Here you can establish the position where mrv2's window will start.
	   To activate saving the position, make sure to mark the check box
	   next to "Fixed Position".

.. topic:: Fixed Size

	   With this, you can establish the size of mrv2's window at start.
	   To activate saving the size, make sure to mark the check box
	   next to "Fixed Size".

.. topic:: Take Current Window Values

	   When clicked on this button, the Fixed Position and Fixed Size values
	   will be filled with the current position and size of the mrv2 window.


Render
======

.. image:: ./images/preferences/en_render.png
   :align: center
	   
.. topic:: Vídeo Levels

	   - From File

	     The value is extracted from the movie or image if present.

	   - Legal Range

	     Valid range for Video.
	     
	   - Full Range

	     All 8 bit values (0...255).
	     
.. topic:: Alpha Blend

	   - None

	     The alpha channel is not considered for compositing.

	   - Straight

	     The alpha chnnael is considered straight.  It is needed for
	     dissolves in OTIO.

	   - Premultiplied

	     Color and alpha are considered premultiplied.
	     
.. topic:: Minify Fukter

	   - Linear

	     Linear when downsizing.

	   - Nearest

	     No filter.
	     
.. topic:: Magnify Fukter

	   - Linear

	     Linear when upsizing.

	   - Nearest

	     No filter.
	     
Thumbnails
==========

.. image:: ./images/preferences/en_thumbnails.png
   :align: center

Here you can select the behavior of thumbnails in all the interface.

.. topic:: Edit Viewport

	   You can select between None, Small, Medium and Large images.

.. topic:: Preview Thumbnails above Timeline

	   Whether to show a thumbnail above the timeline when moving or dragging on the timeline.

.. topic:: Preview Thumbnails on Panels

	   Whether to show a thumbnail for each image in the Files, Compare,
	   Stereo 3D and Playlist Panels.
	   
Timeline
========

.. image:: ./images/preferences/en_timeline.png
   :align: center

.. topic:: Display

	   Whether to show the time as Frames, Seconds or Timecode.

.. topic:: Remove EDLs in Temporary Folder

	   When creating playlists (EDLs), .otio files are saved in your
	   temporary folder.  Selecting this will remove those files on
	   program exit.

.. topic:: Start in Edit mode

	   When selected, the UI will start in Edit mode by default.
	   
	     
Playback
========

.. image:: ./images/preferences/en_playback.png
   :align: center

.. topic:: Auto Playback

	   With this setting, mrv2 will start playing the movie or file sequence as soon as it is loaded.

.. topic:: FPS

	   With this setting, you can control the frames per second of file sequences that don't have one embedded in the image.
	   
.. topic:: Looping Mode

	   Default looping mode.

.. topic:: Scrub Sensitivity

	   How fast or slow does dragging the mouse work when scrubbing.
	   
.. topic:: Scrub with Audio

	   When this is on, scrubbing will automatically start playback so
	   that audio can be heard.  Note that this makes scrubbing more
	   jerky.

Edit
++++

.. image:: ./images/preferences/en_edit.png
   :align: center

.. topic:: Default View

	   Whether to show Video Only or Video and Audio in the Edit
	   Viewport.

.. topic:: Show Transitions

	   Whether Transitions are shown as a bar in Edit mode.

.. topic:: Show Markers

	   Whether Markers are shown as a bar in Edit mode.

.. topic:: Editable

	   Whether the Edit viewport is editable by default.

.. topic:: Edit Associated Clips

	   Whether Video and Audio clips with *exactly* equal start times
	   and durations can be moved together by default.

	   
OCIO
====

.. image:: ./images/preferences/en_ocio.png
   :align: center

.. topic:: Configs Incorporadas

	   OpenColorIO 2.1 ha agregado configuraciones por defecto.
	   Aquí podes seleccionarlas.  Tienen el prefijo ocio://
	   y ningún archivo config.ocio.
      
.. topic:: OCIO Config File

	   Configuration Setting for OCIO.

.. note::

   If the environment variable OCIO is set, this setting will be ignored.

	   
OCIO Defaults
=============

.. image:: ./images/preferences/en_ocio_defaults.png
   :align: center


.. topic:: Use Active Views and Active Displays

	   When selected, if the OCIO config.ocio file has active views or active displays, these will be used (filtered).  Otherwise, they will be ignored.

.. topic:: Input Color Space

	   Establish the Input Color Space preferred for each image bit depth.
	   
Loading
=======

.. image:: ./images/preferences/en_loading.png
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
   
Paths Mapping
=============

.. image:: ./images/preferences/en_path_mapping.png
   :align: center


Paths Mapping allows you to share images and movies on a network even when the drive and paths do not match.  

The path mapping paths are saved in a simple text file in your .filmaura home directory, as mrv2.paths.prefs.

.. topic:: Add Path

	   It allows you to add a new remote/local path mapping.

.. topic:: Remove Path

	   It removes the selected remote/local path from the list.

Network
=======

.. image:: ./images/preferences/en_network.png
   :align: center


The Network preferences allows you to set what settings are sent and received by the local machine when connected on a network to another server or client.

OpenGL
======

.. image:: ./images/preferences/en_opengl.png
   :align: center

OpenGL is the default API used for 3D rendering in the timeline and viewports.

.. topic:: Monitor VSync

	   Defaults to Always.  It allows drawings to wait for the monitor sync
	   to avoid tearing.  You can set it to No or to Presentation.
	   Turning off Monitor Vsync will improve performance, but will probably
	   result in tearing.

.. topic:: Color Buffers' Accuracy

	   Controls the quality used in the calculations of OpenGL.

	   - Automatic: Default, which will set the buffers to
	     be equal to the bit depth of the image.
	   - Half Float: It will set the bit depth to half float.  It will
	     keep most floating point information with some precision errors.
	   - Float: It will preserve floating point information accurately.
	   - Fast: It will work in 8-bits.
	   
.. topic:: Blit Viewports

	   Assuming your Desktop system allows it, blitting will be used to
	   moving and panning in the image, which is usually faster.  Otherwise
	   a shader must be used.
	   
.. topic:: Blit Timeline

	   Assuming your Desktop system allows it, blitting will be used to
	   moving and panning in the timeline, which is usually faster.
	   Otherwise a shader must be used.

Errors
======

.. image:: ./images/preferences/en_errors.png
   :align: center


The Errors preferences allows you to establish what to do in case of an error.

.. topic:: On FFmpeg Error

	   You can choose to Do Nothing (Default), Open Logs on Dock or Open
	   Logs on Window.

.. topic:: On Error

	   You can choose to Do Nothing (Default), Open Logs on Dock or Open
	   Logs on Window (Default).

Behavior
========

.. image:: ./images/preferences/en_behavior.png
   :align: center


The Behavior Window allows you to control the behavior aspects of the viewer.

.. topic:: Check for Updates.

	   mrv2 can automatically check for updates, download and install a
	   new version assuming you have administrative privileges.
	   You need of course an internet connection and have compiled mrv2
	   with Python support.
	   Once downloaded, you will need to follow the standard procedures
	   for an install on your platform.
	   If the install is successful, the new mrv2 should start
	   automatically.

	   - On Demand From Help Menu.

	     Updates are handled manually, only when selecting
	     Help->Update mrv2.

	    - At Start Up

	      Updates are checked automatically at Start up.  If there's an
	      update, a Window will open for you to update, reinstall or maybe
	      downgrade if you have a beta version.
