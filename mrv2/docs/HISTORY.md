v0.7.8
======

- Added a hotkey selection to switch pen colors.
- Fixed hotkey search highlight redraw showing the wrong hotkey.

v0.7.7
======

- Fixed adding a file to playlist when the path was empty (ie. the file was
  loaded from the current directory).
- Fixed adding audio to a playlist when there was an empty audio track and a
  video clip (ie. a sequence of images and then a video clip).
- Fixed Text annotations having been turned off by mistake in v0.7.5.
- Fixed drag and drop on Windows only allowing to load 4 clips before not
  allowing drag and drop to work anymore.
- Fixed file requester hanging when reading .py files in a directory.
- Updated to OpenEXR 3.2, OpenColorIO 2.3.0, etc.
- Fixed Frame/Timecode/Seconds display in the PDF Exporter which was showing
  always 0.
- Fixed PDF export to save out annotations in increasing time order.
- Added User Documentation in English and Spanish, roughly based on xStudio
  documentation.
- Added 7 saved color presets to the color picker, like Krita, so you can
  easily choose from them.
- Added two colors to drawing tooldock.  You can switch between them with the
  arrows that point to them.
- Wipe Comparison is now fixed which had gotten broken in v0.7.5.  Thanks to
  Darby Johnston.
- Fixed Fullscreen Mode (F11 hotkey) on Windows which got broken in v0.7.5.


v0.7.6
======

This is mainly a bug fix release to Edit features and general issues found
with v0.7.5.

- Fixed hotkey check when entering a hotkey of the first 5 entries (a legacy
  from mrViewer).
- Fixed Load/Save hotkey file requester on Windows that would redraw
  incorrectly.
- Fixed message about corruption in hotkeys, when the reason was a new
  forced hotkey.
- Fixed Windows installer not installing the icon on Windows 11's Settings->
  Apps->Installed Apps.
- Fixed a memory corruption when pasting or inserting one frame of audio and
  playing back in reverse.
- Fixed log window showing up when there was a corruption on hotkeys even when
  the Preferences->Errors->Do Nothing was set.
- Fixed copying frames from one video to another even when they have different
  frame rates.
- Added an option in Preferences->Timeline to remove the EDLs from the
  temporary directory once the application exits.
- Improved focus handling of current frame, start frame and end frame widgets,
  which would loose it once the cursor was moved to the timeline.
- Fixed Edit/Frame/Insert when the movie had timecode in it and did not start
  at 0.
- Improved quality of Windows' icon.


v0.7.5
======

Playlist and Editing
--------------------

This is the first version that supports some basic editing and improves upon
the playlist panel by making it interactive.

- The Playlist panel's functionality has been simplified. It is there only to
  create an empty track or start a new EDL with one clip from the Files Panel.
- Added an Edit/Frame/Cut, Edit/Frame/Copy, Edit/Frame/Paste and
  Edit/Frame/Insert to cut, copy, paste and insert one frame (video and audio)
  of any media.  Currently, it does not support transitions, that are removed.
  As soon as one of these commands is used, a new EDL is created.
- Added an Edit/Slice to cut a clip in half at the current time location.
- Added an Edit/Remove to remove the clips that intersect the current time
  location.
- Added Drag and Drop functionality to the Files Panel into the Timeline
  Window as well as to the Playlist panel to add clips and create an EDL.
- Currently, there's still no support for trimming the Timeline clips yet.
  
- Documented Python USD module.
- Fixed DWA compression on non English locales (with commas as decimal
  separators)
- Allowed saving movies as EXR frames if Annotations is turned on.
- Fixed Media Information Depth display for floating point lumma and lumma with
  alpha images.
- Added pixel type saving to OpenEXR saving.  It can be Half or Float when
  Annotations is on.
- Added all libraries and their versions (when possible) to the About window.
- Fixed tlRender's version macro.
- Changed OpenEXR saving to use multipart api to allow future support to save
  all layers of an exr.
- Fixed edit viewport leaving room when show transitions was active but there
  were no transitions in the timeline.
- Added Timeline Preferences to show thumbnails, transitions and markers.
- Fixed pixel aspect ratio of saved OpenEXR images when they were not 1.
- Made Window resizing take into account Editing Viewport at start up.
- Fixed Log Panel when an error was shown to resize to the size of the window
  and not smaller.
- Log Panel will no longer open when the file requester is open. 
- Fixed Undo/Redo of annotations, which was incorrect.
- Fixed keyboard (menu) shortcuts not working in the Files Panel.
- Annotations are now kept with RationalTime instead of frames to be more
  precise.  Note, however, that old session files that use annotations will be
  incompatible.
- Fixed Network connections which had gotten broken on v0.7.1.
- Fixed Network connections on client startup, leading it to change the
  selected file on the server.
