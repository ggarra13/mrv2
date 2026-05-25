============
Introduction
============


What are mrv2 and vmrv2 ?
*************************

mrv2 and vmrv2 are two open source professional flipbook and review tools for the Visual Effects, Animation and Computer Graphics Industries.

The difference between mrv2 and vmrv2 is their graphics engine.

mrv2 uses OpenGL which is more battle tested but it is being deprecated while vmrv2 uses Vulkan which is the primary engine on Linux and modern Windows.  vmrv2 generally plays images and videos faster on modern GPUs and has support for Dynamic HDR, while mrv2 supports OpenUSD and uses static tone-mapping to simulate HDR, which may lead to somewhat incorrect colors.

To put it in perspective, Netflix, YouTube and Disney in their premium subscription packages use HDR, so if you are working on a movie for them or one that will be released in Cinemas you will want to watch your dailies with vmrv2.

From now on, when we refer to mrv2, we mean both mrv2 and vmrv2.

mrv2 is focused on providing an intuitive, easy to use interface with the highest performance playback engine available at its code and a Python API for pipeline integration and customisation for total flexibility.

mrv2 can handle collections of media sources quickly, loads specialised image formats and displays images with colour management. Users can quickly import, organise and group media into playlists and 'subsets', playing through and looping on media items and adding review notes and sketched annotations, allowing one to view the media in a highly interactive and collaborative way. This enables workflows that are essential for teams in VFX, animation and other post-production activities who need to see, on demand, the artwork that they and their colleagues are creating. For example one can jump between the viewed media source instantaneously, inspect pixels close-up, do frame-by-frame comparisons across multiple media sources, annotate the media with drawings and captions or add feedback notes to share.

The Pro versions of mrv2 and vmrv2 take dailies to a new level with the use of voice and link annotations for even faster feedback during the review process.

These features are useful for group reviews now but its access also through its Python API will, if I am right, in a potential future, allow controlling GenAI in the most intuitive way, by "directing" how you want your characters to behave by using your voice and a pointer.  GenAI is currently not allowing that yet, but mrv2 paves the way.

In addition to that, you can use Link Annotations to have linked images with an AI prompt for image references anywhere on your timeline so your AI engine can blend between them. This is also available from Python to feed it to your favorite AI service, propietary or local open source tool, if they allow it.

Albeit I use AI for math, coding, learning and general trouble-shooting, it is not for me as an artist.  I had a short stint on an AI project, but decided it was not for me.  Also doing hard-core AI requires thorough math knowledge which is not my strongest point.  I would rather concentrate on building a tool that helps artists get familiar with AI by using the same review concepts that appear in dailies.

Current Version: v1.6.6 - Overview
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

  - Add notes and annotations to media on individual frames or all frames.
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

  - Session syncing is provided to synchronize one or more viewers in a review session across a LAN or the Internet.  You can have a server and multiple clients and they can all control all aspects of mrv2 (user selectable).
  - Session files can be saved to disk to save the state of the UI and loaded media.
  - Currently, for the security conscious media is not streamed through the
    connection.
    
**Hotkeys**

  - User defined hotkeys saved in a separate file (mrv2.keys.prefs) allows
    changing the settings of all the menus and some viewer controls.

**API features for pipeline developers**

*Python API*

  - An embedded Python interpreter is available to execute scripts within mrv2, add or create new menu entries.
  - Create and build media playlists through straightforward API methods.
  - Control all the panel settings and the timeline (start, stop, step frame,
    seek, etc).
  - Link your review sessions and takes to your planning and schedualing tool
    or your choice.

