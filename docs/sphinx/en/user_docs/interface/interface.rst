.. _interface:

##################
The mrv2 Interface
##################

.. image:: ../images/interface-02.png
   :align: center

mrv2's main window provides 6 different toolbars that can be hidden or shown.

The first toolbar is the menu toolbar.  It can be toggled with Shift + F1. The menus are also available from the Right Mouse Button on the View window.  The menu toolbar also has the Edit button to toggle the editing mode and the Fit Window to image button.

The second toolbar is the main layer, exposure, OCIO and gamma controls.  It can be toggled with F1.

The third toolbar is the Timeline Viewport and controls.  You can toggle it with F2.

The fourth toolbar is the Pixel Toolbar, which shows the current pixel under the cursor.  You can toggle it with F3.

Finally, the last toolbar is the Status toolbar.  It will print out errors and let you know in what action mode you are in (Default is Scrubbing).

Hiding/Showing UI elements
++++++++++++++++++++++++++

Some useful (default) shortcuts are as follows:

============  =======================================
Shortcut      Action
============  =======================================
Shift + F1    Toggle Menu Toolbar.
F1            Toggle Topbar.
F2            Toggle Timeline Viewport and controls.
F3            Toggle Pixel Toolbar.
Shift + F7    Toggle Drawing and Action Tools.
e             Toggle Thumbnails (Edit) of Timeline.
F11           Toggle Fullscreen mode.
F12           Toggle Presentation mode (no toolbars).
============  =======================================


Customising the Interface
-------------------------

.. image:: ../images/interface-03.png
   :align: center

mrv2 can be customized to show any of the toolbars from **Window->Preferences->User Interface**.  These settings are saved when you exit mrv2 and will allow you to always start mrv2 in a certain configuration.

Mouse interaction in the Viewer
-------------------------------

A three buttoned mouse can be used for closer image inspection. Hold the middle button and drag to pan the image within the viewport. Hold down the Alt key on your keyboard and drag left/right with the right mouse button to perform a centred zoom in/out on the image.  You can also use the mousewheel which is more confortable.
The current zoom factor is shown on the pixel toolbar to the left.

.. note::
    To 'reset' the viewer so that the image is auto-fitted, you can select "Fit" from the Zoom display pulldown in the Pixel Toolbar or use the 'f' hotkey.

.. note::
    To 'center' or 'home' the view, without changing the zoom factor, you can
    use the 'h' hotkey.
    
.. note::
   If you want to zoom in or out at a particular percentage (say 2x), you can
   choose it from the pull-down menu on the pixel toolbar.

The Top Bar
+++++++++++

mrv2's Top Bar contains the controls of layers or image channels (usually from an OpenEXR).

The gain (controlled by the slider) and/or exposure, driven by the arrows and toggled by the button that shows the F-stop (f/8 by default).

The Input Color Space is right next to it.  This is the OpenColorIO (OCIO) control of the image.

Coming next, is the view and display control of OpenColorIO (OCIO).

The "L" button brings up the OpenColorIO Looks, which allow adding an artistic look to the image, besides the normal ACES workflow. 

Finally, the last control is the gamma one which is driven by the slider and is toggled between two previous values with the "Y" button.

.. note::

   The OpenColorIO (OCIO) controls are derived from your configuration file, which is specified in **Window->Preferences->OCIO**.  By default, the OCIO configuration file used is the cg-config one.  mrv2 ships also with the nuke-default and with the studio-config ones.
   If you set the OCIO environment variable, it will take precedence over the one saved in mrv2's Preferences file.

The Timeline
++++++++++++

.. image:: ../images/timeline-01.png
   :align: center

The Timeline Viewport allows you to scale the Editing's Thumbnails and Audio waveforms by dragging the viewport up and down.  For a quick display of all the tracks, you can click on the Edit button (Shortcut 'e' by default) in the Menu toolbar.
When showing the pictures, you can zoom in and out with the mousewheel.

Frame Indicator
---------------

Immediately to the left of the timeline is the 'current frame' indicator. Click on the button next to it to get a list of options as to how the current time is displayed:
    - *Frames:* absolute frame, starting at 0 for movies, and, usually at 1 for sequences.
    - *Seconds:* the current time from the start of the media in seconds.
    - *Timecode:* the 8 digit timecode. If the media has timecode metadata this will be used.

Transport Controls
------------------

These are pretty universal and don't need much explanation. There's a play backwards/forwards/pause button, step forwards/backwards buttons and jump to start and end.

FPS
---

The frames-per-second (FPS) indicator showing the desired FPS.  The FPS button is a popup that allows you to quickly switch to a new frame rate.

Start and End Frame Indicator
-----------------------------

To the right of the timeline, the Start frame and End Frame indicators are shown.  The S and E buttons can be clicked to set the In and Out points at the current frame.  It is equivalent to pressing the 'I' or 'O' hotkeys.

Player/Viewer Controls
----------------------

Two buttons to the bottom of the timeline viewport provide the following interactions
    - *Volume/mute control:* click on the speaker icon to toggle the mute control. Drag on the slider next to it to control the volume.
    - *Loop mode:* set whether the clip will loop, play it once and stop on the final frame or 'ping-pong' loop.

View Menu
+++++++++

The view menu provides controls for modifying the appearance and behaviour of the viewer:

.. topic:: Auto Frame

   The Auto Frame toggle handles how mrv2 behaves when switching from one clip
   to the next, or when resizing the windows.  If auto frame is on, the image
   is always set to fit the area.  When it is off, the zoom factor is kept
   between changing each clip.
   
.. topic:: Safe Areas

   The Safe Areas toggle allows you to display the film and video safe areas.
    