- Added Edit mode to the sessions file.
- Added Edit mode to the network connection (it will load as timeline or full).
- Laser annotations are no longer added to the draw undo/redo queue.
- Laser annotations now work properly on Network connections.
- Fixed Recent Files with entries with backslashes (ie. '\').
- Added new controls to Playlist panel.  Added a new Save icon.
- Fixed an annoying repositioning of window when loading new clips.
- Added support for Markers in timeline viewport.
- Made FPS display show only three decimal digits to simplify.
- Added File->Save->Single Frame to save a single frame only.
- Sped up Python compilation on Windows.
- Fixed OpenColorIO Active Displays and Active Views when they were set to an
  empty string which would turn off the View menu.
- The OCIO Defaults now has an option to use or ignore active_views and
  active_displays on the OCIO .config file.  The default is now to ignore them,
  as it was suggested using them in production was usually not the hassle.
- Fixed Image Information Panel size when it was saved as a window with the
  tabs open.
- Made Network connections more solid.  In case of wrong data sent through the
  network, it will discard it.
- Fixed Environment Mapping editing of the subdivisions no longer changing the
  sphere.
- Fixed menus still showing the panels open when they were closed from the
  Close button in network connections.
- Fixed a crash when selecting a new clip with the <- or -> arrows in the
  Files Panel.
- Fixed changing of volume and muting on network connections not showing the
  change on the remote client's interface.
- Added "Save/Single Frame" to save the current frame as an image.
- Fixed Timeline redraw issues on Windows.

v0.7.1
------
- Made Secondary Window respond to menu shortcuts, like F12.
- Made Secondary Window resize to Presentation mode or Fullscreen if it is
  present, instead of the normal viewport.
- Fixed Timeline redraw when playing the movie and the Secondary window was
  closed.
- Made default pen color (if not saved in preferences) be yellow to avoid
  conflicts with green screens.
- Fixed a random crash on Linux when using the -h or -v flags due to forcing
  an exit (NVidia driver would crash).
- Added usd python module and usd.setRenderOptions method.
- Fixed default values of USD stageCacheCount and diskCacheByteCount.
- Added laser drawing to annotations.  This allows the shape to not be
  permanent and disappear after a second.

v0.7.0
------
- Added Edit view (OpenTimelineIO) with thumbnails and audio waveforms, courtesy
  of Darby Johnston.
- First pass at USD OpenGL support courtesy of the great Darby Johnston.
- Added USD panel and -usd* command-line switches to control the quality and
  behavior of the USD display.
- Fixed pixel aspect ratio of OpenEXR, Cineon and DPX images when run on a
  locale that uses commas as decimal separator.
- Added Zip Compression support to saving OpenEXR images.
- Fixed Video Levels radio menus being toggle menus instead.
- Made menu items and pulldown labels smaller so they fit when mrv2 is sized
  to its minimum size.
- Fixed all overlapping widgets which could cause problems with FLTK.
- Signed the Windows installer with a self-certificate.  It does not prevent
  Windows and Chrome from complaining but it gives Publisher info.
- Fixed a minor memory leak when opening menus in the Python Editor.
- Added a Right Mouse Button menu to Log Panel to allow to copy text more
  easily.
- Fixed an incorrect use of OpenGL's GL_LINE_LOOP in a VAO.
- Fixed a flickering OpenGL issue when the Secondary Window was opened with a
  selection and then closed.
- Fixed incorrect use of OpenGL resources being shared with Secondary view
  leading to display issues.
- Made Secondary Window also display the name, type and audio of the
  video/image being played.
- Fixed Wayland and XWayland off-screen framebuffers.  Wayland support *must*
  be compiled with a recent Linux version like Ubuntu 22.04 LTS.  The binaries
  we distribute are compiled with a very old version of Wayland.
- Fixed a potential OpenGL redraw issue when drawing both soft and hard lines.
- Made draw cursor be a white/black shape for easier display.
- Fixed RPM package to install to /usr/local/mrv2-v${VERSION]-Linux-64 without library conflicts.  You can now install it just with something like:

```
  $ sudo rpm -i mrv2-v0.7.0-Linux-amd64.rpm
```

- Made Linux .deb and .rpm installers set the mrv2 desktop icon to Allow
  Launching by default.
- Made Linux .deb and .rpm installers set xdg-mime file associations properly
  for video, image, otio and USD files.
- Added mrv2.io.Options class to Python bindings.  With it, you can set the
  options when running cmd.save() to, for example, save annotations.
- Added a Always Save on Exit to Positioning preferences to always save the
  positioning and size of the window upon exiting the program.
- Added support for .otioz (Open Timeline IO .zip files).
- Added annotations Python module to allow adding (add function) notes to a
  certain time, frame or seconds.
- Fixed timeline thumbnail caching the last thumbnails of the movie shown when
  switching or closing movies.
- Fixed missing frame scratch display when playing a gap in an .otio file.
- Added Right Mouse Button menu option to File Panel to copy the name of the
  file to the clipboard and to open the location of the file in your file
  browser.
- Session files now also store information from the color panel (color
  adjustments).
- RPM and DEB packages have the version number in them to allow installing
  multiple versions of mrv2.  Besides /usr/bin/mrv2 pointing to the last
  installed version of mrv2, symlinks with the version number in them are
  also creaetd, like:
       /usr/bin/mrv2-v0.7.0
  

v0.6.4
------
- Improved Python plug-in API.  Now plug-ins are defined with a base class,
  and menus with a dict (without tuples) like:

```
      class HelloPlugin(mrv2.plugin.Plugin):
          def hello(self):
              print("Hello from plug-in!")

          def menus(self):
              menus = { "New Menu/Hello" : self.hello }
              return menus
```

- You can have multiple plug-ins in a single .py and have the class be named
  whatever you like, as long as you derive from mrv2.plugin.Plugin.
- Improved the look of Gamma, Gain and Volume sliders.
- Fixed Window on Top check mark when run from the Context menu.
- Fixed Presentation mode not returning to its previous state when switched off.
- Fixed an internal OpenGL error.
- Fixed Playback menu status at the beginning when Auto Playback was checked.
- Fixed pixel color look-up when loading a single frame.


v0.6.3
------
- Added a python plug-in system which is now documented in the
  Help->Documentation.  The environment variable used to look up plug-ins is:

  	MRV2_PYTHON_PLUGINS

  It is a list of colon (Linux or macOS) or semi-colon (Windows) paths.
  Plug-ins are defined, like:

```
      class Plugin(mrv2.plugin.Plugin):
          def hello(self):
              print("Hello from plug-in!")

          def menus(self):
              menus = { "New Menu/Hello" : self.hello }
              return menus
```
     	

- Added a mrv2_hello.py plug-in for demo purposes.
- Fixed a bug in the log panel appearing compressed on start up when docked.
- Allowed creation of .otio files of a single clip in Playlist Panel.
- Fixed scratched frames showing up on .otio files with gaps in them.
- Fixed preferences not hiding the different bars anymore (regression in
  v0.6.2).


v0.6.2
------
- Fixed the Media Info Panel crashing on start-up when the panel was open and
  the media was an OpenEXR with multiple layers.
- Made timeline cursor be white for easier reading on the eyes.
- Fixed timeline cursor not ending in last frame when dealing with sequences.
- Fixed Auto Refit Image preference working only after a restart of the
  application.
- Fixed Media Info Panel showing up with scrollbars when mrv2 was started
  command-line and with a movie.
- Fixed Save Movie or Sequence and Save PDF Document allowing to be selected
  even when no movie was loaded.
- Fixed mrv2's File requester saving always overwriting the file that was
  selected instead of using the filename in the filename input widget.
- Improved file requester selecting a file or directory when typing.
- Added a ffmpeg_windows_gpl.sh script to compile a GPL version of FFmpeg with
  libx264 and libvpx suport with MSVC.
- Made mrv2's GL window swallow Left Alt key presses when pressed alone to
  avoid Windows' taking over the Window.
- Fixed some typos in English installer (thanks to BigRoy!).


v0.6.1
------
- Split the Save Session menu entry in two.  There's now a Save Session As and a
  Save Session.  The session filename is kept if the session file was loaded, so
  you can just use Save Session to overwrite it.
- Fixed creating of playlists with file sequences with absolute and relative
  paths.
- Fixed creating of playlists with different layers as it is not possible in
  .otio files to specify the layer to load.
- Fixed a refresh issue on color lookups that would show the previous frame
  values (or previous redraw values).
- Added video and audio codec names to the HUD Attributes and the Media Info
  Panel.
- Made all tabs in all panels adjust the packing of the other panels. Tabs
  open/close are also now stored in the preferences.
- Added nuke-default ocio config once again.
- Added studio ocio config to distribution.
- Added TGA, BMP and PSD 8 and 16 bit readers.
- Added TGA and BMP 8 bit writers.
- Added a Scripts/Add to Script List to Python Panel.  It allows you to store
  up to 10 scripts in the list and run them just by accessing the menu.
  The script list is saved in the preferences.
- Fixed window size on starting mrv2 when Dock Group was open.
- Fixed PDF thumbnail creation when the clip was taller than its width.
- Fixed annotations not keeping the soft parameter in session or network
  connection.

v0.6.0
------
- Added the options for missing frames on the Preferences.  You can now:
  	* Display black
	* Repeat last frame
	* Repeat last frame scratched
	
- Made loading of session files use Path Mapping for files and OCIO config
  so that if a session file is loaded from different OSes the files will be
  found.
- Fixed loading session from the command-line not showing the opened panels that
  were also open in the preferences file.
- Added the name of the layer to the thumnail description in the files, compare,
  playlist and stereo panels.
- Added anaglyph, scanline, columns and checkered stereo 3D.
- Added a new Stereo 3D Panel to control the stereo.
   * To use it, you load a clip with left and right views (usually a v2
     multipart openexr).  Then, open the Files Panel and select the clip and
     layer to use.
   * Open the Stereo Panel and select the Input to "Image".  That will clone
     the clip and select the opposite view (ie. right if you selected left).
   * Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).

