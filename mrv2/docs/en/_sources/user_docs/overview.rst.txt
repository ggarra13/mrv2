============
Introduction
============


What is mrv2 ?
**************

mrv2 is an open source professional flipbook and review tool for the Visual Effects, Animation and Computer Graphics Industries.  mrv2 is focused on providing an intuitive, easy to use interface with the highest performance playback engine available at its code and a Python API for pipeline integration and customisation for total flexibility.

mrv2 can handle collections of media sources quickly, loads specialised image formats and displays images with colour management. Users can quickly import, organise and group media into playlists and 'subsets', playing through and looping on media items and adding review notes and sketched annotations, allowing one to view the media in a highly interactive and collaborative way. This enables workflows that are essential for teams in VFX, animation and other post-production activities who need to see, on demand, the artwork that they and their colleagues are creating. For example one can jump between the viewed media source instantaneously, inspect pixels close-up, do frame-by-frame comparisons across multiple media sources, annotate the media with drawings and captions or add feedback notes to share.


Current Version: v1.0.7 - Overview
**********************************

This version of the application is a robust and high performance playback and review solution. mrv2 has been deployed at multiple facilities and is used by multiple individuals daily since August 2022.

Here are some key features:

**Loading Media**

  - Display virtually any image format in common use today (EXR, TIF, TGA, JPG, PSD, MOV, MP4, WEBM, etc).
  - Drag and drop media from the desktop file system browser directly into the mrv2 interface.
  - Use the Python API to build playlists with your own custom scripts, control
    the player, save movies, compare clips and control your pipeline
    integration.
  - Audio playback for sources with an embedded sound track is provided with
    scrubbing and reverse playback.
  - Native .otio playback support allows playing back OpenTimelineIO timelines
    with dissolves.
  - Pixar's USD (Universal Scene Description) support in OpenGL.
  - NDI network communication.

**Playlists**

  - Create any number of playlists, save them into an .otio file, and edit them
    with the built-in editing tools.

**Annotations and Notes**

  - Add notes and annotations to media on individual frames o todos los cuadros.
  - On screen annotations can be created with easy to use, highly responsive sketching tools. Annotations features currently include:
      
    1. Adjustable color, opacity and size of brush strokes. 
    2. Shapes tool for boxes, circles, lines and arrows etc. 
    3. Eraser pen for even more sketching flexibility.
    4. Editable text captions with adjustable fonts, position, scale, color
       and opacity.
    5. Save annotations to disk as a movie file or any of the supported image
       saving formats.
       
  - Navigate your media collection through the notes interface by jumping directly to bookmarked frames.
  - Annotations and Notes can be saved as a PDF document for easily sharing with the production staff.

.. image:: images/interface-01.png

**The Viewer**

  - Color accurate display (OCIO v2 colour management).
  - Hotkey driven interaction.
  - Adjust exposure and playback rate.
  - Color correction tools to control gain, gamma, tint, saturation, etc.
  - Zoom/pan image tools, RGBA channel display.
  - A/B, Wipe, Overlay, Difference, Horizontal, Vertical and
    Tile 'Compare Modes'.
  - Predefined masking overlay and safe areas guide-lines.
  - 'Pop-out' 2nd viewer for dual display set-ups.

**Sessions**

  - Session syncing is provided to synchronize one or more viewers in a review session across a LAN.  You can have a server and multiple clients and they can all control all aspects of mrv2 (user selectable).
  - Session files can be saved to disk to save the state of the UI and loaded media.
    
**Hotkeys**

  - User defined hotkeys saved in a separate file (mrv2.keys.prefs) allows
    changing the settings of all the menus and some viewer controls.

**API features for pipeline developers**

*Python API*

  - An embedded Python interpreter is available to execute scripts within mrv2, add or create new menu entries.
  - Create and build media playlists through straightforward API methods.
  - Control all the panel settings and the timeline (start, stop, step frame,
    seek, etc).

