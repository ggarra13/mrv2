.. _interface:

##################
The mrv2 Interface
##################

.. image:: ../images/interface-02.png
   :align: center

mrv2's main window provides 6 different toolbars that can be hidden or shown.

The first toolbar is the menu toolbar.  It can be toggled with Shift + F1. The menus are also available from the Right Mouse Button on the View window.  The menu toolbar also has the Edit button to toggle the editing mode and the Fit Window to image button.

The second toolbar is the main layer, exposure, OCIO and gamma controls.  It can be toggled with F1.

The third toolbar is the Timeline Viewport and controls.  You can toggle it with F3.

The fourth toolbar is the Pixel Toolbar, which shows the current pixel under the cursor.  You can toggle it with F2.

Finally, the last toolbar is the Status toolbar.  It will print out errors and let you know in what action mode you are in (Default is Scrubbing).

Hiding/Showing UI elements
++++++++++++++++++++++++++

Some useful (default) shortcuts are as follows:

============  =======================================
Shortcut      Action
============  =======================================
Shift + F1    Toggle Menu Toolbar.
F1            Toggle Topbar.
F2            Toggle Pixel Toolbar.
F3            Toggle Timeline Viewport and controls.
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

.. topic:: Safe Areas

   The Safe Areas toggle allows you to display the film and video safe areas.
    
.. topic:: Data Window

   Toggling this on will show or hide the OpenEXR's Data Window.
   
.. topic:: Display Window

   Toggling this on will show or hide the OpenEXR's Display Window.	
	   
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

The Panels
++++++++++

mrv2 supports Panels to organize the information logically.  These panels can be docked to the right of the main viewport or can be made floating windows if dragged from their main drag bar.

Divider
+++++++

The Panels have a divider, just like the Timeline Viewport, and can be dragged to make the panel bigger or smaller (and change the size of the main viewport).