- You can also use the Stereo 3D Panel with two clips (movies or sequences),
  but you need to set it manually.
   * Open the Files Panel, load the two clips. Select one of them.
   * Open the Stereo 3D Panel, select the other clip.  Then select Input as
     "Image".
   * Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).
  
- Fixed loading of multiple clips from a session messing up the video layers.
- Made movie's default layer be labeled "Color" to be consistant with images.
- Fixed OpenEXR's v2 multipart images with view (stereo) parameter.
- Fixed OpenEXR's v2 multipart images with changing data windows between frames.
- Fixed mrv2's native file chooser on Windows not cd'ing to the file path
  when the location input field was manually edited.
- Fixed playback starting when session was loaded command line and the session
  was not originally playing.
- Fixed thumbnail display in Files, Compare, Stereo 3D and Playlist panels.
- Fixed order of panels when loaded from a session file.
- Improved performance of exiting the application.
- Made HUD Attributes display the (sometimes changing) frame attributes.
- Added a Data and Display Window display option to the menus and to the
  view window display.
- Added compare and stereo options sent when a client syncs to the server.
- Made File/Clone (Right Mouse Button on Files Panel clip) respect the frame
  and playback state of the original clip.
- Added a File/Refresh Cache (Right Mouse Button on Files Panel clip) to
  refresh the cache.  This is useful when viewing a partially rendered
  sequence.