.. topic:: Data Window

   Toggling this on will show or hide the OpenEXR's Data Window.
   
.. topic:: Display Window

   Toggling this on will show or hide the OpenEXR's Display Window.	

.. topic:: Ignore Display Window

   By default, mrv2 will crop OpenEXRs to the display window set in the file.
   However, if the Data Window is *bigger* than the display window, this may
   not be desired.
   
.. topic:: Mask

   The mask allows drawing a black mask cropping your picture to a certain film aspect.

.. topic:: HUD

   Click this to enter the HUD (heads up display) settings. The HUD allows displaying of a lot of metadata of your media directly on the viewport.
      
Render Menu
+++++++++++

The Render menu provides controls for modifying the rendering of the image on the viewer:

.. topic:: Channels

   You can choose to display the Color, Red, Green, Blue or Alpha channels independently.  By default, you can toggle the channels with the "R", "G", "B" and "A" keys.
    
.. topic:: Mirror

   With these two controls, you can mirror the image vertically or horizontally.
   
.. topic:: Background

   By default, mrv2 uses a dark gray background to display the empty areas of the viewport.  With this, you can toggle it to show a black background instead.	
	   
.. topic:: Video Levels

   With this control, you can choose whether the video levels of the movie file are used, or whether you use the Legal or Full Range.

.. topic:: Alpha Blend

   You can select how the alpha channel is handled when the image or video has one.  You can choose between None, Straight or Premultiplied.
      
.. topic:: Minify and Magnify Filters

   With these two controls, you can select how mrv2 displays the images when zoomed in or zoomed out.  You can choose whether to use a Nearest (Pixelated) Filter or a Linear one.  The Magnify Filter can be toggled with Shift + F.

Playback Menu
+++++++++++++

The playback menu holds the standard playback functions that work just like the buttons in the timeline section of the main UI.  In addition to that, you can:

.. topic:: Toggle In Point

	   With this option you can toggle the starting point of the clip in the timeline.

.. topic:: Toggle Out Point

	   With this option you can toggle the ending point of the clip in the timeline.

.. topic:: Go to Next/Previous Annotation

	   Once you have created more than one annotation you can use these menu options to jump to each frame where the annotation resides.
	   
.. topic:: Annotation/Clear, Annotation/Clear All
	   
	   With these commands, once one or more annotations have been created, you will be able to clear the annotation on the current frame or all the annoations on the timeline.

Timeline Menu
+++++++++++++

The Timeline menu provides controls for modifying the timeline viewport at the bottom of the view window:

.. topic:: Editable

   When set to on, you will be able to move several clips created with the built-in Playlist Panel, Edit/Slice tool or when read from an .otio file.  The top part of the timeline (that with numbers), will allow you to go from one frame to the next.  When inactive, you can click on any of picture images, too, to move from frame to the next. 
    
.. topic:: Edit Associated Clips

   When this control is on, and Editable is on, video and audio clips can be
   moved together if they start and end *EXACTLY* at the same time.  Note that
   it is often difficult to get audio tracks to match video tracks exactly.
   
.. topic:: Thumbnails

   This setting allows you to turn off the picture thumbnails in the timeline
   viewport as well as select the size of them if you have larger monitor
   resolutions.	
	   
.. topic:: Transitions

   With this on, you can show audio and video transitions in .otio files.
   (Currently not implemented in v1.1.0).

.. topic:: Markers

   With this setting on, you can show .otio markers in the timeline viewport.
   Markers are used in .otio files to mark interesting areas in the timeline.
   
Image Menu
++++++++++

This menu appears only when a versioned clip is detected on disk.  By default, this is a directory or file or both named with "_v" and a number, like::

  Fluid_v0001.0001.exr
  Bunny_v1/Bunny.0001.exr

Note that this can be changed with a regular expression on the Preferences Window->Loading.

.. topic:: Version/First, Version/Last

	   It will check the disk for the first and last version it can find.  By default, it will accept a maximum gap of 10 versions before giving up.  You can see how it matches the clip in the Log Panel or in the shell console if you started it command-line.

.. topic:: Version/Previous, Version/Next

	   It will look for the previous or next version of the current clip.  By default, it will accept a maximum gap of 10 versions before giving up.  You can see how it matches the clip in the Log Panel or in the shell console if you started it command-line.
  
Edit Menu
+++++++++

The edit menu provides some quick editing functionality to edit the timeline and the clips.  It is not meant to be a full-blown Non Linear Editor, but a quick way to see and adjust your animations.

.. topic:: Frame/Cut, Frame/Copy, Frame/Paste, Frame/Insert

	   These controls allow you to cut, copy, paste and insert a single frame of animation.  It is useful for animators to block their timing, without having to actually go to their animation package itself.
    
.. topic:: Audio Gap/Insert, Audio Gap/Remove

	   This menu options allow you to add or remove an audio gap from a timeline portion that has no audio.  Just position the timeline frame above the clip you want to add the audio gap to and select Insert.  To remove it, do the same but use Remove.
   
.. topic:: Slice

	   This command will slice the clip(s) at current frame of the timeline, creating two clips.
	   
.. topic:: Remove

	   This command will remove the current clip(s) at the point in the timeline.

.. topic:: Undo/Remove

	   These command undo or redo the latest edit.  They should not be confused with the Undo/Redo annotations.

The Panels
++++++++++

mrv2 supports Panels to organize the information logically.  These panels can be docked to the right of the main viewport or can be made floating windows if dragged from their main drag bar.

Divider
+++++++

The Panels have a divider, just like the Timeline Viewport, and can be dragged to make the panel bigger or smaller (and change the size of the main viewport).