- Made thumbnails in Files, Compare, Stereo 3D and Playlist panels show the
  actual layer (color channel).
- Made timeline thumbnail reflect the actual layer (color channel).
- Allowed saving annotations to a PDF.  Both picture thumbnails as well as
  text notes are saved.
- Made Media Info Panel refresh every frame when there's a Data Window present.
- Fixed safe areas partially disappearing when zooming out.


v0.5.4
------
- Made Playlist thumbnail reflect the current or in times.
- Changed extension of Session files to be .mrv2s to distinguish them from
  .m2s video/audio files.
- Fixed copying of colors from the Color Area Panel.
- Fixed refreshing of timeline when Close All was executed.
- Fixed sending and receiving notes through the network.
- Fixed saving of annotations in session files that were on the timeline.
- Fixed loading of annotations from a session file.
- Made clicking twice on area selection open/close the color area panel.
- Fixed annotations' ghosting which was not fading in/out correctly.
- Allowed loading a session file from the command-line.  Just do:

    $ mrv2 test.mrv2s

- Added accidentally missing licenses of Python and pybind11 to docs/Legal.
- Added a File/Clone right mouse button menu option to Files Panel.  This is
  useful when creating a playlist of the same element but different in/out
  points.
- Added support for OCIO settings in session file.
- Added support for Color Channel (Layers) settings in session file.
- Added session files to the list of recent files.
- Fixed channel (layer) shown in the color channel pulldown when switching
  files.
- Fixed macOS start-up script not passing the command-line arguments.
- Made session file store and restore the current time.


v0.5.3
------
- Made area selection allow it to select 1 pixel easier by a single click.
  To disable it, you just need to switch to a new action mode (drawing, etc).
- Some users on older macOS versions reported problems with the Privacy
  mechanism of the OS on Documents, Desktop and Download directories.
  The problem is not there if we use the native file chooser.  I've switched
  the default on macOS to use the native file chooser.
- Added a soft brush for annotations on all shapes.  You access it from the
  Annotation panel which can be opened from the menus or by clicking twice on
  any of the draw tools.  The algorithm for smooth brushes is not yet perfect,
  as it can lead to an overlapping triangle on self intersections.
- Allowed splatting a brush stroke if clicking only once.
- Made Pen size in annotations go as low as 2 pixels.  One pixel tends to
  vanish and have issues when panels are open.
- Added license and code attribution to the Polyline2D.h code which was missing
  and I had lost where I downloaded it from.  I have further modified it to
  support UV mapping and indexed triangles.
- Fixed flickering of timeline thumbnail if switched to on first and then
  later set it to off in the preferences.
- Added a session file to store a mrv2 session (.m2s files)
  All files loaded, ui elements, panel values, etc. are saved and restored.
- Fixed a potential crash when using One Panel Only.
- Added Notes to Annotation Panel.  This allows you to add comments on a frame,
  without having to draw anything (or in addition to the drawn elements).
- Made view take the focus upon entering except when typing in the text tool.
- Fixed search in the Hotkey window which was missing the last character of
  the function.
- Fixed search repeatedly in the Hotkey window which was searching from the
  topline instead of from the last selected item.
- Allowed annotation drawing outside of the canvas once again.
- Fixed precision issues on annotation drawings.
- Made annotations respond to R, G, B, A channels changing.
- Removed ngrok documentation as it was incorrect for internet access.
- Fixed resizing of viewport not taking into account the status bar, leading
  to zoom factors of 1/1.04 instead of 1.

v0.5.2
------
- TCP Control Network port number is now saved in the preferences.
- Volume control is now saved in the preferences.
- Mute control is now saved in the preferences.
- Moved TCP volume and mute control to App.
- Fixed a bug in selection of items in Files Panel when two or more images
  had the same path.
- Fixed a bug in selection of items in Compare Panel which would show unselected
  files as selected.
- Added volume/setVolume to python cmds module.
- Added isMuted/setMute to python cmds module.
- Fixed resizing of log window when an error appears not remembering the user
  size settings.
- Fixed a horrible math bug in the calculation of zooming with Rig ht
  Mouse Button + ALT key.
- Made paths sent through network connections be garbled with a simple cypher
  scheme.
- Fixed bundle identifier on macOS having the same ID as the old mrViewer.
- Added -server, -client and -port command-line flags to start a network
  connection.
- Added documentation on how to establish a server-client connection on the
  internet using the free ngrok service.  This allows a single mrv2 server and
  a single mrv2 client to connect for free albeit for non-commercial projects.
  For multiple clients or commercial ventures, you need to pay for one of
  ngrok's plans or use another server of your choosing that will allow you to
  open a network port or remote ssh connection.
- Added parsing of hostname to extract tcp:// and :port from it.
- Upped the network protocol version used.  Now it is 2.  You can no longer
  use v0.5.1 with v0.5.2 or else the paths will get garbled.
- Fixed drawing and erasing of shapes getting drawn in different order.
- Annotations now can only be drawn inside the image instead of everywhere in
  the viewport.
- Fixed annotations ghosting not being drawn transparent in some areas and more
  solid in others.
- Made volume slider knob more attractive.
- Hotkey editor now has a close button on Windows.
- Fixed toggling of magnify texture filtering.
- Added hotkey entry for toggling minify texture filtering.
- Added magnify texture filtering to the list of hotkeys as it was missing.
- Added opacity (alpha) to drawing tools.
- Fixed a major memory leak when switching images which would show up mostly
  on Linux.


v0.5.1
------
- Made Path Mappings get saved to a different file (mrv2.paths.prefs) instead
  of the main preferences file.
- Fixed a Windows input of accented (foreign) characters in Text tool.
- Fixed on Windows opening files with spaces on them when the language was
  not the same as the language of the OS.
- Improved the Save Options file requester with FFmpeg and OpenEXR options
  (not yet functional in tlRender).
- Made double clicking on any of the annotation tools in the action dock
  panel toggle the Annotation Panel.
- Added Send and Accept Media to send and receive media files opening,
  closing and syncing.
- Improved drawing overlaps of multiple annotations.  Only when the erase tool
  is used does the drawing get reversed.
- Made cursor re-appear if drawing and using the right mouse button menu.
- Fixed saving of annotations in EXR images when they were big. 

v0.5.0
------
- Added networking to mrv2.  You can have a server and one or more clients and
  they will all colaborate with UI, pan and zoom, color transformations,
  playback, audio and annotations.  They can all be set to send or accept any
  item individually, from either the Preferences or the Sync menu.
  The server should contain the media to be reviewed.  Upon a connection by any
  client, the client will attempt to synchronize with the server.
  The sever and client are on a LAN and if both the client and server use the
  same paths to the media, the client will get all of its media loaded
  automatically.
  If they don't have the same paths, each file will be to the list of path
  mappings set in the Preferences.
  Finally, if that fails, the files will be compared on its base name
  and if matched, it will get accepted as the same clip, with a warning.
  If none of this is true, an error will appear, but the connection will
  continue.  However, syncing among multiple clips may show the wrong clip.
- Added Path Mapping to deal with paths being different on each platform, client
  or server.
- Fixed dragging of the timeline outside of the in-out range.  Now it will
  clamp the slider.
- Fixed a subtle bug in translations of Preferences' tree view which could lead
  to the wizard panel not show.
- Fixed a potential crash on log panel opening (when it was already opened).
- Fixed a bug on Windows and macOS that would size the panels beyond the bottom
  of the window.
- Added Environment Map options to python API.
- Fixed Luminance label spilling into the black areas of the pixel bar.
- Fixed Luminance tooltip flickering on macOS.
- Fixed Media Info Panel not showing up when the dockgroup was created for the
  first time.
- Fixed cursor disappearing on the action tool bar when a draw mode was
  selected.  Now it only disappears when it is in one of the views.
- Added saving of annotations when saving movie files or sequence of images.
  


v0.4.0
------
- Added Search on Hotkeys for functions and hotkeys.
- Updated all Python on every OS to 3.10.9, which is the sanctioned Python
  for VFX Platform 2023.
- Fixed resizing of dock and close button on macOS.
- Exposed all Python symbols on Linux when linked statically in mrv2 executable.
  This prevented on Linux from loading some external symbols on some libraries.
- Fixed PYTHONPATH on Linux and macOS to point to the mrv2 directory, whcih
  was preventing loading some modules.
- Improved Docker building by not cloning the git repository in the Dockerfile.
  The cloning now happens in the etc/entrypoint.sh script.
- Fixed mrv2.sh permissions on .tar.gz files.
- Added Reverse playback with audio!!!!
- Made input widgets in the timeline (current frame, fps, start frame and end
  frame), return the focus to the main window once you press return.
- Updated cmd.update() to return the number of seconds (usually milliseconds)
  the UI took to update.
- Updated the timelineDemo.py to play the clip for 5 seconds instead of a
  random number.
- Made Text input tool (widget) not loose focus when it is dragged somewhere
  else.
- Fixed loop mode at start not showing the appropiate loop mode.
- Made default loop mode be Loop.
- Updated to newer tlRender (new OpenColorIO 2.1, FFmpeg 6.0, etc).
- Due to changes in OpenColorIO, support for Windows 8.1 is no longer
  provided.
- Fixed Panel/Logs not showing as a toggle menu entry.
- We are also dropping support for 32-bit Windows machines, as it waa
  causing a lot of confusion with users downloading the wrong version
  from sourceforge.net when the amd64 (64-bits) version was not tagged as
  default or that it was called amd (and not Intel :)
- Fixed a random crash when invoking panels from hotkeys.
- Fixed hotkeys in menu bar not working when the menubar was hidden.
- Added all python libs to Linux distribution.
- Fixed a redrawing issue when the Media Information Panel was put as a window.
- Fixed zombie process on exit on Windows.
- Fixed Hotkeys window that had gotten broken in v0.4.0.
- Improved the performance of dragging panels as Windows (mainly on Linux).
- Added remembering of which tabs where open/closed in Media Information Panel.
- Fixed Spanish translations on Color Panel.
  

v0.3.8
------
- Changed language handling in preferences.  Now the locale code is stored.
- Removed all languages except for English and Spanish.  Note that on Windows,
  if you had Spanish selected, it will revert to English.  You will need to
  change it once again.
- Added reporting of memory use to HUD.
- Added Cache in Gigabytes to Settings Panel.  When this is non-zero the
  Read Ahead and the Read Behind are calculated automatically based on
  the Gigabytes number set here.  It divides it by image size, pixel type,
  fps and number of active movies.  It also takes into account audio, but
  poorly.
- Fixed a resizing issue on Python Panel, not resizing the tile group.
- Documented Python API in both English and Spanish, with Search browser.
- Fixed sorting of recent files so that they don't change order.
- Fixed reccent files to not list files that cannot be found on disk.
- Made recent files list the files in order of how they were loaded, with last
  loaded first.
- Fixed original pixel lookups on clips that have a pixel aspect ratio != 1.0.
- Fixed original pixel lookups on YUV420P_U16, YUV444P_U16 format.
  Missing testing YUV422P_U16, but it should work.
- Made audio volume and audio mute / track selection not active if the clip
  has no audio.
- Added number of Cache Ahead and Behind Video and Audio frames to HUD.
  If Ahead Video cache becomes 0 when playing forwards, playback will stop.
- Fixed Text tool input on Wayland.
- Removed libharfbuzz from the Linux distribution as it was causing trouble
  with some newer Linux distros.
- Added a Render->Black Background option to quickly switch from a gray
  background to a black background on images or movies that have an alpha
  channel.

v0.3.7
------
- Added a half OCIO default to handle OpenEXR half images.
- Added timeRange and inOutRange to timeline module.
- Added setIn() and setOut() to timeline module to set the in and out
  time/frame/seconds.
- Fixed timeRange conversion to string (__str__) and repr (__repr__).
- Fixed Presentation toggle from the menus and from the right mouse button menu.
- Added media.firstVersion(), media.previousVersion(), media.nextVersion(),
  and media.lastVersion() to move from one version of the clip to the next.
- Allowed saving of sequences if you use something like bunny.0001.exr.
- Allowed saving of .otio files with relative paths.
- Flushed the cout buffer.
- Added creating playlists from python.
- Fixed audio slider which would jump from 0 to 1 abruptly.
- Fixed resizing of panel windows when they were created first as windows,
  not from undocked.
- Panel windows now remember their undocked state even after being docked once.
- Added help text to viewport.  Now it will report when you click once on the
  viewport to Play or Stop the playback.
- Switching languages on Windows now works properly, both from the command-line
  and from the GUI.
- Fixed a crash on exiting the application.
- Made upgrading mrv2 more painless, as it will now update the OCIO config
  automatically to the new version, unless the path does not contain mrv2.
- Removed the outdated nuke-default OCIO config, replacing it with OCIO2's
  cgstudio config.
- Added Cut/Copy/Paste to Python editor (it was possible before, but just
  from the keyboard shortcuts).
- Renamed Python Editor's Python menu to File.
- Added a hint when playback is started or stopped by single clicking on
  the viewport.


v0.3.6
------
- Fixed Python Editor crashes (memory trashing).
- Made Python Editor remember its text when closed and reopened.
- Fixed Python Editor's coloring sometimes getting mixed up.
- Improved Python Editor's tabulation when a colon ends the line
  (to handle for, def, class, etc).
- Added a contactSheet.py demo for showing all the layers of an OpenEXR
  in Compare tile mode.
- Made cmd.compare() use the item index instead of item itself to avoid
  confusion when the same file was loaded more than once.
- Made CompareMode be part of the media module instead of the timeline module.
- Made mrv2 exit cleanly once the process calls _wexecv on windows.
- Fixed Compare Panel selection when paths were the same on two clips.
- Fixed a zombie process being left on Windows exit.

v0.3.5
------

- Bug fixed default OCIO input color spaces not being applied.
- Bug fixed an issue with scrubbing forwards not scrubbing smoothly.
- Bug fixed OCIO ICS when the color space had slashes (/) in it.
- Fixed printing of command-line arguments when run from cmd.exe or similar
  on Windows.
- Added a --version switch to command-line arguments to report version number.
- Made Drag and Drop in Linux work with other file requesters other than
  Nautilus (nemo, thunar, etc).
- Fixed sliders not appearing in Compare Panel.
- Fixed some missing libraries from Linux distribution.
- Fixed language switching on Windows when paths had spaces in them.
- Added Python bindings and a Python Panel with an editor and output window
  to run code interactively.
  There's not any documentation yet for it, but there are some sample scripts
  in the python/demos directory.
  Currently, you can:
     * Open images, videos and otio timelines.
     * Control the timeline.
     * Change colors and LUT config.
     * Compare two images and change the compare settings.
     * Change the layer of the image.
     * Change the R, G, B, A channels of the image.
     * Change the foreground (A) and compare (B) images either by index
       or by file media item.
     * Use libraries from the python standard library, except threads.

     The modules are:
     	 import mrv2
	 from mrv2 import cmd, math, imaging, media, timeline

v0.3.4
------

- Bug fixed a crash that would happen when the OCIO config was not found.
  This would happen mostly on Linux, when switching versions.
- Added popping the log panel when an error occurs if the preference is
  set that way.
- Fixed audio problems on Linux.
- Fixed a crash that would happen when the movie entered command-line was
  not found.
- Fixed a thumbnail exiting when the file was not being found.
- Added logging to all messages from the start of mrv2 on.  They can now
  be viewed in the Logs panel/window.
- Fixed log window popping up when errors are presented.
- Improved Pulse Audio complaining about devices in use on Linux.
- Fixed Spanish translation of main UI's tooltips and Preferences Window.
- Fixed threading hang up race condition which would mostly be seen on
  Linux.

v0.3.3
------

- Added a spin option to Environment Maps to instead of panning around with
  middle mouse, it allows you to push and spin in one direction.
- Added all licenses to docs/Legal.
- Fixed file attachments on Linux.
- Fixed unistaller on Linux to remove icon and desktop file from
  /usr/share/*.
- Fixed installer on Windows to not popup the file association panel if not
  requested to do so (it asks now, instead of listing as one the things to
  insall).  This is better as it allows us to translate into other natural
  languages that part of the installer.
- Added Natural Language translations (.mo files).  Currently only Spanish is
  provided.
- Added comprehensive documentation on how to translate mrv2 to other natural
  languages.
- Fixed a bug in thumbnails changing the group that it was attached.  This
  would effect the FilesPanel, ComparePanel, and PlaylistPanel.
- Fixed several crashes in the Prefereneces window.
- Fixed a race condition in the mrv2 File requester when creating thumbnails.
  This was most noticeable on Windows, where the thumbnails would get corrupted.
- Fixed a thread crashing on Linux when creating thumbnails.
- Fixed the logic in the OCIO file preferences which would prevent from
  selecting a new .ocio oonfig file.
- Made File/Open and Open button in the Files Panel open the movie and then
  play it if the Preferences' autoplay button is on.
- Fixed favorites directory in custom file requester not getting saved on Linux.
- Fixed xcb_ and _XRead multithread errors on custom file requester on Linux.
- Added stacktrace and signal handler routines on Linux and Windows.
- Fixed log window/dock to pop up when an error occurs.
- Mostly fixed audio problems on Linux when switching clips.  There can still
  be issues, but it is a matter of switching the clip again to make it work.
- Fixed text tool not working in v0.3.2.

v0.3.2
------

- Updated the build to rely on media-autobuild_suite exclusively on Windows.
- Fixed menu bar hiding not showing in the view menu properly
  (it was always on).
- Fixed Spherical environment mapping (not using a shader anymore).
- Added Cubic environment maps with the OpenEXR distribution.
- Fixed repositioning of text input field when clicking inside the text input.
- Fixed locating libintl.h on Windows.
- Added this HISTORY.md file to docs/ directory in distribution.
- Improved build instructions.
- Fixed mouse rotation of environment maps.
- Fixed middle mouse button click starting playback, like left mouse button.


v0.3.1
------

*******************************************************************************
- Linux Binary releases that work on Rocky Linux 8, RedHat 8 and Ubuntu 20.04.
*******************************************************************************

*******************************************************************************
- Added a Dockerfile for easy building and disting on all Linux distros.
  The base distro it builds on is Rocky Linux 8.
*******************************************************************************

*******************************************************************************
- Updated manual building documentation for Rocky Linux, Ubuntu, macOS and
  Windows separately to make it clearer.
*******************************************************************************

- The main executable is mrv2.exe (Windows) or mrv2.sh (Linux / macOS ).
- Fixed the build system to use mrv2 everywhere instead of mrv2 or mrViewer2.
- Added getting all .so dependencies in CMake to distribute the executable
  appropiately.
- Changed hard-coded file extensions to use Darby's IO plugin system.
- Fixed crash on Linux GNOME when using native file requester.
- Added tooltips to Read Ahead/Read Behind caches to clarify they are in
  seconds.
- Added single click playback and stop on the view window, like RV.
- Added Doxygen documentation (very incomplete).
- Added displaying of spherical environment maps in a virtual sphere
  ( courtesy of an open source OpenRV shader from The Mill ).
- Updated building documentation for Rocky Linux, Ubuntu, macOS and Windows.
- Added pen size change thru hotkeys.
- Fixed thumbnail creation on Windows.
- Removed memory leak of thumbnail creation.

v0.3.0
------

- Improved UI: menus, status bar, functionality.
- Moved status bar and status tool to bottom of the screen.
- Added preferences and menu toggle for status bar.
- Added a Panel menu to hold all dockable panels/windows.
- Added a One Panel Only toggle to show one panel at a time instead of packing
  all panels one after the other.  Floating windows are not effecte by this
  setting.
- Fixed video layer (channels) displayed when switching from one clip version
  to the next.
- Added a gamma switch to switch between 1 and the previous value.
- Added CONTRIBUTORS.md list.
- Automated version bumps in C++ code by looking at cmake/version.cmake.
- Fixed a refresh bug in FPS display when selecting Default FPS.
- I finally fixed a horrible FLTK crashing bug on thumbnail on timeline slider.
- Fixed a  crash when setting loop mode with no media loaded.
- Fixed playback of clips where fps did not match tbr.
- Fixed autoplayback when setting is set in the preferences.


v0.2.0
------

- Added support for multipart OpenEXR files.
- Fixed crashes on Windows due to time slider thumbnail.
- Made time slider thumbnail appear.
- Improved redrawing of thumbnails.
- Fixed crash on too long attributes when displayed in the HUD.
- Moved all tools into their own library (mrvTools).
- Fixed cursor drawing and slow performance of drawing tools.
- Fixed default gamma keyboard shortcuts not working.
- Added a rather rudimentary OTIO Playlist.  You select clips in the file
  window, change their in/out points and add them to the Playlist.
  When the playlist is done, you click OT Playlist and the clips are
  assembled in an otio file that is saved in $TEMP.
  Currently, you cannot nest OTIO files within another OTIO file.
- Added menu entry for Presentation mode.
- Added menu entries for deleting an annotation and all annotations
  from the movie.
- Made annotation menus appear as soon as a drawing is made.
